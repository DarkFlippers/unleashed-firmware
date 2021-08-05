#include <furi.h>
#include "scene.h"
#include "assets/items.h"
#include "assets/meta.h"
#include <gui/elements.h>

void dolphin_scene_transition_handler(SceneState* state) {
    uint8_t speed_mod = (state->player_v.x || state->player_v.y || state->transition) ? 6 : 10;

    if(state->player_v.x < 0) {
        state->frame_pending = DirLeft;
    } else if(state->player_v.x > 0) {
        state->frame_pending = DirRight;
    } else if(state->player_v.y < 0) {
        state->frame_pending = DirUp;
    } else if(state->player_v.y > 0) {
        state->frame_pending = DirDown;
    }
    state->transition_pending = state->frame_group != state->frame_pending;

    if(*&frames[state->frame_group][state->frame_type]->frames[state->frame_idx].f) {
        state->current_frame = *&frames[state->frame_group][state->frame_type];
    }

    uint8_t total = state->current_frame->frames[2].f == NULL ? 2 : 3;

    if(state->transition_pending && !state->frame_idx) {
        state->transition_pending = false;
        state->transition = true;
    }

    if(state->transition) {
        state->frame_type = state->frame_pending;
        state->frame_group = state->last_group;
        state->transition = !(state->frame_idx == total - 1);
    } else {
        state->frame_group = state->frame_type;
    }

    state->player_anim++;

    if(!(state->player_anim % speed_mod)) {
        state->frame_idx = (state->frame_idx + 1) % total;
    }
}

void dolphin_scene_render_dolphin(SceneState* state, Canvas* canvas) {
    furi_assert(state);
    furi_assert(canvas);

    dolphin_scene_transition_handler(state);

    canvas_set_bitmap_mode(canvas, true);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_icon(
        canvas, state->player.x, state->player.y, state->current_frame->frames[state->frame_idx].b);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon(
        canvas, state->player.x, state->player.y, state->current_frame->frames[state->frame_idx].f);
    canvas_set_bitmap_mode(canvas, false);
}

static bool item_screen_bounds_x(int32_t pos) {
    return pos > -SCREEN_WIDTH && pos < (SCREEN_WIDTH * 2);
}
static bool item_screen_bounds_y(int32_t pos) {
    return pos > -SCREEN_HEIGHT * 2 && pos < (SCREEN_HEIGHT * 2);
}

void dolphin_scene_render(SceneState* state, Canvas* canvas, uint32_t t) {
    furi_assert(state);
    furi_assert(canvas);

    canvas_set_font(canvas, FontSecondary);
    canvas_set_color(canvas, ColorBlack);
    const Item** current_scene = get_scene(state);

    for(uint8_t l = 0; l < LAYERS; l++) {
        for(uint8_t i = 0; i < ItemsEnumTotal; i++) {
            int32_t item_pos_X = (current_scene[i]->pos.x - state->player_global.x);
            int32_t item_pos_Y = (current_scene[i]->pos.y - state->player_global.y);

            if(item_screen_bounds_x(item_pos_X) && item_screen_bounds_y(item_pos_Y)) {
                if(l == current_scene[i]->layer) {
                    if(current_scene[i]->draw) {
                        current_scene[i]->draw(canvas, state);
                    }
                }
            }
        }

        if(l == DOLPHIN_LAYER) dolphin_scene_render_dolphin(state, canvas);
    }
}

void dolphin_scene_render_state(SceneState* state, Canvas* canvas) {
    furi_assert(state);
    furi_assert(canvas);

    char buf[64];

    canvas_set_font(canvas, FontSecondary);
    canvas_set_color(canvas, ColorBlack);

    // dolphin_scene_debug
    if(state->debug) {
        sprintf(
            buf,
            "%d:%d %d/%dP%dL%d T%d-%d",
            state->frame_idx,
            state->current_frame->frames[2].f == NULL ? 2 : 3,
            state->frame_group,
            state->frame_type,
            state->frame_pending,
            state->last_group,
            state->transition_pending,
            state->transition);
        canvas_draw_str(canvas, 0, 13, buf);
    }
    if(state->action == INTERACT) scene_activate_item_callback(state, canvas);
}