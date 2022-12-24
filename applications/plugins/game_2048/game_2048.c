/* 
 * Copyright 2022 Eugene Kirzhanov
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 * 
 * Thanks to:
 *  - DroomOne: https://github.com/DroomOne/flipperzero-firmware
 *  - x27: https://github.com/x27/flipperzero-game15
 */

#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>

#include "digits.h"
#include "array_utils.h"

#define CELLS_COUNT 4
#define CELL_INNER_SIZE 14
#define FRAME_LEFT 10
#define FRAME_TOP 1
#define FRAME_SIZE 61

#define SAVING_DIRECTORY "/ext/apps/Games"
#define SAVING_FILENAME SAVING_DIRECTORY "/game_2048.save"

typedef enum {
    GameStateMenu,
    GameStateInProgress,
    GameStateGameOver,
} State;

typedef struct {
    State state;
    uint8_t table[CELLS_COUNT][CELLS_COUNT];
    uint32_t score;
    uint32_t moves;
    int8_t selected_menu_item;
    uint32_t top_score;
} GameState;

typedef struct {
    uint32_t points;
    bool is_table_updated;
} MoveResult;

#define MENU_ITEMS_COUNT 2
static const char* popup_menu_strings[] = {"Resume", "New Game"};

static void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

static void draw_frame(Canvas* canvas) {
    canvas_draw_frame(canvas, FRAME_LEFT, FRAME_TOP, FRAME_SIZE, FRAME_SIZE);

    uint8_t offs = FRAME_LEFT + CELL_INNER_SIZE + 1;
    for(uint8_t i = 0; i < CELLS_COUNT - 1; i++) {
        canvas_draw_line(canvas, offs, FRAME_TOP + 1, offs, FRAME_TOP + FRAME_SIZE - 2);
        offs += CELL_INNER_SIZE + 1;
    }
    offs = FRAME_TOP + CELL_INNER_SIZE + 1;
    for(uint8_t i = 0; i < CELLS_COUNT - 1; i++) {
        canvas_draw_line(canvas, FRAME_LEFT + 1, offs, FRAME_LEFT + FRAME_SIZE - 2, offs);
        offs += CELL_INNER_SIZE + 1;
    }
}

static void draw_digit(Canvas* canvas, uint8_t row, uint8_t column, uint8_t value) {
    if(value == 0) return;

    uint8_t left = FRAME_LEFT + 1 + (column * (CELL_INNER_SIZE + 1));
    uint8_t top = FRAME_TOP + 1 + (row * (CELL_INNER_SIZE + 1));

    for(uint8_t r = 0; r < CELL_INNER_SIZE; r++) {
        for(u_int8_t c = 0; c < CELL_INNER_SIZE; c++) {
            if(digits[value - 1][r][c] == 1) {
                canvas_draw_dot(canvas, left + c, top + r);
            }
        }
    }
}

static void draw_table(Canvas* canvas, const uint8_t table[CELLS_COUNT][CELLS_COUNT]) {
    for(uint8_t row = 0; row < CELLS_COUNT; row++) {
        for(uint8_t column = 0; column < CELLS_COUNT; column++) {
            draw_digit(canvas, row, column, table[row][column]);
        }
    }
}

static void gray_canvas(Canvas* const canvas) {
    canvas_set_color(canvas, ColorWhite);
    for(int x = 0; x < 128; x += 2) {
        for(int y = 0; y < 64; y++) {
            canvas_draw_dot(canvas, x + (y % 2 == 1 ? 0 : 1), y);
        }
    }
}

