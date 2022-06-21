#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <notification/notification_messages.h>
#include <gui/view.h>

#define TAG "TicTacToe"

uint8_t selBoxX;
uint8_t selBoxY;

uint8_t selX = 2;
uint8_t selY = 2;

uint16_t scoreX;
uint16_t scoreO;

char player = 'X';

char field[3][3];
bool fieldx[3][3];

const uint8_t coords[3] = {6, 27, 48};

bool button_state = false;

typedef enum { EventTypeTick, EventTypeKey } EventType;

typedef enum { DirectionUp, DirectionRight, DirectionDown, DirectionLeft } Direction;

typedef enum { GameStatePlaying, GameStateGameOver } GameState;

typedef struct {
    GameState game_state;
    osTimerId_t timer;
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

void player_switch() {
    if(player == 'O') {
        player = 'X';
    } else if(player == 'X') {
        player = 'O';
    }
}

void tictactoe_draw(Canvas* canvas) {
    // Draws the game field
    canvas_draw_frame(canvas, 0, 0, 64, 64); // frame
    canvas_draw_line(canvas, 0, 21, 63, 21); // horizontal line
    canvas_draw_line(canvas, 0, 42, 63, 42); // horizontal line
    canvas_draw_line(canvas, 21, 0, 21, 63); // vertical line
    canvas_draw_line(canvas, 42, 0, 42, 63); // vertical line

    // Draws the game field elements (X or O)
    for(uint8_t i = 0; i <= 2; i++) {
        for(uint8_t j = 0; j <= 2; j++) {
            if(field[i][j] == 'O') {
                drawCircle(canvas, coords[i], coords[j]);
            } else if(field[i][j] == 'X') {
                drawCross(canvas, coords[i], coords[j]);
            }
        }
    }

    // Draws the selection box
    if(selX == 1) {
        selBoxX = 1;
    } else if(selX == 2) {
        selBoxX = 22;
    } else if(selX == 3) {
        selBoxX = 43;
    }

    if(selY == 1) {
        selBoxY = 1;
    } else if(selY == 2) {
        selBoxY = 22;
    } else if(selY == 3) {
        selBoxY = 43;
    }

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, selBoxX, selBoxY, 20, 20);
    canvas_draw_frame(canvas, selBoxX + 1, selBoxY + 1, 18, 18);

    // Draws the sidebar
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 81, 10, "SCORE");
    canvas_draw_str(canvas, 75, 24, "X:");

    char scoreXBuffer[10];
    sprintf(scoreXBuffer, "%d", scoreX);
    canvas_draw_str(canvas, 88, 24, scoreXBuffer);
    canvas_draw_str(canvas, 75, 35, "O:");

    char scoreOBuffer[10];
    sprintf(scoreOBuffer, "%d", scoreO);
    canvas_draw_str(canvas, 88, 35, scoreOBuffer);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 75, 46, "Player:");

    if(player == 'X') {
        drawCross(canvas, 93, 50);
    } else if(player == 'O') {
        drawCircle(canvas, 93, 50);
    }
}

void clear_game_field() {
    // Clears the game field arrays
    for(uint8_t i = 0; i <= 2; i++) {
        for(uint8_t j = 0; j <= 2; j++) {
            field[i][j] = ' ';
            fieldx[i][j] = false;
        }
    }

    selX = 2; // Centers the selection box on X axis
    selY = 2; // Centers the selection box on Y axis
}

void reset_game_data() {
    scoreO = 0;
    scoreX = 0;
    player = 'X';
}

void draw_win(Canvas* canvas, char player) {
    // Handles the score table
    if(player == 'X') {
        scoreX++;
    } else if(player == 'O') {
        scoreO++;
    }

    // Switches the players
    player_switch();

    // Draws the board with players switched
    tictactoe_draw(canvas);

    // Clear the game field
    clear_game_field();

    // Draw the new board
    tictactoe_draw(canvas);
}

static void tictactoe_state_init(TicTacToeState* const tictactoe_state) {
    // Set the initial game state
    tictactoe_state->game_state = GameStatePlaying;

    clear_game_field();

    reset_game_data();
}

