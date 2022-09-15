#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

#include "bird.h"

#define TAG "Flappy"
#define DEBUG false

#define FLAPPY_BIRD_HEIGHT 15
#define FLAPPY_BIRD_WIDTH 10

#define FLAPPY_PILAR_MAX 6
#define FLAPPY_PILAR_DIST 40

#define FLAPPY_GAB_HEIGHT 25
#define FLAPPY_GAB_WIDTH 5

#define FLAPPY_GRAVITY_JUMP -1.1
#define FLAPPY_GRAVITY_TICK 0.15

#define FLIPPER_LCD_WIDTH 128
#define FLIPPER_LCD_HEIGHT 64

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    int x;
    int y;
} POINT;

typedef struct {
    float gravity;
    POINT point;
} BIRD;

typedef struct {
    POINT point;
    int height;
    int visible;
    bool passed;
} PILAR;

typedef enum {
    GameStateLife,
    GameStateGameOver,
} State;

typedef struct {
    BIRD bird;
    int points;
    int pilars_count;
    PILAR pilars[FLAPPY_PILAR_MAX];
    bool debug;
    State state;
} GameState;

typedef struct {
    EventType type;
    InputEvent input;
} GameEvent;

typedef enum {
    DirectionUp,
    DirectionRight,
    DirectionDown,
    DirectionLeft,
} Direction;

static void flappy_game_random_pilar(GameState* const game_state) {
    PILAR pilar;

    pilar.passed = false;
    pilar.visible = 1;
    pilar.height = random() % (FLIPPER_LCD_HEIGHT - FLAPPY_GAB_HEIGHT) + 1;
    pilar.point.y = 0;
    pilar.point.x = FLIPPER_LCD_WIDTH + FLAPPY_GAB_WIDTH + 1;

    game_state->pilars_count++;
    game_state->pilars[game_state->pilars_count % FLAPPY_PILAR_MAX] = pilar;
}

static void flappy_game_state_init(GameState* const game_state) {
    BIRD bird;
    bird.gravity = 0.0f;
    bird.point.x = 15;
    bird.point.y = 32;

    game_state->debug = DEBUG;
    game_state->bird = bird;
    game_state->pilars_count = 0;
    game_state->points = 0;
    game_state->state = GameStateLife;
    memset(game_state->pilars, 0, sizeof(game_state->pilars));

    flappy_game_random_pilar(game_state);
}

// static void flappy_game_reset(GameState* const game_state) {
// FURI_LOG_I(TAG, "Reset Game State\r\n"); // Resetting State
// }

static void flappy_game_tick(GameState* const game_state) {
    if(game_state->state == GameStateLife) {
        if(!game_state->debug) {
            game_state->bird.gravity += FLAPPY_GRAVITY_TICK;
            game_state->bird.point.y += game_state->bird.gravity;
        }

        // Checking the location of the last respawned pilar.
        PILAR* pilar = &game_state->pilars[game_state->pilars_count % FLAPPY_PILAR_MAX];
        if(pilar->point.x == (FLIPPER_LCD_WIDTH - FLAPPY_PILAR_DIST))
            flappy_game_random_pilar(game_state);

        // Updating the position/status of the pilars (visiblity, posotion, game points)
        //        |  |      |  |  |
        //        |  |      |  |  |
        //        |__|      |  |__|
        //   _____X         |      X_____
        //  |     |         |      |     |   // [Pos + Width of pilar] >= [Bird Pos]
        //  |_____|         |      |_____|
        // X <---->         |     X <->
        // Bird Pos + Lenght of the  bird] >= [Pilar]
        for(int i = 0; i < FLAPPY_PILAR_MAX; i++) {
            PILAR* pilar = &game_state->pilars[i];
            if(pilar != NULL && pilar->visible && game_state->state == GameStateLife) {
                pilar->point.x--;
                if(game_state->bird.point.x >= pilar->point.x + FLAPPY_GAB_WIDTH &&
                   pilar->passed == false) {
                    pilar->passed = true;
                    game_state->points++;
                }
                if(pilar->point.x < -FLAPPY_GAB_WIDTH) pilar->visible = 0;

                // Checking out of bounds
                if(game_state->bird.point.y < 0 - FLAPPY_BIRD_WIDTH ||
                   game_state->bird.point.y > FLIPPER_LCD_HEIGHT) {
                    game_state->state = GameStateGameOver;
                    break;
                }

                // Bird inbetween pipes
                if((game_state->bird.point.x + FLAPPY_BIRD_HEIGHT >= pilar->point.x) &&
                   (game_state->bird.point.x <= pilar->point.x + FLAPPY_GAB_WIDTH)) {
                    // Bird below Bottom Pipe
                    if(game_state->bird.point.y + FLAPPY_BIRD_WIDTH - 2 >=
                       pilar->height + FLAPPY_GAB_HEIGHT) {
                        game_state->state = GameStateGameOver;
                        break;
                    }

                    // Bird above Upper Pipe
                    if(game_state->bird.point.y < pilar->height) {
                        game_state->state = GameStateGameOver;
                        break;
                    }
                }
            }
        }
    }
}

