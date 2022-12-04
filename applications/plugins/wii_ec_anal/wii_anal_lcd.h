#ifndef WII_ANAL_LCD_H_
#define WII_ANAL_LCD_H_

//----------------------------------------------------------------------------- ----------------------------------------
// A couple of monospaced hex fonts
//
#include "gfx/images.h"

extern const image_t* img_6x8[];
extern const image_t* img_5x7[];

//============================================================================= ========================================
// macros to draw only two sides of a box
// these are used for drawing the wires on the WAIT screen
//
#define BOX_TL(x1, y1, x2, y2)                                     \
    do {                                                           \
        canvas_draw_frame(canvas, x1, y1, x2 - x1 + 1, 2);         \
        canvas_draw_frame(canvas, x1, y1 + 2, 2, y2 - y1 + 1 - 2); \
    } while(0)

#define BOX_BL(x1, y1, x2, y2)                                 \
    do {                                                       \
        canvas_draw_frame(canvas, x1, y2 - 1, x2 - x1 + 1, 2); \
        canvas_draw_frame(canvas, x1, y1, 2, y2 - y1 + 1 - 2); \
    } while(0)

//============================================================================= ========================================
// Function prototypes
//
void patBacklight(state_t* state);

void showHex(
    Canvas* const canvas,
    uint8_t x,
    uint8_t y,
    const uint32_t val,
    const uint8_t cnt,
    const int b);

void showPeakHold(state_t* const state, Canvas* const canvas, const int hold);

void showJoy(
    Canvas* const canvas,
    const uint8_t x,
    const uint8_t y, // x,y is the CENTRE of the Joystick
    const uint8_t xMin,
    const uint8_t xMid,
    const uint8_t xMax,
    const uint8_t yMin,
    const uint8_t yMid,
    const uint8_t yMax,
    const uint8_t xPos,
    const uint8_t yPos,
    const uint8_t bits);

#endif //WII_ANAL_LCD_H_
