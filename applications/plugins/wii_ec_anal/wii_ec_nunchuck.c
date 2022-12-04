#include <stdint.h>
#include <furi.h> // Core API

#include "wii_anal.h"
#include "wii_i2c.h"
#include "bc_logging.h"

#include "gfx/images.h" // Images
#include "wii_anal_lcd.h" // Drawing functions
#include "wii_anal_keys.h" // key mappings

// ** If you want to see what this source code looks like with all the MACROs expanded
// **   grep -v '#include  ' wii_ec_nunchuck.c | gcc -E -o /dev/stdout -xc -
#include "wii_ec_macros.h"

//+============================================================================ ========================================
// Standard Nunchuck : 2 buttons, 1 analogue joystick, 1 3-axis accelerometer
//
void nunchuck_decode(wiiEC_t* const pec) {
    ecDecNunchuck_t* p = &pec->dec[(pec->decN = !pec->decN)].nunchuck;
    uint8_t* joy = pec->joy;

    p->btnC = !(joy[5] & 0x02); // !{1}
    p->btnZ = !(joy[5] & 0x01); // !{1}

    p->joyX = joy[0]; // {8}
    p->joyY = joy[1]; // {8}

    p->accX = ((uint16_t)joy[2] << 2) | ((joy[5] >> 2) & 0x03); // {10}
    p->accY = ((uint16_t)joy[3] << 2) | ((joy[5] >> 4) & 0x03); // {10}
    p->accZ = ((uint16_t)joy[4] << 2) | ((joy[5] >> 6) & 0x03); // {10}

    DEBUG(
        ">%d> C:%c, Z:%c, Joy{x:%02X, y:%02X}, Acc{x:%03X, y:%03X, z:%03X}",
        pec->decN,
        (p->btnC ? '#' : '.'),
        (p->btnZ ? '#' : '.'),
        p->joyX,
        p->joyY,
        p->accX,
        p->accY,
        p->accZ);
}

//+============================================================================ ========================================
// Give each button a unique character identifier
//
void nunchuck_msg(wiiEC_t* const pec, FuriMessageQueue* const queue) {
    ecDecNunchuck_t* new = &pec->dec[pec->decN].nunchuck;
    ecDecNunchuck_t* old = &pec->dec[!pec->decN].nunchuck;

    eventMsg_t msg = {
        .id = EVID_WIIEC,
        .wiiEc = {
            .type = WIIEC_NONE,
            .in = ' ',
            .val = 0,
        }};

    BUTTON(btnC, 'c');
    BUTTON(btnZ, 'z');

    ANALOG(joyX, 'x');
    ANALOG(joyY, 'y');

    ACCEL(accX, 'x');
    ACCEL(accY, 'y');
    ACCEL(accZ, 'z');
}

