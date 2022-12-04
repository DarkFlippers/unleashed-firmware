#ifndef WII_EC_UDRAW_H_
#define WII_EC_UDRAW_H_

#include <stdint.h>
#include <stdbool.h>

//============================================================================= =======================================
// Function prototypes
//
typedef struct wiiEC wiiEC_t;
typedef enum ecCalib ecCalib_t;
typedef struct eventMsg eventMsg_t;
typedef struct state state_t;

bool udraw_init(wiiEC_t* const pec);
bool udraw_key(const eventMsg_t* const msg, state_t* const state);

#endif //WII_EC_UDRAW_H_
