#include "wii_anal.h"
#include "gfx/images.h" // Images

//----------------------------------------------------------------------------- ----------------------------------------
// A couple of monospaced hex fonts
//
const image_t* img_6x8[16] = {
    &img_6x8_0,
    &img_6x8_1,
    &img_6x8_2,
    &img_6x8_3,
    &img_6x8_4,
    &img_6x8_5,
    &img_6x8_6,
    &img_6x8_7,
    &img_6x8_8,
    &img_6x8_9,
    &img_6x8_A,
    &img_6x8_B,
    &img_6x8_C,
    &img_6x8_D,
    &img_6x8_E,
    &img_6x8_F,
};

const image_t* img_5x7[16] = {
    &img_5x7_0,
    &img_5x7_1,
    &img_5x7_2,
    &img_5x7_3,
    &img_5x7_4,
    &img_5x7_5,
    &img_5x7_6,
    &img_5x7_7,
    &img_5x7_8,
    &img_5x7_9,
    &img_5x7_A,
    &img_5x7_B,
    &img_5x7_C,
    &img_5x7_D,
    &img_5x7_E,
    &img_5x7_F,
};

//+============================================================================ ========================================
//	void  backlightOn (void)
//	{
//		// Acquire a handle for the system notification queue
//		// Do this ONCE ... at plugin startup
//		NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
//
//		// Pat the backlight watchdog
//		// Send the (predefined) message sequence {backlight_on, end}
//		// --> applications/notification/*.c
//		notification_message(notifications, &sequence_display_backlight_on);
//
//		// Release the handle for the system notification queue
//		// Do this ONCE ... at plugin quit
//		furi_record_close(RECORD_NOTIFICATION);
//	}
void patBacklight(state_t* state) {
    notification_message(state->notify, &sequence_display_backlight_on);
}

//============================================================================= ========================================
// Show a hex number in an inverted box (for ananlogue readings)
//
void showHex(
    Canvas* const canvas,
    uint8_t x,
    uint8_t y,
    const uint32_t val,
    const uint8_t cnt,
    const int b) {
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, x++, y++, 1 + (cnt * (6 + 1)), 10);

    // thicken border
    if(b == 2) canvas_draw_frame(canvas, x - 2, y - 2, 1 + (cnt * (6 + 1)) + 2, 10 + 2);

    for(int i = (cnt - 1) * 4; i >= 0; i -= 4, x += 6 + 1)
        show(canvas, x, y, img_6x8[(val >> i) & 0xF], SHOW_SET_WHT);
}

//============================================================================= ========================================
// Show the up/down "peak hold" controls in the bottom right
//
void showPeakHold(state_t* const state, Canvas* const canvas, const int hold) {
    switch(hold) {
    case 0:
        show(canvas, 119, 51, &img_key_U, SHOW_CLR_BLK);
        show(canvas, 119, 56, &img_key_D, SHOW_CLR_BLK);
        break;
    case +1:
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, 120, 52, 7, 6);
        show(canvas, 119, 51, &img_key_U, SHOW_CLR_WHT);
        show(canvas, 119, 56, &img_key_D, SHOW_CLR_BLK);
        break;
    case -1:
        show(canvas, 119, 51, &img_key_U, SHOW_CLR_BLK);
        canvas_draw_box(canvas, 120, 57, 7, 6);
        show(canvas, 119, 56, &img_key_D, SHOW_CLR_WHT);
        break;
    default:
        break;
    }
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, 119, 51, 9, 13);

    // calibration indicator
    show(
        canvas,
        108,
        55,
        ((state->calib & CAL_RANGE) && (++state->flash & 8)) ? &img_key_OKi : &img_key_OK,
        SHOW_SET_BLK);
}

