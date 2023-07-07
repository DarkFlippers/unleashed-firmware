#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <dolphin/dolphin.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

typedef struct {
    //    +-----x
    //    |
    //    |
    //    y
    uint8_t x;
    uint8_t y;
} Point;

typedef enum {
    GameStateLife,

    // https://melmagazine.com/en-us/story/snake-nokia-6110-oral-history-taneli-armanto
    // Armanto: While testing the early versions of the game, I noticed it was hard
    // to control the snake upon getting close to and edge but not crashing — especially
    // in the highest speed levels. I wanted the highest level to be as fast as I could
    // possibly make the device "run," but on the other hand, I wanted to be friendly
    // and help the player manage that level. Otherwise it might not be fun to play. So
    // I implemented a little delay. A few milliseconds of extra time right before
    // the player crashes, during which she can still change the directions. And if
    // she does, the game continues.
    GameStateLastChance,

    GameStateGameOver,
} GameState;

// Note: do not change without purpose. Current values are used in smart
// orthogonality calculation in `snake_game_get_turn_snake`.
typedef enum {
    DirectionUp,
    DirectionRight,
    DirectionDown,
    DirectionLeft,
} Direction;

#define MAX_SNAKE_LEN 128 * 64 / 4

typedef struct {
    Point points[MAX_SNAKE_LEN];
    uint16_t len;
    Direction currentMovement;
    Direction nextMovement; // if backward of currentMovement, ignore
    Point fruit;
    GameState state;
    FuriMutex* mutex;
} SnakeState;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} SnakeEvent;

const NotificationSequence sequence_fail = {
    &message_vibro_on,

    &message_note_ds4,
    &message_delay_10,
    &message_sound_off,
    &message_delay_10,

    &message_note_ds4,
    &message_delay_10,
    &message_sound_off,
    &message_delay_10,

    &message_note_ds4,
    &message_delay_10,
    &message_sound_off,
    &message_delay_10,

    &message_vibro_off,
    NULL,
};

const NotificationSequence sequence_eat = {
    &message_note_c7,
    &message_delay_50,
    &message_sound_off,
    NULL,
};

static void snake_game_render_callback(Canvas* const canvas, void* ctx) {
    furi_assert(ctx);
    const SnakeState* snake_state = ctx;

    furi_mutex_acquire(snake_state->mutex, FuriWaitForever);

    // Frame
    canvas_draw_frame(canvas, 0, 0, 128, 64);

    // Fruit
    Point f = snake_state->fruit;
    f.x = f.x * 4 + 1;
    f.y = f.y * 4 + 1;
    canvas_draw_rframe(canvas, f.x, f.y, 6, 6, 2);

    // Snake
    for(uint16_t i = 0; i < snake_state->len; i++) {
        Point p = snake_state->points[i];
        p.x = p.x * 4 + 2;
        p.y = p.y * 4 + 2;
        canvas_draw_box(canvas, p.x, p.y, 4, 4);
    }

    // Game Over banner
    if(snake_state->state == GameStateGameOver) {
        // Screen is 128x64 px
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 34, 20, 62, 24);

        canvas_set_color(canvas, ColorBlack);
        canvas_draw_frame(canvas, 34, 20, 62, 24);

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 37, 31, "Game Over");

        canvas_set_font(canvas, FontSecondary);
        char buffer[12];
        snprintf(buffer, sizeof(buffer), "Score: %u", snake_state->len - 7U);
        canvas_draw_str_aligned(canvas, 64, 41, AlignCenter, AlignBottom, buffer);
    }

    furi_mutex_release(snake_state->mutex);
}

static void snake_game_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    SnakeEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void snake_game_update_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    SnakeEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

static void snake_game_init_game(SnakeState* const snake_state) {
    Point p[] = {{8, 6}, {7, 6}, {6, 6}, {5, 6}, {4, 6}, {3, 6}, {2, 6}};
    memcpy(snake_state->points, p, sizeof(p)); //-V1086

    snake_state->len = 7;

    snake_state->currentMovement = DirectionRight;

    snake_state->nextMovement = DirectionRight;

    Point f = {18, 6};
    snake_state->fruit = f;

    snake_state->state = GameStateLife;
}

static Point snake_game_get_new_fruit(SnakeState const* const snake_state) {
    // 1 bit for each point on the playing field where the snake can turn
    // and where the fruit can appear
    uint16_t buffer[8];
    memset(buffer, 0, sizeof(buffer));
    uint8_t empty = 8 * 16;

    for(uint16_t i = 0; i < snake_state->len; i++) {
        Point p = snake_state->points[i];

        if(p.x % 2 != 0 || p.y % 2 != 0) {
            continue;
        }
        p.x /= 2;
        p.y /= 2;

        buffer[p.y] |= 1 << p.x;
        empty--;
    }
    // Bit set if snake use that playing field

    uint16_t newFruit = rand() % empty;

    // Skip random number of _empty_ fields
    for(uint8_t y = 0; y < 8; y++) {
        for(uint16_t x = 0, mask = 1; x < 16; x += 1, mask <<= 1) {
            if((buffer[y] & mask) == 0) {
                if(newFruit == 0) {
                    Point p = {
                        .x = x * 2,
                        .y = y * 2,
                    };
                    return p;
                }
                newFruit--;
            }
        }
    }
    // We will never be here
    Point p = {0, 0};
    return p;
}