static void tictactoe_draw_callback(Canvas* const canvas, void* ctx) {
    const TicTacToeState* tictactoe_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(tictactoe_state == NULL) {
        return;
    }

    if(selX > 3) {
        selX = 3;
    } else if(selX < 1) {
        selX = 1;
    }

    if(selY > 3) {
        selY = 3;
    } else if(selY < 1) {
        selY = 1;
    }

    // Assigns the game field elements their value (X or O) when the OK button is pressed
    if(button_state) {
        button_state = false;
        for(uint8_t i = 0; i <= 2; i++) {
            for(uint8_t j = 0; j <= 2; j++) {
                if((selX == i + 1) && (selY == j + 1) && (fieldx[i][j] == false)) {
                    if(player == 'X') {
                        field[i][j] = 'X';
                        fieldx[i][j] = true;
                        player_switch();
                    } else if(player == 'O') {
                        field[i][j] = 'O';
                        fieldx[i][j] = true;
                        player_switch();
                    }
                }
            }
        }
    }

    // Checks the game field for winning combinations
    if((field[0][0] == 'X') && (field[1][0] == 'X') && (field[2][0] == 'X')) {
        draw_win(canvas, 'X');
    } else if((field[0][1] == 'X') && (field[1][1] == 'X') && (field[2][1] == 'X')) {
        draw_win(canvas, 'X');
    } else if((field[0][2] == 'X') && (field[1][2] == 'X') && (field[2][2] == 'X')) {
        draw_win(canvas, 'X');
    } else if((field[0][0] == 'X') && (field[0][1] == 'X') && (field[0][2] == 'X')) {
        draw_win(canvas, 'X');
    } else if((field[1][0] == 'X') && (field[1][1] == 'X') && (field[1][2] == 'X')) {
        draw_win(canvas, 'X');
    } else if((field[2][0] == 'X') && (field[2][1] == 'X') && (field[2][2] == 'X')) {
        draw_win(canvas, 'X');
    } else if((field[0][0] == 'X') && (field[1][1] == 'X') && (field[2][2] == 'X')) {
        draw_win(canvas, 'X');
    } else if((field[2][0] == 'X') && (field[1][1] == 'X') && (field[0][2] == 'X')) {
        draw_win(canvas, 'X');
    } else if((field[0][0] == 'O') && (field[1][0] == 'O') && (field[2][0] == 'O')) {
        draw_win(canvas, 'O');
    } else if((field[0][1] == 'O') && (field[1][1] == 'O') && (field[2][1] == 'O')) {
        draw_win(canvas, 'O');
    } else if((field[0][2] == 'O') && (field[1][2] == 'O') && (field[2][2] == 'O')) {
        draw_win(canvas, 'O');
    } else if((field[0][0] == 'O') && (field[0][1] == 'O') && (field[0][2] == 'O')) {
        draw_win(canvas, 'O');
    } else if((field[1][0] == 'O') && (field[1][1] == 'O') && (field[1][2] == 'O')) {
        draw_win(canvas, 'O');
    } else if((field[2][0] == 'O') && (field[2][1] == 'O') && (field[2][2] == 'O')) {
        draw_win(canvas, 'O');
    } else if((field[0][0] == 'O') && (field[1][1] == 'O') && (field[2][2] == 'O')) {
        draw_win(canvas, 'O');
    } else if((field[2][0] == 'O') && (field[1][1] == 'O') && (field[0][2] == 'O')) {
        draw_win(canvas, 'O');
    } else if(
        (fieldx[0][0] == true) && (fieldx[0][1] == true) && (fieldx[0][2] == true) &&
        (fieldx[1][0] == true) && (fieldx[1][1] == true) && (fieldx[1][2] == true) &&
        (fieldx[2][0] == true) && (fieldx[2][1] == true) && (fieldx[2][2] == true)) {
        draw_win(canvas, 'T');
    }

    tictactoe_draw(canvas);

    release_mutex((ValueMutex*)ctx, tictactoe_state);
}

static void tictactoe_input_callback(InputEvent* input_event, osMessageQueueId_t event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeKey, .input = *input_event};
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

static void tictactoe_update_timer_callback(osMessageQueueId_t event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeTick};
    osMessageQueuePut(event_queue, &event, 0, 0);
}

int32_t tictactoe_game_app(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(GameEvent), NULL);

    TicTacToeState* tictactoe_state = malloc(sizeof(TicTacToeState));

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, tictactoe_state, sizeof(TicTacToeState))) {
        FURI_LOG_E(TAG, "Cannot create mutex\r\n");
        free(tictactoe_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, tictactoe_draw_callback, &state_mutex);
    view_port_input_callback_set(view_port, tictactoe_input_callback, event_queue);

    tictactoe_state->timer =
        osTimerNew(tictactoe_update_timer_callback, osTimerPeriodic, event_queue, NULL);
    osTimerStart(tictactoe_state->timer, osKernelGetTickFreq() / 22);

    tictactoe_state_init(tictactoe_state);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    GameEvent event;
    for(bool processing = true; processing;) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 100);
        TicTacToeState* tictactoe_state = (TicTacToeState*)acquire_mutex_block(&state_mutex);

        if(event_status == osOK) {
            // Key events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyBack:
                        processing = false;
                        break;
                    case InputKeyRight:
                        selX++;
                        break;
                    case InputKeyLeft:
                        selX--;
                        break;
                    case InputKeyUp:
                        selY--;
                        break;
                    case InputKeyDown:
                        selY++;
                        break;
                    case InputKeyOk:
                        button_state = true;
                        break;
                    }
                }
            }
        } else {
            // Event timeout
            FURI_LOG_D(TAG, "osMessageQueue: Event timeout");
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, tictactoe_state);
    }

    osTimerDelete(tictactoe_state->timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);
    furi_record_close("notification");
    delete_mutex(&state_mutex);
    free(tictactoe_state);

    return 0;
}