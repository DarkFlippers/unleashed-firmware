#include <stdint.h>
#include <gui/gui.h>
#include <time.h>

#include "font.h"

/*
 0 empty
 1    2
 2    4
 3    8
 4   16
 5   32
 6   64
 7  128
 8  256
 9  512
10 1024
11 2048
 */
typedef uint8_t cell_state;

/* DirectionLeft <--
+????++????++????++????+
?    ??    ??    ??    ?
+????++????++????++????+
+????++????++????++????+
?    ??    ??    ??    ?
+????++????++????++????+
+??+????+??++????++????+
? 2? 2  ?  ??    ??    ?
+??+????+??++????++????+
+??+????++??+????++????+
? 4? 4  ?? 2? 2  ??    ?
+??+????++??+????++????+
*/
typedef enum {
    DirectionIdle,
    DirectionUp,
    DirectionRight,
    DirectionDown,
    DirectionLeft,
} Direction;

typedef struct {
    uint8_t y; // 0 <= y <= 3
    uint8_t x; // 0 <= x <= 3
} Point;

typedef struct {
    /*
    +----X
    |
    |  field[x][y]
    Y
 */
    uint8_t field[4][4];

    uint8_t next_field[4][4];

    Direction direction;
    /*
    field {
        animation-timing-function: linear;
        animation-duration: 300ms;
    }
 */
    uint32_t animation_start_ticks;

    Point keyframe_from[4][4];

    Point keyframe_to[4][4];

} GameState;

#define XtoPx(x) (33 + x * 15)

#define YtoPx(x) (1 + y * 15)

static void game_2048_render_callback(Canvas* const canvas, ValueMutex* const vm) {
    const GameState* game_state = acquire_mutex(vm, 25);
    if(game_state == NULL) {
        return;
    }

    // Before the function is called, the state is set with the canvas_reset(canvas)

    if(game_state->direction == DirectionIdle) {
        for(uint8_t y = 0; y < 4; y++) {
            for(uint8_t x = 0; x < 4; x++) {
                uint8_t field = game_state->field[y][x];
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_frame(canvas, XtoPx(x), YtoPx(y), 16, 16);
                if(field != 0) {
                    game_2048_draw_number(canvas, XtoPx(x), YtoPx(y), 1 << field);
                }
            }
        }
    } else { // if animation
        // for animation
        // (osKernelGetSysTimerCount() - game_state->animation_start_ticks) / osKernelGetSysTimerFreq();

        // TODO: end animation event/callback/set AnimationIdle
    }

    release_mutex(vm, game_state);
}

