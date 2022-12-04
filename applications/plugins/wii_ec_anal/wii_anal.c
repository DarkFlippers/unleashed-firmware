//----------------------------------------------------------------------------- ----------------------------------------
// Includes
//

// System libs
#include <stdlib.h> // malloc
#include <stdint.h> // uint32_t
#include <stdarg.h> // __VA_ARGS__
#include <stdio.h>
#include <ctype.h>

// FlipperZero libs
#include <furi.h> // Core API
#include <gui/gui.h> // GUI (screen/keyboard) API
#include <input/input.h> // GUI Input extensions
#include <notification/notification_messages.h>

// Do this first!
#define ERR_C_ // Do this in precisely ONE file
#include "err.h" // Error numbers & messages

#include "bc_logging.h"

// Local headers
#include "wii_anal.h" // Various enums and struct declarations
#include "wii_i2c.h" // Wii i2c functions
#include "wii_ec.h" // Wii Extension Controller functions (eg. draw)
#include "wii_anal_keys.h" // key mappings
#include "gfx/images.h" // Images
#include "wii_anal_lcd.h" // Drawing functions
#include "wii_anal_ec.h" // Wii controller events

#include "wii_anal_ver.h" // Version number

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   OOOOO    // SSSSS       CCCCC  AAA  L     L     BBBB   AAA   CCCC K   K  SSSSS
//   O   O   /// S           C     A   A L     L     B   B A   A C     K  K   S
//   O   O  ///  SSSSS       C     AAAAA L     L     BBBB  AAAAA C     KKK    SSSSS
//   O   O ///       S       C     A   A L     L     B   B A   A C     K  K       S
//   OOOOO //    SSSSS       CCCCC A   A LLLLL LLLLL BBBB  A   A  CCCC K   K  SSSSS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//+============================================================================ ========================================
// OS Callback : Timer tick
// We register this function to be called when the OS signals a timer 'tick' event
//
static void cbTimer(FuriMessageQueue* queue) {
    ENTER;
    furi_assert(queue);

    eventMsg_t message = {.id = EVID_TICK};
    furi_message_queue_put(queue, &message, 0);

    LEAVE;
    return;
}

//+============================================================================ ========================================
// OS Callback : Keypress
// We register this function to be called when the OS detects a keypress
//
static void cbInput(InputEvent* event, FuriMessageQueue* queue) {
    ENTER;
    furi_assert(queue);
    furi_assert(event);

    // Put an "input" event message on the message queue
    eventMsg_t message = {.id = EVID_KEY, .input = *event};
    furi_message_queue_put(queue, &message, FuriWaitForever);

    LEAVE;
    return;
}

//+============================================================================
// Show version number
//
static void showVer(Canvas* const canvas) {
    show(canvas, 0, 59, &img_3x5_v, SHOW_SET_BLK);
    show(canvas, 4, 59, VER_MAJ, SHOW_SET_BLK);
    canvas_draw_frame(canvas, 8, 62, 2, 2);
    show(canvas, 11, 59, VER_MIN, SHOW_SET_BLK);
}

