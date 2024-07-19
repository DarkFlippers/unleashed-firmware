#include "hid_keyboard.h"
#include <furi.h>
#include <gui/elements.h>
#include <gui/icon_i.h>
#include "../hid.h"
#include "hid_icons.h"

#define TAG "HidKeyboard"

struct HidKeyboard {
    View* view;
    Hid* hid;
};

typedef struct {
    bool shift;
    bool alt;
    bool ctrl;
    bool gui;
    uint8_t x;
    uint8_t y;
    uint8_t last_key_code;
    bool ok_pressed;
    bool back_pressed;
    bool connected;
} HidKeyboardModel;

typedef struct {
    uint8_t width;
    char key;
    char shift_key;
    const Icon* icon;
    const Icon* icon_shift;
    const Icon* icon_toggled;
    uint8_t value;
} HidKeyboardKey;

typedef struct {
    int8_t x;
    int8_t y;
} HidKeyboardPoint;
// 4 BY 12
#define MARGIN_TOP   0
#define MARGIN_LEFT  3
#define KEY_WIDTH    11
#define KEY_HEIGHT   13
#define KEY_PADDING  -1
#define ROW_COUNT    7
#define COLUMN_COUNT 12

// 0 width items are not drawn, but their value is used
const HidKeyboardKey hid_keyboard_keyset[ROW_COUNT][COLUMN_COUNT] = {
    {
        {.width = 1, .icon = &I_ButtonF1_5x8, .value = HID_KEYBOARD_F1},
        {.width = 1, .icon = &I_ButtonF2_5x8, .value = HID_KEYBOARD_F2},
        {.width = 1, .icon = &I_ButtonF3_5x8, .value = HID_KEYBOARD_F3},
        {.width = 1, .icon = &I_ButtonF4_5x8, .value = HID_KEYBOARD_F4},
        {.width = 1, .icon = &I_ButtonF5_5x8, .value = HID_KEYBOARD_F5},
        {.width = 1, .icon = &I_ButtonF6_5x8, .value = HID_KEYBOARD_F6},
        {.width = 1, .icon = &I_ButtonF7_5x8, .value = HID_KEYBOARD_F7},
        {.width = 1, .icon = &I_ButtonF8_5x8, .value = HID_KEYBOARD_F8},
        {.width = 1, .icon = &I_ButtonF9_5x8, .value = HID_KEYBOARD_F9},
        {.width = 1, .icon = &I_ButtonF10_5x8, .value = HID_KEYBOARD_F10},
        {.width = 1, .icon = &I_ButtonF11_5x8, .value = HID_KEYBOARD_F11},
        {.width = 1, .icon = &I_ButtonF12_5x8, .value = HID_KEYBOARD_F12},
    },
    {
        {.width = 1, .icon = NULL, .key = '1', .shift_key = '!', .value = HID_KEYBOARD_1},
        {.width = 1, .icon = NULL, .key = '2', .shift_key = '@', .value = HID_KEYBOARD_2},
        {.width = 1,
         .icon = NULL,
         .icon_shift = &I_hash_button_9x11,
         .key = '3',
         .value = HID_KEYBOARD_3},
        {.width = 1, .icon = NULL, .key = '4', .shift_key = '$', .value = HID_KEYBOARD_4},
        {.width = 1,
         .icon = NULL,
         .icon_shift = &I_percent_button_9x11,
         .key = '5',
         .value = HID_KEYBOARD_5},
        {.width = 1, .icon = NULL, .key = '6', .shift_key = '^', .value = HID_KEYBOARD_6},
        {.width = 1, .icon = NULL, .key = '7', .shift_key = '&', .value = HID_KEYBOARD_7},
        {.width = 1, .icon = NULL, .key = '8', .shift_key = '*', .value = HID_KEYBOARD_8},
        {.width = 1, .icon = NULL, .key = '9', .shift_key = '(', .value = HID_KEYBOARD_9},
        {.width = 1, .icon = NULL, .key = '0', .shift_key = ')', .value = HID_KEYBOARD_0},
        {.width = 2, .icon = &I_backspace_19x11, .value = HID_KEYBOARD_DELETE},
        {.width = 0, .value = HID_KEYBOARD_DELETE},
    },
    {
        {.width = 1, .icon = NULL, .key = 'q', .shift_key = 'Q', .value = HID_KEYBOARD_Q},
        {.width = 1, .icon = NULL, .key = 'w', .shift_key = 'W', .value = HID_KEYBOARD_W},
        {.width = 1, .icon = NULL, .key = 'e', .shift_key = 'E', .value = HID_KEYBOARD_E},
        {.width = 1, .icon = NULL, .key = 'r', .shift_key = 'R', .value = HID_KEYBOARD_R},
        {.width = 1, .icon = NULL, .key = 't', .shift_key = 'T', .value = HID_KEYBOARD_T},
        {.width = 1, .icon = NULL, .key = 'y', .shift_key = 'Y', .value = HID_KEYBOARD_Y},
        {.width = 1, .icon = NULL, .key = 'u', .shift_key = 'U', .value = HID_KEYBOARD_U},
        {.width = 1, .icon = NULL, .key = 'i', .shift_key = 'I', .value = HID_KEYBOARD_I},
        {.width = 1, .icon = NULL, .key = 'o', .shift_key = 'O', .value = HID_KEYBOARD_O},
        {.width = 1, .icon = NULL, .key = 'p', .shift_key = 'P', .value = HID_KEYBOARD_P},
        {.width = 1,
         .icon = &I_sq_bracket_left_button_9x11,
         .icon_shift = &I_brace_left_button_9x11,
         .value = HID_KEYBOARD_OPEN_BRACKET},
        {.width = 1,
         .icon = &I_sq_bracket_right_button_9x11,
         .icon_shift = &I_brace_right_button_9x11,
         .value = HID_KEYBOARD_CLOSE_BRACKET},
    },
    {
        {.width = 1, .icon = NULL, .key = 'a', .shift_key = 'A', .value = HID_KEYBOARD_A},
        {.width = 1, .icon = NULL, .key = 's', .shift_key = 'S', .value = HID_KEYBOARD_S},
        {.width = 1, .icon = NULL, .key = 'd', .shift_key = 'D', .value = HID_KEYBOARD_D},
        {.width = 1, .icon = NULL, .key = 'f', .shift_key = 'F', .value = HID_KEYBOARD_F},
        {.width = 1, .icon = NULL, .key = 'g', .shift_key = 'G', .value = HID_KEYBOARD_G},
        {.width = 1, .icon = NULL, .key = 'h', .shift_key = 'H', .value = HID_KEYBOARD_H},
        {.width = 1, .icon = NULL, .key = 'j', .shift_key = 'J', .value = HID_KEYBOARD_J},
        {.width = 1, .icon = NULL, .key = 'k', .shift_key = 'K', .value = HID_KEYBOARD_K},
        {.width = 1, .icon = NULL, .key = 'l', .shift_key = 'L', .value = HID_KEYBOARD_L},
        {.width = 1, .icon = NULL, .key = ';', .shift_key = ':', .value = HID_KEYBOARD_SEMICOLON},
        {.width = 2, .icon = &I_Return_10x7, .value = HID_KEYBOARD_RETURN},
        {.width = 0, .value = HID_KEYBOARD_RETURN},
    },
    {
        {.width = 1, .icon = NULL, .key = 'z', .shift_key = 'Z', .value = HID_KEYBOARD_Z},
        {.width = 1, .icon = NULL, .key = 'x', .shift_key = 'X', .value = HID_KEYBOARD_X},
        {.width = 1, .icon = NULL, .key = 'c', .shift_key = 'C', .value = HID_KEYBOARD_C},
        {.width = 1, .icon = NULL, .key = 'v', .shift_key = 'V', .value = HID_KEYBOARD_V},
        {.width = 1, .icon = NULL, .key = 'b', .shift_key = 'B', .value = HID_KEYBOARD_B},
        {.width = 1, .icon = NULL, .key = 'n', .shift_key = 'N', .value = HID_KEYBOARD_N},
        {.width = 1, .icon = NULL, .key = 'm', .shift_key = 'M', .value = HID_KEYBOARD_M},
        {.width = 1, .icon = &I_slash_button_9x11, .shift_key = '?', .value = HID_KEYBOARD_SLASH},
        {.width = 1,
         .icon = &I_backslash_button_9x11,
         .shift_key = '|',
         .value = HID_KEYBOARD_BACKSLASH},
        {.width = 1,
         .icon = &I_backtick_button_9x11,
         .shift_key = '~',
         .value = HID_KEYBOARD_GRAVE_ACCENT},
        {.width = 1, .icon = &I_ButtonUp_7x4, .value = HID_KEYBOARD_UP_ARROW},
        {.width = 1,
         .icon = NULL,
         .icon_shift = &I_underscore_button_9x11,
         .key = '-',
         .value = HID_KEYBOARD_MINUS},
    },
    {
        {.width = 1,
         .icon = &I_Shift_inactive_7x9,
         .icon_toggled = &I_Shift_active_7x9,
         .value = HID_KEYBOARD_L_SHIFT},
        {.width = 1, .icon = NULL, .key = ',', .shift_key = '<', .value = HID_KEYBOARD_COMMA},
        {.width = 1, .icon = NULL, .key = '.', .shift_key = '>', .value = HID_KEYBOARD_DOT},
        {.width = 4, .icon = NULL, .key = ' ', .value = HID_KEYBOARD_SPACEBAR},
        {.width = 0, .value = HID_KEYBOARD_SPACEBAR},
        {.width = 0, .value = HID_KEYBOARD_SPACEBAR},
        {.width = 0, .value = HID_KEYBOARD_SPACEBAR},
        {.width = 1,
         .icon = &I_apostrophe_button_9x11,
         .icon_shift = &I_quote_button_9x11,
         .value = HID_KEYBOARD_APOSTROPHE},
        {.width = 1,
         .icon = &I_equals_button_9x11,
         .shift_key = '+',
         .value = HID_KEYBOARD_EQUAL_SIGN},
        {.width = 1, .icon = &I_ButtonLeft_4x7, .value = HID_KEYBOARD_LEFT_ARROW},
        {.width = 1, .icon = &I_ButtonDown_7x4, .value = HID_KEYBOARD_DOWN_ARROW},
        {.width = 1, .icon = &I_ButtonRight_4x7, .value = HID_KEYBOARD_RIGHT_ARROW},
    },
    {
        {.width = 2,
         .icon = &I_Ctrl_17x10,
         .icon_toggled = &I_Ctrl_active_17x9,
         .value = HID_KEYBOARD_L_CTRL},
        {.width = 0, .value = HID_KEYBOARD_L_CTRL},
        {.width = 2,
         .icon = &I_Alt_17x10,
         .icon_toggled = &I_Alt_active_17x9,
         .value = HID_KEYBOARD_L_ALT},
        {.width = 0, .value = HID_KEYBOARD_L_ALT},
        {.width = 2,
         .icon = &I_Cmd_17x10,
         .icon_toggled = &I_Cmd_active_17x9,
         .value = HID_KEYBOARD_L_GUI},
        {.width = 0, .value = HID_KEYBOARD_L_GUI},
        {.width = 2, .icon = &I_Tab_17x10, .value = HID_KEYBOARD_TAB},
        {.width = 0, .value = HID_KEYBOARD_TAB},
        {.width = 2, .icon = &I_Esc_17x10, .value = HID_KEYBOARD_ESCAPE},
        {.width = 0, .value = HID_KEYBOARD_ESCAPE},
        {.width = 2, .icon = &I_Del_17x10, .value = HID_KEYBOARD_DELETE_FORWARD},
        {.width = 0, .value = HID_KEYBOARD_DELETE_FORWARD},
    },
};

