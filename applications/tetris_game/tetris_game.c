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
typedef enum {
    OffsetTypeCommon,
    OffsetTypeI,
    OffsetTypeO
} OffsetType;

// Since we only support rotating clockwise, these are actual translation values,
// not values to be subtracted to get translation values

static const Point rotOffsetTranslation[3][4][5] = {
    {
        { {0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2} },
        { {0,0}, {1,0}, {1,1}, {0,-2}, {1,-2} },
        { {0,0}, {1,0}, {1,-1}, {0,2}, {1,2} },
        { {0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2} }
    },
    {
        { {1,0}, {-1,0}, {2,0}, {-1,1}, {2,-2} },
        { {0,1}, {-1,1}, {2,1}, {-1,-1}, {2,2} },
        { {-1,0}, {1,0}, {-2,0}, {1,-1}, {-2,2} },
        { {0,-1}, {1,-1}, {-2,-1}, {1,1}, {-2,-2} }
    },
    {
        { {0,-1}, {0,0}, {0,0}, {0,0}, {0,0} },
        { {1,0}, {0,0}, {0,0}, {0,0}, {0,0} },
        { {0,1}, {0,0}, {0,0}, {0,0}, {0,0} },
        { {-1,0}, {0,0}, {0,0}, {0,0}, {0,0} }
    }
};

typedef struct {
    Point p[4];
    uint8_t rotIdx;
    OffsetType offsetType;
} Piece;

// Shapes @ spawn locations, rotation point first
static const Piece shapes[] = {
    { .p = {{5, 1}, {4, 0}, {5, 0}, {6, 1}}, .rotIdx = 0, .offsetType = OffsetTypeCommon }, // Z
    { .p = {{5, 1}, {4, 1}, {5, 0}, {6, 0}}, .rotIdx = 0, .offsetType = OffsetTypeCommon }, // S
    { .p = {{5, 1}, {4, 1}, {6, 1}, {6, 0}}, .rotIdx = 0, .offsetType = OffsetTypeCommon }, // L
    { .p = {{5, 1}, {4, 0}, {4, 1}, {6, 1}}, .rotIdx = 0, .offsetType = OffsetTypeCommon }, // J
    { .p = {{5, 1}, {4, 1}, {5, 0}, {6, 1}}, .rotIdx = 0, .offsetType = OffsetTypeCommon }, // T
    { .p = {{5, 1}, {4, 1}, {6, 1}, {7, 1}}, .rotIdx = 0, .offsetType = OffsetTypeI }, // I
    { .p = {{5, 1}, {5, 0}, {6, 0}, {6, 1}}, .rotIdx = 0, .offsetType = OffsetTypeO } // O
};

