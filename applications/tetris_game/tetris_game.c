#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <furi_hal_resources.h>
#include <furi_hal_gpio.h>

#define BORDER_OFFSET 1
#define MARGIN_OFFSET 3
#define BLOCK_HEIGHT 6
#define BLOCK_WIDTH 6

#define FIELD_WIDTH 11
#define FIELD_HEIGHT 24

typedef struct Point {
    // Also used for offset data, which is sometimes negative
    int8_t x, y;
} Point;

// Rotation logic taken from
// https://www.youtube.com/watch?v=yIpk5TJ_uaI
typedef enum { OffsetTypeCommon, OffsetTypeI, OffsetTypeO } OffsetType;

// Since we only support rotating clockwise, these are actual translation values,
// not values to be subtracted to get translation values

static const Point rotOffsetTranslation[3][4][5] = {
    {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
     {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
     {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
     {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}},
    {{{1, 0}, {-1, 0}, {2, 0}, {-1, 1}, {2, -2}},
     {{0, 1}, {-1, 1}, {2, 1}, {-1, -1}, {2, 2}},
     {{-1, 0}, {1, 0}, {-2, 0}, {1, -1}, {-2, 2}},
     {{0, -1}, {1, -1}, {-2, -1}, {1, 1}, {-2, -2}}},
    {{{0, -1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
     {{1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
     {{0, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
     {{-1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}}};

typedef struct {
    Point p[4];
    uint8_t rotIdx;
    OffsetType offsetType;
} Piece;

// Shapes @ spawn locations, rotation point first
static Piece shapes[] = {
    {.p = {{5, 1}, {4, 0}, {5, 0}, {6, 1}}, .rotIdx = 0, .offsetType = OffsetTypeCommon}, // Z
    {.p = {{5, 1}, {4, 1}, {5, 0}, {6, 0}}, .rotIdx = 0, .offsetType = OffsetTypeCommon}, // S
    {.p = {{5, 1}, {4, 1}, {6, 1}, {6, 0}}, .rotIdx = 0, .offsetType = OffsetTypeCommon}, // L
    {.p = {{5, 1}, {4, 0}, {4, 1}, {6, 1}}, .rotIdx = 0, .offsetType = OffsetTypeCommon}, // J
    {.p = {{5, 1}, {4, 1}, {5, 0}, {6, 1}}, .rotIdx = 0, .offsetType = OffsetTypeCommon}, // T
    {.p = {{5, 1}, {4, 1}, {6, 1}, {7, 1}}, .rotIdx = 0, .offsetType = OffsetTypeI}, // I
    {.p = {{5, 1}, {5, 0}, {6, 0}, {6, 1}}, .rotIdx = 0, .offsetType = OffsetTypeO} // O
};

typedef enum { GameStatePlaying, GameStateGameOver } GameState;

typedef struct {
    bool playField[FIELD_HEIGHT][FIELD_WIDTH];
    Piece currPiece;
    uint16_t numLines;
    uint16_t fallSpeed;
    GameState gameState;
    FuriTimer* timer;
} TetrisState;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} TetrisEvent;

static void tetris_game_draw_border(Canvas* const canvas) {
    canvas_draw_line(canvas, 0, 0, 0, 127);
    canvas_draw_line(canvas, 0, 127, 63, 127);
    canvas_draw_line(canvas, 63, 127, 63, 0);

    canvas_draw_line(canvas, 2, 0, 2, 125);
    canvas_draw_line(canvas, 2, 125, 61, 125);
    canvas_draw_line(canvas, 61, 125, 61, 0);
}

static void tetris_game_draw_playfield(Canvas* const canvas, const TetrisState* tetris_state) {
    // Playfield: 11 x 24

    for(int y = 0; y < FIELD_HEIGHT; y++) {
        for(int x = 0; x < FIELD_WIDTH; x++) {
            if(tetris_state->playField[y][x]) {
                uint16_t xOffset = x * 5;
                uint16_t yOffset = y * 5;

                canvas_draw_rframe(
                    canvas,
                    BORDER_OFFSET + MARGIN_OFFSET + xOffset,
                    BORDER_OFFSET + MARGIN_OFFSET + yOffset - 1,
                    BLOCK_WIDTH,
                    BLOCK_HEIGHT,
                    1);
                canvas_draw_dot(
                    canvas,
                    BORDER_OFFSET + MARGIN_OFFSET + xOffset + 2,
                    BORDER_OFFSET + MARGIN_OFFSET + yOffset + 1);
                canvas_draw_dot(
                    canvas,
                    BORDER_OFFSET + MARGIN_OFFSET + xOffset + 3,
                    BORDER_OFFSET + MARGIN_OFFSET + yOffset + 1);
                canvas_draw_dot(
                    canvas,
                    BORDER_OFFSET + MARGIN_OFFSET + xOffset + 2,
                    BORDER_OFFSET + MARGIN_OFFSET + yOffset + 2);
            }
        }
    }
}

static void tetris_game_render_callback(Canvas* const canvas, void* ctx) {
    const TetrisState* tetris_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(tetris_state == NULL) {
        FURI_LOG_E("TetrisGame", "it null");
        return;
    }

    tetris_game_draw_border(canvas);
    tetris_game_draw_playfield(canvas, tetris_state);

    // Show score on the game field
    if(tetris_state->gameState == GameStatePlaying) {
        char buffer2[6];
        snprintf(buffer2, sizeof(buffer2), "%u", tetris_state->numLines);
        canvas_draw_str_aligned(canvas, 58, 10, AlignRight, AlignBottom, buffer2);
    }

    if(tetris_state->gameState == GameStateGameOver) {
        // 128 x 64
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 1, 52, 62, 24);

        canvas_set_color(canvas, ColorBlack);
        canvas_draw_frame(canvas, 1, 52, 62, 24);

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 4, 63, "Game Over");

        char buffer[13];
        snprintf(buffer, sizeof(buffer), "Lines: %u", tetris_state->numLines);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 32, 73, AlignCenter, AlignBottom, buffer);
    }
    release_mutex((ValueMutex*)ctx, tetris_state);
}

static void tetris_game_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    TetrisEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void tetris_game_init_state(TetrisState* tetris_state) {
    tetris_state->gameState = GameStatePlaying;
    tetris_state->numLines = 0;
    tetris_state->fallSpeed = 500;
    memset(tetris_state->playField, 0, sizeof(tetris_state->playField));

    memcpy(&tetris_state->currPiece, &shapes[rand() % 7], sizeof(tetris_state->currPiece));

    furi_timer_start(tetris_state->timer, tetris_state->fallSpeed);
}

static void tetris_game_remove_curr_piece(TetrisState* tetris_state) {
    for(int i = 0; i < 4; i++) {
        uint8_t x = tetris_state->currPiece.p[i].x;
        uint8_t y = tetris_state->currPiece.p[i].y;

        tetris_state->playField[y][x] = false;
    }
}

static void tetris_game_render_curr_piece(TetrisState* tetris_state) {
    for(int i = 0; i < 4; i++) {
        uint8_t x = tetris_state->currPiece.p[i].x;
        uint8_t y = tetris_state->currPiece.p[i].y;

        tetris_state->playField[y][x] = true;
    }
}

static void tetris_game_rotate_shape(Point currShape[], Point newShape[]) {
    // Copy shape data
    for(int i = 0; i < 4; i++) {
        newShape[i] = currShape[i];
    }

    for(int i = 1; i < 4; i++) {
        int8_t relX = currShape[i].x - currShape[0].x;
        int8_t relY = currShape[i].y - currShape[0].y;

        // Matrix rotation thing
        int8_t newRelX = (relX * 0) + (relY * -1);
        int8_t newRelY = (relX * 1) + (relY * 0);

        newShape[i].x = currShape[0].x + newRelX;
        newShape[i].y = currShape[0].y + newRelY;
    }
}

static void tetris_game_apply_kick(Point points[], Point kick) {
    for(int i = 0; i < 4; i++) {
        points[i].x += kick.x;
        points[i].y += kick.y;
    }
}

static bool tetris_game_is_valid_pos(TetrisState* tetris_state, Point* shape) {
    for(int i = 0; i < 4; i++) {
        if(shape[i].x < 0 || shape[i].x > (FIELD_WIDTH - 1) ||
           tetris_state->playField[shape[i].y][shape[i].x] == true) {
            return false;
        }
    }
    return true;
}

static void tetris_game_try_rotation(TetrisState* tetris_state, Piece* newPiece) {
    uint8_t currRotIdx = tetris_state->currPiece.rotIdx;

    Point* rotatedShape = malloc(sizeof(Point) * 4);
    Point* kickedShape = malloc(sizeof(Point) * 4);

    memcpy(rotatedShape, &tetris_state->currPiece.p, sizeof(tetris_state->currPiece.p));

    tetris_game_rotate_shape(tetris_state->currPiece.p, rotatedShape);

    for(int i = 0; i < 5; i++) {
        memcpy(kickedShape, rotatedShape, (sizeof(Point) * 4));
        tetris_game_apply_kick(
            kickedShape, rotOffsetTranslation[newPiece->offsetType][currRotIdx][i]);

        if(tetris_game_is_valid_pos(tetris_state, kickedShape)) {
            memcpy(&newPiece->p, kickedShape, sizeof(newPiece->p));
            newPiece->rotIdx = (newPiece->rotIdx + 1) % 4;
            break;
        }
    }
    free(rotatedShape);
    free(kickedShape);
}

static bool tetris_game_row_is_line(bool row[]) {
    for(int i = 0; i < FIELD_WIDTH; i++) {
        if(row[i] == false) return false;
    }
    return true;
}

static void
    tetris_game_check_for_lines(TetrisState* tetris_state, uint8_t* lines, uint8_t* numLines) {
    for(int i = 0; i < FIELD_HEIGHT; i++) {
        if(tetris_game_row_is_line(tetris_state->playField[i])) {
            *(lines++) = i;
            *numLines += 1;
        }
    }
}

static bool tetris_game_piece_at_bottom(TetrisState* tetris_state, Piece* newPiece) {
    for(int i = 0; i < 4; i++) {
        Point* pos = (Point*)&newPiece->p;
        if(pos[i].y >= FIELD_HEIGHT || tetris_state->playField[pos[i].y][pos[i].x] == true) {
            return true;
        }
    }
    return false;
}

static void tetris_game_update_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    TetrisEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void
    tetris_game_process_step(TetrisState* tetris_state, Piece* newPiece, bool wasDownMove) {
    if(tetris_state->gameState == GameStateGameOver) return;

    tetris_game_remove_curr_piece(tetris_state);

    if(wasDownMove) {
        if(tetris_game_piece_at_bottom(tetris_state, newPiece)) {
            furi_timer_stop(tetris_state->timer);

            tetris_game_render_curr_piece(tetris_state);
            uint8_t numLines = 0;
            uint8_t lines[] = {0, 0, 0, 0};

            tetris_game_check_for_lines(tetris_state, lines, &numLines);
            if(numLines > 0) {
                for(int i = 0; i < numLines; i++) {
                    // zero out row
                    for(int j = 0; j < FIELD_WIDTH; j++) {
                        tetris_state->playField[lines[i]][j] = false;
                    }
                    // move all above rows down
                    for(int k = lines[i]; k >= 0; k--) {
                        for(int m = 0; m < FIELD_WIDTH; m++) {
                            tetris_state->playField[k][m] =
                                (k == 0) ? false : tetris_state->playField[k - 1][m];
                        }
                    }
                }

                uint16_t oldNumLines = tetris_state->numLines;
                tetris_state->numLines += numLines;
                if((oldNumLines / 10) % 10 != (tetris_state->numLines / 10) % 10) {
                    tetris_state->fallSpeed -= 50;
                }
            }

            // Check for game over
            Piece* spawnedPiece = &shapes[rand() % 7];
            if(!tetris_game_is_valid_pos(tetris_state, spawnedPiece->p)) {
                tetris_state->gameState = GameStateGameOver;
            } else {
                memcpy(&tetris_state->currPiece, spawnedPiece, sizeof(tetris_state->currPiece));
                furi_timer_start(tetris_state->timer, tetris_state->fallSpeed);
            }
        }
    }

    if(tetris_game_is_valid_pos(tetris_state, newPiece->p)) {
        memcpy(&tetris_state->currPiece, newPiece, sizeof(tetris_state->currPiece));
    }

    tetris_game_render_curr_piece(tetris_state);
}

int32_t tetris_game_app() {
    srand(DWT->CYCCNT);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(TetrisEvent));

    TetrisState* tetris_state = malloc(sizeof(TetrisState));

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, tetris_state, sizeof(TetrisState))) {
        FURI_LOG_E("TetrisGame", "cannot create mutex\r\n");
        free(tetris_state);
        return 255;
    }

    // Not doing this eventually causes issues with TimerSvc due to not sleeping/yielding enough in this task
    TaskHandle_t timer_task = xTaskGetHandle(configTIMER_SERVICE_TASK_NAME);
    TaskHandle_t curr_task = xTaskGetHandle("Tetris Game");

    uint32_t origTimerPrio = uxTaskPriorityGet(timer_task);
    uint32_t myPrio = uxTaskPriorityGet(curr_task);
    vTaskPrioritySet(timer_task, myPrio + 1);

    ViewPort* view_port = view_port_alloc();
    view_port_set_orientation(view_port, ViewPortOrientationVertical);
    view_port_draw_callback_set(view_port, tetris_game_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, tetris_game_input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    tetris_state->timer =
        furi_timer_alloc(tetris_game_update_timer_callback, FuriTimerTypePeriodic, event_queue);
    tetris_game_init_state(tetris_state);

    TetrisEvent event;

    Piece* newPiece = malloc(sizeof(Piece));
    uint8_t downRepeatCounter = 0;

    for(bool processing = true; processing;) {
        // This 10U implicitly sets the game loop speed. downRepeatCounter relies on this value
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 10U);

        TetrisState* tetris_state = (TetrisState*)acquire_mutex_block(&state_mutex);

        memcpy(newPiece, &tetris_state->currPiece, sizeof(tetris_state->currPiece));
        bool wasDownMove = false;

        if(!furi_hal_gpio_read(&gpio_button_right)) {
            if(downRepeatCounter > 3) {
                for(int i = 0; i < 4; i++) {
                    newPiece->p[i].y += 1;
                }
                downRepeatCounter = 0;
                wasDownMove = true;
            } else {
                downRepeatCounter++;
            }
        }

        if(event_status == FuriStatusOk) {
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress || event.input.type == InputTypeLong ||
                   event.input.type == InputTypeRepeat) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        break;
                    case InputKeyDown:
                        break;
                    case InputKeyRight:
                        for(int i = 0; i < 4; i++) {
                            newPiece->p[i].x += 1;
                        }
                        break;
                    case InputKeyLeft:
                        for(int i = 0; i < 4; i++) {
                            newPiece->p[i].x -= 1;
                        }
                        break;
                    case InputKeyOk:
                        if(tetris_state->gameState == GameStatePlaying) {
                            tetris_game_remove_curr_piece(tetris_state);
                            tetris_game_try_rotation(tetris_state, newPiece);
                            tetris_game_render_curr_piece(tetris_state);
                        } else {
                            tetris_game_init_state(tetris_state);
                        }
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    }
                }
            } else if(event.type == EventTypeTick) {
                // TODO: This is inverted.  it returns true when the button is not pressed.
                // see macro in input.c and do that
                if(furi_hal_gpio_read(&gpio_button_right)) {
                    for(int i = 0; i < 4; i++) {
                        newPiece->p[i].y += 1;
                    }
                    wasDownMove = true;
                }
            }
        }

        tetris_game_process_step(tetris_state, newPiece, wasDownMove);

        view_port_update(view_port);
        release_mutex(&state_mutex, tetris_state);
    }

    furi_timer_free(tetris_state->timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);
    vTaskPrioritySet(timer_task, origTimerPrio);
    free(newPiece);
    free(tetris_state);

    return 0;
}