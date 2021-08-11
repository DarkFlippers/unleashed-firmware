#include <gui/elements.h>
#include "applications.h"
#include "items_i.h"
#include "emotes.h"
#include <gui/icon_i.h>

const Item Food = {
    .layer = 4,
    .timeout = 100,
    .pos =
        {
            .x = 0,
            .y = 90,
        },
    .width = 60,
    .height = 50,
    .draw = food_redraw,
    .callback = food_callback};

const Item Console = {
    .layer = 4,
    .timeout = 100,
    .pos =
        {
            .x = 357,
            .y = 190,
        },
    .width = 40,
    .height = 20,
    .draw = console_redraw,
    .callback = console_callback};

const Item* Home[] = {&Food, &Console};

const Item** Scenes[] = {Home};

const Item** get_scene(SceneState* state) {
    return Scenes[state->scene_id];
}

static void dolphin_scene_start_app(SceneState* state, const FlipperApplication* flipper_app) {
    furi_assert(state);
    furi_assert(flipper_app);

    state->scene_app_thread = furi_thread_alloc();

    furi_assert(flipper_app->app);
    furi_assert(flipper_app->name);

    furi_thread_set_name(state->scene_app_thread, flipper_app->name);
    furi_thread_set_stack_size(state->scene_app_thread, flipper_app->stack_size);
    furi_thread_set_callback(state->scene_app_thread, flipper_app->app);
    furi_thread_start(state->scene_app_thread);
}

uint16_t roll_new(uint16_t prev, uint16_t max) {
    uint16_t val = 999;
    while(val != prev) {
        val = random() % max;
        break;
    }
    return val;
}

static void dolphin_scene_type_text(
    Canvas* canvas,
    SceneState* state,
    uint8_t x,
    uint8_t y,
    const char* text) {
    char dialog_str[64];
    char buf[64];

    strcpy(dialog_str, (char*)text);

    if(state->dialog_progress <= strlen(dialog_str)) {
        if(HAL_GetTick() / 10 % 2 == 0) state->dialog_progress++;
        dialog_str[state->dialog_progress] = '\0';
        snprintf(buf, state->dialog_progress, dialog_str);
    } else {
        snprintf(buf, 64, dialog_str);
    }

    canvas_draw_str_aligned(canvas, x, y, AlignCenter, AlignCenter, buf);
}

const void scene_activate_item_callback(SceneState* state, Canvas* canvas) {
    furi_assert(state);
    furi_assert(canvas);

    const Item* near = is_nearby(state);
    if(near && state->use_pending == true) {
        state->action_timeout = near->timeout;
        near->callback(canvas, state);
        state->use_pending = false;
    } else if(near) {
        near->callback(canvas, state);
    }
}

const Vec2 item_get_pos(SceneState* state, ItemsEnum item) {
    const Item** current = get_scene(state);
    Vec2 rel_pos = {0, 0};

    rel_pos.x = DOLPHIN_WIDTH / 2 + (current[item]->pos.x * PARALLAX(current[item]->layer));
    rel_pos.y = DOLPHIN_WIDTH / 4 + (current[item]->pos.y * PARALLAX(current[item]->layer));

    return rel_pos;
}

const Item* is_nearby(SceneState* state) {
    furi_assert(state);
    uint8_t item = 0;
    bool found = false;
    const Item** current = get_scene(state);
    while(item < ItemsEnumTotal) {
        int32_t rel_x =
            (DOLPHIN_CENTER + DOLPHIN_WIDTH / 2 -
             (current[item]->pos.x - state->player_global.x) * PARALLAX(current[item]->layer));

        uint8_t item_height = current[item]->height;
        uint8_t item_width = current[item]->width;

        int32_t rel_y = current[item]->pos.y - state->player_global.y;

        if(abs(rel_x) <= item_width && abs(rel_y) <= item_height) {
            found = !found;
            break;
        }
        ++item;
    }
    return found ? current[item] : NULL;
}

void food_redraw(Canvas* canvas, void* s) {
    furi_assert(s);
    SceneState* state = s;

    const Icon* food_frames[] = {
        &I_food1_61x98,
        &I_food2_61x98,
        &I_food3_61x98,
        &I_food4_61x98,
        &I_food5_61x98,
        &I_food6_61x98,
        &I_food7_61x98,
        &I_food8_61x98,
        &I_food9_61x98,
        &I_food10_61x98,
        &I_food11_61x98,
        &I_food12_61x98,
    };

    uint8_t frame = ((HAL_GetTick() / 200) % SIZEOF_ARRAY(food_frames));

    if(is_nearby(state) && (state->player_global.y > Food.pos.y)) {
        dolphin_scene_type_text(
            canvas,
            state,
            (Food.pos.x - state->player_global.x) * PARALLAX(Food.layer) + 90,
            state->screen.y + 8,
            console_emotes[state->emote_id]);

    } else {
        state->dialog_progress = 0;
        state->emote_id = roll_new(state->previous_emote, SIZEOF_ARRAY(console_emotes));
    }

    canvas_draw_icon(
        canvas,
        (Food.pos.x - state->player_global.x) * PARALLAX(Food.layer),
        Food.pos.y - state->player_global.y,
        food_frames[frame]);

    canvas_set_bitmap_mode(canvas, true);
}

void food_callback(Canvas* canvas, void* s) {
    furi_assert(s);
    SceneState* state = s;
    if(state->use_pending) {
        dolphin_scene_start_app(state, &FLIPPER_SCENE_APPS[1]);
    }
}

void console_redraw(Canvas* canvas, void* s) {
    furi_assert(s);
    SceneState* state = s;

    const Icon* console[] = {
        &I_Console_74x67_0,
        &I_Console_74x67_1,
        &I_Console_74x67_2,
        &I_Console_74x67_3,
        &I_Console_74x67_4,
        &I_Console_74x67_5,
        &I_Console_74x67_6,
        &I_Console_74x67_7,
        &I_Console_74x67_8,

    };

    uint8_t frame = ((HAL_GetTick() / 100) % SIZEOF_ARRAY(console));

    canvas_draw_icon(
        canvas,
        (Console.pos.x - state->player_global.x) * PARALLAX(Console.layer),
        Console.pos.y - state->player_global.y,
        console[frame]);

    canvas_set_bitmap_mode(canvas, true);

    if(is_nearby(state)) {
        dolphin_scene_type_text(
            canvas,
            state,
            (Console.pos.x - state->player_global.x) * PARALLAX(Console.layer) - 25,
            Console.pos.y - state->player_global.y + 14,
            console_emotes[state->emote_id]);

    } else {
        state->dialog_progress = 0;
        state->emote_id = roll_new(state->previous_emote, SIZEOF_ARRAY(console_emotes));
    }
}

void console_callback(Canvas* canvas, void* s) {
    furi_assert(s);
    SceneState* state = s;
    if(state->use_pending) {
        dolphin_scene_start_app(state, &FLIPPER_SCENE_APPS[0]);
    }
}