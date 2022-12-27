#include "helpers/snake_file_handler.h"
#include "helpers/snake_types.h"

#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <dolphin/dolphin.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <dolphin/dolphin.h>

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
    const SnakeState* snake_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(snake_state == NULL) {
        return;
    }

    // Before the function is called, the state is set with the canvas_reset(canvas)

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

    // Show score on the game field
    if(snake_state->state != GameStateGameOver) {
        char buffer2[6];
        canvas_set_font(canvas, FontBatteryPercent);
        snprintf(buffer2, sizeof(buffer2), "%u", (snake_state->len - 7));
        canvas_draw_str_aligned(canvas, 124, 10, AlignRight, AlignBottom, buffer2);
    }
    // Game Over banner
    if(snake_state->state == GameStateGameOver) {
        // Screen is 128x64 px
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 32, 20, 64, 34);

        canvas_set_color(canvas, ColorBlack);
        canvas_draw_frame(canvas, 32, 20, 64, 34);

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 37, 31, "Game Over");

        char buffer[18];
        canvas_set_font(canvas, FontSecondary);
        snprintf(buffer, sizeof(buffer), "Score: %u", snake_state->len);
        canvas_draw_str_aligned(canvas, 64, 41, AlignCenter, AlignBottom, buffer);

        snprintf(buffer, sizeof(buffer), "Highscore: %d", snake_state->highscore);
        canvas_draw_str_aligned(canvas, 64, 51, AlignCenter, AlignBottom, buffer);
    }
    if((snake_state->len - 7) % 20 == 0 && (snake_state->len - 7) != 0) {
        DOLPHIN_DEED(getRandomDeed());
    }

    release_mutex((ValueMutex*)ctx, snake_state);
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

static void snake_game_game_over(SnakeState* const snake_state, NotificationApp* notification) {
    snake_state->state = GameStateGameOver;
    snake_state->len = snake_state->len - 7;
    if(snake_state->len > snake_state->highscore) {
        snake_state->isNewHighscore = true;
        snake_state->highscore = snake_state->len;
    }

    notification_message_block(notification, &sequence_fail);
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
            snake_game_game_over(snake_state, notification);
            return;
        }
    } else {
        if(snake_state->state == GameStateLastChance) {
            snake_state->state = GameStateLife;
        }
    }

    crush = snake_game_collision_with_tail(snake_state, next_step);
    if(crush) {
        snake_game_game_over(snake_state, notification);
        return;
    }

    bool eatFruit = (next_step.x == snake_state->fruit.x) && (next_step.y == snake_state->fruit.y);
    if(eatFruit) {
        snake_state->len++;
        if(snake_state->len >= MAX_SNAKE_LEN) {
            snake_game_game_over(snake_state, notification);
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
    snake_state->isNewHighscore = false;
    snake_state->highscore = 0;
    if(!snake_game_init_game_from_file(snake_state)) {
        snake_game_init_game(snake_state);
    }

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, snake_state, sizeof(SnakeState))) {
        FURI_LOG_E("SnakeGame", "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(snake_state);
        return 255;
    }

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, snake_game_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, snake_game_input_callback, event_queue);

    FuriTimer* timer =
        furi_timer_alloc(snake_game_update_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 4);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);

    notification_message_block(notification, &sequence_display_backlight_enforce_on);

    DOLPHIN_DEED(DolphinDeedPluginGameStart);

    SnakeEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        SnakeState* snake_state = (SnakeState*)acquire_mutex_block(&state_mutex);

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
                        if(snake_state->state == GameStateLife) {
                            snake_game_save_game_to_file(snake_state);
                        }
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
        release_mutex(&state_mutex, snake_state);
    }

    if(snake_state->isNewHighscore) {
        snake_game_save_score_to_file(snake_state->highscore);
    }
    // Wait for all notifications to be played and return backlight to normal state
    notification_message_block(notification, &sequence_display_backlight_enforce_auto);

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);
    free(snake_state);

    return 0;
}
