#include "types.h"

/*template <class T>
inline T sq(T value) {
    return value * value;
}*/

double sq(double val) {
    return val * val;
}

//extern "C"
Coords create_coords(double x, double y) {
    Coords cord;
    cord.x = x;
    cord.y = y;
    return cord;
}

//extern "C"
uint8_t coords_distance(Coords* a, Coords* b) {
    return sqrt(sq(a->x - b->x) + sq(a->y - b->y)) * 20;
}

//extern "C"
UID create_uid(uint8_t type, uint8_t x, uint8_t y) {
    return ((y << 6) | x) << 4 | type;
}

//extern "C"
uint8_t uid_get_type(UID uid) {
    return uid & 0x0F;
}