//============================================================================= ========================================
// This code performs a FULL calibration on the device EVERY time it draws a joystick
//...This is NOT a good way forward for anything other than a test tool.
//
// Realistically you would do all the maths when the controller is connected
// or, if you prefer (and it IS a good thing), have a "calibrate controller" menu option
// ...and then just use a lookup table, or trivial formual
//
// THIS algorithm chops the joystick in to one of 9 zones
//    Eg.  {FullLeft, Left3, Left2, Left1, Middle, Right1, Right2, Right3, FullRight}
// FullLeft and FullRight have a deadzone of N [qv. xDead] ..a total of N+1 positions
// Middle has a deadzone of N EACH WAY ...a total of 2N+1 positions
//
// If the remaining range does not divide evenly in to three zones,
//   the first remainder is added to zone3,
//   and the second remainder (if there is one) is added to zone2
//   ...giving finer control near the centre of the joystick
//
// The value of the deadzone is based on the number of bits in the
// joystcik {x,y} values - the larger the range, the larger the deadzone.
//
//          03                                  15                                        29
//         |<<|      Calibration points        |==|                                      |>>|
// 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
//          |---| |________________________| |------| |______________________________| |---|
//          |r=2| |       range = 9        | | r=3  | |        range = 11            | |r=2|
//  Zones:  |-4 | |-3     |-2      |-1     | |0     | |+1     |+2         |+3        | |+4 |
//
// This is not "the right way to do it" ...this is "one way to do it"
// Consider you application, and what the user is trying to achieve
//   Aim a gun - probably need to be more accurate
//   Turn and object - this is probably good enough
//   Start slowly & pick up speed - how about a log or sine curve?
//
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
    const uint8_t bits) {
    int xOff = 0; // final offset of joystick hat image
    int yOff = 0;

    int xDead = (bits < 7) ? (1 << 0) : (1 << 3); // dead zone (centre & limits)
    int yDead = xDead;

    // This code is NOT optimised ...and it's still barely readable!
    if((xPos >= (xMid - xDead)) && (xPos <= (xMid + xDead)))
        xOff = 0; // centre [most likely]
    else if(xPos <= (xMin + xDead))
        xOff = -4; // full left
    else if(xPos >= (xMax - xDead))
        xOff = +4; // full right
    else if(xPos < (xMid - xDead)) { // part left
        // very much hard-coded for 3 interim positions
        int lo = (xMin + xDead) + 1; // lowest position
        int hi = (xMid - xDead) - 1; // highest position

        // this is the only duplicated bit of code
        int range = (hi - lo) + 1; // range covered
        int div = range / 3; // each division (base amount, eg. 17/3==5)
        int rem = range - (div * 3); // remainder (ie. range%3)

        //		int  hi1   = hi;                     // lowest  value for zone #-1
        //		int  lo1   = hi1 -div +1;            // highest value for zone #-1
        //		int  hi2   = lo1 -1;                 // lowest  value for zone #-2
        //		int  lo2   = hi2 -div +1 -(rem==2);  // highest value for zone #-2 expand out remainder
        //		int  hi3   = lo2 -1;                 // lowest  value for zone #-3
        //		int  lo3   = hi3 -div +1 -(rem>=1);  // highest value for zone #-3 expand out remainder

        int lo1 = hi - div + 1; // (in brevity)
        int hi3 = hi - div - div - (rem == 2); // ...

        if(xPos <= hi3)
            xOff = -3; // zone #-3
        else if(xPos >= lo1)
            xOff = -1; // zone #-1
        else
            xOff = -2; // zone #-2

    } else /*if (xPos > (xMid +xDead))*/ { // part right
        // very much hard-coded for 3 interim positions
        int lo = (xMid + xDead) + 1; // lowest position
        int hi = (xMax - xDead) - 1; // highest position

        int range = (hi - lo) + 1; // range covered
        int div = range / 3; // each division (base amount, eg. 17/3==5)
        int rem = range - (div * 3); // remainder (ie. range%3)

        //		int  lo1   = lo;                     // lowest  value for zone #+1
        //		int  hi1   = lo +div -1;             // highest value for zone #+1
        //		int  lo2   = hi1 +1;                 // lowest  value for zone #+2
        //		int  hi2   = lo2 +div -1 +(rem==2);  // highest value for zone #+2 expand out remainder
        //		int  lo3   = hi2 +1;                 // lowest  value for zone #+3
        //		int  hi3   = lo3 +div -1 +(rem>=1);  // highest value for zone #+3 expand out remainder

        int hi1 = lo + div - 1; // (in brevity)
        int lo3 = lo + div + div + (rem == 2); // ...

        if(xPos <= hi1)
            xOff = 1; // zone #1
        else if(xPos >= lo3)
            xOff = 3; // zone #3
        else
            xOff = 2; // zone #2
    }

    // All this to print a 3x3 square (in the right place) - LOL!
    if((yPos >= (yMid - yDead)) && (yPos <= (yMid + yDead)))
        yOff = 0; // centre [most likely]
    else if(yPos <= (yMin + yDead))
        yOff = +4; // full down
    else if(yPos >= (yMax - yDead))
        yOff = -4; // full up
    else if(yPos < (yMid - yDead)) { // part down
        int lo = (yMin + yDead) + 1; // lowest position
        int hi = (yMid - yDead) - 1; // highest position

        int range = (hi - lo) + 1; // range covered
        int div = range / 3; // each division (base amount, eg. 17/3==5)
        int rem = range - (div * 3); // remainder (ie. range%3)

        int lo1 = hi - div + 1; // (in brevity)
        int hi3 = hi - div - div - (rem == 2); // ...

        if(yPos <= hi3)
            yOff = +3; // zone #3
        else if(yPos >= lo1)
            yOff = +1; // zone #1
        else
            yOff = +2; // zone #2

    } else /*if (yPos > (yMid +yDead))*/ { // part up
        int lo = (yMid + yDead) + 1; // lowest position
        int hi = (yMax - yDead) - 1; // highest position

        int range = (hi - lo) + 1; // range covered
        int div = range / 3; // each division (base amount, eg. 17/3==5)
        int rem = range - (div * 3); // remainder (ie. range%3)

        int hi1 = lo + div - 1; // (in brevity)
        int lo3 = lo + div + div + (rem == 2); // ...

        if(yPos <= hi1)
            yOff = -1; // zone #-1
        else if(yPos >= lo3)
            yOff = -3; // zone #-3
        else
            yOff = -2; // zone #-2
    }

    show(canvas, x - (img_cc_Joy.w / 2), y - (img_cc_Joy.h / 2), &img_cc_Joy, SHOW_SET_BLK);

    // All ^that^ for v-this-v - LOL!!
    canvas_draw_box(canvas, (x - 1) + xOff, (y - 1) + yOff, 3, 3);
}
