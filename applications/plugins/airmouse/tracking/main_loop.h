#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*MouseMoveCallback)(int8_t x, int8_t y, void* context);

void calibration_begin();
bool calibration_step();
void calibration_end();

void tracking_begin();
void tracking_step(MouseMoveCallback mouse_move, void* context);
void tracking_end();

#ifdef __cplusplus
}
#endif
