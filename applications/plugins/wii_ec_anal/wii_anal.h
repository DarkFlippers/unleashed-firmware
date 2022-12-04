#ifndef WII_ANAL_H_
#define WII_ANAL_H_

#include <furi.h> // Core API
#include <input/input.h> // GUI Input extensions
#include <notification/notification_messages.h>

//----------------------------------------------------------------------------- ----------------------------------------
// GUI scenes
//
typedef enum scene {
    SCENE_NONE = 0,
    SCENE_SPLASH = 1,
    SCENE_RIP = 2,
    SCENE_WAIT = 3,
    SCENE_DEBUG = 4,
    SCENE_DUMP = 5,
    SCENE_CLASSIC = 6,
    SCENE_CLASSIC_N = 7,
    SCENE_NUNCHUCK = 8,
    SCENE_NUNCHUCK_ACC = 9,
} scene_t;

//----------------------------------------------------------------------------- ----------------------------------------
#include "wii_i2c.h"
#include "wii_ec.h"

//----------------------------------------------------------------------------- ----------------------------------------
// A list of event IDs handled by this plugin
//
typedef enum eventID {
    EVID_NONE,
    EVID_UNKNOWN,

    // A full list of events can be found with:  `grep -r --color  "void.*set_.*_callback"  applications/gui/*`
    // ...A free gift to you from the makers of well written code that conforms to a good coding standard
    EVID_KEY, // keypad
    EVID_TICK, // tick
    EVID_WIIEC, // wii extension controller
} eventID_t;

//----------------------------------------------------------------------------- ----------------------------------------
// An item in the event message-queue
//
typedef struct eventMsg {
    eventID_t id;
    union {
        InputEvent input; // --> applications/input/input.h
        wiiEcEvent_t wiiEc; // --> local
    };
} eventMsg_t;

//----------------------------------------------------------------------------- ----------------------------------------
// State variables for this plugin
// An instance of this is allocated on the heap, and the pointer is passed back to the OS
// Access to this memory is controlled by mutex
//
typedef struct state {
    bool run; // true : plugin is running

    bool timerEn; // controller scanning enabled
    FuriTimer* timer; // the timer
    uint32_t timerHz; // system ticks per second
    int fps; // poll/refresh [frames]-per-second

    int cnvW; // canvas width
    int cnvH; // canvas height
    scene_t scene; // current scene
    scene_t scenePrev; // previous scene
    scene_t scenePegg; // previous scene for easter eggs
    int flash; // flash counter (flashing icons)

    int hold; // hold type: {-1=tough-peak, 0=none, +1=peak-hold}
    ecCalib_t calib; // Software calibration mode

    bool pause; // Accelerometer animation pause
    bool apause; // Accelerometer animation auto-pause

    NotificationApp* notify; // OS nitifcation queue (for patting the backlight watchdog timer)

    wiiEC_t ec; // Extension Controller details
} state_t;

//============================================================================= ========================================
// Function prototypes
//
void timerEn(state_t* state, bool on);

#endif //WII_ANAL_H_
