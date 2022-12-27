#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

#include <notification/notification_messages.h>
#include <dialogs/dialogs.h>
#include <m-string.h>

#include "assets.h"

#define PLAYFIELD_WIDTH 16
#define PLAYFIELD_HEIGHT 7
#define TILE_WIDTH 8
#define TILE_HEIGHT 8

#define MINECOUNT 20

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef enum {
    TileType0, // this HAS to be in order, for hint assignment to be ez pz
    TileType1,
    TileType2,
    TileType3,
    TileType4,
    TileType5,
    TileType6,
    TileType7,
    TileType8,
    TileTypeUncleared,
    TileTypeFlag,
    TileTypeMine
} TileType;

typedef enum { FieldEmpty, FieldMine } Field;

typedef struct {
    Field minefield[PLAYFIELD_WIDTH][PLAYFIELD_HEIGHT];
    TileType playfield[PLAYFIELD_WIDTH][PLAYFIELD_HEIGHT];
    FuriTimer* timer;
    int cursor_x;
    int cursor_y;
    int mines_left;
    int fields_cleared;
    int flags_set;
    bool game_started;
    uint32_t game_started_tick;
} Minesweeper;

static void timer_callback(void* ctx) {
    UNUSED(ctx);
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message(notification, &sequence_reset_vibro);
    furi_record_close(RECORD_NOTIFICATION);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void render_callback(Canvas* const canvas, void* ctx) {
    const Minesweeper* minesweeper_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(minesweeper_state == NULL) {
        return;
    }
    FuriString* mineStr;
    FuriString* timeStr;
    mineStr = furi_string_alloc();
    timeStr = furi_string_alloc();

    furi_string_printf(mineStr, "Mines: %d", MINECOUNT - minesweeper_state->flags_set);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, furi_string_get_cstr(mineStr));

    int seconds = 0;
    int minutes = 0;
    if(minesweeper_state->game_started) {
        uint32_t ticks_elapsed = furi_get_tick() - minesweeper_state->game_started_tick;
        seconds = (int)ticks_elapsed / furi_kernel_get_tick_frequency();
        minutes = (int)seconds / 60;
        seconds = seconds % 60;
    }
    furi_string_printf(timeStr, "%01d:%02d", minutes, seconds);
    canvas_draw_str_aligned(canvas, 128, 0, AlignRight, AlignTop, furi_string_get_cstr(timeStr));

    uint8_t* tile_to_draw;

    for(int y = 0; y < PLAYFIELD_HEIGHT; y++) {
        for(int x = 0; x < PLAYFIELD_WIDTH; x++) {
            if(x == minesweeper_state->cursor_x && y == minesweeper_state->cursor_y) {
                canvas_invert_color(canvas);
            }
            switch(minesweeper_state->playfield[x][y]) {
            case TileType0:
                tile_to_draw = tile_0_bits;
                break;
            case TileType1:
                tile_to_draw = tile_1_bits;
                break;
            case TileType2:
                tile_to_draw = tile_2_bits;
                break;
            case TileType3:
                tile_to_draw = tile_3_bits;
                break;
            case TileType4:
                tile_to_draw = tile_4_bits;
                break;
            case TileType5:
                tile_to_draw = tile_5_bits;
                break;
            case TileType6:
                tile_to_draw = tile_6_bits;
                break;
            case TileType7:
                tile_to_draw = tile_7_bits;
                break;
            case TileType8:
                tile_to_draw = tile_8_bits;
                break;
            case TileTypeFlag:
                tile_to_draw = tile_flag_bits;
                break;
            case TileTypeUncleared:
                tile_to_draw = tile_uncleared_bits;
                break;
            case TileTypeMine:
                tile_to_draw = tile_mine_bits;
                break;
            default:
                // this should never happen
                tile_to_draw = tile_mine_bits;
                break;
            }
            canvas_draw_xbm(
                canvas,
                x * TILE_HEIGHT, // x
                8 + (y * TILE_WIDTH), // y
                TILE_WIDTH,
                TILE_HEIGHT,
                tile_to_draw);
            if(x == minesweeper_state->cursor_x && y == minesweeper_state->cursor_y) {
                canvas_invert_color(canvas);
            }
        }
    }

    furi_string_free(mineStr);
    furi_string_free(timeStr);
    release_mutex((ValueMutex*)ctx, minesweeper_state);
}

