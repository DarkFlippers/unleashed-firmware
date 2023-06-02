#include <stdlib.h>

#include <flappy_bird_icons.h>
#include <furi.h>
#include <gui/gui.h>
#include <gui/icon_animation_i.h>
#include <input/input.h>
#include <dolphin/dolphin.h>

#define TAG "Flappy"
#define DEBUG false

#define FLAPPY_BIRD_HEIGHT 15
#define FLAPPY_BIRD_WIDTH 10

#define FLAPPY_PILAR_MAX 6
#define FLAPPY_PILAR_DIST 35

#define FLAPPY_GAB_HEIGHT 25
#define FLAPPY_GAB_WIDTH 10

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
    IconAnimation* sprite;
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
    FuriMutex* mutex;
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
    bird.sprite = icon_animation_alloc(&A_bird);

    game_state->debug = DEBUG;
    game_state->bird = bird;
    game_state->pilars_count = 0;
    game_state->points = 0;
    game_state->state = GameStateLife;
    memset(game_state->pilars, 0, sizeof(game_state->pilars));

    flappy_game_random_pilar(game_state);
}

static void flappy_game_state_free(GameState* const game_state) {
    icon_animation_free(game_state->bird.sprite);
    free(game_state);
}

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

                if(game_state->bird.point.y <= 0 - FLAPPY_BIRD_WIDTH) {
                    game_state->bird.point.y = 64;
                }

                if(game_state->bird.point.y > 64 - FLAPPY_BIRD_WIDTH) {
                    game_state->bird.point.y = FLIPPER_LCD_HEIGHT - FLAPPY_BIRD_WIDTH;
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
    furi_assert(ctx);
    const GameState* game_state = ctx;
    furi_mutex_acquire(game_state->mutex, FuriWaitForever);

    canvas_draw_frame(canvas, 0, 0, 128, 64);

    if(game_state->state == GameStateLife) {
        // Pilars
        for(int i = 0; i < FLAPPY_PILAR_MAX; i++) {
            const PILAR* pilar = &game_state->pilars[i];
            if(pilar != NULL && pilar->visible == 1) {
                canvas_draw_frame(
                    canvas, pilar->point.x, pilar->point.y, FLAPPY_GAB_WIDTH, pilar->height);

                canvas_draw_frame(
                    canvas, pilar->point.x + 1, pilar->point.y, FLAPPY_GAB_WIDTH, pilar->height);

                canvas_draw_frame(
                    canvas,
                    pilar->point.x + 2,
                    pilar->point.y,
                    FLAPPY_GAB_WIDTH - 1,
                    pilar->height);

                canvas_draw_frame(
                    canvas,
                    pilar->point.x,
                    pilar->point.y + pilar->height + FLAPPY_GAB_HEIGHT,
                    FLAPPY_GAB_WIDTH,
                    FLIPPER_LCD_HEIGHT - pilar->height - FLAPPY_GAB_HEIGHT);

                canvas_draw_frame(
                    canvas,
                    pilar->point.x + 1,
                    pilar->point.y + pilar->height + FLAPPY_GAB_HEIGHT,
                    FLAPPY_GAB_WIDTH - 1,
                    FLIPPER_LCD_HEIGHT - pilar->height - FLAPPY_GAB_HEIGHT);

                canvas_draw_frame(
                    canvas,
                    pilar->point.x + 2,
                    pilar->point.y + pilar->height + FLAPPY_GAB_HEIGHT,
                    FLAPPY_GAB_WIDTH - 1,
                    FLIPPER_LCD_HEIGHT - pilar->height - FLAPPY_GAB_HEIGHT);
            }
        }

        // Switch animation
        game_state->bird.sprite->frame = 1;
        if(game_state->bird.gravity < -0.5)
            game_state->bird.sprite->frame = 0;
        else if(game_state->bird.gravity > 0.5)
            game_state->bird.sprite->frame = 2;

        canvas_draw_icon_animation(
            canvas, game_state->bird.point.x, game_state->bird.point.y, game_state->bird.sprite);

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

        canvas_set_font(canvas, FontSecondary);
        char buffer[12];
        snprintf(buffer, sizeof(buffer), "Score: %u", game_state->points);
        canvas_draw_str_aligned(canvas, 64, 41, AlignCenter, AlignBottom, buffer);
    }

    furi_mutex_release(game_state->mutex);
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

    game_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(!game_state->mutex) {
        FURI_LOG_E(TAG, "cannot create mutex\r\n");
        return_code = 255;
        goto free_and_exit;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, flappy_game_render_callback, game_state);
    view_port_input_callback_set(view_port, flappy_game_input_callback, event_queue);

    FuriTimer* timer =
        furi_timer_alloc(flappy_game_update_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 25);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Call dolphin deed on game start
    DOLPHIN_DEED(DolphinDeedPluginGameStart);

    GameEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        furi_mutex_acquire(game_state->mutex, FuriWaitForever);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        if(game_state->state == GameStateLife) {
                            flappy_game_flap(game_state);
                        }

                        break;
                    case InputKeyDown:
                        break;
                    case InputKeyRight:
                        break;
                    case InputKeyLeft:
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
                    default:
                        break;
                    }
                }
            } else if(event.type == EventTypeTick) {
                flappy_game_tick(game_state);
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
    flappy_game_state_free(game_state);
    furi_message_queue_free(event_queue);

    return return_code;
}
