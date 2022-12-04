#include <stdint.h>
#include <furi.h> // Core API

#include "wii_anal.h"
#include "wii_ec.h"
#include "bc_logging.h"

//#include  "gfx/images.h"     // Images
#include "wii_anal_lcd.h" // Drawing functions
#include "wii_anal_keys.h" // key mappings

// ** If you want to see what this source code looks like with all the MACROs expanded
// **   grep -v '#include  ' wii_i2c_classic.c | gcc -E -o /dev/stdout -xc -
#include "wii_ec_macros.h"

//----------------------------------------------------------------------------- ----------------------------------------
// Classic Controller ... Classic Controller Pro is electronically the same
//
//      ANA{l}                                        ANA{r}
//      BTN{l}     BTN{L}                   BTN{R}    BTN{r}
//    ,--------.    ,-,                      ,-,    .--------,
//  .----------------------------------------------------------.
//  |                                                          |
//  |     BTN{W}                                    BTN{x}     |
//  | BTN{A}  BTN{D}    BTN{-} BTN{h} BTN{+}    BTN{y}  BTN{a} |
//  |     BTN{S}                                    BTN{b}     |
//  |                                                          |
//  |               ANA{y}          ANA{Y}                     |
//  |           ANA{x}                  ANA{X}                 |
//  |                                                          |
//  `----------------------------------------------------------'

//+============================================================================ ========================================
// https://wiibrew.org/wiki/Wiimote/Extension_Controllers/Classic_Controller
// I think a LOT of drugs went in to "designing" this layout
// ...And yes, the left-joystick has an extra 'bit' of precision!
// ...Also: trgZ{L|R} WILL continue to increase after btnZ{L|R} has gone active
//
void classic_decode(wiiEC_t* const pec) {
    ecDecClassic_t* p = &pec->dec[(pec->decN = !pec->decN)].classic;
    uint8_t* joy = pec->joy;

    p->trgZL = ((joy[2] >> 2) & 0x18) | ((joy[3] >> 5) & 0x07); // {5}
    p->btnZL = !(joy[4] & 0x20); // !{1}

    p->trgZR = joy[3] & 0x1F; // {5}
    p->btnZR = !(joy[4] & 0x02); // !{1}

    p->btnL = !(joy[5] & 0x80); // !{1}
    p->btnR = !(joy[5] & 0x04); // !{1}

    p->padU = !(joy[5] & 0x01); // !{1}
    p->padD = !(joy[4] & 0x40); // !{1}
    p->padL = !(joy[5] & 0x02); // !{1}
    p->padR = !(joy[4] & 0x80); // !{1}

    p->btnM = !(joy[4] & 0x10); // !{1}
    p->btnH = !(joy[4] & 0x08); // !{1}
    p->btnP = !(joy[4] & 0x04); // !{1}

    p->btnX = !(joy[5] & 0x08); // !{1}
    p->btnY = !(joy[5] & 0x20); // !{1}

    p->btnA = !(joy[5] & 0x10); // !{1}
    p->btnB = !(joy[5] & 0x40); // !{1}

    p->joyLX = joy[0] & 0x3F; // {6}
    p->joyLY = joy[1] & 0x3F; // {6}

    p->joyRX = ((joy[0] >> 3) & 0x18) | ((joy[1] >> 5) & 0x06) | ((joy[2] >> 7) & 0x01); // {5}
    p->joyRY = joy[2] & 0x1F; // {5}

    DEBUG(
        ">%d> ZL{%02X}%c, L:%c, R:%c, ZR{%02X}%c",
        pec->decN,
        p->trgZL,
        (p->btnZL ? '#' : '.'),
        (p->btnL ? '#' : '.'),
        (p->btnR ? '#' : '.'),
        p->trgZR,
        (p->btnZR ? '#' : '.'));
    DEBUG(
        ">%d> D:{%c,%c,%c,%c}, H:{%c,%c,%c}, B:{%c,%c,%c,%c}",
        pec->decN,
        (p->padU ? 'U' : '.'),
        (p->padD ? 'D' : '.'),
        (p->padL ? 'L' : '.'),
        (p->padR ? 'R' : '.'),
        (p->btnM ? '-' : '.'),
        (p->btnH ? 'H' : '.'),
        (p->btnP ? '+' : '.'),
        (p->btnX ? 'X' : '.'),
        (p->btnY ? 'Y' : '.'),
        (p->btnA ? 'A' : '.'),
        (p->btnB ? 'B' : '.'));
    DEBUG(
        ">%d> JoyL{x:%02X, y:%02X}, JoyR{x:%02X, y:%02X}",
        pec->decN,
        p->joyLX,
        p->joyLY,
        p->joyRX,
        p->joyRY);
}

