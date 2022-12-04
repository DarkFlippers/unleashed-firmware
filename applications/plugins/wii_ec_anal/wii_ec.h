#ifndef WII_EC_H_
#define WII_EC_H_

#include <stdbool.h>

#include <furi.h>

#include "wii_ec_nunchuck.h"
#include "wii_ec_classic.h"
#include "wii_ec_udraw.h"

//----------------------------------------------------------------------------- ----------------------------------------
// Crypto key (PSK),      base register : {0x40..0x4F}[2][8]
#define ENC_LEN (2 * 8)

// Controller State data, base register : {0x00..0x05}[6]
#define JOY_LEN (6)

// Calibration data,      base register : {0x20..0x2F}[16]
#define CAL_LEN (16)

// Controller ID,         base register : {0xFA..0xFF}[6]
#define PID_LEN (6)

//----------------------------------------------------------------------------- ----------------------------------------
// Perhipheral specific parameters union
//
typedef union ecDec {
    ecDecNunchuck_t nunchuck;
    ecDecClassic_t classic;
} ecDec_t;

//-----------------------------------------------------------------------------
typedef union ecCal {
    // 0=lowest seen ; 1=min ; 2=mid ; 3=max ; 4=highest seen
    ecCalNunchuck_t nunchuck[5];
    ecCalClassic_t classic[5];
} ecCal_t;

//----------------------------------------------------------------------------- ----------------------------------------
// Wii Extension Controller events
//
typedef enum wiiEcEventType {
    WIIEC_NONE,
    WIIEC_CONN, // Connect
    WIIEC_DISCONN, // Disconnect
    WIIEC_PRESS, // Press button
    WIIEC_RELEASE, // Release button
    WIIEC_ANALOG, // Analogue change (Joystick/Trigger)
    WIIEC_ACCEL, // Accelerometer change
} wiiEcEventType_t;

//-----------------------------------------------------------------------------
typedef struct wiiEcEvent {
    wiiEcEventType_t type; // event type
    char in; // input (see device specific options)
    uint32_t val; // new value - meaningless for digital button presses
} wiiEcEvent_t;

//----------------------------------------------------------------------------- ----------------------------------------
// Known perhipheral types
//
typedef enum ecPid {
    PID_UNKNOWN = 0,
    PID_FIRST = 1,
    PID_NUNCHUCK = PID_FIRST,

    // If you're wise, ONLY edit this section
    PID_NUNCHUCK_R2,
    PID_CLASSIC,
    PID_CLASSIC_PRO,
    PID_BALANCE,
    PID_GH_GUITAR,
    PID_GH_DRUMS,
    PID_TURNTABLE,
    PID_TAIKO_DRUMS,
    PID_UDRAW, //! same as drawsome?
    // -----

    PID_ERROR,
    PID_NULL,
    PID_CNT,
} ecPid_t;

//-----------------------------------------------------------------------------
// Calibration strategies
//
typedef enum ecCalib {
    CAL_FACTORY = 0x01, // (re)set to factory defaults
    CAL_TRACK = 0x02, // track maximum and minimum values seen
    CAL_RESET = 0x04, // initialise ready for software calibration
    CAL_RANGE = 0x08, // perform software calibration step
    CAL_CENTRE = 0x10, // reset centre point of joystick
    CAL_NOTJOY = 0x20, // do NOT calibrate the joystick
} ecCalib_t;

//-----------------------------------------------------------------------------
// ecId table entry
//
typedef struct ecId {
    uint8_t id[6]; // 6 byte ID string returned by Extension Controller
    char* name; // Friendly name
    scene_t scene; // Default scene
    bool (*init)(wiiEC_t*); // Additional initialisation code
    void (*decode)(wiiEC_t*); // Decode function
    void (*check)(wiiEC_t*, FuriMessageQueue*); // check (for action) function
    void (*calib)(wiiEC_t*, ecCalib_t); // calibrate analogue controllers [SOFTWARE]
    void (*show)(Canvas* const, state_t* const); // Draw scene
    bool (*keys)(const eventMsg_t* const, state_t* const); // Interpret keys
} ecId_t;

//-----------------------------------------------------------------------------
// List of known perhipherals
//
// More perhipheral ID codes here:  https://wiibrew.org/wiki/Wiimote/Extension_Controllers#The_New_Way
//
extern const ecId_t ecId[PID_CNT];

//----------------------------------------------------------------------------- ----------------------------------------
// Data pertaining to a single Perhipheral instance
//
typedef struct wiiEC {
    // Perhipheral state
    bool init; // Initialised?

    uint8_t pid[PID_LEN]; // PID string - eg. {0x00, 0x00, 0xA4, 0x20, 0x00, 0x00}
    ecPid_t pidx; // Index in to ecId table
    const char* sid; // just for convenience

    bool encrypt; // encryption enabled?
    uint8_t encKey[ENC_LEN]; // encryption key

    uint8_t calF[CAL_LEN]; // factory calibration data (not software)
    uint8_t joy[JOY_LEN]; // Perhipheral raw data

    ecDec_t dec[2]; // device specific decode (two, so we can spot changes)
    int decN; // which decode set is most recent {0, 1}
    ecCal_t calS; // software calibration data
} wiiEC_t;

//----------------------------------------------------------------------------- ----------------------------------------
// Function prototypes
//
// top level calls will work out which sub-function to call
// top level check() function will handle connect/disconnect messages
//

#include <gui/gui.h> // Canvas
typedef struct wiiEC wiiEC_t;
typedef enum ecCalib ecCalib_t;
typedef struct state state_t;
typedef struct eventMsg eventMsg_t;

void ecDecode(wiiEC_t* const pec);
void ecPoll(wiiEC_t* const pec, FuriMessageQueue* const queue);
void ecCalibrate(wiiEC_t* const pec, ecCalib_t c);

void ec_show(Canvas* const canvas, state_t* const state);
bool ec_key(const eventMsg_t* const msg, state_t* const state);

#endif //WII_EC_H_
