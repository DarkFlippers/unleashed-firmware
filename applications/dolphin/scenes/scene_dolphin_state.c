#include <furi.h>
#include "scene.h"
#include "assets/items.h"

static void scene_proceed_action(SceneState* state) {
    furi_assert(state);
    state->prev_action = state->action;
    state->action = roll_new(state->prev_action, ACTIONS_NUM);
    state->action_timeout = default_timeout[state->action];
}

static void scene_action_handler(SceneState* state) {
    furi_assert(state);
    if(state->action == MINDCONTROL) {
        if(state->player_v.x != 0 || state->player_v.y != 0) {
            state->action_timeout = default_timeout[state->action];
        }
    }

    if(state->action_timeout > 0) {
        state->action_timeout--;
    }
}

void dolphin_scene_update_state(SceneState* state, uint32_t t, uint32_t dt) {
    furi_assert(state);
    scene_action_handler(state);

    switch(state->action) {
    case INTERACT:
        if(state->action_timeout == 0) {
            if(state->prev_action == MINDCONTROL) {
                state->action = MINDCONTROL;
            } else {
                scene_proceed_action(state);
            }
        }
        break;

    default:

        if(state->action_timeout == 0) {
            scene_proceed_action(state);
        }
        break;
    }
}
