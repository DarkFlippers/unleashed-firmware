#include <stdint.h>
#include <furi.h> // Core API

#include "wii_anal.h"
#include "wii_i2c.h"
#include "wii_ec.h"
#include "bc_logging.h"

#include "gfx/images.h" // Images
#include "wii_anal_lcd.h" // Drawing functions
#include "wii_anal_keys.h" // key mappings

//----------------------------------------------------------------------------- ----------------------------------------
// List of known perhipherals
//
// More perhipheral ID codes here:  https://wiibrew.org/wiki/Wiimote/Extension_Controllers#The_New_Way
//
const ecId_t ecId[PID_CNT] = {
    [PID_UNKNOWN] =
        {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
         "Unknown Perhipheral",
         SCENE_DUMP,
         NULL,
         NULL,
         NULL,
         NULL,
         ec_show,
         ec_key},

    // If you're wise, ONLY edit this bit
    [PID_NUNCHUCK] =
        {{0x00, 0x00, 0xA4, 0x20, 0x00, 0x00},
         "Nunchuck",
         SCENE_NUNCHUCK,
         NULL,
         nunchuck_decode,
         nunchuck_msg,
         nunchuck_calib,
         nunchuck_show,
         nunchuck_key},

    [PID_NUNCHUCK_R2] =
        {{0xFF, 0x00, 0xA4, 0x20, 0x00, 0x00},
         "Nunchuck (rev2)",
         SCENE_NUNCHUCK,
         NULL,
         nunchuck_decode,
         nunchuck_msg,
         nunchuck_calib,
         nunchuck_show,
         nunchuck_key},

    [PID_CLASSIC] =
        {{0x00, 0x00, 0xA4, 0x20, 0x01, 0x01},
         "Classic Controller",
         SCENE_CLASSIC,
         NULL,
         classic_decode,
         classic_msg,
         classic_calib,
         classic_show,
         classic_key},

    [PID_CLASSIC_PRO] =
        {{0x01, 0x00, 0xA4, 0x20, 0x01, 0x01},
         "Classic Controller Pro",
         SCENE_CLASSIC,
         NULL,
         classic_decode,
         classic_msg,
         classic_calib,
         classic_show,
         classic_key},

    [PID_BALANCE] =
        {{0x00, 0x00, 0xA4, 0x20, 0x04, 0x02},
         "Balance Board",
         SCENE_DUMP,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL},

    [PID_GH_GUITAR] =
        {{0x00, 0x00, 0xA4, 0x20, 0x01, 0x03},
         "Guitar Hero Guitar",
         SCENE_DUMP,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL},

    [PID_GH_DRUMS] =
        {{0x01, 0x00, 0xA4, 0x20, 0x01, 0x03},
         "Guitar Hero World Tour Drums",
         SCENE_DUMP,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL},

    [PID_TURNTABLE] =
        {{0x03, 0x00, 0xA4, 0x20, 0x01, 0x03},
         "DJ Hero Turntable",
         SCENE_DUMP,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL},

    [PID_TAIKO_DRUMS] =
        {{0x00, 0x00, 0xA4, 0x20, 0x01, 0x11},
         "Taiko Drum Controller)",
         SCENE_DUMP,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL}, // Taiko no Tatsujin TaTaCon (Drum controller)

    [PID_UDRAW] =
        {{0xFF, 0x00, 0xA4, 0x20, 0x00, 0x13},
         "uDraw Tablet",
         SCENE_DUMP,
         udraw_init,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL}, //! same as drawsome?
    // -----

    [PID_ERROR] =
        {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         "Read Error",
         SCENE_NONE,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL},

    [PID_NULL] = {{0}, NULL, SCENE_NONE, NULL, NULL, NULL, NULL, NULL, NULL} // last entry
};

//+============================================================================ ========================================
void ecDecode(wiiEC_t* pec) {
    if(ecId[pec->pidx].decode) ecId[pec->pidx].decode(pec);
}

//+============================================================================ ========================================
void ecCalibrate(wiiEC_t* const pec, ecCalib_t c) {
    if(ecId[pec->pidx].calib) ecId[pec->pidx].calib(pec, c);
}