//+============================================================================
// OS Callback : Draw request
// We register this function to be called when the OS requests that the screen is redrawn
//
// We actually instruct the OS to perform this request, after we update the interface
// I guess it's possible that this instruction may able be issued by other threads !?
//
static void cbDraw(Canvas* const canvas, void* ctx) {
    ENTER;
    furi_assert(canvas);
    furi_assert(ctx);

    state_t* state = NULL;

    // Try to acquire the mutex for the plugin state variables, timeout = 25mS
    if(!(state = (state_t*)acquire_mutex((ValueMutex*)ctx, 25))) return;

    switch(state->scene) {
    //---------------------------------------------------------------------
    case SCENE_SPLASH:
        show(canvas, 2, 0, &img_csLogo_FULL, SHOW_SET_BLK);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 43, AlignCenter, AlignTop, "Wii Extension Controller");
        canvas_draw_str_aligned(canvas, 64, 55, AlignCenter, AlignTop, "Protocol Analyser");

        showVer(canvas);

        break;

    //---------------------------------------------------------------------
    case SCENE_RIP:
        show(canvas, 0, 0, &img_RIP, SHOW_SET_BLK);
        break;

    //---------------------------------------------------------------------
    case SCENE_WAIT:
#define xo 2

        show(canvas, 3 + xo, 10, &img_ecp_port, SHOW_SET_BLK);

        BOX_TL(22 + xo, 6, 82 + xo, 23); // 3v3
        BOX_TL(48 + xo, 21, 82 + xo, 23); // C1
        BOX_BL(22 + xo, 41, 82 + xo, 58); // C0
        BOX_BL(48 + xo, 41, 82 + xo, 44); // Gnd

        show(canvas, 90 + xo, 3, &img_6x8_3, SHOW_SET_BLK); // 3v3
        show(canvas, 97 + xo, 3, &img_6x8_v, SHOW_SET_BLK);
        show(canvas, 104 + xo, 3, &img_6x8_3, SHOW_SET_BLK);

        show(canvas, 90 + xo, 18, &img_6x8_C, SHOW_SET_BLK); // C1 <->
        show(canvas, 98 + xo, 18, &img_6x8_1, SHOW_SET_BLK);
        show(canvas, 107 + xo, 16, &img_ecp_SDA, SHOW_SET_BLK);

        show(canvas, 90 + xo, 40, &img_6x8_G, SHOW_SET_BLK); // Gnd
        show(canvas, 97 + xo, 40, &img_6x8_n, SHOW_SET_BLK);
        show(canvas, 104 + xo, 40, &img_6x8_d, SHOW_SET_BLK);

        show(canvas, 90 + xo, 54, &img_6x8_C, SHOW_SET_BLK); // C0 _-_-
        show(canvas, 98 + xo, 54, &img_6x8_0, SHOW_SET_BLK);
        show(canvas, 108 + xo, 54, &img_ecp_SCL, SHOW_SET_BLK);

        show(canvas, 0, 0, &img_csLogo_Small, SHOW_SET_BLK);
        showVer(canvas);

#undef xo
        break;

    //---------------------------------------------------------------------
    case SCENE_DEBUG:
        canvas_set_font(canvas, FontSecondary);

        show(canvas, 0, 0, &img_key_U, SHOW_SET_BLK);
        canvas_draw_str_aligned(canvas, 11, 0, AlignLeft, AlignTop, "Initialise Perhipheral");

        show(canvas, 0, 11, &img_key_OK, SHOW_SET_BLK);
        canvas_draw_str_aligned(canvas, 11, 11, AlignLeft, AlignTop, "Read values [see log]");

        show(canvas, 0, 23, &img_key_D, SHOW_SET_BLK);
        canvas_draw_str_aligned(canvas, 11, 22, AlignLeft, AlignTop, "Restart Scanner");

        show(canvas, 0, 33, &img_key_Back, SHOW_SET_BLK);
        canvas_draw_str_aligned(canvas, 11, 33, AlignLeft, AlignTop, "Exit");

        break;

    //---------------------------------------------------------------------
    default:
        if(state->ec.pidx >= PID_ERROR) {
            ERROR("%s : bad PID = %d", __func__, state->ec.pidx);
        } else {
            if((state->scene == SCENE_DUMP) || !ecId[state->ec.pidx].show)
                ecId[PID_UNKNOWN].show(canvas, state);
            else
                ecId[state->ec.pidx].show(canvas, state);
        }
        break;
    }

    // Release    the  mutex
    release_mutex((ValueMutex*)ctx, state);

    LEAVE;
    return;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   SSSSS TTTTT  AAA  TTTTT EEEEE       V   V  AAA  RRRR  IIIII  AAA  BBBB  L     EEEEE SSSSS
