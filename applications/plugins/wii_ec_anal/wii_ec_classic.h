#ifndef WII_EC_CLASSIC_H_
#define WII_EC_CLASSIC_H_

#include <stdint.h>
#include <stdbool.h>

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
//

//----------------------------------------------------------------------------- ----------------------------------------
// Controllers which have calibration must have their calibratable controls here
//! Is there a better way to get the start of the decode struct to match the calibration struct ?
#define CLASSIC_ANALOGUE                                                        \
    uint8_t trgZL, trgZR; /* ANA{l, l} lowercase=trigger  5bit values {5}    */ \
    uint8_t joyLX, joyLY; /* ANA{x, y} left=lowercase     6bit values {6}<-- */ \
    uint8_t joyRX, joyRY; /* ANA{X, Y}                    5bit values {5}    */

//-----------------------------------------------------------------------------
// Calibratable controls
//
typedef struct ecCalClassic {
    CLASSIC_ANALOGUE
} ecCalClassic_t;

//-----------------------------------------------------------------------------
// All controls
//
typedef struct ecDecClassic {
    CLASSIC_ANALOGUE // MUST be first

        // Digital controls
        bool btnZL,
        btnZR; // BTN{l, l}

    bool btnL, btnR; // BTN{L, R} upperrcase=shoulder

    bool padU, padL, padD, padR; // BTN{W, A, S, D}

    bool btnM, btnH, btnP; // BTN{-, h, +}

    bool btnX, btnY; // BTN{x, y}
    bool btnA, btnB; // BTN{a, b}

} ecDecClassic_t;

#undef CLASSIC_ANALOGUE

//============================================================================= ========================================
// Function prototypes
//
#include <gui/gui.h> // Canvas
typedef struct wiiEC wiiEC_t;
typedef enum ecCalib ecCalib_t;
typedef struct state state_t;
typedef struct eventMsg eventMsg_t;

void classic_decode(wiiEC_t* const pec);
void classic_msg(wiiEC_t* const pec, FuriMessageQueue* const queue);
void classic_calib(wiiEC_t* const pec, ecCalib_t c);

void classic_show(Canvas* const canvas, state_t* const state);
bool classic_key(const eventMsg_t* const msg, state_t* const state);

#endif //WII_EC_CLASSIC_H_