//+============================================================================ ========================================
void ecPoll(wiiEC_t* const pec, FuriMessageQueue* const queue) {
    ENTER;
    furi_assert(queue);

    if(!pec->init) {
        // Attempt to initialise
        if(ecInit(pec, NULL)) { //! need a way to auto-start with encryption enabled
            eventMsg_t msg = {
                .id = EVID_WIIEC, .wiiEc = {.type = WIIEC_CONN, .in = '<', .val = pec->pidx}};
            furi_message_queue_put(queue, &msg, 0);
        }

    } else {
        // Attempt to read
        switch(ecRead(pec)) {
        case 2: { // device gone
            eventMsg_t msg = {
                .id = EVID_WIIEC, .wiiEc = {.type = WIIEC_DISCONN, .in = '>', .val = pec->pidx}};
            furi_message_queue_put(queue, &msg, 0);
            break;
        }

        case 0: { // read OK
            void (*fn)(wiiEC_t*, FuriMessageQueue*) = ecId[pec->pidx].check;
            if(fn) fn(pec, queue);
            break;
        }

        case 3: // read fail
            // this is probably temporary  just ignore it
            break;

        default: // bug: unknown
        case 1: // bug: not initialised - should never happen
            ERROR("%s : read bug", __func__);
            break;
        }
    }

    LEAVE;
    return;
}

//+============================================================================ ========================================
// This is the screen drawn for an unknown controller
// It is also available by pressing LEFT (at least once) on a "known controller" screen
//
void ec_show(Canvas* const canvas, state_t* const state) {
    wiiEC_t* pec = &state->ec;
    int h = 11; // line height
    int x = 1; // (initial) offset for bits
    int y = -h; // previous y value
    int yb = 0; // y for bit patterns
    int c2 = 17; // column 2

    // Headings
    canvas_set_font(canvas, FontSecondary);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, "SID:");
    canvas_draw_str_aligned(canvas, c2, 0, AlignLeft, AlignTop, pec->sid);

    canvas_draw_str_aligned(canvas, 0, 11, AlignLeft, AlignTop, "PID:");
    canvas_draw_str_aligned(canvas, 0, 22, AlignLeft, AlignTop, "Cal:");

    // PID
    x = c2;
    for(int i = 0; i < 6; i++) {
        show(canvas, x, 11, img_5x7[pec->pid[i] >> 4], SHOW_SET_BLK);
        x += 5 + 1;
        show(canvas, x, 11, img_5x7[pec->pid[i] & 0xF], SHOW_SET_BLK);
        x += 5 + 1 + 2;
    }

    // Calibrations data
    y = 11;
    for(int j = 0; j <= 8; j += 8) {
        x = c2;
        y += 11;
        for(int i = 0; i < 8; i++) {
            show(canvas, x, y, img_5x7[pec->calF[i + j] >> 4], SHOW_SET_BLK);
            x += 5 + 1;
            show(canvas, x, y, img_5x7[pec->calF[i + j] & 0xF], SHOW_SET_BLK);
            x += 5 + 1 + 2;
        }
    }

    // Reading
    x = 1;
    y++;
    yb = (y += h) + h + 2;

    canvas_draw_line(canvas, x, y - 1, x, yb + 4);
    x += 2;

    for(int i = 0; i < JOY_LEN; i++) {
        show(canvas, x + 1, y, img_6x8[pec->joy[i] >> 4], SHOW_SET_BLK);
        show(canvas, x + 11, y, img_6x8[pec->joy[i] & 0xF], SHOW_SET_BLK);

        // bits
        for(int m = 0x80; m; m >>= 1) {
            x += 2 * !!(m & 0x08); // nybble step
            canvas_draw_box(canvas, x, yb + (2 * !(pec->joy[i] & m)), 2, 2);
            x += 2; // bit step
        }

        // byte step
        x += 1;
        canvas_draw_line(canvas, x, y - 1, x, yb + 4);
        x += 2;
    }

    // Scene navigation
    if(state->scenePrev != SCENE_WAIT) show(canvas, 120, 0, &img_key_R, SHOW_SET_BLK);
}

//+============================================================================ ========================================
// The DUMP screen is
//
bool ec_key(const eventMsg_t* const msg, state_t* const state) {
    int used = false; // assume key is NOT-handled

    if(state->scenePrev != SCENE_WAIT) {
        //# <L [ SHORT-RIGHT ]
        if((msg->input.type == InputTypeShort) && (msg->input.key == InputKeyRight)) {
            sceneSet(state, state->scenePrev);
            used = true;
        }
    }

    return used;
}
