#include <stdbool.h>
#include <stdint.h>

#include "wii_anal.h"
#include "wii_anal_lcd.h"
#include "wii_anal_keys.h"

//+============================================================================ ========================================
// Handle Wii Extension Controller events
//
bool evWiiEC(const eventMsg_t* const msg, state_t* const state) {
    bool redraw = false;

#if LOG_LEVEL >= 4
    {
        const char* s = NULL;
        switch(msg->wiiEc.type) {
        case WIIEC_NONE:
            s = "Error";
            break;
        case WIIEC_CONN:
            s = "Connect";
            break;
        case WIIEC_DISCONN:
            s = "Disconnect";
            break;
        case WIIEC_PRESS:
            s = "Press";
            break;
        case WIIEC_RELEASE:
            s = "Release";
            break;
        case WIIEC_ANALOG:
            s = "Analog";
            break;
        case WIIEC_ACCEL:
            s = "Accel";
            break;
        default:
            s = "Bug";
            break;
        }
        INFO(
            "WIIP : %s '%c' = %d",
            s,
            (isprint((int)msg->wiiEc.in) ? msg->wiiEc.in : '_'),
            msg->wiiEc.val);
        if((msg->wiiEc.type == WIIEC_CONN) || (msg->wiiEc.type == WIIEC_DISCONN))
            INFO("...%d=\"%s\"", msg->wiiEc.val, ecId[msg->wiiEc.val].name);
    }
#endif

    switch(msg->wiiEc.type) {
    case WIIEC_CONN:
        patBacklight(state);
        state->hold = 0;
        state->calib = CAL_TRACK;
        sceneSet(state, ecId[msg->wiiEc.val].scene);
        redraw = true;

#if 1 // Workaround for Classic Controller Pro, which shows 00's for Factory Calibration Data!?
        if(state->ec.pidx == PID_CLASSIC_PRO) {
            // Simulate a Long-OK keypress, to start Software Calibration mode
            eventMsg_t msg = {//					.id         = EVID_KEY,
                              .input.type = InputTypeLong,
                              .input.key = InputKeyOk};
            key_calib(&msg, state);
        }
#endif
        break;

    case WIIEC_DISCONN:
        patBacklight(state);
        sceneSet(state, SCENE_WAIT);
        redraw = true;
        break;

    case WIIEC_PRESS:
        if(state->scene == SCENE_NUNCHUCK_ACC) switch(msg->wiiEc.in) {
            case 'z': // un-pause
                state->pause = !state->pause;
                break;
            case 'c': // toggle auto-pause
                state->pause = false;
                state->apause = !state->apause;
                break;
            default:
                break;
            }

#if 1 //! factory calibration method not known for classic triggers - this will set the digital switch point
        if((state->ec.pidx == PID_CLASSIC) || (state->ec.pidx == PID_CLASSIC_PRO)) {
            if(msg->wiiEc.in == 'l') state->ec.calS.classic[2].trgZL = msg->wiiEc.val;
            if(msg->wiiEc.in == 'r') state->ec.calS.classic[2].trgZR = msg->wiiEc.val;
        }
#endif
        __attribute__((fallthrough));

    case WIIEC_RELEASE:
        patBacklight(state);
        redraw = true;
        break;

    case WIIEC_ANALOG:
    case WIIEC_ACCEL:
        ecCalibrate(&state->ec, state->calib);
        redraw = true;
        break;

    default:
        break;
    }

    return redraw;
}
