#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <gui/view.h>
#include <dolphin/dolphin.h>

#define TAG "TicTacToe"

typedef enum { EventTypeTick, EventTypeKey } EventType;

typedef struct {
    FuriTimer* timer;
    uint8_t selBoxX;
    uint8_t selBoxY;

    uint8_t selX;
    uint8_t selY;

    uint16_t scoreX;
    uint16_t scoreO;

    char player;

    char field[3][3];
    bool fieldx[3][3];

    uint8_t coords[3];

    bool button_state;

} TicTacToeState;

typedef struct {
    EventType type;
    InputEvent input;
} GameEvent;

void drawCross(Canvas* const canvas, uint8_t x, uint8_t y) {
    canvas_draw_line(canvas, x, y, x + 9, y + 9); // top left - bottom right slash
    canvas_draw_line(canvas, x + 9, y, x, y + 9); // down left - top right slash
}

void drawCircle(Canvas* const canvas, uint8_t x, uint8_t y) {
    canvas_draw_circle(canvas, x + 4, y + 5, 5);
}

void player_switch(TicTacToeState* ts) {
    if(ts->player == 'O') {
        ts->player = 'X';
    } else if(ts->player == 'X') {
        ts->player = 'O';
    }
}

void tictactoe_draw(Canvas* canvas, TicTacToeState* ts) {
    // Draws the game field
    canvas_draw_frame(canvas, 0, 0, 64, 64); // frame
    canvas_draw_line(canvas, 0, 21, 63, 21); // horizontal line
    canvas_draw_line(canvas, 0, 42, 63, 42); // horizontal line
    canvas_draw_line(canvas, 21, 0, 21, 63); // vertical line
    canvas_draw_line(canvas, 42, 0, 42, 63); // vertical line

    // Draws the game field elements (X or O)
    for(uint8_t i = 0; i <= 2; i++) {
        for(uint8_t j = 0; j <= 2; j++) {
            if(ts->field[i][j] == 'O') {
                drawCircle(canvas, ts->coords[i], ts->coords[j]);
            } else if(ts->field[i][j] == 'X') {
                drawCross(canvas, ts->coords[i], ts->coords[j]);
            }
        }
    }

    // Draws the selection box
    if(ts->selX == 1) {
        ts->selBoxX = 1;
    } else if(ts->selX == 2) {
        ts->selBoxX = 22;
    } else if(ts->selX == 3) {
        ts->selBoxX = 43;
    }

    if(ts->selY == 1) {
        ts->selBoxY = 1;
    } else if(ts->selY == 2) {
        ts->selBoxY = 22;
    } else if(ts->selY == 3) {
        ts->selBoxY = 43;
    }

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, ts->selBoxX, ts->selBoxY, 20, 20);
    canvas_draw_frame(canvas, ts->selBoxX + 1, ts->selBoxY + 1, 18, 18);

    // Draws the sidebar
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 81, 10, "SCORE");
    canvas_draw_str(canvas, 75, 24, "X:");

    char scoreXBuffer[10];
    snprintf(scoreXBuffer, sizeof(scoreXBuffer), "%d", ts->scoreX);
    canvas_draw_str(canvas, 88, 24, scoreXBuffer);
    canvas_draw_str(canvas, 75, 35, "O:");

    char scoreOBuffer[10];
    snprintf(scoreOBuffer, sizeof(scoreOBuffer), "%d", ts->scoreO);
    canvas_draw_str(canvas, 88, 35, scoreOBuffer);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 75, 46, "Player:");

    if(ts->player == 'X') {
        drawCross(canvas, 93, 50);
    } else if(ts->player == 'O') {
        drawCircle(canvas, 93, 50);
    }
}

void clear_game_field(TicTacToeState* ts) {
    // Clears the game field arrays
    for(uint8_t i = 0; i <= 2; i++) {
        for(uint8_t j = 0; j <= 2; j++) {
            ts->field[i][j] = ' ';
            ts->fieldx[i][j] = false;
        }
    }

    ts->selX = 2; // Centers the selection box on X axis
    ts->selY = 2; // Centers the selection box on Y axis
}