static bool snake_game_collision_with_frame(Point const next_step) {
    // if x == 0 && currentMovement == left then x - 1 == 255 ,
    // so check only x > right border
    return next_step.x > 30 || next_step.y > 14;
}

static bool
    snake_game_collision_with_tail(SnakeState const* const snake_state, Point const next_step) {
    for(uint16_t i = 0; i < snake_state->len; i++) {
        Point p = snake_state->points[i];
        if(p.x == next_step.x && p.y == next_step.y) {
            return true;
        }
    }

    return false;
}

static Direction snake_game_get_turn_snake(SnakeState const* const snake_state) {
    // Sum of two `Direction` lies between 0 and 6, odd values indicate orthogonality.
    bool is_orthogonal = (snake_state->currentMovement + snake_state->nextMovement) % 2 == 1;
    return is_orthogonal ? snake_state->nextMovement : snake_state->currentMovement;
}

static Point snake_game_get_next_step(SnakeState const* const snake_state) {
    Point next_step = snake_state->points[0];
    switch(snake_state->currentMovement) {
    // +-----x
    // |
    // |
    // y
    case DirectionUp:
        next_step.y--;
        break;
    case DirectionRight:
        next_step.x++;
        break;
    case DirectionDown:
        next_step.y++;
        break;
    case DirectionLeft:
        next_step.x--;
        break;
    }
    return next_step;
}

static void snake_game_move_snake(SnakeState* const snake_state, Point const next_step) {
    memmove(snake_state->points + 1, snake_state->points, snake_state->len * sizeof(Point));
    snake_state->points[0] = next_step;
}

static void
    snake_game_process_game_step(SnakeState* const snake_state, NotificationApp* notification) {
    if(snake_state->state == GameStateGameOver) {
        return;
    }

    snake_state->currentMovement = snake_game_get_turn_snake(snake_state);

    Point next_step = snake_game_get_next_step(snake_state);

    bool crush = snake_game_collision_with_frame(next_step);
    if(crush) {
        if(snake_state->state == GameStateLife) {
            snake_state->state = GameStateLastChance;
            return;
        } else if(snake_state->state == GameStateLastChance) {
            snake_state->state = GameStateGameOver;
            notification_message_block(notification, &sequence_fail);
            return;
        }
    } else {
        if(snake_state->state == GameStateLastChance) {
            snake_state->state = GameStateLife;
        }
    }

    crush = snake_game_collision_with_tail(snake_state, next_step);
    if(crush) {
        snake_state->state = GameStateGameOver;
        notification_message_block(notification, &sequence_fail);
        return;
    }

    bool eatFruit = (next_step.x == snake_state->fruit.x) && (next_step.y == snake_state->fruit.y);
    if(eatFruit) {
        snake_state->len++;
        if(snake_state->len >= MAX_SNAKE_LEN) {
            snake_state->state = GameStateGameOver;
            notification_message_block(notification, &sequence_fail);
            return;
        }
    }

    snake_game_move_snake(snake_state, next_step);

    if(eatFruit) {
        snake_state->fruit = snake_game_get_new_fruit(snake_state);
        notification_message(notification, &sequence_eat);
    }
}

int32_t snake_game_app(void* p) {
    UNUSED(p);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(SnakeEvent));

    SnakeState* snake_state = malloc(sizeof(SnakeState));
    snake_game_init_game(snake_state);

    snake_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    if(!snake_state->mutex) {
        FURI_LOG_E("SnakeGame", "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(snake_state);
        return 255;
    }

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, snake_game_render_callback, snake_state);
    view_port_input_callback_set(view_port, snake_game_input_callback, event_queue);

    FuriTimer* timer =
        furi_timer_alloc(snake_game_update_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 4);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);

    notification_message_block(notification, &sequence_display_backlight_enforce_on);

    dolphin_deed(DolphinDeedPluginGameStart);

    SnakeEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        furi_mutex_acquire(snake_state->mutex, FuriWaitForever);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        snake_state->nextMovement = DirectionUp;
                        break;
                    case InputKeyDown:
                        snake_state->nextMovement = DirectionDown;
                        break;
                    case InputKeyRight:
                        snake_state->nextMovement = DirectionRight;
                        break;
                    case InputKeyLeft:
                        snake_state->nextMovement = DirectionLeft;
                        break;
                    case InputKeyOk:
                        if(snake_state->state == GameStateGameOver) {
                            snake_game_init_game(snake_state);
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
                snake_game_process_game_step(snake_state, notification);
            }
        } else {
            // event timeout
        }

        view_port_update(view_port);
        furi_mutex_release(snake_state->mutex);
    }

    // Return backlight to normal state
    notification_message(notification, &sequence_display_backlight_enforce_auto);

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(snake_state->mutex);
    free(snake_state);

    return 0;
}
