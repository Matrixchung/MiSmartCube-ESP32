#include <Arduino.h>
#include <CubeModel.hpp>
string colorToString(COLOR color)
{
    switch(color)
    {
        case COLOR::WHITE:
            return "W";
        case COLOR::YELLOW:
            return "Y";
        case COLOR::GREEN:
            return "G";
        case COLOR::BLUE:
            return "B";
        case COLOR::RED:
            return "R";
        case COLOR::ORANGE:
            return "O";
        default:
            return "X";
    }
}
void printCube(CubeModel &cube)
{
    array<array<COLOR, 3>, 3> tempFaceColor;
    string res = "";
    // First print UP face
    tempFaceColor = cube.getFaceColors(FACE::UP);
    for(uint8_t i = 0; i < 3; i++)
    {
        Serial.print("      ");
        for(uint8_t j = 0; j < 3; j++)
        {
            res = colorToString(tempFaceColor[i][j]);
            Serial.print(res.c_str());
            Serial.print(" ");
        }
        Serial.println();
    }
    array<array<array<COLOR, 3>, 3>, 4> tempFaceColors;
    tempFaceColors[0] = cube.getFaceColors(FACE::LEFT);
    tempFaceColors[1] = cube.getFaceColors(FACE::FRONT);
    tempFaceColors[2] = cube.getFaceColors(FACE::RIGHT);
    tempFaceColors[3] = cube.getFaceColors(FACE::BACK);
    for(uint8_t i = 0; i < 3; i++)
    {
        for(uint8_t p = 0; p < 4; p++)
        {
            for(uint8_t j = 0; j < 3; j++)
            {
                res = colorToString(tempFaceColors[p][i][j]);
                Serial.print(res.c_str());
                Serial.print(" ");
            }
        }
        Serial.println();
    }
    tempFaceColor = cube.getFaceColors(FACE::DOWN);
    for(uint8_t i = 0; i < 3; i++)
    {
        Serial.print("      ");
        for(uint8_t j = 0; j < 3; j++)
        {
            res = colorToString(tempFaceColor[i][j]);
            Serial.print(res.c_str());
            Serial.print(" ");
        }
        Serial.println();
    }
}