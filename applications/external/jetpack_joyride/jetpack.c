#include <stdlib.h>

#include <jetpack_joyride_icons.h>
#include <furi.h>
#include <gui/gui.h>
#include <gui/icon_animation.h>
#include <input/input.h>
#include <storage/storage.h>

#include "includes/point.h"
#include "includes/barry.h"
#include "includes/scientist.h"
#include "includes/particle.h"
#include "includes/coin.h"
#include "includes/missile.h"
#include "includes/background_assets.h"

#include "includes/game_state.h"

#define TAG "Jetpack Joyride"
#define SAVING_DIRECTORY "/ext/apps/Games"
#define SAVING_FILENAME SAVING_DIRECTORY "/jetpack.save"
static GameState* global_state;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} GameEvent;

typedef struct {
    int max_distance;
    int total_coins;
} SaveGame;

static SaveGame save_game;

static bool storage_game_state_load() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    uint16_t bytes_readed = 0;
    if(storage_file_open(file, SAVING_FILENAME, FSAM_READ, FSOM_OPEN_EXISTING))
        bytes_readed = storage_file_read(file, &save_game, sizeof(SaveGame));
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return bytes_readed == sizeof(SaveGame);
}

static void storage_game_state_save() {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    if(storage_common_stat(storage, SAVING_DIRECTORY, NULL) == FSE_NOT_EXIST) {
        if(!storage_simply_mkdir(storage, SAVING_DIRECTORY)) {
            return;
        }
    }

    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, SAVING_FILENAME, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        storage_file_write(file, &save_game, sizeof(SaveGame));
    }
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

void handle_death() {
    global_state->state = GameStateGameOver;
    global_state->new_highscore = global_state->distance > save_game.max_distance;

    if(global_state->distance > save_game.max_distance) {
        save_game.max_distance = global_state->distance;
    }

    save_game.total_coins += global_state->total_coins;

    storage_game_state_save();
}

static void jetpack_game_state_init(GameState* const game_state) {
    UNUSED(game_state);
    UNUSED(storage_game_state_save);
    BARRY barry;
    barry.gravity = 0;
    barry.point.x = 32 + 5;
    barry.point.y = 32;
    barry.isBoosting = false;

    GameSprites sprites;
    sprites.barry = icon_animation_alloc(&A_barry);
    sprites.barry_infill = &I_barry_infill;

    sprites.scientist_left = (&I_scientist_left);
    sprites.scientist_left_infill = (&I_scientist_left_infill);
    sprites.scientist_right = (&I_scientist_right);
    sprites.scientist_right_infill = (&I_scientist_right_infill);

    sprites.coin = (&I_coin);
    sprites.coin_infill = (&I_coin_infill);

    sprites.missile = icon_animation_alloc(&A_missile);
    sprites.missile_infill = &I_missile_infill;

    sprites.alert = icon_animation_alloc(&A_alert);

    icon_animation_start(sprites.barry);
    icon_animation_start(sprites.missile);
    icon_animation_start(sprites.alert);

    game_state->barry = barry;
    game_state->total_coins = 0;
    game_state->distance = 0;
    game_state->new_highscore = false;
    game_state->sprites = sprites;
    game_state->state = GameStateLife;
    game_state->death_handler = handle_death;

    memset(game_state->bg_assets, 0, sizeof(game_state->bg_assets));

    memset(game_state->scientists, 0, sizeof(game_state->scientists));
    memset(game_state->coins, 0, sizeof(game_state->coins));
    memset(game_state->particles, 0, sizeof(game_state->particles));
    memset(game_state->missiles, 0, sizeof(game_state->missiles));
}

static void jetpack_game_state_free(GameState* const game_state) {
    icon_animation_free(game_state->sprites.barry);
    icon_animation_free(game_state->sprites.missile);
    icon_animation_free(game_state->sprites.alert);

    free(game_state);
}

static void jetpack_game_tick(GameState* const game_state) {
    if(game_state->state == GameStateGameOver) return;
    barry_tick(&game_state->barry);
    game_state_tick(game_state);
    coin_tick(game_state->coins, &game_state->barry, &game_state->total_coins);
    particle_tick(game_state->particles, game_state->scientists);
    scientist_tick(game_state->scientists);
    missile_tick(game_state->missiles, &game_state->barry, game_state->death_handler);

    background_assets_tick(game_state->bg_assets);

    // generate background every 64px aka. ticks
    if(game_state->distance % 64 == 0 && rand() % 3 == 0) {
        spawn_random_background_asset(game_state->bg_assets);
    }

    if(game_state->distance % 48 == 0 && rand() % 2 == 0) {
        spawn_random_coin(game_state->coins);
    }

    if(game_state->distance % get_rocket_spawn_distance(game_state->distance) == 0 &&
       rand() % 2 == 0) {
        spawn_random_missile(game_state->missiles);
    }

    spawn_random_scientist(game_state->scientists);

    if(game_state->barry.isBoosting) {
        spawn_random_particles(game_state->particles, &game_state->barry);
    }
}