static void draw_callback(Canvas* const canvas, void* ctx) {
    const GameState* game_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(game_state == NULL) return;

    canvas_clear(canvas);

    draw_frame(canvas);
    draw_table(canvas, game_state->table);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 128, FRAME_TOP, AlignRight, AlignTop, "Score");
    canvas_draw_str_aligned(canvas, 128, FRAME_TOP + 20, AlignRight, AlignTop, "Moves");
    canvas_draw_str_aligned(canvas, 128, FRAME_TOP + 40, AlignRight, AlignTop, "Top Score");

    int bufSize = 12;
    char buf[bufSize];
    snprintf(buf, sizeof(buf), "%lu", game_state->score);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 128, FRAME_TOP + 10, AlignRight, AlignTop, buf);

    memset(buf, 0, bufSize);
    snprintf(buf, sizeof(buf), "%lu", game_state->moves);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 128, FRAME_TOP + 30, AlignRight, AlignTop, buf);

    memset(buf, 0, bufSize);
    snprintf(buf, sizeof(buf), "%lu", game_state->top_score);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 128, FRAME_TOP + 50, AlignRight, AlignTop, buf);

    if(game_state->state == GameStateMenu) {
        gray_canvas(canvas);

        canvas_set_color(canvas, ColorWhite);
        canvas_draw_rbox(canvas, 28, 16, 72, 32, 4);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_rframe(canvas, 28, 16, 72, 32, 4);

        for(int i = 0; i < MENU_ITEMS_COUNT; i++) {
            if(i == game_state->selected_menu_item) {
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_box(canvas, 34, 20 + 12 * i, 60, 12);
            }

            canvas_set_color(
                canvas, i == game_state->selected_menu_item ? ColorWhite : ColorBlack);
            canvas_draw_str_aligned(
                canvas, 64, 26 + 12 * i, AlignCenter, AlignCenter, popup_menu_strings[i]);
        }

    } else if(game_state->state == GameStateGameOver) {
        gray_canvas(canvas);

        bool record_broken = game_state->score > game_state->top_score;

        canvas_set_color(canvas, ColorWhite);
        canvas_draw_rbox(canvas, 14, 12, 100, 40, 4);

        canvas_set_color(canvas, ColorBlack);
        canvas_draw_line(canvas, 14, 26, 114, 26);
        canvas_draw_rframe(canvas, 14, 12, 100, 40, 4);

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 15, AlignCenter, AlignTop, "Game Over");

        canvas_set_font(canvas, FontSecondary);
        if(record_broken) {
            canvas_draw_str_aligned(canvas, 64, 29, AlignCenter, AlignTop, "New Top Score!!!");
        } else {
            canvas_draw_str_aligned(canvas, 64, 29, AlignCenter, AlignTop, "Your Score");
        }

        memset(buf, 0, bufSize);
        snprintf(buf, sizeof(buf), "%lu", game_state->score);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 48, AlignCenter, AlignBottom, buf);
    }

    release_mutex((ValueMutex*)ctx, game_state);
}

void calculate_move_to_left(uint8_t arr[], MoveResult* const move_result) {
    uint8_t index = 0;
    uint8_t next_index;
    uint8_t offset;
    bool was_moved;
    while(index < CELLS_COUNT - 1) {
        // find offset from [index] to next non-empty value
        offset = 1;
        while(index + offset < CELLS_COUNT && arr[index + offset] == 0) offset++;

        // if all remaining values in this row are empty then go to next row
        if(index + offset >= CELLS_COUNT) break;

        // if current cell is empty then shift all cells [index+offset .. CELLS_COUNT-1] to [index]
        if(arr[index] == 0) {
            was_moved = shift_array_to_left(CELLS_COUNT, arr, index, offset);
            if(was_moved) move_result->is_table_updated = true;
        }

        next_index = index + 1;
        if(arr[next_index] == 0) {
            // find offset from [next_index] to next non-empty value
            offset = 1;
            while(next_index + offset < CELLS_COUNT && arr[next_index + offset] == 0) offset++;

            // if all remaining values in this row are empty then go to next row
            if(next_index + offset >= CELLS_COUNT) break;

            // if next cell is empty then shift cells [next_index+offset .. CELLS_COUNT-1] to [next_index]
            was_moved = shift_array_to_left(CELLS_COUNT, arr, next_index, offset);
            if(was_moved) move_result->is_table_updated = true;
        }

        if(arr[index] == arr[next_index]) {
            arr[index]++;
            shift_array_to_left(CELLS_COUNT, arr, next_index, 1);

            move_result->is_table_updated = true;
            move_result->points += 2 << (arr[index] - 1);
        }

        index++;
    }
}

void move_left(uint8_t table[CELLS_COUNT][CELLS_COUNT], MoveResult* const move_result) {
    for(uint8_t row_index = 0; row_index < CELLS_COUNT; row_index++) {
        calculate_move_to_left(table[row_index], move_result);
    }
}

void move_right(uint8_t table[CELLS_COUNT][CELLS_COUNT], MoveResult* const move_result) {
    for(uint8_t row_index = 0; row_index < CELLS_COUNT; row_index++) {
        reverse_array(CELLS_COUNT, table[row_index]);
        calculate_move_to_left(table[row_index], move_result);
        reverse_array(CELLS_COUNT, table[row_index]);
    }
}