//+============================================================================ ========================================
// https://www.hackster.io/infusion/using-a-wii-nunchuk-with-arduino-597254#toc-5--read-actual-calibration-data-from-the-device-14
//
void nunchuck_calib(wiiEC_t* const pec, ecCalib_t c) {
    ecDecNunchuck_t* src = &pec->dec[pec->decN].nunchuck; // from input
    ecCalNunchuck_t* dst = pec->calS.nunchuck; // to calibration data

    if(c & CAL_RESET) { // initialise ready for software calibration
        // LO is set to the MAXIMUM value (so it can be reduced)
        // HI is set to ZERO              (so it can be increased)
        RESET_LO_HI(accX, 10); // 10bit value
        RESET_LO_HI(accY, 10); // 10bit value
        RESET_LO_HI(accZ, 10); // 10bit value

        RESET_LO_HI(joyX, 8); // 8bit value
        RESET_LO_HI(joyY, 8); // 8bit value
    }
    if(c & CAL_FACTORY) { // (re)set to factory defaults
        //! "[4] LSB of Zero value of X,Y,Z axes" ...helpful!
        //! ...Well, my test nunchuck has bits set in the bottom 6 bits, so let's guess ;)

        // No value available - annecdotal tests suggest 8 is reasonable
        FACTORY_LO(accX, 8);
        FACTORY_LO(accY, 8);
        FACTORY_LO(accZ, 8);

        // @ 0G
        FACTORY_MID(accX, ((pec->calF[0] << 2) | ((pec->calF[3] >> 4) & 0x3)));
        FACTORY_MID(accY, ((pec->calF[1] << 2) | ((pec->calF[3] >> 2) & 0x3)));
        FACTORY_MID(accZ, ((pec->calF[2] << 2) | ((pec->calF[3]) & 0x3)));

        // @ 1G
        FACTORY_HI(accX, ((pec->calF[4] << 2) | ((pec->calF[7] >> 4) & 0x3)));
        FACTORY_HI(accY, ((pec->calF[5] << 2) | ((pec->calF[7] >> 2) & 0x3)));
        FACTORY_HI(accZ, ((pec->calF[6] << 2) | ((pec->calF[7]) & 0x3)));

        // Joysticks
        FACTORY_LO(joyX, pec->calF[9]);
        FACTORY_MID(joyX, pec->calF[10]);
        FACTORY_HI(joyX, pec->calF[8]);

        FACTORY_LO(joyY, pec->calF[12]);
        FACTORY_MID(joyY, pec->calF[13]);
        FACTORY_HI(joyY, pec->calF[11]);
    }
    if(c & CAL_TRACK) { // track maximum and minimum values seen
        TRACK_LO_HI(accX);
        TRACK_LO_HI(accY);
        TRACK_LO_HI(accZ);

        TRACK_LO_HI(joyX);
        TRACK_LO_HI(joyY);
    }
    if(c & CAL_RANGE) { // perform software calibration step
        RANGE_LO_HI(accX);
        RANGE_LO_HI(accY);
        RANGE_LO_HI(accZ);

        if(!(c & CAL_NOTJOY)) { // double negative!
            RANGE_LO_HI(joyX);
            RANGE_LO_HI(joyY);
        }
    }
    if(c & CAL_CENTRE) { // reset centre point of joystick
        CENTRE(accX);
        CENTRE(accY);
        CENTRE(accZ);

        CENTRE(joyX);
        CENTRE(joyY);
    }
}

//============================================================================= ========================================
// Accelerometer screen ...might this be useful for other controllers?
//
// https://bootlin.com/labs/doc/nunchuk.pdf
// X : Move Left/Right : -left / +right
// Y : Move Fwd/Bkwd   : -fwd  / +bkwd
// Z : Move Down/Up    : -down / +up
//
// Movement         in the direction of an axis changes   that axis   reading
// Twisting/tilting        around       an axis changes the other two readings
//
// EG. Move left will effect X ; turn left will effect Y & Z
//
#define aw 110 // axis width
#define ah 15 //      height   {0......7......14}
#define am 7 //      midpoint {       7       }
#define ar 7 //      range    {1234567 1234567}

enum {
    ACC_X = 0,
    ACC_Y = 1,
    ACC_Z = 2,
    ACC_CNT = 3,
    ACC_1 = ACC_X, // first
    ACC_N = ACC_Z, // last
};