static void flappy_game_flap(GameState* const game_state) {
    game_state->bird.gravity = FLAPPY_GRAVITY_JUMP;
}

static void flappy_game_render_callback(Canvas* const canvas, void* ctx) {
    const GameState* game_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(game_state == NULL) {
        return;
    }

    canvas_draw_frame(canvas, 0, 0, 128, 64);

    if(game_state->state == GameStateLife) {
        // Pilars
        for(int i = 0; i < FLAPPY_PILAR_MAX; i++) {
            const PILAR* pilar = &game_state->pilars[i];
            if(pilar != NULL && pilar->visible == 1) {
                canvas_draw_frame(
                    canvas, pilar->point.x, pilar->point.y, FLAPPY_GAB_WIDTH, pilar->height);

                canvas_draw_frame(
                    canvas,
                    pilar->point.x,
                    pilar->point.y + pilar->height + FLAPPY_GAB_HEIGHT,
                    FLAPPY_GAB_WIDTH,
                    FLIPPER_LCD_HEIGHT - pilar->height - FLAPPY_GAB_HEIGHT);
            }
        }
        // Flappy
        for(int h = 0; h < FLAPPY_BIRD_HEIGHT; h++) {
            for(int w = 0; w < FLAPPY_BIRD_WIDTH; w++) {
                // Switch animation
                int bird = 0;
                if(game_state->bird.gravity < -0.5)
                    bird = 1;
                else
                    bird = 2;

                // Draw bird pixels
                if(bird_array[bird][h][w] == 1) {
                    int x = game_state->bird.point.x + h;
                    int y = game_state->bird.point.y + w;

                    canvas_draw_dot(canvas, x, y);
                }
            }
        }

        // Stats

        canvas_set_font(canvas, FontSecondary);
        char buffer[12];
        snprintf(buffer, sizeof(buffer), "Score: %u", game_state->points);
        canvas_draw_str_aligned(canvas, 100, 12, AlignCenter, AlignBottom, buffer);

        if(game_state->debug) {
            char coordinates[20];
            snprintf(coordinates, sizeof(coordinates), "Y: %u", game_state->bird.point.y);
            canvas_draw_str_aligned(canvas, 1, 12, AlignCenter, AlignBottom, coordinates);
        }
    }

    if(game_state->state == GameStateGameOver) {
        // Screen is 128x64 px
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 34, 20, 62, 24);

        canvas_set_color(canvas, ColorBlack);
        canvas_draw_frame(canvas, 34, 20, 62, 24);

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 37, 31, "Game Over");

        /*if(game_state->points != 0 && game_state->points % 5 == 0) {
            DOLPHIN_DEED(getRandomDeed());
        }*/

        canvas_set_font(canvas, FontSecondary);
        char buffer[12];
        snprintf(buffer, sizeof(buffer), "Score: %u", game_state->points);
        canvas_draw_str_aligned(canvas, 64, 41, AlignCenter, AlignBottom, buffer);
    }

    release_mutex((ValueMutex*)ctx, game_state);
}

static void flappy_game_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void flappy_game_update_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t flappy_game_app(void* p) {
    UNUSED(p);
    int32_t return_code = 0;

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(GameEvent));

    GameState* game_state = malloc(sizeof(GameState));
    flappy_game_state_init(game_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, game_state, sizeof(GameState))) {
        FURI_LOG_E(TAG, "cannot create mutex\r\n");
        return_code = 255;
        goto free_and_exit;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, flappy_game_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, flappy_game_input_callback, event_queue);

    FuriTimer* timer =
        furi_timer_alloc(flappy_game_update_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 25);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    GameEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        GameState* game_state = (GameState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        game_state->bird.point.y--;
                        break;
                    case InputKeyDown:
                        game_state->bird.point.y++;
                        break;
                    case InputKeyRight:
                        game_state->bird.point.x++;
                        break;
                    case InputKeyLeft:
                        game_state->bird.point.x--;
                        break;
                    case InputKeyOk:
                        if(game_state->state == GameStateGameOver) {
                            flappy_game_state_init(game_state);
                        }

                        if(game_state->state == GameStateLife) {
                            flappy_game_flap(game_state);
                        }

                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    }
                }
            } else if(event.type == EventTypeTick) {
                flappy_game_tick(game_state);
            }
        } else {
            //FURI_LOG_D(TAG, "osMessageQueue: event timeout");
            // event timeout
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, game_state);
    }

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    delete_mutex(&state_mutex);

free_and_exit:
    free(game_state);
    furi_message_queue_free(event_queue);

    return return_code;
}
