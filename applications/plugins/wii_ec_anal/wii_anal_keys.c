#include <stdbool.h>

#include "bc_logging.h"

#include "wii_anal.h"

//+============================================================================ ========================================
// Stop Calibration mode
//
static void calStop(state_t* const state) {
    state->hold = 0; // stop calibration mode
    state->calib &= ~(CAL_RANGE | CAL_NOTJOY); // ...
}

//+============================================================================ ========================================
// Change to another scene
//
void sceneSet(state_t* const state, const scene_t scene) {
    calStop(state); // Stop software calibration
    state->scenePrev = state->scene; // Remember where we came from
    state->scene = scene; // Go to new scene
    INFO("Scene : %d -> %d", state->scenePrev, state->scene);
}

//+============================================================================ ========================================
// Change to an easter egg scene
//
static void sceneSetEgg(state_t* const state, const scene_t scene) {
    calStop(state); // Stop software calibration
    state->scenePegg = state->scene; // Remember where we came from
    state->scene = scene; // Go to new scene
    INFO("Scene* : %d => %d", state->scenePegg, state->scene);
}

//+============================================================================ ========================================
// Several EC screens have 'peak-hold' and 'calibration' features
// Enabling peak-hold on screen with no peak meters will have no effect
// So, to avoid code duplication...
//
bool key_calib(const eventMsg_t* const msg, state_t* const state) {
    int used = false; // assume key is NOT-handled

    switch(msg->input.type) {
    case InputTypeShort: //# <!  After InputTypeRelease within INPUT_LONG_PRESS interval
        switch(msg->input.key) {
        case InputKeyUp: //# <U [ SHORT-UP ]
            state->hold = (state->hold == +1) ? 0 : +1; // toggle peak hold
            used = true;
            break;

        case InputKeyDown: //# <D [ SHORT-DOWN ]
            state->hold = (state->hold == -1) ? 0 : -1; // toggle trough hold
            used = true;
            break;

        case InputKeyOk: //# <O [ SHORT-OK ]
            if(state->calib & CAL_RANGE)
                calStop(state); // STOP softare calibration
            else
                ecCalibrate(&state->ec, CAL_CENTRE); // perform centre calibration
            used = true;
            break;

        default:
            break;
        }
        break;

    case InputTypeLong: //# >!  After INPUT_LONG_PRESS interval, asynch to InputTypeRelease
        switch(msg->input.key) {
        case InputKeyOk: //# >O [ LONG-OK ]
            ecCalibrate(&state->ec, CAL_RESET | CAL_CENTRE); // START software calibration
            state->hold = 0;
            state->calib |= CAL_RANGE;
            state->flash = 8; // start with flash ON
            used = true;
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return used;
}

//+============================================================================ ========================================
// WAIT screen
//
static inline bool wait_key(const eventMsg_t* const msg, state_t* const state) {
    int used = false; // assume key is NOT-handled

    if(msg->input.type == InputTypeShort) {
        switch(msg->input.key) {
        case InputKeyLeft: //# <L [ SHORT-LEFT ]
            sceneSet(state, SCENE_SPLASH);
            used = true;
            break;

        case InputKeyUp: //# <U [ SHORT-UP ]
            sceneSet(state, SCENE_RIP);
            used = true;
            break;

        case InputKeyBack: //# <B [ SHORT-BACK ]
            state->run = false;
            used = true;
            break;

        default:
            break;
        }
    }

    return used;
}

//+============================================================================ ========================================
// DEBUG screen
//
static inline bool debug_key(const eventMsg_t* const msg, state_t* const state) {
    int used = false; // assume key is NOT-handled

    switch(msg->input.type) {
    case InputTypeShort: //# <!  After InputTypeRelease within INPUT_LONG_PRESS interval
        switch(msg->input.key) {
        case InputKeyUp: { //# <U [ SHORT-UP ]
            bool init =
                ecInit(&state->ec, NULL); // Initialise the controller //! NULL = no encryption
            (void)init; // in case INFO is optimised out
            INFO("%s : %s", __func__, (init ? "init OK" : "init fail"));
            used = true;
            break;
        }

        case InputKeyOk: //# <O [ SHORT-OK ]
            if(ecRead(&state->ec) == 0) { // Read the controller
                INFO(
                    "%s : joy: {%02X,%02X,%02X,%02X,%02X,%02X}",
                    __func__,
                    state->ec.joy[0],
                    state->ec.joy[1],
                    state->ec.joy[2],
                    state->ec.joy[3],
                    state->ec.joy[4],
                    state->ec.joy[5]);
            }
            used = true;
            break;

        case InputKeyDown: //# <D [ SHORT-DOWN ]
            timerEn(state, true); // restart the timer
            sceneSet(state, state->scenePrev);
            used = true;
            break;

        case InputKeyBack: //# <B [ SHORT-BACK ]
            state->run = false;
            used = true;
            break;

        default:
            break; //# <?
        }
        break;

    default:
        break;
    }

    return used;
}

//+============================================================================ ========================================
// SPLASH and RIP screen
//
static inline bool splash_key(const eventMsg_t* const msg, state_t* const state) {
    // Back key on the initial SPLASH screen (this will catch the InputKeyPress)
    if((msg->input.key == InputKeyBack) && (state->scenePrev == SCENE_NONE)) state->run = false;

    // ANY-other-KEY press
    if(msg->input.type == InputTypeShort) {
        timerEn(state, true); // Restart the timer
        state->scene = state->scenePegg;
    }

    return true;
}

//+============================================================================ ========================================
// "_pre" allows the plugin to use the key before the active scene gets a chance
//
static inline bool key_pre(const eventMsg_t* const msg, state_t* const state) {
    (void)msg;
    (void)state;

    return false;
}

//+============================================================================ ========================================
// "_post" allows the plugin to use a key if it was not used by the active scene
//
static inline bool key_post(const eventMsg_t* const msg, state_t* const state) {
    int used = false; // assume key is NOT-handled

    if(msg->input.key == InputKeyBack) {
        if(msg->input.type == InputTypeShort) { //# <B [SHORT-BACK]
            state->ec.init = false; // reset/disconnect the controller
            sceneSet(state, SCENE_WAIT);
            used = true;

        } else if(msg->input.type == InputTypeLong) { //# >B [LONG-BACK]
            state->run = false; // Signal the plugin to exit
            used = true;
        }
    }

    // Easter eggs
    switch(state->scene) {
    case SCENE_SPLASH: // Scenes that do NOT offer Easter eggs
    case SCENE_RIP:
    case SCENE_DEBUG:
        break;
    default:
        if(msg->input.type == InputTypeLong) {
            switch(msg->input.key) {
            case InputKeyDown: //# >D [LONG-DOWN]
                timerEn(state, false); // Stop the timer
                sceneSetEgg(state, SCENE_DEBUG);
                used = true;
                break;

            case InputKeyLeft: //# >L [ LONG-LEFT ]
                timerEn(state, false); // Stop the timer
                sceneSetEgg(state, SCENE_SPLASH);
                used = true;
                break;

            case InputKeyUp: //# >U [ LONG-UP ]
                timerEn(state, false); // Stop the timer
                sceneSetEgg(state, SCENE_RIP);
                used = true;
                break;

            default:
                break;
            }
        }
        break;
    }

    return used;
}

//+============================================================================ ========================================
// Handle a key press event
//
bool evKey(const eventMsg_t* const msg, state_t* const state) {
    furi_assert(msg);
    furi_assert(state);

    bool used = key_pre(msg, state);

    if(!used) switch(state->scene) {
        case SCENE_SPLASH: //...
        case SCENE_RIP:
            used = splash_key(msg, state);
            break;

        case SCENE_WAIT:
            used = wait_key(msg, state);
            break;
        case SCENE_DEBUG:
            used = debug_key(msg, state);
            break;

        default:
            if(state->ec.pidx >= PID_ERROR) {
                ERROR("%s : bad PID = %d", __func__, state->ec.pidx);
            } else {
                if((state->scene == SCENE_DUMP) || !ecId[state->ec.pidx].keys)
                    ecId[PID_UNKNOWN].keys(msg, state);
                else
                    ecId[state->ec.pidx].keys(msg, state);
            }
            break;

        case SCENE_NONE:
            break;
        }

    if(!used) used = key_post(msg, state);

    return used;
}
