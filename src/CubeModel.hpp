/**
 * @author Matrixchung
 * @brief  A CubeModel class represents a 3x3x3 Rubik's Cube.
 * 
 * It stores each cubie(made of 3x3x3 small cubes) as an index and orientation,
 * with edges and corners in separate arrays.
 * 
 * Six colors: (G)reen, (R)ed, (B)lue, (O)range, (W)hite, (Y)ellow.
 * 
 * As the original Mi Smart Cube's data protocol (36 half bytes), 
 * the cube is oriented with green on top and white on front, with red left and orange right, blue on bottom.
 * 
 * data[0] - data[15]: CORNERS
 * data[0] - data[3] means top face's corner, start from ULB, counter-clockwise. (ULB-ULF-URF-URB)
 * data[4] - data[7] means bottom face's corner, start from DLB, counter-clockwise. (DLB-DLF-DRF-DRB)
 * data[8] - data[15] (range 1-3) means each corner's orientation. A corner piece can be in three different orientations, oriented, rotated once, or rotated twice (o∈{3, 2, 1}). 
 * 
 * ** CORNER COLOR ORIENTATION NOTES **
 * See from the visible Z-AXIS and Y-AXIS and X-AXIS side (the same as 'ULB' - Up(Z), Left(Y), Back(X)): (A33 = 6)
 *        |      3     |      2     |      1     |
 * #1 GRY |  GRY, GYR  |  RGY, YGR  |  RYG, YRG  |
 * #2 GRW |  GRW, GWR  |  RGW, WGR  |  RWG, WRG  |
 * #3 GOW |  GOW, GWO  |  OGW, WGO  |  OWG, WOG  |
 * #4 GOY |  GYO, GOY  |  YGO, OGY  |  YOG, OYG  |
 * #5 BRY |  BRY, BYR  |  RBY, YBR  |  RYB, YRB  |
 * #6 BRW |  BRW, BWR  |  RBW, WBR  |  RWB, WRB  |
 * #7 BOW |  BOW, BWO  |  OBW, WBO  |  OWB, WOB  |
 * #8 BOY |  BOY, BYO  |  OBY, YBO  |  OYB, YOB  |
 * 
 * data[16] - data[27]: EDGES
 * Order(if have): Z-AXIS, X-AXIS, Y-AXIS
 * data[16] - data[19] means top face's edge, start from UB, counter-clockwise. (UB-UL-UF-UR) (1-2-3-4)
 * data[20] - data[23] means front and back faces' edge, start from BL, counter-clockwise. (BL-FL-FR-BR) (5-6-7-8)
 * data[24] - data[27] means bottom face's edge, start from DB, counter-clockwise. (DB-DL-DF-DR) (9-A-B-C)
 * 
 * data[28] - data[30]: 
 * BACK Face(YELLOW) roll +- 90 degree: 8 9 8
 * FRONT Face(WHITE) roll +- 90 degree: 2 6 2
 * Both: A F A (10 15 10)
 * A edge piece can be in two states, oriented or flipped.
 * 
 * data[31] = 0
 * 
 * Face index: 1 - Blue, 2 - Yellow, 3 - Orange, 4 - White, 5 - Red, 6 - Green
 * Direction index: 3 - Clockwise, 1 - Counter-clockwise
 * data[32] means the face index shifted this time.
 * data[33] means the direction index of the face shifted this time.
 * 
 * data[34] means the face index shifted last time.
 * data[35] means the direction index of the face shifted last time.
 * 
 * Original corner color with indexes are listed below:
 *  1   2   3   4   5   6   7   8
 * ULB ULF URF URB DLB DLF DRF DRB
 * GRY GRW GOW GOY BRY BRW BOW BOY
 * 
 * Original edge color with indexes are listed below:
 * 1  2  3  4  5  6  7  8  9  A  B  C
 * UB UL UF UR BL FL FR BR DB DL DF DR
 * GY GR GW GO RY RW OW OY BY BR BW BO
 * 
 * Solved cube cubeData example:
 * 1 2 3 4 5 6 7 8
 * 3 3 3 3 3 3 3 3
 * 1 2 3 4 5 6 7 8 9 A B C
 * 0 0 0 0
 * 5 3 5 1
 * 
 * **/
#ifndef _CUBE_MODEL_HPP
#define _CUBE_MODEL_HPP