static void setup_playfield(Minesweeper* minesweeper_state) {
    int mines_left = MINECOUNT;
    for(int y = 0; y < PLAYFIELD_HEIGHT; y++) {
        for(int x = 0; x < PLAYFIELD_WIDTH; x++) {
            minesweeper_state->minefield[x][y] = FieldEmpty;
            minesweeper_state->playfield[x][y] = TileTypeUncleared;
        }
    }
    while(mines_left > 0) {
        int rand_x = rand() % PLAYFIELD_WIDTH;
        int rand_y = rand() % PLAYFIELD_HEIGHT;
        // make sure first guess isn't a mine
        if(minesweeper_state->minefield[rand_x][rand_y] == FieldEmpty &&
           (minesweeper_state->cursor_x != rand_x && minesweeper_state->cursor_y != rand_y)) {
            minesweeper_state->minefield[rand_x][rand_y] = FieldMine;
            mines_left--;
        }
    }
    minesweeper_state->mines_left = MINECOUNT;
    minesweeper_state->fields_cleared = 0;
    minesweeper_state->flags_set = 0;
    minesweeper_state->game_started_tick = furi_get_tick();
    minesweeper_state->game_started = false;
}

static void place_flag(Minesweeper* minesweeper_state) {
    if(minesweeper_state->playfield[minesweeper_state->cursor_x][minesweeper_state->cursor_y] ==
       TileTypeUncleared) {
        minesweeper_state->playfield[minesweeper_state->cursor_x][minesweeper_state->cursor_y] =
            TileTypeFlag;
        minesweeper_state->flags_set++;
    } else if(
        minesweeper_state->playfield[minesweeper_state->cursor_x][minesweeper_state->cursor_y] ==
        TileTypeFlag) {
        minesweeper_state->playfield[minesweeper_state->cursor_x][minesweeper_state->cursor_y] =
            TileTypeUncleared;
        minesweeper_state->flags_set--;
    }
}

static bool game_lost(Minesweeper* minesweeper_state) {
    // returns true if the player wants to restart, otherwise false
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);

    DialogMessage* message = dialog_message_alloc();
    const char* header_text = "Game Over";
    const char* message_text = "You hit a mine!";

    dialog_message_set_header(message, header_text, 64, 3, AlignCenter, AlignTop);
    dialog_message_set_text(message, message_text, 64, 32, AlignCenter, AlignCenter);
    dialog_message_set_buttons(message, NULL, "Play again", NULL);

    dialog_message_set_icon(message, NULL, 0, 10);

    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
    notification_message(notifications, &sequence_set_vibro_on);
    furi_record_close(RECORD_NOTIFICATION);
    furi_timer_start(minesweeper_state->timer, (uint32_t)furi_kernel_get_tick_frequency() * 0.2);

    DialogMessageButton choice = dialog_message_show(dialogs, message);
    dialog_message_free(message);
    furi_record_close(RECORD_DIALOGS);

    return choice == DialogMessageButtonCenter;
}

static bool game_won(Minesweeper* minesweeper_state) {
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);

    FuriString* tempStr;
    tempStr = furi_string_alloc();

    int seconds = 0;
    int minutes = 0;
    uint32_t ticks_elapsed = furi_get_tick() - minesweeper_state->game_started_tick;
    seconds = (int)ticks_elapsed / furi_kernel_get_tick_frequency();
    minutes = (int)seconds / 60;
    seconds = seconds % 60;

    DialogMessage* message = dialog_message_alloc();
    const char* header_text = "Game won!";
    furi_string_cat_printf(tempStr, "Minefield cleared in %01d:%02d", minutes, seconds);
    dialog_message_set_header(message, header_text, 64, 3, AlignCenter, AlignTop);
    dialog_message_set_text(
        message, furi_string_get_cstr(tempStr), 64, 32, AlignCenter, AlignCenter);
    dialog_message_set_buttons(message, NULL, "Play again", NULL);
    dialog_message_set_icon(message, NULL, 72, 17);

    DialogMessageButton choice = dialog_message_show(dialogs, message);
    dialog_message_free(message);
    furi_string_free(tempStr);
    furi_record_close(RECORD_DIALOGS);
    return choice == DialogMessageButtonCenter;
}

