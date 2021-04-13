#include <furi.h>
#include <gui/elements.h>
#include "dolphin_scene/dolphin_scene.h"

void dolphin_scene_render_dolphin(SceneState* state, Canvas* canvas) {
    furi_assert(state);
    furi_assert(canvas);

    if(state->scene_zoom == SCENE_ZOOM) {
        state->dolphin_gfx = I_DolphinExcited_64x63;
    } else if(state->action == SLEEP && state->player_global.x == 154) {
        state->dolphin_gfx = A_FX_Sitting_40x27;
        state->dolphin_gfx_b = I_FX_SittingB_40x27;
    } else if(state->action != INTERACT) {
        if(state->player_v.x < 0 || state->player_flipped) {
            if(state->player_anim == 0) {
                state->dolphin_gfx = I_WalkL1_32x32;
                state->dolphin_gfx_b = I_WalkLB1_32x32;

            } else {
                state->dolphin_gfx = I_WalkL2_32x32;
                state->dolphin_gfx_b = I_WalkLB2_32x32;
            }
        } else if(state->player_v.x > 0 || !state->player_flipped) {
            if(state->player_anim == 0) {
                state->dolphin_gfx = I_WalkR1_32x32;
                state->dolphin_gfx_b = I_WalkRB1_32x32;

            } else {
                state->dolphin_gfx = I_WalkR2_32x32;
                state->dolphin_gfx_b = I_WalkRB2_32x32;
            }
        }
    }

    // zoom handlers
    canvas_set_bitmap_mode(canvas, true);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_icon_name(canvas, state->player.x, state->player.y, state->dolphin_gfx_b);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon_name(canvas, state->player.x, state->player.y, state->dolphin_gfx);
    canvas_set_bitmap_mode(canvas, false);
}

void dolphin_scene_handle_user_input(SceneState* state, InputEvent* input) {
    furi_assert(state);
    furi_assert(input);

    // dolphin_scene_debug
    if(input->type == InputTypeShort) {
        if(input->key == InputKeyUp) {
            state->debug = !state->debug;
        }
    }
    // toggle mind control on any user interaction
    if(input->type == InputTypePress) {
        if(input->key == InputKeyLeft || input->key == InputKeyRight || input->key == InputKeyOk) {
            state->action = MINDCONTROL;
        }
    }
    // zoom poc for tests
    if(input->type == InputTypePress) {
        if(input->key == InputKeyDown) {
            state->zoom_v = SPEED_X;
        }
    } else if(input->type == InputTypeRelease) {
        if(input->key == InputKeyDown) {
            state->zoom_v = -SPEED_X * 2;
            state->dialog_progress = 0;
        }
    }
    // mind control
    if(state->action == MINDCONTROL) {
        if(input->type == InputTypePress) {
            if(input->key == InputKeyRight) {
                state->player_flipped = false;
                state->player_v.x = SPEED_X;
            } else if(input->key == InputKeyLeft) {
                state->player_flipped = true;
                state->player_v.x = -SPEED_X;
            }
        } else if(input->type == InputTypeRelease) {
            if(input->key == InputKeyRight || input->key == InputKeyLeft) {
                state->player_v.x = 0;
            }
        } else if(input->type == InputTypeShort) {
            if(input->key == InputKeyOk) {
                state->prev_action = MINDCONTROL;
                state->action = INTERACT;
                state->use_pending = true;
                state->action_timeout = 0;
            }
        }
    }
}

void dolphin_scene_coordinates(SceneState* state, uint32_t dt) {
    furi_assert(state);

    // global pos
    state->player_global.x = CLAMP(state->player_global.x + state->player_v.x, WORLD_WIDTH, 0);

    // zoom handlers
    state->scene_zoom = CLAMP(state->scene_zoom + state->zoom_v, SCENE_ZOOM, 0);
    state->player.x = CLAMP(state->player.x - (state->zoom_v * (SPEED_X * 2)), DOLPHIN_CENTER, 0);
    state->player.y = CLAMP(state->player.y - (state->zoom_v * SPEED_X / 2), DOLPHIN_DEFAULT_Y, 3);

    //center screen
    state->screen.x = state->player_global.x - state->player.x;
    state->player_anim = (state->player_global.x / 10) % 2;
}