static void jetpack_game_render_callback(Canvas* const canvas, void* ctx) {
    furi_assert(ctx);
    const GameState* game_state = ctx;
    furi_mutex_acquire(game_state->mutex, FuriWaitForever);

    if(game_state->state == GameStateLife) {
        canvas_set_bitmap_mode(canvas, false);

        draw_background_assets(game_state->bg_assets, canvas, game_state->distance);

        canvas_set_bitmap_mode(canvas, true);

        draw_coins(game_state->coins, canvas, &game_state->sprites);
        draw_scientists(game_state->scientists, canvas, &game_state->sprites);
        draw_particles(game_state->particles, canvas);
        draw_missiles(game_state->missiles, canvas, &game_state->sprites);

        draw_barry(&game_state->barry, canvas, &game_state->sprites);

        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontSecondary);
        char buffer[12];
        snprintf(buffer, sizeof(buffer), "%u m", game_state->distance / 10);
        canvas_draw_str_aligned(canvas, 123, 15, AlignRight, AlignBottom, buffer);

        snprintf(buffer, sizeof(buffer), "$%u", game_state->total_coins);
        canvas_draw_str_aligned(canvas, 5, 15, AlignLeft, AlignBottom, buffer);
    }

    if(game_state->state == GameStateGameOver) {
        // Show highscore
        char buffer[64];

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignTop, "You flew");

        snprintf(
            buffer,
            sizeof(buffer),
            game_state->new_highscore ? "%u m (new best)" : "%u m",
            game_state->distance / 10);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 16, AlignCenter, AlignTop, buffer);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignTop, "and collected");

        snprintf(buffer, sizeof(buffer), "$%u", game_state->total_coins);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 41, AlignCenter, AlignTop, buffer);

        snprintf(
            buffer,
            sizeof(buffer),
            "Best: %u m, Tot: $%u",
            save_game.max_distance / 10,
            save_game.total_coins);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 63, AlignCenter, AlignBottom, buffer);

        canvas_draw_rframe(canvas, 0, 3, 128, 49, 5);

        // char buffer[12];
        // snprintf(buffer, sizeof(buffer), "Dist: %u", game_state->distance);
        // canvas_draw_str_aligned(canvas, 123, 12, AlignRight, AlignBottom, buffer);

        // snprintf(buffer, sizeof(buffer), "Score: %u", game_state->points);
        // canvas_draw_str_aligned(canvas, 5, 12, AlignLeft, AlignBottom, buffer);

        // canvas_draw_str_aligned(canvas, 64, 34, AlignCenter, AlignCenter, "Highscore:");
        // snprintf(buffer, sizeof(buffer), "Dist: %u", save_game.max_distance);
        // canvas_draw_str_aligned(canvas, 123, 50, AlignRight, AlignBottom, buffer);

        // snprintf(buffer, sizeof(buffer), "Score: %u", save_game.max_score);
        // canvas_draw_str_aligned(canvas, 5, 50, AlignLeft, AlignBottom, buffer);

        // canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "boom.");

        // if(furi_timer_is_running(game_state->timer)) {
        //     furi_timer_start(game_state->timer, 0);
        // }
    }

    // canvas_draw_frame(canvas, 0, 0, 128, 64);

    furi_mutex_release(game_state->mutex);
}

static void jetpack_game_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void jetpack_game_update_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t jetpack_game_app(void* p) {
    UNUSED(p);
    int32_t return_code = 0;

    if(!storage_game_state_load()) {
        memset(&save_game, 0, sizeof(save_game));
    }

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(GameEvent));

    GameState* game_state = malloc(sizeof(GameState));

    global_state = game_state;
    jetpack_game_state_init(game_state);

    game_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(!game_state->mutex) {
        FURI_LOG_E(TAG, "cannot create mutex\r\n");
        return_code = 255;
        goto free_and_exit;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, jetpack_game_render_callback, game_state);
    view_port_input_callback_set(view_port, jetpack_game_input_callback, event_queue);

    FuriTimer* timer =
        furi_timer_alloc(jetpack_game_update_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 25);

    game_state->timer = timer;

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    GameEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        furi_mutex_acquire(game_state->mutex, FuriWaitForever);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypeRelease && event.input.key == InputKeyOk) {
                    game_state->barry.isBoosting = false;
                }

                // Reset highscore, for debug purposes
                if(event.input.type == InputTypeLong && event.input.key == InputKeyLeft) {
                    save_game.max_distance = 0;
                    save_game.total_coins = 0;
                    storage_game_state_save();
                }

                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        break;
                    case InputKeyDown:
                        break;
                    case InputKeyRight:
                        break;
                    case InputKeyLeft:
                        break;
                    case InputKeyOk:
                        if(game_state->state == GameStateGameOver) {
                            jetpack_game_state_init(game_state);
                        }

                        if(game_state->state == GameStateLife) {
                            // Do something
                            game_state->barry.isBoosting = true;
                        }

                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    default:
                        break;
                    }
                }
            } else if(event.type == EventTypeTick) {
                jetpack_game_tick(game_state);
            }
        }

        view_port_update(view_port);
        furi_mutex_release(game_state->mutex);
    }

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_mutex_free(game_state->mutex);

free_and_exit:
    jetpack_game_state_free(game_state);
    furi_message_queue_free(event_queue);

    return return_code;
}