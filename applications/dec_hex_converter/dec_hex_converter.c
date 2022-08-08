#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

#define DEC_HEX_CONVERTER_NUMBER_DIGITS 9
#define DEC_HEX_CONVERTER_KEYS 18
#define DEC_HEX_CONVERTER_KEY_DEL 16
// #define DEC_HEX_CONVERTER_KEY_SWAP       17 // actually not used...

#define DEC_HEX_CONVERTER_CHAR_DEL '<'
#define DEC_HEX_CONVERTER_CHAR_SWAP 's'
#define DEC_HEX_CONVERTER_CHAR_MODE '#'
#define DEC_HEX_CONVERTER_CHAR_OVERFLOW '#'

#define DEC_HEX_CONVERTER_KEY_FRAME_MARGIN 3
#define DEC_HEX_CONVERTER_KEY_CHAR_HEIGHT 8

#define DEC_HEX_MAX_SUPORTED_DEC_INT 999999999

typedef enum {
    EventTypeKey,
} EventType;

typedef struct {
    InputEvent input;
    EventType type;
} DecHexConverterEvent;

typedef enum {
    ModeDec,
    ModeHex,
} Mode;

// setting up one char array next to the other one causes the canvas_draw_str to display both of them
// when addressing the first one if there's no string terminator or similar indicator. Adding a \0 seems
// to work fine to prevent that, so add a final last char outside the size constants (added on init
// and NEVER changed nor referenced again)
//
// (as a reference, canvas_draw_str ends up calling u8g2_DrawStr from u8g2_font.c,
// that finally ends up calling u8g2_draw_string)
typedef struct {
    char dec_number[DEC_HEX_CONVERTER_NUMBER_DIGITS + 1];
    char hex_number[DEC_HEX_CONVERTER_NUMBER_DIGITS + 1];
    Mode mode; // dec / hex
    int8_t cursor; // position on keyboard (includes digit letters and other options)
    int8_t digit_pos; // current digit on selected mode
} DecHexConverterState;

// move cursor left / right (TODO: implement menu nav in a more "standard" and reusable way?)
void dec_hex_converter_logic_move_cursor_lr(
    DecHexConverterState* const dec_hex_converter_state,
    int8_t d) {
    dec_hex_converter_state->cursor += d;

    if(dec_hex_converter_state->cursor > DEC_HEX_CONVERTER_KEYS - 1)
        dec_hex_converter_state->cursor = 0;
    else if(dec_hex_converter_state->cursor < 0)
        dec_hex_converter_state->cursor = DEC_HEX_CONVERTER_KEYS - 1;

    // if we're moving left / right to the letters keys on ModeDec just go to the closest available key
    if(dec_hex_converter_state->mode == ModeDec) {
        if(dec_hex_converter_state->cursor == 10)
            dec_hex_converter_state->cursor = 16;
        else if(dec_hex_converter_state->cursor == 15)
            dec_hex_converter_state->cursor = 9;
    }
}

// move cursor up / down; there're two lines, so we basically toggle
void dec_hex_converter_logic_move_cursor_ud(DecHexConverterState* const dec_hex_converter_state) {
    if(dec_hex_converter_state->cursor < 9) {
        // move to second line ("down")
        dec_hex_converter_state->cursor += 9;

        // if we're reaching the letter keys while ModeDec, just move left / right for the first available key
        if(dec_hex_converter_state->mode == ModeDec &&
           (dec_hex_converter_state->cursor >= 10 && dec_hex_converter_state->cursor <= 15)) {
            if(dec_hex_converter_state->cursor <= 12)
                dec_hex_converter_state->cursor = 9;
            else
                dec_hex_converter_state->cursor = 16;
        }
    } else {
        // move to first line ("up")
        dec_hex_converter_state->cursor -= 9;
    }
}

// fetch number from current mode and modifies the destination one, RM dnt stel pls
// (if destination is shorter than the output value, overried with "-" chars or something similar)
void dec_hex_converter_logic_convert_number(DecHexConverterState* const dec_hex_converter_state) {
    char* s_ptr;
    char* d_ptr;

    char dest[DEC_HEX_CONVERTER_NUMBER_DIGITS];
    int i = 0; // current index on destination array

    if(dec_hex_converter_state->mode == ModeDec) {
        // DEC to HEX cannot overflow if they're, at least, the same size

        s_ptr = dec_hex_converter_state->dec_number;
        d_ptr = dec_hex_converter_state->hex_number;

        int a = atoi(s_ptr);
        int r;
        while(a != 0) {
            r = a % 16;
            dest[i] = r + (r < 10 ? '0' : ('A' - 10));
            a /= 16;
            i++;
        }

    } else {
        s_ptr = dec_hex_converter_state->hex_number;
        d_ptr = dec_hex_converter_state->dec_number;

        int a = strtol(s_ptr, NULL, 16);
        if(a > DEC_HEX_MAX_SUPORTED_DEC_INT) {
            // draw all "###" if there's an overflow
            for(int j = 0; j < DEC_HEX_CONVERTER_NUMBER_DIGITS; j++) {
                d_ptr[j] = DEC_HEX_CONVERTER_CHAR_OVERFLOW;
            }
            return;
        } else {
            while(a > 0) {
                dest[i++] = (a % 10) + '0';
                a /= 10;
            }
        }
    }

    // dest is reversed, copy to destination pointer and append empty chars at the end
    for(int j = 0; j < DEC_HEX_CONVERTER_NUMBER_DIGITS; j++) {
        if(i >= 1)
            d_ptr[j] = dest[--i];
        else
            d_ptr[j] = ' ';
    }
}