//+============================================================================ ========================================
// Give each button a unique character identifier
//
void classic_msg(wiiEC_t* const pec, FuriMessageQueue* const queue) {
    ecDecClassic_t* new = &pec->dec[pec->decN].classic;
    ecDecClassic_t* old = &pec->dec[!pec->decN].classic;

    eventMsg_t msg = {
        .id = EVID_WIIEC,
        .wiiEc = {
            .type = WIIEC_NONE,
            .in = ' ',
            .val = 0,
        }};

    ANALOG(trgZL, 'l'); // FIVE bit value
    ANABTN(btnZL, trgZL, 'l');

    BUTTON(btnL, 'L');
    BUTTON(btnR, 'R');

    ANALOG(trgZR, 'r'); // FIVE bit value
    ANABTN(btnZR, trgZR, 'r');

    BUTTON(padU, 'W');
    BUTTON(padL, 'A');
    BUTTON(padD, 'S');
    BUTTON(padR, 'D');

    BUTTON(btnM, '-');
    BUTTON(btnH, 'h');
    BUTTON(btnP, '+');

    BUTTON(btnX, 'x');
    BUTTON(btnY, 'y');
    BUTTON(btnA, 'a');
    BUTTON(btnB, 'b');

    ANALOG(joyLX, 'x'); // SIX bit values
    ANALOG(joyLY, 'y');

    ANALOG(joyRX, 'X'); // FIVE bit values
    ANALOG(joyRY, 'Y');
}

//+============================================================================ ========================================
// https://web.archive.org/web/20090415045219/http://www.wiili.org/index.php/Wiimote/Extension_Controllers/Classic_Controller#Calibration_data
//
// Calibration data
//    0..2  left  analog stick X axis {maximum, minimum, center} ... JoyL is 6bits, so >>2 to compare to readings
//    3..5  left  analog stick Y axis {maximum, minimum, center} ... JoyL is 6bits, so >>2 to compare to readings
//    6..8  right analog stick X axis {maximum, minimum, center} ... JoyR is 5bits, so >>3 to compare to readings
//    9..11 right analog stick Y axis {maximum, minimum, center} ... JoyR is 5bits, so >>3 to compare to readings
//   12..15 somehow describe the shoulder {5bit} button values!?
//
void classic_calib(wiiEC_t* const pec, ecCalib_t c) {
    ecDecClassic_t* src = &pec->dec[pec->decN].classic; // from input
    ecCalClassic_t* dst = pec->calS.classic; // to calibration data

    if(c & CAL_RESET) { // initialise ready for software calibration
        // LO is set to the MAXIMUM value (so it can be reduced)
        // HI is set to ZERO              (so it can be increased)
        RESET_LO_HI(trgZL, 5); // 5bit value
        RESET_LO_HI(trgZR, 5); // 5bit value

        RESET_LO_MID_HI(joyLX, 6); // 6bit value
        RESET_LO_MID_HI(joyLY, 6); // 6bit value

        RESET_LO_MID_HI(joyRX, 5); // 5bit value
        RESET_LO_MID_HI(joyRY, 5); // 5bit value
    }
    if(c & CAL_FACTORY) { // (re)set to factory defaults
        //! strategy for factory calibration for classic controller [pro] triggers is (currently) unknown
        //!		FACTORY_LO( trgZL, pec->calF[12..15]);
        //!		FACTORY_MID(trgZL, pec->calF[12..15]);
        //!		FACTORY_HI( trgZL, pec->calF[12..15]);

        //!		FACTORY_LO( trgZR, pec->calF[12..15]);
        //!		FACTORY_MID(trgZR, pec->calF[12..15]);
        //!		FACTORY_HI( trgZR, pec->calF[12..15]);

#if 1
        FACTORY_LO(trgZL, 0x03);
        FACTORY_LO(trgZR, 0x03);

        FACTORY_MID(trgZL, 0x1B); //! these will be set every time the digital switch changes to ON
        FACTORY_MID(trgZR, 0x1B);
#endif

        FACTORY_LO(joyLX, pec->calF[1] >> 2);
        FACTORY_MID(joyLX, pec->calF[2] >> 2);
        FACTORY_HI(joyLX, pec->calF[0] >> 2);

        FACTORY_LO(joyLY, pec->calF[4] >> 2);
        FACTORY_MID(joyLY, pec->calF[5] >> 2);
        FACTORY_HI(joyLY, pec->calF[3] >> 2);

        FACTORY_LO(joyRX, pec->calF[7] >> 3);
        FACTORY_MID(joyRX, pec->calF[8] >> 3);
        FACTORY_HI(joyRX, pec->calF[6] >> 3);

        FACTORY_LO(joyRY, pec->calF[10] >> 3);
        FACTORY_MID(joyRY, pec->calF[11] >> 3);
        FACTORY_HI(joyRY, pec->calF[9] >> 3);
    }
    if(c & CAL_TRACK) { // track maximum and minimum values seen
        TRACK_LO_HI(trgZL);
        TRACK_LO_HI(trgZR);

        TRACK_LO_HI(joyLX);
        TRACK_LO_HI(joyLY);

        TRACK_LO_HI(joyRX);
        TRACK_LO_HI(joyRY);
    }
    if(c & CAL_RANGE) { // perform software calibration step
        RANGE_LO_HI(trgZL);
        RANGE_LO_HI(trgZR);

        RANGE_LO_HI(joyLX);
        RANGE_LO_HI(joyLY);

        RANGE_LO_HI(joyRX);
        RANGE_LO_HI(joyRY);
    }
    if(c & CAL_CENTRE) { // reset centre point of joystick
        CENTRE(joyLX);
        CENTRE(joyLY);

        CENTRE(joyRX);
        CENTRE(joyRY);
    }
}