//+============================================================================
static void nunchuck_showAcc(Canvas* const canvas, state_t* const state) {
    ecDecNunchuck_t* d = &state->ec.dec[state->ec.decN].nunchuck;
    ecCalNunchuck_t* lo = &state->ec.calS.nunchuck[1];
    ecCalNunchuck_t* mid = &state->ec.calS.nunchuck[2];
    ecCalNunchuck_t* hi = &state->ec.calS.nunchuck[3];

    int y[ACC_CNT] = {0, 0 + (ah + 4), 0 + ((ah + 4) * 2)};
    int x = 10;

    static uint16_t v[ACC_CNT][aw] = {0};
    //	static uint16_t    tv[ACC_CNT][aw] = {0};

    static uint16_t idx = 0;
    static uint16_t cnt = aw - 1;

    // Only record when scanner NOT-paused
    if(!state->pause) {
        uint16_t dead = (1 << 5);

        // Find axes y-offsets
        for(int a = ACC_1; a <= ACC_N; a++) {
            uint16_t* dp = NULL; // data value  (current reading)
            uint16_t* lp = NULL; //   lo value
            uint16_t* mp = NULL; //  mid value
            uint16_t* hp = NULL; //   hi value
            uint16_t* vp = NULL; // value       (result)

            switch(a) {
            case ACC_X:
                dp = &d->accX; // data (input)
                lp = &lo->accX; // low  \.
                mp = &mid->accX; // mid   > calibration
                hp = &hi->accX; // high /
                vp = &v[ACC_X][idx]; // value (where to store the result)
                break;
            case ACC_Y:
                dp = &d->accY;
                lp = &lo->accY;
                mp = &mid->accY;
                hp = &hi->accY;
                vp = &v[ACC_Y][idx];
                break;
            case ACC_Z:
                dp = &d->accZ;
                lp = &lo->accZ;
                mp = &mid->accZ;
                hp = &hi->accZ;
                vp = &v[ACC_Z][idx];
                break;
            default:
                break;
            }

            // Again - qv. the joysick calibration:
            //   This is not the "right way" to do this, it is just "one way" to do it
            // ...mid point and extreme zones have a deadzone
            // ...the rest is evenly divided by the amount of space on the graph
            if((*dp >= (*mp - dead)) && (*dp <= (*mp + dead)))
                *vp = ar;
            else if(*dp >= (*hp - dead))
                *vp = ah - 1;
            else if(*dp <= (*lp + dead))
                *vp = 0;
            else if(*dp < *mp) {
                uint16_t min = ((*lp + dead) + 1);
                uint16_t max = ((*mp - dead) - 1);
                float range = (max - min) + 1;
                float m = range / (ar - 1); // 6 evenly(/fairly) divided zones
                *vp = ((int)((*dp - min) / m)) + 1;

            } else { //if (*dp > *mp)
                uint16_t min = ((*mp + dead) + 1);
                uint16_t max = ((*hp - dead) - 1);
                float range = (max - min) + 1;
                float m = range / (ar - 1); // 6 evenly(/fairly) divided zones
                *vp = ((int)((*dp - min) / m)) + 1 + ar;
            }
        }

        //! If we decide to offer "export to CSV"
        //! I suggest we keep a second array of true-values, rather than do all the maths every time
        //! Also - the data will need to me moved to the 'state' table - so a.n.other function can save it off
        //		tv[ACC_X][idx] = d->accX;
        //		tv[ACC_Y][idx] = d->accY;
        //		tv[ACC_Z][idx] = d->accZ;

        // Prepare for the next datapoint
        if(++idx >= aw) idx = 0;
        if(cnt) cnt--;
    }

    // Auto-pause
    if(state->apause && !idx) state->pause = true;

    // *** Draw axes ***
    show(canvas, 0, y[ACC_X] + ((ah - img_6x8_X.h) / 2), &img_6x8_X, SHOW_SET_BLK);
    show(canvas, 0, y[ACC_Y] + ((ah - img_6x8_Y.h) / 2), &img_6x8_Y, SHOW_SET_BLK);
    show(canvas, 0, y[ACC_Z] + ((ah - img_6x8_Z.h) / 2), &img_6x8_Z, SHOW_SET_BLK);

    canvas_set_color(canvas, ColorBlack);
    for(int a = ACC_1; a <= ACC_N; a++) {
        canvas_draw_line(canvas, x - 1, y[a], x - 1, y[a] + ah);
        canvas_draw_line(canvas, x, y[a] + ah, x + aw - 1, y[a] + ah);

        // Mid & Peak lines
        for(int i = 1; i < aw; i += 3) {
            canvas_draw_dot(canvas, x + i, y[a]);
            canvas_draw_dot(canvas, x + i, y[a] + (ah / 2));
        }
    }

    // Data (wiper display - see notes.txt for scrolling algorithm)
    int end = idx ? idx : aw;
    for(int a = ACC_1; a <= ACC_N; a++) {
        canvas_draw_dot(canvas, x, y[a] + v[a][idx]);
        for(int i = 1; i < end; i++)
            canvas_draw_line(canvas, x + i, y[a] + v[a][i - 1], x + i, y[a] + v[a][i]);
        if(!state->apause)
            for(int i = end + 10; i < aw - cnt; i++)
                canvas_draw_line(canvas, x + i, y[a] + v[a][i - 1], x + i, y[a] + v[a][i]);
    }
    // Wipe bar
    if(end < aw) canvas_draw_line(canvas, x + end, y[0], x + end, y[2] + ah - 1);
    if(++end < aw) canvas_draw_line(canvas, x + end, y[0], x + end, y[2] + ah - 1);
    if(++end < aw) canvas_draw_line(canvas, x + end, y[0], x + end, y[2] + ah - 1);

    // *** Mode buttons ***
    show(canvas, 0, 55, &img_key_L, SHOW_SET_BLK); // mode key

    if((state->calib & CAL_RANGE) || state->pause) state->flash++;

    // -pause- ...yeah, this got a little out of hand! LOL!
    if(state->pause || state->apause) {
        if(state->pause && state->apause && !idx) {
            if(state->flash & 8) {
                show(canvas, 108, 56, &img_key_U, SHOW_SET_BLK);
            } else {
                show(canvas, 108, 56, &img_key_Ui, SHOW_SET_BLK);
                canvas_draw_line(canvas, x + aw, y[0], x + aw, y[2] + ah - 1);
            }
        } else {
            show(canvas, 108, 56, &img_key_Ui, SHOW_SET_BLK);
        }
    } else {
        show(canvas, 108, 56, &img_key_U, SHOW_SET_BLK); // pause
    }

    // -calibration-
    if(state->calib & CAL_RANGE) {
        show(canvas, 119, 55, (state->flash & 8) ? &img_key_OKi : &img_key_OK, SHOW_SET_BLK);
    } else {
        show(canvas, 119, 55, &img_key_OK, SHOW_SET_BLK);
    }
}