void move_up(uint8_t table[CELLS_COUNT][CELLS_COUNT], MoveResult* const move_result) {
    uint8_t column[CELLS_COUNT];
    for(uint8_t column_index = 0; column_index < CELLS_COUNT; column_index++) {
        get_column_from_array(CELLS_COUNT, CELLS_COUNT, table, column_index, column);
        calculate_move_to_left(column, move_result);
        set_column_to_array(CELLS_COUNT, CELLS_COUNT, table, column_index, column);
    }
}

void move_down(uint8_t table[CELLS_COUNT][CELLS_COUNT], MoveResult* const move_result) {
    uint8_t column[CELLS_COUNT];
    for(uint8_t column_index = 0; column_index < CELLS_COUNT; column_index++) {
        get_column_from_array(CELLS_COUNT, CELLS_COUNT, table, column_index, column);
        reverse_array(CELLS_COUNT, column);
        calculate_move_to_left(column, move_result);
        reverse_array(CELLS_COUNT, column);
        set_column_to_array(CELLS_COUNT, CELLS_COUNT, table, column_index, column);
    }
}

void add_new_digit(GameState* const game_state) {
    uint8_t empty_cell_indexes[CELLS_COUNT * CELLS_COUNT];
    uint8_t empty_cells_count = 0;
    for(u_int8_t i = 0; i < CELLS_COUNT; i++) {
        for(u_int8_t j = 0; j < CELLS_COUNT; j++) {
            if(game_state->table[i][j] == 0) {
                empty_cell_indexes[empty_cells_count++] = i * CELLS_COUNT + j;
            }
        }
    }
    if(empty_cells_count == 0) return;

    int random_empty_cell_index = empty_cell_indexes[random() % empty_cells_count];
    u_int8_t row = random_empty_cell_index / CELLS_COUNT;
    u_int8_t col = random_empty_cell_index % CELLS_COUNT;

    int random_value_percent = random() % 100;
    game_state->table[row][col] = random_value_percent < 90 ? 1 : 2; // 90% for 2, 25% for 4
}

void init_game(GameState* const game_state, bool clear_top_score) {
    memset(game_state->table, 0, CELLS_COUNT * CELLS_COUNT * sizeof(uint8_t));
    add_new_digit(game_state);
    add_new_digit(game_state);

    game_state->score = 0;
    game_state->moves = 0;
    game_state->state = GameStateInProgress;
    game_state->selected_menu_item = 0;
    if(clear_top_score) {
        game_state->top_score = 0;
    }
}

bool load_game(GameState* game_state) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    File* file = storage_file_alloc(storage);
    uint16_t bytes_readed = 0;
    if(storage_file_open(file, SAVING_FILENAME, FSAM_READ, FSOM_OPEN_EXISTING)) {
        bytes_readed = storage_file_read(file, game_state, sizeof(GameState));
    }
    storage_file_close(file);
    storage_file_free(file);

    furi_record_close(RECORD_STORAGE);

    return bytes_readed == sizeof(GameState);
}

void save_game(GameState* game_state) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    if(storage_common_stat(storage, SAVING_DIRECTORY, NULL) == FSE_NOT_EXIST) {
        if(!storage_simply_mkdir(storage, SAVING_DIRECTORY)) {
            return;
        }
    }

    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, SAVING_FILENAME, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        storage_file_write(file, game_state, sizeof(GameState));
    }
    storage_file_close(file);
    storage_file_free(file);

    furi_record_close(RECORD_STORAGE);
}