//+============================================================================ ========================================
// bits that are common to both screens
//
static void classic_show_(Canvas* const canvas, state_t* const state) {
    ecDecClassic_t* d = &state->ec.dec[state->ec.decN].classic;
    ecCalClassic_t* js = state->ec.calS.classic;

    static const int dead = 1; // trigger deadzone
    const image_t* img = NULL; // trigger image

    show(canvas, 6, 0, &img_cc_Main, SHOW_SET_BLK);
    show(canvas, 62, 53, &img_cc_Cable, SHOW_SET_BLK);

    // classic triggers
    if(d->trgZL >= js[2].trgZL)
        img = &img_cc_trg_L4;
    else if(d->trgZL <= js[1].trgZL + dead)
        img = NULL;
    else {
        // copied from the joystick calibration code
        int lo = js[1].trgZL + dead + 1;
        int hi = js[2].trgZL - 1;
        int range = hi - lo + 1;
        int div = range / 3; // each division (base amount, eg. 17/3==5)
        int rem = range - (div * 3); // remainder (ie. range%3)
        int hi1 = lo + div - 1; // (in brevity)
        int lo3 = lo + div + div + (rem == 2); // ...

        if(d->trgZL <= hi1)
            img = &img_cc_trg_L1; // zone #1
        else if(d->trgZL >= lo3)
            img = &img_cc_trg_L3; // zone #3
        else
            img = &img_cc_trg_L2; // zone #2
    }
    if(img) show(canvas, 22, 1, img, SHOW_SET_BLK);

    if(d->trgZR >= js[2].trgZR)
        img = &img_cc_trg_R4;
    else if(d->trgZR <= js[1].trgZR + dead)
        img = NULL;
    else {
        // copied from the joystick calibration code
        int lo = js[1].trgZR + dead + 1;
        int hi = js[2].trgZR - 1;
        int range = hi - lo + 1;
        int div = range / 3; // each division (base amount, eg. 17/3==5)
        int rem = range - (div * 3); // remainder (ie. range%3)
        int hi1 = lo + div - 1; // (in brevity)
        int lo3 = lo + div + div + (rem == 2); // ...

        if(d->trgZR <= hi1)
            img = &img_cc_trg_R1; // zone #1
        else if(d->trgZR >= lo3)
            img = &img_cc_trg_R3; // zone #3
        else
            img = &img_cc_trg_R2; // zone #2
    }
    if(img) show(canvas, 89, 1, img, SHOW_SET_BLK);

    if(d->padU) show(canvas, 27, 16, &img_cc_pad_UD1, SHOW_ALL);
    if(d->padL) show(canvas, 20, 23, &img_cc_pad_LR1, SHOW_ALL);
    if(d->padD) show(canvas, 27, 28, &img_cc_pad_UD1, SHOW_ALL);
    if(d->padR) show(canvas, 32, 23, &img_cc_pad_LR1, SHOW_ALL);

    if(d->btnX) show(canvas, 96, 16, &img_cc_btn_X1, SHOW_ALL);
    if(d->btnY) show(canvas, 85, 23, &img_cc_btn_Y1, SHOW_ALL);
    if(d->btnA) show(canvas, 107, 23, &img_cc_btn_A1, SHOW_ALL);
    if(d->btnB) show(canvas, 96, 30, &img_cc_btn_B1, SHOW_ALL);

    canvas_set_color(canvas, ColorBlack);
    if(d->btnL) canvas_draw_box(canvas, 46, 2, 5, 4);
    if(d->btnR) canvas_draw_box(canvas, 77, 2, 5, 4);

    if(d->btnM) canvas_draw_box(canvas, 54, 24, 4, 4);
    if(d->btnH) canvas_draw_box(canvas, 62, 24, 4, 4);
    if(d->btnP) canvas_draw_box(canvas, 70, 24, 4, 4);

    // Show joysticks
    showJoy(
        canvas,
        48,
        42,
        js[1].joyLX,
        js[2].joyLX,
        js[3].joyLX,
        js[1].joyLY,
        js[2].joyLY,
        js[3].joyLY,
        d->joyLX,
        d->joyLY,
        6);
    showJoy(
        canvas,
        78,
        42,
        js[1].joyRX,
        js[2].joyRX,
        js[3].joyRX,
        js[1].joyRY,
        js[2].joyRY,
        js[3].joyRY,
        d->joyRX,
        d->joyRY,
        5);

    show(canvas, 0, 55, &img_key_L, SHOW_SET_BLK);
}