static void hid_keyboard_draw_key(
    Canvas* canvas,
    HidKeyboardModel* model,
    uint8_t x,
    uint8_t y,
    HidKeyboardKey key,
    bool selected) {
    if(!key.width) return;

    canvas_set_color(canvas, ColorBlack);
    uint8_t keyWidth = KEY_WIDTH * key.width + KEY_PADDING * (key.width - 1);
    if(selected) {
        // Draw a filled box
        elements_slightly_rounded_box(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING),
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING),
            keyWidth,
            KEY_HEIGHT);
        canvas_set_color(canvas, ColorWhite);
    } else {
        // Draw a framed box
        elements_slightly_rounded_frame(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING),
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING),
            keyWidth,
            KEY_HEIGHT);
    }

    if(model->shift && key.icon_shift != NULL) {
        // Icon and shift
        const Icon* key_icon = key.icon_shift;

        if((model->ctrl && key.value == HID_KEYBOARD_L_CTRL) ||
           (model->alt && key.value == HID_KEYBOARD_L_ALT) ||
           (key.value == HID_KEYBOARD_L_SHIFT) ||
           (model->gui && key.value == HID_KEYBOARD_L_GUI)) {
            if(key.icon_toggled) {
                key_icon = key.icon_toggled;
            }
        }
        // Draw the icon centered on the button
        canvas_draw_icon(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING) + keyWidth / 2 - key_icon->width / 2,
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING) + KEY_HEIGHT / 2 - key_icon->height / 2,
            key_icon);

        return;
    }
    if(model->shift && key.shift_key != 0) {
        // Text and shift
        char key_str[2] = {key.shift_key, '\0'};

        canvas_draw_str_aligned(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING) + keyWidth / 2 + 1,
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING) + KEY_HEIGHT / 2 + 1,
            AlignCenter,
            AlignCenter,
            key_str);

        return;
    }

    if(key.icon != NULL) {
        // Icon with no shift
        const Icon* key_icon = key.icon;

        if((model->ctrl && key.value == HID_KEYBOARD_L_CTRL) ||
           (model->alt && key.value == HID_KEYBOARD_L_ALT) ||
           (model->shift && key.value == HID_KEYBOARD_L_SHIFT) ||
           (model->gui && key.value == HID_KEYBOARD_L_GUI)) {
            if(key.icon_toggled) {
                key_icon = key.icon_toggled;
            }
        }
        // Draw the icon centered on the button
        canvas_draw_icon(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING) + keyWidth / 2 - key_icon->width / 2,
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING) + KEY_HEIGHT / 2 - key_icon->height / 2,
            key_icon);

        return;
    }

    if(key.key != 0) {
        // Text with no shift
        char key_str[2] = {key.key, '\0'};
        uint8_t key_offset = 0;

        // Special case for numbers, draw them one pixel lower
        if(key.value >= HID_KEYBOARD_1 && key.value <= HID_KEYBOARD_0) {
            key_offset = 1;
        }

        canvas_draw_str_aligned(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING) + keyWidth / 2 + 1,
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING) + KEY_HEIGHT / 2 + key_offset,
            AlignCenter,
            AlignCenter,
            key_str);

        return;
    }
}