//   S       T   A   A   T   E           V   V A   A R   R   I   A   A B   B L     E     S
//   SSSSS   T   AAAAA   T   EEE          V V  AAAAA RRRR    I   AAAAA BBBB  L     EEE   SSSSS
//       S   T   A   A   T   E            V V  A   A R R     I   A   A B   B L     E         S
//   SSSSS   T   A   A   T   EEEEE         V   A   A R  R  IIIII A   A BBBB  LLLLL EEEEE SSSSS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//+============================================================================ ========================================
// Initialise plugin state variables
//
static inline bool stateInit(state_t* const state) {
    ENTER;
    furi_assert(state);

    bool rv = true; // assume success

    // Enable the main loop
    state->run = true;

    // Timer
    state->timerEn = false;
    state->timer = NULL;
    state->timerHz = furi_kernel_get_tick_frequency();
    state->fps = 30;

    // Scene
    state->scene = SCENE_SPLASH;
    state->scenePrev = SCENE_NONE;
    state->scenePegg = SCENE_NONE;

    state->hold = 0; // show hold meters (-1=lowest, 0=current, +1=highest}
    state->calib = CAL_TRACK;
    state->pause = false; // animation running
    state->apause = false; // auto-pause animation

    // Notifications
    state->notify = NULL;

    // Perhipheral
    state->ec.init = false;
    state->ec.pidx = PID_UNKNOWN;
    state->ec.sid = ecId[state->ec.pidx].name;

    // Controller data
    memset(state->ec.pid, 0xC5, PID_LEN); // Cyborg 5ystems
    memset(state->ec.calF, 0xC5, CAL_LEN);
    memset(state->ec.joy, 0xC5, JOY_LEN);

    // Encryption details
    state->ec.encrypt = false;
    memset(state->ec.encKey, 0x00, ENC_LEN);

    // Seed the PRNG
    // CYCCNT --> lib/STM32CubeWB/Drivers/CMSIS/Include/core_cm7.h
    //	srand(DWT->CYCCNT);

    LEAVE;
    return rv;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   MM MM  AAA  IIIII N   N
//   M M M A   A   I   NN  N
//   M M M AAAAA   I   N N N
//   M   M A   A   I   N  NN
//   M   M A   A IIIII N   N
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//+============================================================================ ========================================
// Enable/Disable scanning
//
void timerEn(state_t* state, bool on) {
    ENTER;
    furi_assert(state);

    // ENable scanning
    if(on) {
        if(state->timerEn) {
            WARN(wii_errs[WARN_SCAN_START]);
        } else {
            // Set the timer to fire at 'fps' times/second
            if(furi_timer_start(state->timer, state->timerHz / state->fps) == FuriStatusOk) {
                state->timerEn = true;
                INFO("%s : monitor started", __func__);
            } else {
                ERROR(wii_errs[ERR_TIMER_START]);
            }
        }

        // DISable scanning
    } else {
        if(!state->timerEn) {
            WARN(wii_errs[WARN_SCAN_STOP]);
        } else {
            // Stop the timer
            if(furi_timer_stop(state->timer) == FuriStatusOk) {
                state->timerEn = false;
                INFO("%s : monitor stopped", __func__);
            } else {
                ERROR(wii_errs[ERR_TIMER_STOP]);
            }
        }
    }

    LEAVE;
    return;
}

//+============================================================================ ========================================
// Plugin entry point
//
int32_t wii_ec_anal(void) {
    ENTER;

    // ===== Variables =====
    err_t error = 0; // assume success
    Gui* gui = NULL;
    ViewPort* vpp = NULL;
    state_t* state = NULL;
    ValueMutex mutex = {0};
    FuriMessageQueue* queue = NULL;
    const uint32_t queueSz = 20; // maximum messages in queue
    uint32_t tmo = (3.5f * 1000); // timeout splash screen after N seconds

    // The queue will contain plugin event-messages
    // --> local
    eventMsg_t msg = {0};

    INFO("BEGIN");

    // ===== Message queue =====
    // 1. Create a message queue (for up to 8 (keyboard) event messages)
    if(!(queue = furi_message_queue_alloc(queueSz, sizeof(msg)))) {
        ERROR(wii_errs[(error = ERR_MALLOC_QUEUE)]);
        goto bail;
    }

    // ===== Create GUI Interface =====
    // 2. Create a GUI interface
    if(!(gui = furi_record_open("gui"))) {
        ERROR(wii_errs[(error = ERR_NO_GUI)]);
        goto bail;
    }

    // ===== Plugin state variables =====
    // 3. Allocate space on the heap for the plugin state variables
    if(!(state = malloc(sizeof(state_t)))) {
        ERROR(wii_errs[(error = ERR_MALLOC_STATE)]);
        goto bail;
    }
    // 4. Initialise the plugin state variables
    if(!stateInit(state)) {
        // error message(s) is/are output by stateInit()
        error = 15;
        goto bail;
    }
    // 5. Create a mutex for (reading/writing) the plugin state variables
    if(!init_mutex(&mutex, state, sizeof(state))) {
        ERROR(wii_errs[(error = ERR_NO_MUTEX)]);
        goto bail;
    }

    // ===== Viewport =====
    // 6. Allocate space on the heap for the viewport
    if(!(vpp = view_port_alloc())) {
        ERROR(wii_errs[(error = ERR_MALLOC_VIEW)]);
        goto bail;
    }
    // 7a. Register a callback for input events
    view_port_input_callback_set(vpp, cbInput, queue);
    // 7b. Register a callback for draw events
    view_port_draw_callback_set(vpp, cbDraw, &mutex);

    // ===== Start GUI Interface =====
    // 8. Attach the viewport to the GUI
    gui_add_view_port(gui, vpp, GuiLayerFullscreen);

    // ===== Timer =====
    // 9. Allocate a timer
    if(!(state->timer = furi_timer_alloc(cbTimer, FuriTimerTypePeriodic, queue))) {
        ERROR(wii_errs[(error = ERR_NO_TIMER)]);
        goto bail;
    }

    // === System Notifications ===
    // 10. Acquire a handle for the system notification queue
    if(!(state->notify = furi_record_open(RECORD_NOTIFICATION))) {
        ERROR(wii_errs[(error = ERR_NO_NOTIFY)]);
        goto bail;
    }
    patBacklight(state); // Turn on the backlight [qv. remote FAP launch]

    INFO("INITIALISED");

    // ==================== Main event loop ====================

    if(state->run) do {
            bool redraw = false;
            FuriStatus status = FuriStatusErrorTimeout;

            // Wait for a message
            //		while ((status = furi_message_queue_get(queue, &msg, tmo)) == FuriStatusErrorTimeout) ;
            status = furi_message_queue_get(queue, &msg, tmo);

            // Clear splash screen
            if((state->scene == SCENE_SPLASH) &&
               (state->scenePrev == SCENE_NONE) && // Initial splash
               ((status == FuriStatusErrorTimeout) || // timeout
                ((msg.id == EVID_KEY) && (msg.input.type == InputTypeShort))) // or <any> key-short
            ) {
                tmo = 60 * 1000; // increase message-wait timeout to 60secs
                timerEn(state, true); // start scanning the i2c bus
                status = FuriStatusOk; // pass status check
                msg.id = EVID_NONE; // valid msg ID
                sceneSet(state, SCENE_WAIT); // move to wait screen
            }

            // Check for queue errors
            if(status != FuriStatusOk) {
                switch(status) {
                case FuriStatusErrorTimeout:
                    DEBUG(wii_errs[DEBUG_QUEUE_TIMEOUT]);
                    continue;
                case FuriStatusError:
                    ERROR(wii_errs[(error = ERR_QUEUE_RTOS)]);
                    goto bail;
                case FuriStatusErrorResource:
                    ERROR(wii_errs[(error = ERR_QUEUE_RESOURCE)]);
                    goto bail;
                case FuriStatusErrorParameter:
                    ERROR(wii_errs[(error = ERR_QUEUE_BADPRM)]);
                    goto bail;
                case FuriStatusErrorNoMemory:
                    ERROR(wii_errs[(error = ERR_QUEUE_NOMEM)]);
                    goto bail;
                case FuriStatusErrorISR:
                    ERROR(wii_errs[(error = ERR_QUEUE_ISR)]);
                    goto bail;
                default:
                    ERROR(wii_errs[(error = ERR_QUEUE_UNK)]);
                    goto bail;
                }
            }
            // Read successful

            // *** Try to lock the plugin state variables ***
            if(!(state = (state_t*)acquire_mutex_block(&mutex))) {
                ERROR(wii_errs[(error = ERR_MUTEX_BLOCK)]);
                goto bail;
            }

            // *** Handle events ***
            switch(msg.id) {
            //---------------------------------------------
            case EVID_TICK: // Timer events
                //! I would prefer to have ecPoll() called by cbTimer()
                //! ...but how does cbTimer() get the required access to the state variables?  Namely: 'state->ec'
                //! So, for now, the timer pushes a message to call ecPoll()
                //!                                       which, in turn, will push WIIEC event meesages! <facepalm>
                ecPoll(&state->ec, queue);
                break;

            //---------------------------------------------
            case EVID_WIIEC: // WiiMote Perhipheral
                if(evWiiEC(&msg, state)) redraw = true;
                break;

            //---------------------------------------------
            case EVID_KEY: // Key events
                patBacklight(state);
                if(evKey(&msg, state)) redraw = true;
                break;

            //---------------------------------------------
            case EVID_NONE:
                break;

            //---------------------------------------------
            default: // Unknown event
                WARN("Unknown message.ID [%d]", msg.id);
                break;
            }

            // *** Update the GUI screen via the viewport ***
            if(redraw) view_port_update(vpp);

            // *** Try to release the plugin state variables ***
            if(!release_mutex(&mutex, state)) {
                ERROR(wii_errs[(error = ERR_MUTEX_RELEASE)]);
                goto bail;
            }
        } while(state->run);

    // ===== Game Over =====
    INFO("USER EXIT");

bail:
    // 10. Release system notification queue
    if(state->notify) {
        furi_record_close(RECORD_NOTIFICATION);
        state->notify = NULL;
    }

    // 9. Stop the timer
    if(state->timer) {
        (void)furi_timer_stop(state->timer);
        furi_timer_free(state->timer);
        state->timer = NULL;
        state->timerEn = false;
    }

    // 8. Detach the viewport
    gui_remove_view_port(gui, vpp);

    // 7. No need to unreqgister the callbacks
    //    ...they will go when the viewport is destroyed

    // 6. Destroy the viewport
    if(vpp) {
        view_port_enabled_set(vpp, false);
        view_port_free(vpp);
        vpp = NULL;
    }

    // 5. Free the mutex
    if(mutex.mutex) {
        delete_mutex(&mutex);
        mutex.mutex = NULL;
    }

    // 4. Free up state pointer(s)
    // none

    // 3. Free the plugin state variables
    if(state) {
        free(state);
        state = NULL;
    }

    // 2. Close the GUI
    furi_record_close("gui");

    // 1. Destroy the message queue
    if(queue) {
        furi_message_queue_free(queue);
        queue = NULL;
    }

    INFO("CLEAN EXIT ... Exit code: %d", error);
    LEAVE;
    return (int32_t)error;
}
