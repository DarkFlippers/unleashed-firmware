#ifndef WII_ANAL_KEYS_H_
#define WII_ANAL_KEYS_H_

//============================================================================= ========================================
// Function prototypes
//
#include <stdbool.h> // bool
typedef struct eventMsg eventMsg_t;
typedef struct state state_t;
typedef enum scene scene_t;

void sceneSet(state_t* const state, const scene_t scene);
bool key_calib(const eventMsg_t* const msg, state_t* const state);
bool evKey(const eventMsg_t* const msg, state_t* const state);

#endif //WII_ANAL_KEYS_H_