static void hid_keyboard_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidKeyboardModel* model = context;

    // Header
#ifdef HID_TRANSPORT_BLE
    if(!model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
        canvas_set_font(canvas, FontPrimary);
        elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Keyboard");

        canvas_draw_icon(canvas, 68, 3, &I_Pin_back_arrow_10x8);
        canvas_set_font(canvas, FontSecondary);
        elements_multiline_text_aligned(canvas, 127, 4, AlignRight, AlignTop, "Hold to exit");

        elements_multiline_text_aligned(
            canvas, 4, 60, AlignLeft, AlignBottom, "Waiting for Connection...");
        return; // Dont render the keyboard if we are not yet connected
    }
#endif

    canvas_set_font(canvas, FontKeyboard);
    // Start shifting the all keys up if on the next row (Scrolling)
    uint8_t initY = model->y == 0 ? 0 : 1;

    if(model->y > 5) {
        initY = model->y - 4;
    }

    elements_scrollbar(canvas, initY, 3);

    for(uint8_t y = initY; y < ROW_COUNT; y++) {
        const HidKeyboardKey* keyboardKeyRow = hid_keyboard_keyset[y];
        uint8_t x = 0;
        for(uint8_t i = 0; i < COLUMN_COUNT; i++) {
            HidKeyboardKey key = keyboardKeyRow[i];
            // Select when the button is hovered
            // Select if the button is hovered within its width
            // Select if back is clicked and its the backspace key
            // Deselect when the button clicked or not hovered
            bool keySelected = (x <= model->x && model->x < (x + key.width)) && y == model->y;
            bool backSelected = model->back_pressed && key.value == HID_KEYBOARD_DELETE;
            hid_keyboard_draw_key(
                canvas,
                model,
                x,
                y - initY,
                key,
                (!model->ok_pressed && keySelected) || backSelected);
            x += key.width;
        }
    }
}