#undef aw
#undef ah
#undef am
#undef ar

//+============================================================================ ========================================
// Default nunchuck screen
//
void nunchuck_show(Canvas* const canvas, state_t* const state) {
    // Nunchucks have TWO scenes
    if(state->scene == SCENE_NUNCHUCK_ACC) return nunchuck_showAcc(canvas, state);

    // Default scene
    ecDecNunchuck_t* d = &state->ec.dec[state->ec.decN].nunchuck;
    ecCalNunchuck_t* c = (state->hold) ? &state->ec.calS.nunchuck[(state->hold < 0) ? 0 : 4] :
                                         (ecCalNunchuck_t*)d; //! danger will robinson!
    ecCalNunchuck_t* js = state->ec.calS.nunchuck;

    // X, Y, Z
    show(canvas, 42, 0, &img_6x8_X, SHOW_SET_BLK);
    show(canvas, 73, 0, &img_6x8_Y, SHOW_SET_BLK);
    show(canvas, 104, 0, &img_6x8_Z, SHOW_SET_BLK);

    canvas_draw_str_aligned(canvas, 0, 14, AlignLeft, AlignTop, "Accel");
    canvas_draw_str_aligned(canvas, 0, 28, AlignLeft, AlignTop, "Joy");

    // accel values
    showHex(canvas, 34, 12, c->accX, 3, 2);
    showHex(canvas, 65, 12, c->accY, 3, 2);
    showHex(canvas, 96, 12, c->accZ, 3, 2);
    // Joy values
    showHex(canvas, 38, 27, c->joyX, 2, 2);
    showHex(canvas, 69, 27, c->joyY, 2, 2);

    showJoy(
        canvas,
        103,
        32,
        js[1].joyX,
        js[2].joyX,
        js[3].joyX,
        js[1].joyY,
        js[2].joyY,
        js[3].joyY,
        d->joyX,
        d->joyY,
        8);

    // buttons
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str_aligned(canvas, 0, 44, AlignLeft, AlignTop, "Button");

    if(!d->btnC) {
        canvas_draw_rframe(canvas, 36, 42, 18, 12, 6);
        show(canvas, 42, 44, &img_6x8_C, SHOW_SET_BLK);
    } else {
        canvas_draw_rbox(canvas, 36, 42, 18, 12, 6);
        show(canvas, 42, 44, &img_6x8_C, SHOW_SET_WHT);
        canvas_set_color(canvas, ColorBlack);
    }

    if(!d->btnZ) {
        canvas_draw_rframe(canvas, 64, 40, 24, 16, 2);
        show(canvas, 73, 44, &img_6x8_Z, SHOW_SET_BLK);
    } else {
        canvas_draw_rbox(canvas, 64, 40, 24, 16, 2);
        show(canvas, 73, 44, &img_6x8_Z, SHOW_SET_WHT);
    }

    // Navigation
    showPeakHold(state, canvas, state->hold); // peak keys
    show(canvas, 0, 55, &img_key_L, SHOW_SET_BLK); // mode keys
    show(canvas, 9, 55, &img_key_R, SHOW_SET_BLK);
}