void reset_game_data(TicTacToeState* ts) {
    ts->scoreO = 0;
    ts->scoreX = 0;
    ts->player = 'X';
}

void draw_win(Canvas* canvas, char player, TicTacToeState* ts) {
    // Handles the score table
    if(player == 'X') {
        ts->scoreX++;
    } else if(player == 'O') {
        ts->scoreO++;
    }

    // Switches the players
    player_switch(ts);

    // Draws the board with players switched
    tictactoe_draw(canvas, ts);

    // Clear the game field
    clear_game_field(ts);

    // Draw the new board
    tictactoe_draw(canvas, ts);
}

static void tictactoe_state_init(TicTacToeState* tictactoe_state) {
    // Set the initial game state
    tictactoe_state->selX = 2;
    tictactoe_state->selY = 2;
    tictactoe_state->player = 'X';
    tictactoe_state->coords[0] = 6;
    tictactoe_state->coords[1] = 27;
    tictactoe_state->coords[2] = 48;
    tictactoe_state->button_state = false;

    clear_game_field(tictactoe_state);

    reset_game_data(tictactoe_state);
}

static void tictactoe_draw_callback(Canvas* const canvas, void* ctx) {
    TicTacToeState* ticst = acquire_mutex((ValueMutex*)ctx, 25);
    if(ticst == NULL) {
        return;
    }

    if(ticst->selX > 3) {
        ticst->selX = 3;
    } else if(ticst->selX < 1) {
        ticst->selX = 1;
    }

    if(ticst->selY > 3) {
        ticst->selY = 3;
    } else if(ticst->selY < 1) {
        ticst->selY = 1;
    }

    // Assigns the game field elements their value (X or O) when the OK button is pressed
    if(ticst->button_state) {
        ticst->button_state = false;
        for(uint8_t i = 0; i <= 2; i++) {
            for(uint8_t j = 0; j <= 2; j++) {
                if((ticst->selX == i + 1) && (ticst->selY == j + 1) &&
                   (ticst->fieldx[i][j] == false)) {
                    if(ticst->player == 'X') {
                        ticst->field[i][j] = 'X';
                        ticst->fieldx[i][j] = true;
                        player_switch(ticst);
                    } else if(ticst->player == 'O') {
                        ticst->field[i][j] = 'O';
                        ticst->fieldx[i][j] = true;
                        player_switch(ticst);
                    }
                }
            }
        }
    }

    // Checks the game field for winning combinations
    if((ticst->field[0][0] == 'X') && (ticst->field[1][0] == 'X') && (ticst->field[2][0] == 'X')) {
        draw_win(canvas, 'X', ticst);
    } else if(
        (ticst->field[0][1] == 'X') && (ticst->field[1][1] == 'X') &&
        (ticst->field[2][1] == 'X')) {
        draw_win(canvas, 'X', ticst);
    } else if(
        (ticst->field[0][2] == 'X') && (ticst->field[1][2] == 'X') &&
        (ticst->field[2][2] == 'X')) {
        draw_win(canvas, 'X', ticst);
    } else if(
        (ticst->field[0][0] == 'X') && (ticst->field[0][1] == 'X') &&
        (ticst->field[0][2] == 'X')) {
        draw_win(canvas, 'X', ticst);
    } else if(
        (ticst->field[1][0] == 'X') && (ticst->field[1][1] == 'X') &&
        (ticst->field[1][2] == 'X')) {
        draw_win(canvas, 'X', ticst);
    } else if(
        (ticst->field[2][0] == 'X') && (ticst->field[2][1] == 'X') &&
        (ticst->field[2][2] == 'X')) {
        draw_win(canvas, 'X', ticst);
    } else if(
        (ticst->field[0][0] == 'X') && (ticst->field[1][1] == 'X') &&
        (ticst->field[2][2] == 'X')) {
        draw_win(canvas, 'X', ticst);
    } else if(
        (ticst->field[2][0] == 'X') && (ticst->field[1][1] == 'X') &&
        (ticst->field[0][2] == 'X')) {
        draw_win(canvas, 'X', ticst);
    } else if(
        (ticst->field[0][0] == 'O') && (ticst->field[1][0] == 'O') &&
        (ticst->field[2][0] == 'O')) {
        draw_win(canvas, 'O', ticst);
    } else if(
        (ticst->field[0][1] == 'O') && (ticst->field[1][1] == 'O') &&
        (ticst->field[2][1] == 'O')) {
        draw_win(canvas, 'O', ticst);
    } else if(
        (ticst->field[0][2] == 'O') && (ticst->field[1][2] == 'O') &&
        (ticst->field[2][2] == 'O')) {
        draw_win(canvas, 'O', ticst);
    } else if(
        (ticst->field[0][0] == 'O') && (ticst->field[0][1] == 'O') &&
        (ticst->field[0][2] == 'O')) {
        draw_win(canvas, 'O', ticst);
    } else if(
        (ticst->field[1][0] == 'O') && (ticst->field[1][1] == 'O') &&
        (ticst->field[1][2] == 'O')) {
        draw_win(canvas, 'O', ticst);
    } else if(
        (ticst->field[2][0] == 'O') && (ticst->field[2][1] == 'O') &&
        (ticst->field[2][2] == 'O')) {
        draw_win(canvas, 'O', ticst);
    } else if(
        (ticst->field[0][0] == 'O') && (ticst->field[1][1] == 'O') &&
        (ticst->field[2][2] == 'O')) {
        draw_win(canvas, 'O', ticst);
    } else if(
        (ticst->field[2][0] == 'O') && (ticst->field[1][1] == 'O') &&
        (ticst->field[0][2] == 'O')) {
        draw_win(canvas, 'O', ticst);
    } else if(
        (ticst->fieldx[0][0] == true) && (ticst->fieldx[0][1] == true) &&
        (ticst->fieldx[0][2] == true) && (ticst->fieldx[1][0] == true) &&
        (ticst->fieldx[1][1] == true) && (ticst->fieldx[1][2] == true) &&
        (ticst->fieldx[2][0] == true) && (ticst->fieldx[2][1] == true) &&
        (ticst->fieldx[2][2] == true)) {
        draw_win(canvas, 'T', ticst);
    }

    tictactoe_draw(canvas, ticst);

    release_mutex((ValueMutex*)ctx, ticst);
}