// returns false if the move loses the game - otherwise true
static bool play_move(Minesweeper* minesweeper_state, int cursor_x, int cursor_y) {
    if(minesweeper_state->playfield[cursor_x][cursor_y] == TileTypeFlag) {
        // we're on a flagged field, do nothing
        return true;
    }
    if(minesweeper_state->minefield[cursor_x][cursor_y] == FieldMine) {
        // player loses - draw mine
        minesweeper_state->playfield[cursor_x][cursor_y] = TileTypeMine;
        return false;
    }

    if(minesweeper_state->playfield[cursor_x][cursor_y] >= TileType1 &&
       minesweeper_state->playfield[cursor_x][cursor_y] <= TileType8) {
        // click on a cleared cell with a number
        // count the flags around
        int flags = 0;
        for(int y = cursor_y - 1; y <= cursor_y + 1; y++) {
            for(int x = cursor_x - 1; x <= cursor_x + 1; x++) {
                if(x == cursor_x && y == cursor_y) {
                    // we're on the cell the user selected, so ignore.
                    continue;
                }
                // make sure we don't go OOB
                if(x >= 0 && x < PLAYFIELD_WIDTH && y >= 0 && y < PLAYFIELD_HEIGHT) {
                    if(minesweeper_state->playfield[x][y] == TileTypeFlag) {
                        flags++;
                    }
                }
            }
        }
        int mines = minesweeper_state->playfield[cursor_x][cursor_y]; // ¯\_(ツ)_/¯
        if(flags == mines) {
            // auto uncover all non-flags around (to win faster ;)
            for(int auto_y = cursor_y - 1; auto_y <= cursor_y + 1; auto_y++) {
                for(int auto_x = cursor_x - 1; auto_x <= cursor_x + 1; auto_x++) {
                    if(auto_x == cursor_x && auto_y == cursor_y) {
                        continue;
                    }
                    if(auto_x >= 0 && auto_x < PLAYFIELD_WIDTH && auto_y >= 0 &&
                       auto_y < PLAYFIELD_HEIGHT) {
                        if(minesweeper_state->playfield[auto_x][auto_y] == TileTypeUncleared) {
                            if(!play_move(minesweeper_state, auto_x, auto_y)) {
                                // flags were wrong, we got a mine!
                                return false;
                            }
                        }
                    }
                }
            }
            // we're done without hitting a mine - so return
            return true;
        }
    }

    // calculate number of surrounding mines.
    int hint = 0;
    for(int y = cursor_y - 1; y <= cursor_y + 1; y++) {
        for(int x = cursor_x - 1; x <= cursor_x + 1; x++) {
            if(x == cursor_x && y == cursor_y) {
                // we're on the cell the user selected, so ignore.
                continue;
            }
            // make sure we don't go OOB
            if(x >= 0 && x < PLAYFIELD_WIDTH && y >= 0 && y < PLAYFIELD_HEIGHT) {
                if(minesweeper_state->minefield[x][y] == FieldMine) {
                    hint++;
                }
            }
        }
    }
    // 〜(￣▽￣〜) don't judge me (〜￣▽￣)〜
    minesweeper_state->playfield[cursor_x][cursor_y] = hint;
    minesweeper_state->fields_cleared++;
    FURI_LOG_D("Minesweeper", "Setting %d,%d to %d", cursor_x, cursor_y, hint);
    if(hint == 0) {
        // the field is "empty"
        // auto open surrounding fields.
        for(int auto_y = cursor_y - 1; auto_y <= cursor_y + 1; auto_y++) {
            for(int auto_x = cursor_x - 1; auto_x <= cursor_x + 1; auto_x++) {
                if(auto_x == cursor_x && auto_y == cursor_y) {
                    continue;
                }
                if(auto_x >= 0 && auto_x < PLAYFIELD_WIDTH && auto_y >= 0 &&
                   auto_y < PLAYFIELD_HEIGHT) {
                    if(minesweeper_state->playfield[auto_x][auto_y] == TileTypeUncleared) {
                        play_move(minesweeper_state, auto_x, auto_y);
                    }
                }
            }
        }
    }
    return true;
}

static void minesweeper_state_init(Minesweeper* const minesweeper_state) {
    minesweeper_state->cursor_x = minesweeper_state->cursor_y = 0;
    minesweeper_state->game_started = false;
    for(int y = 0; y < PLAYFIELD_HEIGHT; y++) {
        for(int x = 0; x < PLAYFIELD_WIDTH; x++) {
            minesweeper_state->playfield[x][y] = TileTypeUncleared;
        }
    }
}