//+============================================================================ ========================================
static bool nunchuck_keyAcc(const eventMsg_t* const msg, state_t* const state) {
    int used = false; // assume key is NOT-handled

    switch(msg->input.type) {
    case InputTypeShort: //# <!  After InputTypeRelease within INPUT_LONG_PRESS interval
        switch(msg->input.key) {
        case InputKeyDown: //# <D [ SHORT-DOWN ]
            used = true; // Block trough-hold
            break;

        case InputKeyUp: //# <U [ SHORT-UP ]
            if(state->pause)
                state->pause = false; // Paused?  Restart
            else
                state->apause = !state->apause; // No?      toggle auto-pause
            used = true;
            break;

        case InputKeyLeft: //# <L [ SHORT-LEFT ]
            sceneSet(state, SCENE_NUNCHUCK);
            state->calib &= ~CAL_NOTJOY; // DO calibrate joystick in NUNCHUCK mode
            used = true;
            break;

        default:
            break; //# <?
        }
        break;

    default:
        break;
    }

    // Calibration keys
    if(!used) used = key_calib(msg, state);

    return used;
}

//+============================================================================ ========================================
bool nunchuck_key(const eventMsg_t* const msg, state_t* const state) {
    // Nunchucks have TWO scenes
    if(state->scene == SCENE_NUNCHUCK_ACC) return nunchuck_keyAcc(msg, state);

    // Default scene
    int used = false; // assume key is NOT-handled

    switch(msg->input.type) {
    case InputTypeShort: //# <!  After InputTypeRelease within INPUT_LONG_PRESS interval
        switch(msg->input.key) {
        case InputKeyLeft: //# <L [ SHORT-LEFT ]
            sceneSet(state, SCENE_DUMP);
            used = true;
            break;

        case InputKeyRight: //# <R [ SHORT-RIGHT ]
            sceneSet(state, SCENE_NUNCHUCK_ACC);
            state->calib |= CAL_NOTJOY; // do NOT calibrate joystick in _ACC mode
            used = true;
            break;
        default:
            break; //# <?
        }
        break;

    default:
        break;
    }

    // Calibration keys
    if(!used) used = key_calib(msg, state);

    return used;
}