//+============================================================================ ========================================
static void classic_showN(Canvas* const canvas, state_t* const state) {
    ecCalClassic_t* c = (state->hold) ?
                            &state->ec.calS.classic[(state->hold < 0) ? 0 : 4] :
                            (ecCalClassic_t*)(&state->ec.dec[state->ec.decN].classic); //! danger

    classic_show_(canvas, state);

    showHex(canvas, 0, 0, c->trgZL, 2, 1); // 5bits
    showHex(canvas, 113, 0, c->trgZR, 2, 1); // 5bits

    showHex(canvas, 24, 41, c->joyLX, 2, 1); // 6bits
    showHex(canvas, 41, 54, c->joyLY, 2, 1); // 6bits

    showHex(canvas, 88, 41, c->joyRX, 2, 1); // 5bits
    showHex(canvas, 71, 54, c->joyRY, 2, 1); // 5bits

    showPeakHold(state, canvas, state->hold); // peak keys
}

//+============================================================================ ========================================
void classic_show(Canvas* const canvas, state_t* const state) {
    // Classic controllers have TWO scenes
    if(state->scene == SCENE_CLASSIC_N) return classic_showN(canvas, state);

    // Default scene
    classic_show_(canvas, state);
    show(canvas, 9, 55, &img_key_R, SHOW_SET_BLK);

    show(
        canvas,
        119,
        55,
        ((state->calib & CAL_RANGE) && (++state->flash & 8)) ? &img_key_OKi : &img_key_OK,
        SHOW_SET_BLK);
}

//+============================================================================ ========================================
static bool classic_keyN(const eventMsg_t* const msg, state_t* const state) {
    int used = false; // assume key is NOT-handled

    if((msg->input.type == InputTypeShort) && (msg->input.key == InputKeyLeft)) {
        sceneSet(state, SCENE_CLASSIC);
        used = true;
    }

    // Calibration keys
    if(!used) used = key_calib(msg, state);

    return used;
}

//+============================================================================ ========================================
bool classic_key(const eventMsg_t* const msg, state_t* const state) {
    // Classic controllers have TWO scenes
    if(state->scene == SCENE_CLASSIC_N) return classic_keyN(msg, state);

    // Default scene
    int used = false; // assume key is NOT-handled

    switch(msg->input.type) {
    case InputTypeShort: //# <!  After InputTypeRelease within INPUT_LONG_PRESS interval
        switch(msg->input.key) {
        case InputKeyUp: //# <U [ SHORT-UP ]
        case InputKeyDown: //# <D [ SHORT-DOWN ]
            used = true; // Block peak/trough-hold
            break;

        case InputKeyLeft: //# <L [ SHORT-LEFT ]
            sceneSet(state, SCENE_DUMP);
            used = true;
            break;

        case InputKeyRight: //# <R [ SHORT-RIGHT ]
            sceneSet(state, SCENE_CLASSIC_N);
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