#include <cstdint>
#include <array>
using std::array;
#include <algorithm>
using std::swap;
#include <string>
using std::string;

enum class FACE   : uint8_t {UP, LEFT, FRONT, RIGHT, BACK, DOWN, NONE};
enum class COLOR  : uint8_t {BLUE = 0, YELLOW = 1, ORANGE = 2, WHITE = 3, RED = 4, GREEN = 5};
// enum 'BR' conflicts with /esp32/include/xtensa/config/specreg.h
enum class EDGE   : uint8_t {UB, UL, UF, UR, BL, FL, FR, _BR, DB, DL, DF, DR};
enum class CORNER : uint8_t {ULB, ULF, URF, URB, DLB, DLF, DRF, DRB};
enum class DIR    : uint8_t {ORIENTED = 3, FLIPPED = 4, ROTATED = 2, ROTATED_TWICE = 1};

class CubeModel
{
    public:
        struct Cubie
        {
            uint8_t index;
            DIR orientation;
        };
    private:
        // total 26 cubies with 12 two-stickers' edges and 8 three-stickers' corners.
        array<Cubie, 12> edges;
        array<Cubie, 8>  corners;
        array<COLOR, 6>  centers;
        bool _checkValid();
    public:
        CubeModel();
        // CubeModel(const CubeModel& cube);
        bool operator==(const CubeModel &other) const;
        bool operator!=(const CubeModel &other) const;
        bool isSolved() const;
        array<COLOR, 2> getEdgeColors(EDGE edge) const;
        array<COLOR, 3> getCornerColors(CORNER corner) const;
        array<array<COLOR, 3>, 3> getFaceColors(FACE face) const;
        COLOR getColor(FACE face, uint8_t row, uint8_t col) const;

        // ** SPECIAL FOR XIAOMI CUBE **
        CubeModel(const uint8_t *data); // 36 bytes cubeData
        FACE lastTurnedFace;
        uint8_t lastTurnedDir; // 0 - Clockwise, 1 - Counter-clockwise
        FACE turnedFace;
        uint8_t turnedDir;
};

CubeModel::CubeModel()
{
    for(uint8_t i = 0; i < 12; i++)
    {
        this->edges[i].index = i;
        this->edges[i].orientation = DIR::ORIENTED;
    }
    for(uint8_t i = 0; i < 8; i++)
    {
        this->corners[i].index = i;
        this->corners[i].orientation = DIR::ORIENTED;
    }
    this->centers[uint8_t(FACE::UP)] = COLOR::GREEN;
    this->centers[uint8_t(FACE::LEFT)] = COLOR::RED;
    this->centers[uint8_t(FACE::FRONT)] = COLOR::WHITE;
    this->centers[uint8_t(FACE::RIGHT)] = COLOR::ORANGE;
    this->centers[uint8_t(FACE::BACK)] = COLOR::YELLOW;
    this->centers[uint8_t(FACE::DOWN)] = COLOR::BLUE;
    // ** SPECIAL FOR XIAOMI CUBE **
    this->lastTurnedFace = FACE::NONE;
    this->lastTurnedDir = 0;
    this->turnedFace = FACE::NONE;
    this->turnedDir = 0;
}

