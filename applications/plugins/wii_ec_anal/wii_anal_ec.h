#ifndef WII_ANAL_EC_H_
#define WII_ANAL_EC_H_

#include <stdbool.h>

//============================================================================= ========================================
// Function prototypes
//
typedef struct eventMsg eventMsg_t;
typedef struct state state_t;

bool evWiiEC(const eventMsg_t* const msg, state_t* const state);

#endif //WII_ANAL_EC_H_
