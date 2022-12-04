#ifndef WII_EC_NUNCHUCK_H_
#define WII_EC_NUNCHUCK_H_

#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// Controllers which have calibration must have their calibratable controls here
//! Is there a better way to get the start of the decode struct to match the calibration struct ?
#define NUNCHUCK_ANALOGUE \
    uint8_t joyX, joyY;   \
    uint16_t accX, accY, accZ;

//-----------------------------------------------------------------------------
// Calibratable controls
//
typedef struct ecCalNunchuck {
    NUNCHUCK_ANALOGUE
} ecCalNunchuck_t;

//-----------------------------------------------------------------------------
// All controls
//
typedef struct ecDecNunchuck {
    NUNCHUCK_ANALOGUE // MUST be first

        // Digital controls
        bool btnC,
        btnZ; // BTN{c, z}
} ecDecNunchuck_t;

#undef NUNCHUCK_ANALOGUE

//=============================================================================
// Function prototypes
//
#include <gui/gui.h> // Canvas
typedef struct wiiEC wiiEC_t;
typedef enum ecCalib ecCalib_t;
typedef struct state state_t;
typedef struct eventMsg eventMsg_t;

void nunchuck_decode(wiiEC_t* const pec);
void nunchuck_msg(wiiEC_t* const pec, FuriMessageQueue* const queue);
void nunchuck_calib(wiiEC_t* const pec, ecCalib_t c);

void nunchuck_show(Canvas* const canvas, state_t* const state);
bool nunchuck_key(const eventMsg_t* const msg, state_t* const state);

#endif //WII_EC_NUNCHUCK_H_
