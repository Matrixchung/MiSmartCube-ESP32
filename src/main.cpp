/**
 * @file main.cpp
 * @author Matrixchung
 * Credits:
 *  1) https://zhuanlan.zhihu.com/p/482413033
 *  2) https://randomnerdtutorials.com/esp32-ble-server-client
 *  3) https://github.com/playfultechnology/esp32-smartcube
 *  4) https://medium.com/@benjamin.botto/implementing-an-optimal-rubiks-cube-solver-using-korf-s-algorithm-bf750b332cf9
 *  5) https://github.com/benbotto/rubiks-cube-cracker
 *  6) https://github.com/cs0x7f/cstimer
*/

// Cube UUID: 2AA03B60-DBC0-5EB8-C679-53A6BF4A8B36
// Cube MAC: C2:B5:A6:8D:1E:73
#include <Arduino.h>
#include "BLEDevice.h"
#include "CubeModel.hpp"
#include "utils.hpp"

#define SHOW_SCAN_RESULT 0 // For showing bluetooth scan results without connecting to the cube.
#define REGISTER_BATTERY_CALLBACK 0 // For seeing the battery level of cube
#define MAX_CONNECT_RETRIES 10
#define DEBUG_SERIAL_OUTPUT false

const String CUBE_MAC = "C2:B5:A6:8D:1E:73"; // Please change this to your own cube's MAC address
static BLEUUID CUBE_DATA_SERVICE_UUID("0000aadb-0000-1000-8000-00805f9b34fb");
static BLEUUID CUBE_DATA_CHAR_UUID("0000aadc-0000-1000-8000-00805f9b34fb");
static BLEUUID CUBE_RW_SERVICE_UUID("0000aaaa-0000-1000-8000-00805f9b34fb");
static BLEUUID CUBE_RW_READ_CHAR_UUID("0000aaab-0000-1000-8000-00805f9b34fb");
static BLEUUID CUBE_RW_WRITE_CHAR_UUID("0000aaac-0000-1000-8000-00805f9b34fb");

const static int AES_KEY[36] = {176,81,104,224,86,137,237,119,38,26,193,161,210,126,150,81,93,13,236,249,89,235,88,24,113,81,214,131,130,199,2,169,39,165,171,41}; // Keys to decrypt cube color data

BLEAdvertisedDevice *pDevice;
BLERemoteCharacteristic *pColorCharacter;
bool deviceFound = false;
bool deviceConnected = false;
uint8_t batteryLevel = 0;

// Return i-th half byte of pData
uint8_t getHalfByte(uint8_t* pData, int i){
  return i%2==1?(pData[(i/2)|0]%16):(0|(pData[(i/2)|0]/16));
}

class AdvertisedDevCallback : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice device){
    #if SHOW_SCAN_RESULT
      Serial.print(device.getAddress().toString().c_str());
      Serial.print(" : ");
      Serial.println(device.getName().c_str());
    #else
      if(CUBE_MAC.equalsIgnoreCase(device.getAddress().toString().c_str())){
        Serial.print("Found device with MAC address: ");
        Serial.println(CUBE_MAC.c_str());
        device.getScan()->stop();
        pDevice = new BLEAdvertisedDevice(device);
        deviceFound = true;
      }
    #endif
  }
};

