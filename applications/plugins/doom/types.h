#ifndef _types_h
#define _types_h

#include <stdint.h>
#include <math.h>
//#include "constants.h"

#define UID_null 0

// Entity types (legend applies to level.h)
#define E_FLOOR 0x0 // . (also null)
#define E_WALL 0xF // #
#define E_PLAYER 0x1 // P
#define E_ENEMY 0x2 // E
#define E_DOOR 0x4 // D
#define E_LOCKEDDOOR 0x5 // L
#define E_EXIT 0x7 // X
// collectable entities >= 0x8
#define E_MEDIKIT 0x8 // M
#define E_KEY 0x9 // K
#define E_FIREBALL 0xA // not in map

typedef uint16_t UID;
typedef uint8_t EType;

typedef struct Coords {
    double x;
    double y;
} Coords;

UID create_uid(EType type, uint8_t x, uint8_t y);
EType uid_get_type(UID uid);
Coords create_coords(double x, double y);
uint8_t coords_distance(Coords* a, Coords* b);

#endif