static void tictactoe_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void tictactoe_update_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t tictactoe_game_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(GameEvent));

    TicTacToeState* tictactoe_state = malloc(sizeof(TicTacToeState));

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, tictactoe_state, sizeof(TicTacToeState))) {
        FURI_LOG_E(TAG, "Cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(tictactoe_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, tictactoe_draw_callback, &state_mutex);
    view_port_input_callback_set(view_port, tictactoe_input_callback, event_queue);

    tictactoe_state->timer =
        furi_timer_alloc(tictactoe_update_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(tictactoe_state->timer, furi_kernel_get_tick_frequency() / 22);

    tictactoe_state_init(tictactoe_state);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Call dolphin deed on game start
    DOLPHIN_DEED(DolphinDeedPluginGameStart);

    GameEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        TicTacToeState* tictactoe_state = (TicTacToeState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            // Key events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyBack:
                        processing = false;
                        break;
                    case InputKeyRight:
                        tictactoe_state->selX++;
                        break;
                    case InputKeyLeft:
                        tictactoe_state->selX--;
                        break;
                    case InputKeyUp:
                        tictactoe_state->selY--;
                        break;
                    case InputKeyDown:
                        tictactoe_state->selY++;
                        break;
                    case InputKeyOk:
                        tictactoe_state->button_state = true;
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, tictactoe_state);
    }

    furi_timer_free(tictactoe_state->timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);
    free(tictactoe_state);

    return 0;
}