// change from DEC to HEX or HEX to DEC, set the digit_pos to the last position not empty on the destination mode
void dec_hex_converter_logic_swap_mode(DecHexConverterState* const dec_hex_converter_state) {
    char* n_ptr;
    if(dec_hex_converter_state->mode == ModeDec) {
        dec_hex_converter_state->mode = ModeHex;
        n_ptr = dec_hex_converter_state->hex_number;
    } else {
        dec_hex_converter_state->mode = ModeDec;
        n_ptr = dec_hex_converter_state->dec_number;
    }

    dec_hex_converter_state->digit_pos = DEC_HEX_CONVERTER_NUMBER_DIGITS;
    for(int i = 0; i < DEC_HEX_CONVERTER_NUMBER_DIGITS; i++) {
        if(n_ptr[i] == ' ') {
            dec_hex_converter_state->digit_pos = i;
            break;
        }
    }
}

// removes the number on current digit on current mode
static void
    dec_hex_converter_logic_del_number(DecHexConverterState* const dec_hex_converter_state) {
    if(dec_hex_converter_state->digit_pos > 0) dec_hex_converter_state->digit_pos--;

    if(dec_hex_converter_state->mode == ModeDec) {
        dec_hex_converter_state->dec_number[dec_hex_converter_state->digit_pos] = ' ';
    } else {
        dec_hex_converter_state->hex_number[dec_hex_converter_state->digit_pos] = ' ';
    }
}

// append a number to the digit on the current mode
static void dec_hex_converter_logic_add_number(
    DecHexConverterState* const dec_hex_converter_state,
    int8_t number) {
    // ignore HEX values on DEC mode (probably button nav will be disabled too, so cannot reach);
    // also do not add anything if we're out of bound
    if((number > 9 && dec_hex_converter_state->mode == ModeDec) ||
       dec_hex_converter_state->digit_pos >= DEC_HEX_CONVERTER_NUMBER_DIGITS)
        return;

    char* s_ptr;

    if(dec_hex_converter_state->mode == ModeDec) {
        s_ptr = dec_hex_converter_state->dec_number;
    } else {
        s_ptr = dec_hex_converter_state->hex_number;
    }

    if(number < 10) {
        s_ptr[dec_hex_converter_state->digit_pos] = number + '0';
    } else {
        s_ptr[dec_hex_converter_state->digit_pos] = (number - 10) + 'A'; // A-F (HEX only)
    }

    dec_hex_converter_state->digit_pos++;
}

// ---------------

