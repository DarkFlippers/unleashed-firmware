#include <furi.h>
#include "scene.h"
#include "assets/emotes.h"
#include "assets/items.h"
#include <gui/elements.h>

const char* action_str[] = {"Sleep", "Idle", "Walk", "Emote", "Use", "MC"};

static void scene_draw_hint(SceneState* state, Canvas* canvas, bool glitching) {
    furi_assert(state);
    furi_assert(canvas);
    char buf[32];

    const Item* near = is_nearby(state);
    if(near) {
        int32_t hint_pos_x = (near->x - state->player_global.x) * PARALLAX(near->layer) + 25;
        int8_t hint_pos_y = near->y < 15 ? near->y + 4 : near->y - 16;

        strcpy(buf, near->action_name);
        if(glitching) {
            for(size_t g = 0; g != state->action_timeout; g++) {
                buf[(g * 23) % strlen(buf)] = ' ' + (random() % g * 17) % ('z' - ' ');
            }
        }

        canvas_draw_str(canvas, hint_pos_x, hint_pos_y, buf);
    }
}

static void scene_draw_current_emote(SceneState* state, Canvas* canvas) {
    furi_assert(state);
    furi_assert(canvas);
    elements_multiline_text_framed(canvas, 80, 20, (char*)emotes_list[state->emote_id]);
}

static void scene_draw_sleep_emote(SceneState* state, Canvas* canvas) {
    furi_assert(state);
    furi_assert(canvas);

    char dialog_str[] = "zZzZ...";
    char buf[64];
    // 2do - sofa x pos getter
    if(state->player_global.x == 154 && state->action_timeout % 100 < 30) {
        if(state->dialog_progress < strlen(dialog_str)) {
            if(state->action_timeout % 5 == 0) state->dialog_progress++;
            dialog_str[state->dialog_progress] = '\0';
            snprintf(buf, state->dialog_progress, dialog_str);
            // bubble vs just text?
            //elements_multiline_text_framed(canvas, 80, 20, buf);
            canvas_draw_str(canvas, 80, 20, buf);
        }

    } else {
        state->dialog_progress = 0;
    }
}

static void scene_draw_dialog(SceneState* state, Canvas* canvas) {
    furi_assert(state);
    furi_assert(canvas);

    char dialog_str[64];
    char buf[64];

    strcpy(dialog_str, (char*)dialogues_list[state->dialogue_id]);

    if(state->dialog_progress <= strlen(dialog_str)) {
        if(state->action_timeout % 2 == 0) state->dialog_progress++;
        dialog_str[state->dialog_progress] = '\0';
        snprintf(buf, state->dialog_progress, dialog_str);
    } else {
        snprintf(buf, 64, dialog_str);
    }

    elements_multiline_text_framed(canvas, 68, 16, buf);
}

/*
static void draw_idle_emote(SceneState* state, Canvas* canvas){
    if(state->action_timeout % 50 < 40 && state->prev_action == MINDCONTROL){
        elements_multiline_text_framed(canvas, 68, 16, "WUT?!");
    }
}
*/

void dolphin_scene_render_dolphin(SceneState* state, Canvas* canvas) {
    furi_assert(state);
    furi_assert(canvas);

    if(state->scene_zoom == SCENE_ZOOM) {
        state->dolphin_gfx = I_DolphinExcited_64x63;
    } else if(state->action == SLEEP && state->player_global.x == 154) { // 2do - sofa x pos getter
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

    canvas_set_bitmap_mode(canvas, true);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_icon_name(canvas, state->player.x, state->player.y, state->dolphin_gfx_b);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon_name(canvas, state->player.x, state->player.y, state->dolphin_gfx);
    canvas_set_bitmap_mode(canvas, false);
}

static bool item_screen_bounds(int32_t pos) {
    return pos > -SCREEN_WIDTH && pos < (SCREEN_WIDTH * 2);
}

void dolphin_scene_render(SceneState* state, Canvas* canvas, uint32_t t) {
    furi_assert(state);
    furi_assert(canvas);

    canvas_set_font(canvas, FontSecondary);
    canvas_set_color(canvas, ColorBlack);
    const Item** current_scene = get_scene(state);

    for(uint8_t l = 0; l < LAYERS; l++) {
        if(state->scene_zoom < SCENE_ZOOM) {
            for(uint8_t i = 0; i < ITEMS_NUM; i++) {
                int32_t item_pos = (current_scene[i]->x - state->player_global.x);
                if(item_screen_bounds(item_pos)) {
                    if(current_scene[i]->draw) current_scene[i]->draw(canvas, state);

                    if(l == current_scene[i]->layer) {
                        canvas_draw_icon_name(
                            canvas,
                            item_pos * PARALLAX(l),
                            current_scene[i]->y,
                            current_scene[i]->icon);
                        canvas_set_bitmap_mode(canvas, false);
                    }
                }
            }

            if(l == 0) canvas_draw_line(canvas, 0, 42, 128, 42);
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
            "x:%ld>%d %ld %s",
            state->player_global.x,
            state->poi,
            state->action_timeout,
            action_str[state->action]);
        canvas_draw_str(canvas, 0, 13, buf);
    }

    if(state->scene_zoom == SCENE_ZOOM)
        scene_draw_dialog(state, canvas);
    else if(state->action == EMOTE)
        scene_draw_current_emote(state, canvas);
    else if(state->action == MINDCONTROL)
        scene_draw_hint(state, canvas, state->action_timeout > 45);
    else if(state->action == INTERACT)
        scene_activate_item_callback(state, canvas);
    else if(state->action == SLEEP)
        scene_draw_sleep_emote(state, canvas);
    /*
    else if(state->action == IDLE)
        draw_idle_emote(state, canvas);
    */
}