int32_t minesweeper_app(void* p) {
    UNUSED(p);
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);

    DialogMessage* message = dialog_message_alloc();
    const char* header_text = "Minesweeper";
    const char* message_text = "Hold OK pressed to toggle flags.\ngithub.com/panki27";

    dialog_message_set_header(message, header_text, 64, 3, AlignCenter, AlignTop);
    dialog_message_set_text(message, message_text, 64, 32, AlignCenter, AlignCenter);
    dialog_message_set_buttons(message, NULL, "Play", NULL);

    dialog_message_set_icon(message, NULL, 0, 10);

    dialog_message_show(dialogs, message);
    dialog_message_free(message);
    furi_record_close(RECORD_DIALOGS);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    Minesweeper* minesweeper_state = malloc(sizeof(Minesweeper));
    // setup
    minesweeper_state_init(minesweeper_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, minesweeper_state, sizeof(minesweeper_state))) {
        FURI_LOG_E("Minesweeper", "cannot create mutex\r\n");
        free(minesweeper_state);
        return 255;
    }
    // BEGIN IMPLEMENTATION

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);
    minesweeper_state->timer = furi_timer_alloc(timer_callback, FuriTimerTypeOnce, &state_mutex);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        Minesweeper* minesweeper_state = (Minesweeper*)acquire_mutex_block(&state_mutex);
        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypeShort) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        minesweeper_state->cursor_y--;
                        if(minesweeper_state->cursor_y < 0) {
                            minesweeper_state->cursor_y = PLAYFIELD_HEIGHT - 1;
                        }
                        break;
                    case InputKeyDown:
                        minesweeper_state->cursor_y++;
                        if(minesweeper_state->cursor_y >= PLAYFIELD_HEIGHT) {
                            minesweeper_state->cursor_y = 0;
                        }
                        break;
                    case InputKeyRight:
                        minesweeper_state->cursor_x++;
                        if(minesweeper_state->cursor_x >= PLAYFIELD_WIDTH) {
                            minesweeper_state->cursor_x = 0;
                        }
                        break;
                    case InputKeyLeft:
                        minesweeper_state->cursor_x--;
                        if(minesweeper_state->cursor_x < 0) {
                            minesweeper_state->cursor_x = PLAYFIELD_WIDTH - 1;
                        }
                        break;
                    case InputKeyOk:
                        if(!minesweeper_state->game_started) {
                            setup_playfield(minesweeper_state);
                            minesweeper_state->game_started = true;
                        }
                        if(!play_move(
                               minesweeper_state,
                               minesweeper_state->cursor_x,
                               minesweeper_state->cursor_y)) {
                            // ooops. looks like we hit a mine!
                            if(game_lost(minesweeper_state)) {
                                // player wants to restart.
                                setup_playfield(minesweeper_state);
                            } else {
                                // player wants to exit :(
                                processing = false;
                            }
                        } else {
                            // check win condition.
                            if(minesweeper_state->fields_cleared ==
                               (PLAYFIELD_HEIGHT * PLAYFIELD_WIDTH) - MINECOUNT) {
                                if(game_won(minesweeper_state)) {
                                    //player wants to restart
                                    setup_playfield(minesweeper_state);
                                } else {
                                    processing = false;
                                }
                            }
                        }
                        break;
                    case InputKeyBack:
                        // Exit the plugin
                        processing = false;
                        break;
                    case InputKeyMAX:
                        break;
                    }
                } else if(event.input.type == InputTypeLong) {
                    // hold events
                    FURI_LOG_D("Minesweeper", "Got a long press!");
                    switch(event.input.key) {
                    case InputKeyUp:
                    case InputKeyDown:
                    case InputKeyRight:
                    case InputKeyLeft:
                        break;
                    case InputKeyOk:
                        FURI_LOG_D("Minesweeper", "Toggling flag");
                        place_flag(minesweeper_state);
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    case InputKeyMAX:
                        break;
                    }
                }
            }
        } else {
            // event timeout
            ;
        }
        view_port_update(view_port);
        release_mutex(&state_mutex, minesweeper_state);
    }
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);
    furi_timer_free(minesweeper_state->timer);
    free(minesweeper_state);

    return 0;
}