static void dec_hex_converter_render_callback(Canvas* const canvas, void* ctx) {
    const DecHexConverterState* dec_hex_converter_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(dec_hex_converter_state == NULL) {
        return;
    }

    canvas_set_color(canvas, ColorBlack);

    // DEC
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "DEC: ");

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2 + 30, 10, dec_hex_converter_state->dec_number);

    // HEX
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10 + 12, "HEX: ");

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2 + 30, 10 + 12, dec_hex_converter_state->hex_number);

    // current mode indicator
    // char buffer[4];
    // snprintf(buffer, sizeof(buffer), "%u", dec_hex_converter_state->digit_pos); // debug: show digit position instead of selected mode
    if(dec_hex_converter_state->mode == ModeDec) {
        canvas_draw_glyph(canvas, 128 - 10, 10, DEC_HEX_CONVERTER_CHAR_MODE);
    } else {
        canvas_draw_glyph(canvas, 128 - 10, 10 + 12, DEC_HEX_CONVERTER_CHAR_MODE);
    }

    // draw the line
    canvas_draw_line(canvas, 2, 25, 128 - 3, 25);

    // draw the keyboard
    uint8_t _x = 5;
    uint8_t _y = 25 + 15; // line + 10

    for(int i = 0; i < DEC_HEX_CONVERTER_KEYS; i++) {
        char g;
        if(i < 10)
            g = (i + '0');
        else if(i < 16)
            g = ((i - 10) + 'A');
        else if(i == 16)
            g = DEC_HEX_CONVERTER_CHAR_DEL; // '<'
        else
            g = DEC_HEX_CONVERTER_CHAR_SWAP; // 's'

        uint8_t g_w = canvas_glyph_width(canvas, g);

        // disable letters on DEC mode (but keep the previous width for visual purposes - show "blank keys")
        if(dec_hex_converter_state->mode == ModeDec && i > 9 && i < 16) g = ' ';

        if(dec_hex_converter_state->cursor == i) {
            canvas_draw_box(
                canvas,
                _x - DEC_HEX_CONVERTER_KEY_FRAME_MARGIN,
                _y - (DEC_HEX_CONVERTER_KEY_CHAR_HEIGHT + DEC_HEX_CONVERTER_KEY_FRAME_MARGIN),
                DEC_HEX_CONVERTER_KEY_FRAME_MARGIN + g_w + DEC_HEX_CONVERTER_KEY_FRAME_MARGIN,
                DEC_HEX_CONVERTER_KEY_CHAR_HEIGHT + DEC_HEX_CONVERTER_KEY_FRAME_MARGIN * 2);
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_glyph(canvas, _x, _y, g);
            canvas_set_color(canvas, ColorBlack);
        } else {
            canvas_draw_frame(
                canvas,
                _x - DEC_HEX_CONVERTER_KEY_FRAME_MARGIN,
                _y - (DEC_HEX_CONVERTER_KEY_CHAR_HEIGHT + DEC_HEX_CONVERTER_KEY_FRAME_MARGIN),
                DEC_HEX_CONVERTER_KEY_FRAME_MARGIN + g_w + DEC_HEX_CONVERTER_KEY_FRAME_MARGIN,
                DEC_HEX_CONVERTER_KEY_CHAR_HEIGHT + DEC_HEX_CONVERTER_KEY_FRAME_MARGIN * 2);
            canvas_draw_glyph(canvas, _x, _y, g);
        }

        if(i < 8) {
            _x += g_w + DEC_HEX_CONVERTER_KEY_FRAME_MARGIN * 2 + 2;
        } else if(i == 8) {
            _y += (DEC_HEX_CONVERTER_KEY_CHAR_HEIGHT + DEC_HEX_CONVERTER_KEY_FRAME_MARGIN * 2) + 3;
            _x = 7; // some padding at the beginning on second line
        } else {
            _x += g_w + DEC_HEX_CONVERTER_KEY_FRAME_MARGIN * 2 + 1;
        }
    }

    release_mutex((ValueMutex*)ctx, dec_hex_converter_state);
}

static void
    dec_hex_converter_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    DecHexConverterEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void dec_hex_converter_init(DecHexConverterState* const dec_hex_converter_state) {
    dec_hex_converter_state->mode = ModeDec;
    dec_hex_converter_state->digit_pos = 0;

    dec_hex_converter_state->dec_number[DEC_HEX_CONVERTER_NUMBER_DIGITS] = '\0'; // null terminator
    dec_hex_converter_state->hex_number[DEC_HEX_CONVERTER_NUMBER_DIGITS] = '\0'; // null terminator

    for(int i = 0; i < DEC_HEX_CONVERTER_NUMBER_DIGITS; i++) {
        dec_hex_converter_state->dec_number[i] = ' ';
        dec_hex_converter_state->hex_number[i] = ' ';
    }
}

// main entry point
int32_t dec_hex_converter_app(void* p) {
    UNUSED(p);

    // get event queue
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(DecHexConverterEvent));

    // allocate state
    DecHexConverterState* dec_hex_converter_state = malloc(sizeof(DecHexConverterState));

    // set mutex for plugin state (different threads can access it)
    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, dec_hex_converter_state, sizeof(dec_hex_converter_state))) {
        FURI_LOG_E("DecHexConverter", "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(dec_hex_converter_state);
        return 255;
    }

    // register callbacks for drawing and input processing
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, dec_hex_converter_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, dec_hex_converter_input_callback, event_queue);

    // open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    dec_hex_converter_init(dec_hex_converter_state);

    // main loop
    DecHexConverterEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        DecHexConverterState* dec_hex_converter_state =
            (DecHexConverterState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    default:
                        break;
                    case InputKeyUp:
                    case InputKeyDown:
                        dec_hex_converter_logic_move_cursor_ud(dec_hex_converter_state);
                        break;
                    case InputKeyRight:
                        dec_hex_converter_logic_move_cursor_lr(dec_hex_converter_state, 1);
                        break;
                    case InputKeyLeft:
                        dec_hex_converter_logic_move_cursor_lr(dec_hex_converter_state, -1);
                        break;
                    case InputKeyOk:
                        if(dec_hex_converter_state->cursor < DEC_HEX_CONVERTER_KEY_DEL) {
                            // positions from 0 to 15 works as regular numbers (DEC / HEX where applicable)
                            // (logic won't allow add numbers > 9 on ModeDec)
                            dec_hex_converter_logic_add_number(
                                dec_hex_converter_state, dec_hex_converter_state->cursor);
                        } else if(dec_hex_converter_state->cursor == DEC_HEX_CONVERTER_KEY_DEL) {
                            // del
                            dec_hex_converter_logic_del_number(dec_hex_converter_state);
                        } else {
                            // swap
                            dec_hex_converter_logic_swap_mode(dec_hex_converter_state);
                        }

                        dec_hex_converter_logic_convert_number(dec_hex_converter_state);
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    }
                }
            }
        } else {
            // event timeout
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, dec_hex_converter_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);
    free(dec_hex_converter_state);

    return 0;
}