static void game_2048_input_callback(
    const InputEvent* const input_event,
    FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

// if return false then Game Over
static bool game_2048_set_new_number(GameState* const game_state) {
    uint8_t empty = 0;
    for(uint8_t y = 0; y < 4; y++) {
        for(uint8_t x = 0; x < 4; x++) {
            if(game_state->field[y][x] == 0) {
                empty++;
            }
        }
    }

    if(empty == 0) {
        return false;
    }

    if(empty == 1) {
        // If it is 1 move before losing, we help the player and get rid of randomness.
        for(uint8_t y = 0; y < 4; y++) {
            for(uint8_t x = 0; x < 4; x++) {
                if(game_state->field[y][x] == 0) {
                    bool haveFour =
                        // +----X
                        // |
                        // |  field[x][y], 0 <= x, y <= 3
                        // Y

                        // up == 4 or
                        (y > 0 && game_state->field[y - 1][x] == 2) ||
                        // right == 4 or
                        (x < 3 && game_state->field[y][x + 1] == 2) ||
                        // down == 4
                        (y < 3 && game_state->field[y + 1][x] == 2) ||
                        // left == 4
                        (x > 0 && game_state->field[y][x - 1] == 2);

                    if(haveFour) {
                        game_state->field[y][x] = 2;
                        return true;
                    }

                    game_state->field[y][x] = 1;
                    return true;
                }
            }
        }
    }

    uint8_t target = rand() % empty;
    uint8_t twoOrFore = rand() % 4 < 3;
    for(uint8_t y = 0; y < 4; y++) {
        for(uint8_t x = 0; x < 4; x++) {
            if(game_state->field[y][x] == 0) {
                if(target == 0) {
                    if(twoOrFore) {
                        game_state->field[y][x] = 1; // 2^1 == 2  75%
                    } else {
                        game_state->field[y][x] = 2; // 2^2 == 4  25%
                    }
                    goto exit;
                }
                target--;
            }
        }
    }
exit:
    return true;
}

// static void game_2048_process_row(uint8_t before[4], uint8_t *(after[4])) {
//     // move 1 row left.
//     for(uint8_t i = 0; i <= 2; i++) {
//         if(before[i] != 0 && before[i] == before[i + 1]) {
//             before[i]++;
//             before[i + 1] = 0;
//             i++;
//         }
//     }
//     for(uint8_t i = 0, j = 0; i <= 3; i++) {
//         if (before[i] != 0) {
//             before[j] = before[i];
//             i++;
//         }
//     }
// }

static void game_2048_process_move(GameState* const game_state) {
    memset(game_state->next_field, 0, sizeof(game_state->next_field));
    // +----X
    // |
    // |  field[x][y], 0 <= x, y <= 3
    // Y

    // up
    if(game_state->direction == DirectionUp) {
        for(uint8_t x = 0; x < 4; x++) {
            uint8_t next_y = 0;
            for(int8_t y = 0; y < 4; y++) {
                uint8_t field = game_state->field[y][x];
                if(field == 0) {
                    continue;
                }

                if(game_state->next_field[next_y][x] == 0) {
                    game_state->next_field[next_y][x] = field;
                    continue;
                }

                if(field == game_state->next_field[next_y][x]) {
                    game_state->next_field[next_y][x]++;
                    next_y++;
                    continue;
                }

                next_y++;
                game_state->next_field[next_y][x] = field;
            }
        }
    }

    // right
    if(game_state->direction == DirectionRight) {
        for(uint8_t y = 0; y < 4; y++) {
            uint8_t next_x = 3;
            for(int8_t x = 3; x >= 0; x--) {
                uint8_t field = game_state->field[y][x];
                if(field == 0) {
                    continue;
                }

                if(game_state->next_field[y][next_x] == 0) {
                    game_state->next_field[y][next_x] = field;
                    continue;
                }

                if(field == game_state->next_field[y][next_x]) {
                    game_state->next_field[y][next_x]++;
                    next_x--;
                    continue;
                }

                next_x--;
                game_state->next_field[y][next_x] = field;
            }
        }
    }

    // down
    if(game_state->direction == DirectionDown) {
        for(uint8_t x = 0; x < 4; x++) {
            uint8_t next_y = 3;
            for(int8_t y = 3; y >= 0; y--) {
                uint8_t field = game_state->field[y][x];
                if(field == 0) {
                    continue;
                }

                if(game_state->next_field[next_y][x] == 0) {
                    game_state->next_field[next_y][x] = field;
                    continue;
                }

                if(field == game_state->next_field[next_y][x]) {
                    game_state->next_field[next_y][x]++;
                    next_y--;
                    continue;
                }

                next_y--;
                game_state->next_field[next_y][x] = field;
            }
        }
    }

    // 0, 0, 1, 1
    // 1, 0, 0, 0

    // left
    if(game_state->direction == DirectionLeft) {
        for(uint8_t y = 0; y < 4; y++) {
            uint8_t next_x = 0;
            for(uint8_t x = 0; x < 4; x++) {
                uint8_t field = game_state->field[y][x];
                if(field == 0) {
                    continue;
                }

                if(game_state->next_field[y][next_x] == 0) {
                    game_state->next_field[y][next_x] = field;
                    continue;
                }

                if(field == game_state->next_field[y][next_x]) {
                    game_state->next_field[y][next_x]++;
                    next_x++;
                    continue;
                }

                next_x++;
                game_state->next_field[y][next_x] = field;
            }
        }
    }

    // <debug>
    game_state->direction = DirectionIdle;
    memcpy(game_state->field, game_state->next_field, sizeof(game_state->field));
    // </debug>
}

int32_t game_2048_app(void* p) {
    int32_t return_code = 0;

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    GameState* game_state = malloc(sizeof(GameState));

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, game_state, sizeof(GameState))) {
        return_code = 255;
        goto free_and_exit;
    }

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(
        view_port, (ViewPortDrawCallback)game_2048_render_callback, &state_mutex);
    view_port_input_callback_set(
        view_port, (ViewPortInputCallback)game_2048_input_callback, event_queue);

    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    game_state->direction = DirectionIdle;
    game_2048_set_new_number(game_state);
    game_2048_set_new_number(game_state);

    /* <debug>
    game_state->field[0][0] = 0;
    game_state->field[0][1] = 1;
    game_state->field[0][2] = 2;
    game_state->field[0][3] = 3;

    game_state->field[1][0] = 4;
    game_state->field[1][1] = 5;
    game_state->field[1][2] = 6;
    game_state->field[1][3] = 7;
    
    game_state->field[2][0] = 8;
    game_state->field[2][1] = 9;
    game_state->field[2][2] = 10;
    game_state->field[2][3] = 11;
    
    game_state->field[3][0] = 0;
    game_state->field[3][1] = 0;
    game_state->field[3][2] = 0;
    game_state->field[3][3] = 0;
    </debug> */

    InputEvent event;
    for(bool loop = true; loop;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        GameState* game_state = (GameState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            if(event.type == InputTypePress) {
                switch(event.key) {
                case InputKeyUp:
                    game_state->direction = DirectionUp;
                    game_2048_process_move(game_state);
                    game_2048_set_new_number(game_state);
                    break;
                case InputKeyDown:
                    game_state->direction = DirectionDown;
                    game_2048_process_move(game_state);
                    game_2048_set_new_number(game_state);
                    break;
                case InputKeyRight:
                    game_state->direction = DirectionRight;
                    game_2048_process_move(game_state);
                    game_2048_set_new_number(game_state);
                    break;
                case InputKeyLeft:
                    game_state->direction = DirectionLeft;
                    game_2048_process_move(game_state);
                    game_2048_set_new_number(game_state);
                    break;
                case InputKeyOk:; // TODO: reinit in game ower state
                    break;
                case InputKeyBack:
                    loop = false;
                    break;
                }
            }
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, game_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    delete_mutex(&state_mutex);

free_and_exit:
    free(game_state);
    furi_message_queue_free(event_queue);

    return return_code;
}