bool is_game_over(GameState* const game_state) {
    FURI_LOG_I("is_game_over", "====check====");

    // check if table contains at least one empty cell
    for(uint8_t i = 0; i < CELLS_COUNT; i++) {
        for(u_int8_t j = 0; j < CELLS_COUNT; j++) {
            if(game_state->table[i][j] == 0) {
                FURI_LOG_I("is_game_over", "has empty cells");
                return false;
            }
        }
    }
    FURI_LOG_I("is_game_over", "no empty cells");

    uint8_t tmp_table[CELLS_COUNT][CELLS_COUNT];
    MoveResult* tmp_move_result = malloc(sizeof(MoveResult));

    // check if we can move to any direction
    memcpy(tmp_table, game_state->table, CELLS_COUNT * CELLS_COUNT * sizeof(uint8_t));
    move_left(tmp_table, tmp_move_result);
    if(tmp_move_result->is_table_updated) return false;
    FURI_LOG_I("is_game_over", "can't move left");

    memcpy(tmp_table, game_state->table, CELLS_COUNT * CELLS_COUNT * sizeof(uint8_t));
    move_right(tmp_table, tmp_move_result);
    if(tmp_move_result->is_table_updated) return false;
    FURI_LOG_I("is_game_over", "can't move right");

    memcpy(tmp_table, game_state->table, CELLS_COUNT * CELLS_COUNT * sizeof(uint8_t));
    move_up(tmp_table, tmp_move_result);
    if(tmp_move_result->is_table_updated) return false;
    FURI_LOG_I("is_game_over", "can't move up");

    memcpy(tmp_table, game_state->table, CELLS_COUNT * CELLS_COUNT * sizeof(uint8_t));
    move_down(tmp_table, tmp_move_result);
    if(tmp_move_result->is_table_updated) return false;
    FURI_LOG_I("is_game_over", "can't move down");

    return true;
}

int32_t game_2048_app() {
    GameState* game_state = malloc(sizeof(GameState));
    if(!load_game(game_state)) {
        init_game(game_state, true);
    }

    MoveResult* move_result = malloc(sizeof(MoveResult));

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, game_state, sizeof(GameState))) {
        FURI_LOG_E("SnakeGame", "cannot create mutex\r\n");
        free(game_state);
        return 255;
    }

    InputEvent input;
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    bool is_finished = false;
    while(!is_finished) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &input, FuriWaitForever);
        if(event_status == FuriStatusOk) {
            // handle only press event, ignore repeat/release events
            if(input.type != InputTypePress) continue;

            GameState* game_state = (GameState*)acquire_mutex_block(&state_mutex);

            switch(game_state->state) {
            case GameStateMenu:

                switch(input.key) {
                case InputKeyUp:
                    game_state->selected_menu_item--;
                    if(game_state->selected_menu_item < 0) {
                        game_state->selected_menu_item = MENU_ITEMS_COUNT - 1;
                    }
                    break;
                case InputKeyDown:
                    game_state->selected_menu_item++;
                    if(game_state->selected_menu_item >= MENU_ITEMS_COUNT) {
                        game_state->selected_menu_item = 0;
                    }
                    break;
                case InputKeyOk:
                    if(game_state->selected_menu_item == 1) {
                        // new game
                        init_game(game_state, false);
                        save_game(game_state);
                    }
                    game_state->state = GameStateInProgress;
                    break;
                case InputKeyBack:
                    game_state->state = GameStateInProgress;
                    break;
                default:
                    break;
                }

                break;
            case GameStateInProgress:
                move_result->is_table_updated = false;
                move_result->points = 0;

                switch(input.key) {
                case InputKeyLeft:
                    move_left(game_state->table, move_result);
                    break;
                case InputKeyRight:
                    move_right(game_state->table, move_result);
                    break;
                case InputKeyUp:
                    move_up(game_state->table, move_result);
                    break;
                case InputKeyDown:
                    move_down(game_state->table, move_result);
                    break;
                case InputKeyOk:
                    game_state->state = GameStateMenu;
                    game_state->selected_menu_item = 0;
                    break;
                case InputKeyBack:
                    save_game(game_state);
                    is_finished = true;
                    break;
                case InputKeyMAX:
                    break;
                }
                game_state->score += move_result->points;

                if(move_result->is_table_updated) {
                    game_state->moves++;
                    add_new_digit(game_state);
                }

                if(is_game_over(game_state)) {
                    game_state->state = GameStateGameOver;
                    if(game_state->score >= game_state->top_score) {
                        game_state->top_score = game_state->score;
                    }
                }

                break;
            case GameStateGameOver:
                if(input.key == InputKeyOk || input.key == InputKeyBack) {
                    init_game(game_state, false);
                    save_game(game_state);
                }
            }

            view_port_update(view_port);
            release_mutex(&state_mutex, game_state);
        }
    }

    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);

    view_port_free(view_port);

    furi_message_queue_free(event_queue);

    delete_mutex(&state_mutex);

    free(game_state);
    free(move_result);

    return 0;
}