static uint8_t hid_keyboard_get_selected_key(HidKeyboardModel* model) {
    HidKeyboardKey key = hid_keyboard_keyset[model->y][model->x];
    return key.value;
}

static void hid_keyboard_get_select_key(HidKeyboardModel* model, HidKeyboardPoint delta) {
    // Keep going until a valid spot is found, this allows for nulls and zero width keys in the map
    do {
        const int delta_sum = model->y + delta.y;
        model->y = delta_sum < 0 ? ROW_COUNT - 1 : delta_sum % ROW_COUNT;
    } while(delta.y != 0 && hid_keyboard_keyset[model->y][model->x].value == 0);

    do {
        const int delta_sum = model->x + delta.x;
        model->x = delta_sum < 0 ? COLUMN_COUNT - 1 : delta_sum % COLUMN_COUNT;
    } while(delta.x != 0 && hid_keyboard_keyset[model->y][model->x].width ==
                                0); // Skip zero width keys, pretend they are one key
}

static void hid_keyboard_modifier_set(Hid* hid, uint16_t keycode, bool is_pressed) {
    if(is_pressed) {
        hid_hal_keyboard_press(hid, keycode);
    } else {
        hid_hal_keyboard_release(hid, keycode);
    }
}

static void hid_keyboard_process(HidKeyboard* hid_keyboard, InputEvent* event) {
    with_view_model(
        hid_keyboard->view,
        HidKeyboardModel * model,
        {
            if(event->key == InputKeyOk) {
                if(event->type == InputTypePress) {
                    model->ok_pressed = true;
                } else if(event->type == InputTypeLong || event->type == InputTypeShort) {
                    model->last_key_code = hid_keyboard_get_selected_key(model);

                    // Toggle the modifier key when clicked, and click the key
                    if(model->last_key_code == HID_KEYBOARD_L_SHIFT) {
                        model->shift = !model->shift;
                        hid_keyboard_modifier_set(
                            hid_keyboard->hid, KEY_MOD_LEFT_SHIFT, model->shift);
                    } else if(model->last_key_code == HID_KEYBOARD_L_ALT) {
                        model->alt = !model->alt;
                        hid_keyboard_modifier_set(hid_keyboard->hid, KEY_MOD_LEFT_ALT, model->alt);
                    } else if(model->last_key_code == HID_KEYBOARD_L_CTRL) {
                        model->ctrl = !model->ctrl;
                        hid_keyboard_modifier_set(
                            hid_keyboard->hid, KEY_MOD_LEFT_CTRL, model->ctrl);
                    } else if(model->last_key_code == HID_KEYBOARD_L_GUI) {
                        model->gui = !model->gui;
                        hid_keyboard_modifier_set(hid_keyboard->hid, KEY_MOD_LEFT_GUI, model->gui);
                    } else {
                        hid_hal_keyboard_press(hid_keyboard->hid, model->last_key_code);
                    }

                } else if(event->type == InputTypeRelease) {
                    // Release happens after short and long presses
                    hid_hal_keyboard_release(hid_keyboard->hid, model->last_key_code);
                    model->ok_pressed = false;
                }
            } else if(event->key == InputKeyBack) {
                // If back is pressed for a short time, backspace
                if(event->type == InputTypePress) {
                    model->back_pressed = true;
                } else if(event->type == InputTypeShort) {
                    hid_hal_keyboard_press(hid_keyboard->hid, HID_KEYBOARD_DELETE);
                    hid_hal_keyboard_release(hid_keyboard->hid, HID_KEYBOARD_DELETE);
                } else if(event->type == InputTypeRelease) {
                    model->back_pressed = false;
                }
            } else if(event->type == InputTypePress || event->type == InputTypeRepeat) {
                // Cycle the selected keys
                if(event->key == InputKeyUp) {
                    hid_keyboard_get_select_key(model, (HidKeyboardPoint){.x = 0, .y = -1});
                } else if(event->key == InputKeyDown) {
                    hid_keyboard_get_select_key(model, (HidKeyboardPoint){.x = 0, .y = 1});
                } else if(event->key == InputKeyLeft) {
                    hid_keyboard_get_select_key(model, (HidKeyboardPoint){.x = -1, .y = 0});
                } else if(event->key == InputKeyRight) {
                    hid_keyboard_get_select_key(model, (HidKeyboardPoint){.x = 1, .y = 0});
                }
            }
        },
        true);
}