typedef struct {
    bool playField[FIELD_HEIGHT][FIELD_WIDTH];
    Piece currPiece;
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

    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            if (tetris_state->playField[y][x]) {
                uint16_t xOffset = x * 5;
                uint16_t yOffset = y * 5;

                canvas_draw_rframe(
                    canvas, 
                    BORDER_OFFSET + MARGIN_OFFSET + xOffset, 
                    BORDER_OFFSET + MARGIN_OFFSET + yOffset - 1, 
                    BLOCK_WIDTH, 
                    BLOCK_HEIGHT,
                    1
                );
                canvas_draw_dot(
                    canvas, 
                    BORDER_OFFSET + MARGIN_OFFSET + xOffset + 2,
                    BORDER_OFFSET + MARGIN_OFFSET + yOffset + 1
                );
                canvas_draw_dot(
                    canvas, 
                    BORDER_OFFSET + MARGIN_OFFSET + xOffset + 3,
                    BORDER_OFFSET + MARGIN_OFFSET + yOffset + 1
                );
                canvas_draw_dot(
                    canvas, 
                    BORDER_OFFSET + MARGIN_OFFSET + xOffset + 2,
                    BORDER_OFFSET + MARGIN_OFFSET + yOffset + 2
                );
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
    release_mutex((ValueMutex *)ctx, tetris_state);
 }

static void tetris_game_input_callback(InputEvent* input_event, osMessageQueueId_t event_queue) {
    furi_assert(event_queue);

    TetrisEvent event = {.type = EventTypeKey, .input = *input_event};
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

static void tetris_game_init_state(TetrisState* tetris_state) {
    memset(tetris_state->playField, 0, sizeof(tetris_state->playField));

    memcpy(&tetris_state->currPiece, &shapes[rand() % 7], sizeof(tetris_state->currPiece));
}

static void tetris_game_remove_curr_piece(TetrisState* tetris_state) {
    for (int i = 0; i < 4; i++) {
        uint8_t x = tetris_state->currPiece.p[i].x;
        uint8_t y = tetris_state->currPiece.p[i].y;

        tetris_state->playField[y][x] = false;
    }
}

static void tetris_game_render_curr_piece(TetrisState* tetris_state) {
     for (int i = 0; i < 4; i++) {
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

    for (int i = 1; i < 4; i++) {
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
    for (int i = 0; i < 4; i++) {
        if(shape[i].x < 0 || shape[i].x > (FIELD_WIDTH - 1) || tetris_state->playField[shape[i].y][shape[i].x] == true) {
            return false;
        }
    }
    return true;
}

static void tetris_game_try_rotation(TetrisState* tetris_state, Piece *newPiece) {
    uint8_t currRotIdx = tetris_state->currPiece.rotIdx;

    Point *rotatedShape = malloc(sizeof(Point) * 4);
    Point *kickedShape = malloc(sizeof(Point) * 4);

    memcpy(rotatedShape, &tetris_state->currPiece.p, sizeof(tetris_state->currPiece.p));

    tetris_game_rotate_shape(tetris_state->currPiece.p, rotatedShape);

    for(int i = 0; i < 5; i++) {
        memcpy(kickedShape, rotatedShape, (sizeof(Point) * 4));
        tetris_game_apply_kick(kickedShape, rotOffsetTranslation[newPiece->offsetType][currRotIdx][i]);

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
        if(row[i] == false)
            return false;
    }
    return true;
}

static void tetris_game_check_for_lines(TetrisState* tetris_state, uint8_t* lines, uint8_t* numLines) {
    for(int i = 0; i < FIELD_HEIGHT; i++) {
        if(tetris_game_row_is_line(tetris_state->playField[i])) {
            *(lines++) = i;
            *numLines += 1;
        }
    }
}

static bool tetris_game_piece_at_bottom(TetrisState* tetris_state, Piece* newPiece) {
    for (int i = 0; i < 4; i++) {
        Point *pos = (Point *)&newPiece->p;
        if (pos[i].y >= FIELD_HEIGHT || tetris_state->playField[pos[i].y][pos[i].x] == true) {
            return true;
        }
    }
    return false;
}

static void tetris_game_update_timer_callback(osMessageQueueId_t event_queue) {
    furi_assert(event_queue);

    TetrisEvent event = {.type = EventTypeTick};
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}


int32_t tetris_game_app(void* p) {
    (void)p;
    srand(DWT->CYCCNT);

    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(TetrisEvent), NULL);

    TetrisState* tetris_state = malloc(sizeof(TetrisState));
    tetris_game_init_state(tetris_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, tetris_state, sizeof(TetrisState))) {
        FURI_LOG_E("TetrisGame", "cannot create mutex\r\n");
        free(tetris_state);
        return 255;
    }

    ViewPort* view_port = view_port_alloc();
    view_port_set_orientation(view_port, ViewPortOrientationVertical);
    view_port_draw_callback_set(view_port, tetris_game_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, tetris_game_input_callback, event_queue);

    osTimerId_t timer =
        osTimerNew(tetris_game_update_timer_callback, osTimerPeriodic, event_queue, NULL);
    osTimerStart(timer, 500U);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    TetrisEvent event;
    // Point *newShape = malloc(sizeof(Point) * 4);
    Piece *newPiece = malloc(sizeof(Piece));

    for(bool processing = true; processing;) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 100);

        TetrisState* tetris_state = (TetrisState*)acquire_mutex_block(&state_mutex);

        memcpy(newPiece, &tetris_state->currPiece, sizeof(tetris_state->currPiece));
        bool wasDownMove = false;

        if(event_status == osOK) {
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress || event.input.type == InputTypeLong || event.input.type == InputTypeRepeat) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        
                        break;
                    case InputKeyDown:
                        for (int i = 0; i < 4; i++) {
                            newPiece->p[i].y += 1;
                        }
                        wasDownMove = true;
                        break;
                    case InputKeyRight:
                        for (int i = 0; i < 4; i++) {
                            newPiece->p[i].x += 1;
                        }
                        break;
                    case InputKeyLeft:
                        for (int i = 0; i < 4; i++) {
                            newPiece->p[i].x -= 1;
                        }
                        break;
                    case InputKeyOk:
                        tetris_game_remove_curr_piece(tetris_state);
                        tetris_game_try_rotation(tetris_state, newPiece);
                        tetris_game_render_curr_piece(tetris_state);
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
                    for (int i = 0; i < 4; i++) {
                       newPiece->p[i].y += 1;
                    }
                    wasDownMove = true;
                }
            }
        }

        tetris_game_remove_curr_piece(tetris_state);

        if(wasDownMove) {
            if(tetris_game_piece_at_bottom(tetris_state, newPiece)) {
                tetris_game_render_curr_piece(tetris_state);
                uint8_t numLines = 0;
                uint8_t lines[] = { 0,0,0,0 };

                tetris_game_check_for_lines(tetris_state, lines, &numLines);
                for(int i = 0; i < numLines; i++) {

                    // zero/falsify out row
                    for(int j = 0; j < FIELD_WIDTH; j++) {
                        tetris_state->playField[lines[i]][j] = false;
                    }
                    // move all above rows down
                    for(int k = lines[i]; k >= 0 ; k--) {
                        for(int m = 0; m < FIELD_WIDTH; m++) {
                            tetris_state->playField[k][m] = (k == 0) ? false : tetris_state->playField[k-1][m];
                        }
                    }
                }

                memcpy(&tetris_state->currPiece, &shapes[rand() % 7], sizeof(tetris_state->currPiece));
            }
        }

        if(tetris_game_is_valid_pos(tetris_state, newPiece->p)) {
            memcpy(&tetris_state->currPiece, newPiece, sizeof(tetris_state->currPiece));
        }

         tetris_game_render_curr_piece(tetris_state);

        view_port_update(view_port);
        release_mutex(&state_mutex, tetris_state);
    }

    osTimerDelete(timer);  
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);
    delete_mutex(&state_mutex);
    free(newPiece);
    free(tetris_state);

    return 0;
}