// ** SPECIAL FOR XIAOMI CUBE **
// This constructor is special for Xiaomi Smart Cube.
CubeModel::CubeModel(const uint8_t *data)
{
    // Align corners (data[0:15])
    for(uint8_t i = 0; i < 8; i++)
    {
        this->corners[i].index = data[i] - 1;
        this->corners[i].orientation = (DIR)data[i+8];
    }
    // now the pointer = 15
    // Align edges (data[16:27])
    for(uint8_t i = 0; i < 12; i++)
    {
        this->edges[i].index = data[i+16] - 1;
        this->edges[i].orientation = DIR::ORIENTED;
    }
    // now the pointer = 27
    if((data[28] == 8 && data[29] == 9 && data[30] == 8) || (data[28] == 0x0A && data[29] == 0x0F && data[30] == 0x0A))
    {
        this->edges[(uint8_t)EDGE::UB].orientation = DIR::FLIPPED;
        this->edges[(uint8_t)EDGE::BL].orientation = DIR::FLIPPED;
        this->edges[(uint8_t)EDGE::_BR].orientation = DIR::FLIPPED;
        this->edges[(uint8_t)EDGE::DB].orientation = DIR::FLIPPED;
    }
    if((data[28] == 2 && data[29] == 6 && data[30] == 2) || (data[28] == 0x0A && data[29] == 0x0F && data[30] == 0x0A))
    {
        this->edges[(uint8_t)EDGE::UF].orientation = DIR::FLIPPED;
        this->edges[(uint8_t)EDGE::FL].orientation = DIR::FLIPPED;
        this->edges[(uint8_t)EDGE::FR].orientation = DIR::FLIPPED;
        this->edges[(uint8_t)EDGE::DF].orientation = DIR::FLIPPED;
    }
    // data[31] = 0
    this->turnedFace = (FACE)data[32];
    this->turnedDir = data[33] == 1 ? 0 : 1;
    this->lastTurnedFace = (FACE)data[34];
    this->lastTurnedDir = data[35] == 1 ? 0 : 1;
    this->centers[uint8_t(FACE::UP)] = COLOR::GREEN;
    this->centers[uint8_t(FACE::LEFT)] = COLOR::RED;
    this->centers[uint8_t(FACE::FRONT)] = COLOR::WHITE;
    this->centers[uint8_t(FACE::RIGHT)] = COLOR::ORANGE;
    this->centers[uint8_t(FACE::BACK)] = COLOR::YELLOW;
    this->centers[uint8_t(FACE::DOWN)] = COLOR::BLUE;
}