static bool hid_keyboard_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidKeyboard* hid_keyboard = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        hid_hal_keyboard_release_all(hid_keyboard->hid);
        with_view_model(
            hid_keyboard->view,
            HidKeyboardModel * model,
            {
                model->shift = false;
                model->alt = false;
                model->ctrl = false;
                model->gui = false;
            },
            true);
    } else {
        hid_keyboard_process(hid_keyboard, event);
        consumed = true;
    }

    return consumed;
}

HidKeyboard* hid_keyboard_alloc(Hid* bt_hid) {
    HidKeyboard* hid_keyboard = malloc(sizeof(HidKeyboard));
    hid_keyboard->view = view_alloc();
    hid_keyboard->hid = bt_hid;
    view_set_context(hid_keyboard->view, hid_keyboard);
    view_allocate_model(hid_keyboard->view, ViewModelTypeLocking, sizeof(HidKeyboardModel));
    view_set_draw_callback(hid_keyboard->view, hid_keyboard_draw_callback);
    view_set_input_callback(hid_keyboard->view, hid_keyboard_input_callback);

    with_view_model(hid_keyboard->view, HidKeyboardModel * model, { model->y = 1; }, true);

    return hid_keyboard;
}

void hid_keyboard_free(HidKeyboard* hid_keyboard) {
    furi_assert(hid_keyboard);
    view_free(hid_keyboard->view);
    free(hid_keyboard);
}

View* hid_keyboard_get_view(HidKeyboard* hid_keyboard) {
    furi_assert(hid_keyboard);
    return hid_keyboard->view;
}

void hid_keyboard_set_connected_status(HidKeyboard* hid_keyboard, bool connected) {
    furi_assert(hid_keyboard);
    with_view_model(
        hid_keyboard->view, HidKeyboardModel * model, { model->connected = connected; }, true);
}