class ClientCallbacks : public BLEClientCallbacks {
  void onConnect(BLEClient *pClient){
    Serial.println("Connected to cube.");
    digitalWrite(LED_BUILTIN, HIGH);
    deviceConnected = true;
  }
  void onDisconnect(BLEClient *pClient){
    Serial.println("Disconnected from cube.");
    digitalWrite(LED_BUILTIN, LOW);
    deviceConnected = false;
  }
};
#if REGISTER_BATTERY_CALLBACK
static void onRwServiceNotifyCallback(BLERemoteCharacteristic* pCharacter, uint8_t* pData, size_t length, bool isNotify){
  // Serial.print("Received data with length: ");
  // Serial.println(length);
  // for(int i = 0; i < length; i++){
  //   Serial.print(pData[i], HEX);
  //   Serial.print(" ");
  // }
  // Serial.println();
  if(length < 2){
    Serial.println("Received data with invalid length.");
    return;
  }
  if(pData[1] != batteryLevel){
    batteryLevel = pData[1];
    Serial.print("Cube Battery Level: ");
    Serial.print(batteryLevel);
    Serial.println("%");
  }
  
}
/**
 * You may encounter this error code: 
 * [ 17567][E][BLERemoteCharacteristic.cpp:287] retrieveDescriptors(): esp_ble_gattc_get_all_descr: Unknown
 * That is normal because the battery level characteristic doesn't have any descriptors.
*/
bool registerBatteryCallback(BLEClient *client){
  bool state = false;
  BLERemoteService *pService = client->getService(CUBE_RW_SERVICE_UUID);
  state = pService != nullptr;
  if(!state){
    Serial.println("Failed to find RW service.");
    return false;
  }
  BLERemoteCharacteristic *pReadCharacter = pService->getCharacteristic(CUBE_RW_READ_CHAR_UUID);
  state = pReadCharacter != nullptr;
  if(!state){
    Serial.println("Failed to find RW read characteristic.");
    return false;
  }
  pReadCharacter->registerForNotify(onRwServiceNotifyCallback);
  BLERemoteCharacteristic *pWriteCharacter = pService->getCharacteristic(CUBE_RW_WRITE_CHAR_UUID);
  state = pWriteCharacter != nullptr;
  if(!state){
    Serial.println("Failed to find RW write characteristic.");
    return false;
  }
  pWriteCharacter->writeValue(0xB5);
  return true;
}
#endif
static void onDataNotifyCallback(BLERemoteCharacteristic* pCharacter, uint8_t* pData, size_t length, bool isNotify){
  #if DEBUG_SERIAL_OUTPUT
  if(length != 20){
    Serial.print("Received data with invalid length: ");
    Serial.println(length);
    return;
  }
  #endif
  bool isEncrypted = pData[18] == 0xA7; // if pData[18] is 0xA7(167), then the color data is encrypted by AES.
  if(isEncrypted){
    uint8_t offset1 = getHalfByte(pData, 38);
    uint8_t offset2 = getHalfByte(pData, 39);
    for(int i = 0; i < 20; i++) pData[i] += (AES_KEY[offset1+i]+AES_KEY[offset2+i]); // Decrypt AES
  }
  uint8_t colorData[36] = {0};
  for(int i = 0; i < 36; i++) colorData[i] = getHalfByte(pData, i);
  CubeModel newCube = CubeModel(colorData);
  #if DEBUG_SERIAL_OUTPUT
  if(newCube.isSolved()) Serial.println("Cube is solved.");
  printCube(newCube);
  for(int i = 0; i < 36; i++){
    Serial.print(colorData[i], HEX);
    if(i == 7 || i == 15 || i == 27 || i == 31) Serial.println();
    else Serial.print(" ");
  }
  Serial.println();
  Serial.println("--------------------");
  #else
  Serial.print((uint8_t)newCube.turnedFace);
  Serial.print(' ');
  Serial.println(newCube.turnedDir);
  #endif
}
bool connectToServer(BLEAdvertisedDevice device){
  bool connected = false;
  #if DEBUG_SERIAL_OUTPUT
  Serial.print("Connecting ");
  Serial.println(device.getAddress().toString().c_str());
  #endif
  // Step #1: Create BLE Client
  BLEClient *pClient = BLEDevice::createClient();
  // Step #2: Assign BLEClientCallbacks
  pClient->setClientCallbacks(new ClientCallbacks());
  // Step #3: Connect to BLE Server
  for (int i = 1; i <= MAX_CONNECT_RETRIES; i++){
    if(connected = pClient->connect(&device)) break;
  }
  #if DEBUG_SERIAL_OUTPUT
  if(!connected){
    Serial.println("Failed to connect to cube.");
    return false;
  }
  #endif
  // Step #4: Find specified service
  BLERemoteService *pRemoteService = pClient->getService(CUBE_DATA_SERVICE_UUID);
  connected = pRemoteService != nullptr;
  #if DEBUG_SERIAL_OUTPUT
  if(!connected){
    Serial.println("Failed to find specified service.");
    return false;
  }
  #endif
  // Step #5: Find specified characteristic in the given service in step #4
  pColorCharacter = pRemoteService->getCharacteristic(CUBE_DATA_CHAR_UUID);
  connected = pColorCharacter != nullptr;
  #if DEBUG_SERIAL_OUTPUT
  if(!connected){
    Serial.println("Failed to find specified characteristic.");
    return false;
  }
  #endif
  // Step #6: Register callback function for the characteristic
  connected = pColorCharacter->canNotify();
  #if DEBUG_SERIAL_OUTPUT
  if(!connected){
    Serial.println("Failed to register callback - character cannot notify.");
    return false;
  }
  #endif
  pColorCharacter->registerForNotify(onDataNotifyCallback);
  #if DEBUG_SERIAL_OUTPUT
  Serial.println("Successfully registered data callback.");
  #endif
  // Step #7: Register callback function for battery service
  #if REGISTER_BATTERY_CALLBACK
  if(registerBatteryCallback(pClient)) Serial.println("Successfully registered battery callback.");
  else Serial.println("Failed to register battery callback.");
  #endif
  return connected;
}

void setup(){
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  BLEDevice::init("");
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDevCallback);
  pBLEScan->setActiveScan(true);
  do
  {
    Serial.println("Start scanning for device...");
    pBLEScan->start(30);
    if(!deviceFound) Serial.println("Cannot find specific device in last 30 seconds. Retrying...");
  }while(!deviceFound);
  connectToServer(*pDevice);
}

void loop(){
  if(deviceFound){

  }
}