array<COLOR, 2> CubeModel::getEdgeColors(EDGE edge) const
{
    array<COLOR, 2> result;
    Cubie edge_cubie = this->edges.at((uint8_t)edge);
    switch((EDGE)edge_cubie.index)
    {
        case EDGE::UB:
            result[0] = COLOR::GREEN;
            result[1] = COLOR::YELLOW;
            break;
        case EDGE::UL:
            result[0] = COLOR::GREEN;
            result[1] = COLOR::RED;
            break;
        case EDGE::UF:
            result[0] = COLOR::GREEN;
            result[1] = COLOR::WHITE;
            break;
        case EDGE::UR:
            result[0] = COLOR::GREEN;
            result[1] = COLOR::ORANGE;
            break;
        case EDGE::BL:
            result[0] = COLOR::YELLOW;
            result[1] = COLOR::RED;
            break;
        case EDGE::FL:
            result[0] = COLOR::WHITE;
            result[1] = COLOR::RED;
            break;
        case EDGE::FR:
            result[0] = COLOR::WHITE;
            result[1] = COLOR::ORANGE;
            break;
        case EDGE::_BR:
            result[0] = COLOR::YELLOW;
            result[1] = COLOR::ORANGE;
            break;
        case EDGE::DB:
            result[0] = COLOR::BLUE;
            result[1] = COLOR::YELLOW;
            break;
        case EDGE::DL:
            result[0] = COLOR::BLUE;
            result[1] = COLOR::RED;
            break;
        case EDGE::DF:
            result[0] = COLOR::BLUE;
            result[1] = COLOR::WHITE;
            break;
        case EDGE::DR:
            result[0] = COLOR::BLUE;
            result[1] = COLOR::ORANGE;
            break;
    }
    if(edge_cubie.orientation == DIR::FLIPPED) swap(result[0], result[1]);
    return result;
}
/**
 * @return array<COLOR, 3> in the order of Z Y X
*/
array<COLOR, 3> CubeModel::getCornerColors(CORNER corner) const
{
    array<COLOR, 3> result;
    // Get the specified location's corner cubie.
    // ...and check the cubie's original position(stored in corner_cubie.index) to determine the color.
    // ...if the ULB cubie(index=0) is in the ULF position(corner=1) which is equal to (index+corner)%2==1, then swap the i1 and i2 according to CORNER COLOR ORIENTATION NOTES.
    // ...however, if the URB(index=3) is placed in the ULF position(corner=1), we should not swap i1 and i2.
    Cubie corner_cubie = this->corners.at((uint8_t)corner);
    uint8_t i0, i1, i2;
    if(corner_cubie.orientation == DIR::ORIENTED) // 3
    {
        i0 = 0;
        i1 = 1;
        i2 = 2;
        if((corner_cubie.index + (uint8_t)corner) % 2 == 1) 
        {
            swap(i1, i2);
        }
    }
    else if(corner_cubie.orientation == DIR::ROTATED) // 2
    {
        i0 = 2;
        i1 = 0;
        i2 = 1;
        if((corner_cubie.index + (uint8_t)corner) % 2 == 1) swap(i0, i2);
    }
    // TODO: Problem may kick in if the orientation is not corner's, like FLIPPED. But it should not happen.
    else // 1
    {
        i0 = 1;
        i1 = 2;
        i2 = 0;
        if((corner_cubie.index + (uint8_t)corner) % 2 == 1) 
        {
            swap(i0, i1);
        }
        else if(!this->isSolved()) swap(i0, i1);
    }
    switch((CORNER)corner_cubie.index)
    {
        case CORNER::ULB:
            result[i0] = COLOR::GREEN;
            result[i1] = COLOR::RED;
            result[i2] = COLOR::YELLOW;
            break;
        case CORNER::ULF:
            result[i0] = COLOR::GREEN;
            result[i1] = COLOR::RED;
            result[i2] = COLOR::WHITE;
            break;
        case CORNER::URF:
            result[i0] = COLOR::GREEN;
            result[i1] = COLOR::ORANGE;
            result[i2] = COLOR::WHITE;
            break;
        case CORNER::URB:
            result[i0] = COLOR::GREEN;
            result[i1] = COLOR::ORANGE;
            result[i2] = COLOR::YELLOW;
            break;
        case CORNER::DLB:
            result[i0] = COLOR::BLUE;
            result[i1] = COLOR::RED;
            result[i2] = COLOR::YELLOW;
            break;
        case CORNER::DLF:
            result[i0] = COLOR::BLUE;
            result[i1] = COLOR::RED;
            result[i2] = COLOR::WHITE;
            break;
        case CORNER::DRF:
            result[i0] = COLOR::BLUE;
            result[i1] = COLOR::ORANGE;
            result[i2] = COLOR::WHITE;
            break;
        case CORNER::DRB:
            result[i0] = COLOR::BLUE;
            result[i1] = COLOR::ORANGE;
            result[i2] = COLOR::YELLOW;
            break;
    }
    return result;
}
// row and col are 0-indexed and start from the top-left corner.
COLOR CubeModel::getColor(FACE face, uint8_t row, uint8_t col) const
{
    if(row == 1 && col == 1) return (COLOR)this->centers[(uint8_t)face];
    switch(face)
    {
        case FACE::UP:
            if(row == 0)
            {
                if(col == 0) return this->getCornerColors(CORNER::ULB)[0];
                else if (col == 1) return this->getEdgeColors(EDGE::UB)[0];
                else return this->getCornerColors(CORNER::URB)[0];
            }
            else if(row == 1)
            {
                if(col == 0) return this->getEdgeColors(EDGE::UL)[0];
                else return this->getEdgeColors(EDGE::UR)[0];
            }
            else
            {
                if(col == 0) return this->getCornerColors(CORNER::ULF)[0];
                else if (col == 1) return this->getEdgeColors(EDGE::UF)[0];
                else return this->getCornerColors(CORNER::URF)[0];
            }
            break;
        case FACE::LEFT:
            if(row == 0)
            {
                if(col == 0) return this->getCornerColors(CORNER::ULB)[1];
                else if (col == 1) return this->getEdgeColors(EDGE::UL)[1];
                else return this->getCornerColors(CORNER::ULF)[1];
            }
            else if (row == 1)
            {
                if(col == 0) return this->getEdgeColors(EDGE::BL)[1];
                else return this->getEdgeColors(EDGE::FL)[1];
            }
            else
            {
                if(col == 0) return this->getCornerColors(CORNER::DLB)[1];
                else if (col == 1) return this->getEdgeColors(EDGE::DL)[1];
                else return this->getCornerColors(CORNER::DLF)[1];
            }
            break;
        case FACE::FRONT:
            if(row == 0)
            {
                if(col == 0) return this->getCornerColors(CORNER::ULF)[2];
                else if (col == 1) return this->getEdgeColors(EDGE::UF)[1];
                else return this->getCornerColors(CORNER::URF)[2];
            }
            else if (row == 1)
            {
                if(col == 0) return this->getEdgeColors(EDGE::FL)[0];
                else return this->getEdgeColors(EDGE::FR)[0];
            }
            else
            {
                if(col == 0) return this->getCornerColors(CORNER::DLF)[2];
                else if (col == 1) return this->getEdgeColors(EDGE::DF)[1];
                else return this->getCornerColors(CORNER::DRF)[2];
            }
            break;
        case FACE::RIGHT:
            if(row == 0)
            {
                if(col == 0) return this->getCornerColors(CORNER::URF)[1];
                else if (col == 1) return this->getEdgeColors(EDGE::UR)[1];
                else return this->getCornerColors(CORNER::URF)[1];
            }
            else if (row == 1)
            {
                if(col == 0) return this->getEdgeColors(EDGE::FR)[1];
                else return this->getEdgeColors(EDGE::_BR)[1];
            }
            else
            {
                if(col == 0) return this->getCornerColors(CORNER::DRF)[1];
                else if (col == 1) return this->getEdgeColors(EDGE::DR)[1];
                else return this->getCornerColors(CORNER::DRB)[1];
            }
            break;
        case FACE::BACK:
            if(row == 0)
            {
                if(col == 0) return this->getCornerColors(CORNER::URB)[2];
                else if (col == 1) return this->getEdgeColors(EDGE::UB)[1];
                else return this->getCornerColors(CORNER::ULB)[2];
            }
            else if (row == 1)
            {
                if(col == 0) return this->getEdgeColors(EDGE::_BR)[0];
                else return this->getEdgeColors(EDGE::BL)[0];
            }
            else
            {
                if(col == 0) return this->getCornerColors(CORNER::DRB)[2];
                else if (col == 1) return this->getEdgeColors(EDGE::DB)[1];
                else return this->getCornerColors(CORNER::DLB)[2];
            }
            break;
        case FACE::DOWN:
            if(row == 0)
            {
                if(col == 0) return this->getCornerColors(CORNER::DLF)[0];
                else if (col == 1) return this->getEdgeColors(EDGE::DF)[0];
                else return this->getCornerColors(CORNER::DRF)[0];
            }
            else if(row == 1)
            {
                if(col == 0) return this->getEdgeColors(EDGE::DL)[0];
                else return this->getEdgeColors(EDGE::DR)[0];
            }
            else
            {
                if(col == 0) return this->getCornerColors(CORNER::DLB)[0];
                else if (col == 1) return this->getEdgeColors(EDGE::DB)[0];
                else return this->getCornerColors(CORNER::DRB)[0];
            }
            break;
        default:
            return COLOR::WHITE;
    }
}
array<array<COLOR, 3>, 3> CubeModel::getFaceColors(FACE face) const
{
    array<array<COLOR, 3>, 3> faceColors;
    for(uint8_t row = 0; row < 3; row++)
    {
        for(uint8_t col = 0; col < 3; col++)
        {
            faceColors[row][col] = this->getColor(face, row, col);
        }
    }
    return faceColors;
}
bool CubeModel::isSolved() const
{
    for(uint8_t i = 0; i < 12; i++)
    {
        if(this->edges[i].index != i || this->edges[i].orientation != DIR::ORIENTED) return false;
        if(i < 6 && (this->corners[i].index != i || this->corners[i].orientation != DIR::ORIENTED)) return false;
    }
    return true;
}
bool CubeModel::operator==(const CubeModel &other) const
{
    for(uint8_t i = 0; i < 12; i++)
    {
        if(this->edges[i].index != other.edges[i].index || this->edges[i].orientation != other.edges[i].orientation) return false;
        if (i < 8 && (this->corners[i].index != other.corners[i].index || this->corners[i].orientation != other.corners[i].orientation)) return false;
        if (i < 6 && this->centers[i] != other.centers[i]) return false;
    }
    return true;
    // for(uint8_t i = 0; i < this->corners.size(); i++) if(this->corners[i].index != other.corners[i].index || this->corners[i].orientation != other.corners[i].orientation) return false;
    // for(uint8_t i = 0; i < this->edges.size(); i++) if(this->edges[i].index != other.edges[i].index || this->edges[i].orientation != other.edges[i].orientation) return false;
    // for(uint8_t i = 0; i < this->centers.size(); i++) if(this->centers[i] != other.centers[i]) return false;
}
bool CubeModel::operator!=(const CubeModel &other) const
{
    return !(*this == other);
}
#endif