#include "hid_numpad.h"
#include <furi.h>
#include <gui/elements.h>
#include <gui/icon_i.h>
#include "../hid.h"
#include "hid_icons.h"

#define TAG "HidNumpad"

struct HidNumpad {
    View* view;
    Hid* hid;
};

typedef struct {
    uint8_t last_x;
    uint8_t last_y;
    uint8_t x;
    uint8_t y;
    uint8_t last_key_code;
    uint16_t modifier_code;
    bool ok_pressed;
    bool back_pressed;
    bool connected;
    char key_string[5];
    HidTransport transport;
} HidNumpadModel;

typedef struct {
    uint8_t width;
    char* key;
    uint8_t height;
    const Icon* icon;
    uint8_t value;
} HidNumpadKey;

typedef struct {
    int8_t x;
    int8_t y;
} HidNumpadPoint;

#define MARGIN_TOP 32
#define MARGIN_LEFT 1
#define KEY_WIDTH 20
#define KEY_HEIGHT 15
#define KEY_PADDING 1
#define ROW_COUNT 6
#define COLUMN_COUNT 3

const HidNumpadKey hid_numpad_keyset[ROW_COUNT][COLUMN_COUNT] = {
    {
        {.width = 1, .height = 1, .icon = NULL, .key = "NL", .value = HID_KEYPAD_NUMLOCK},
        {.width = 1, .height = 1, .icon = NULL, .key = "/", .value = HID_KEYPAD_SLASH},
        {.width = 1, .height = 1, .icon = NULL, .key = "*", .value = HID_KEYPAD_ASTERISK},
        // {.width = 1, .height = 1, .icon = NULL, .key = "-", .value = HID_KEYPAD_MINUS},
    },
    {
        {.width = 1, .height = 1, .icon = NULL, .key = "7", .value = HID_KEYPAD_7},
        {.width = 1, .height = 1, .icon = NULL, .key = "8", .value = HID_KEYBOARD_8},
        {.width = 1, .height = 1, .icon = NULL, .key = "9", .value = HID_KEYBOARD_9},
        // {.width = 1, .height = 2, .icon = NULL, .key = "+", .value = HID_KEYPAD_PLUS},
    },
    {
        {.width = 1, .height = 1, .icon = NULL, .key = "4", .value = HID_KEYPAD_4},
        {.width = 1, .height = 1, .icon = NULL, .key = "5", .value = HID_KEYPAD_5},
        {.width = 1, .height = 1, .icon = NULL, .key = "6", .value = HID_KEYPAD_6},
    },
    {
        {.width = 1, .height = 1, .icon = NULL, .key = "1", .value = HID_KEYPAD_1},
        {.width = 1, .height = 1, .icon = NULL, .key = "2", .value = HID_KEYPAD_2},
        {.width = 1, .height = 1, .icon = NULL, .key = "3", .value = HID_KEYPAD_3},
        // {.width = 1, .height = 2, .icon = NULL, .key = "En", .value = HID_KEYPAD_ENTER},
    },
    {
        {.width = 2, .height = 1, .icon = NULL, .key = "0", .value = HID_KEYBOARD_0},
        {.width = 0, .height = 0, .icon = NULL, .key = "0", .value = HID_KEYBOARD_0},
        {.width = 1, .height = 1, .icon = NULL, .key = ".", .value = HID_KEYPAD_DOT},
    },
    {
        {.width = 1, .height = 1, .icon = NULL, .key = "En", .value = HID_KEYPAD_ENTER},
        {.width = 1, .height = 1, .icon = NULL, .key = "-", .value = HID_KEYPAD_MINUS},
        {.width = 1, .height = 1, .icon = NULL, .key = "+", .value = HID_KEYPAD_PLUS},
    },
};

static void hid_numpad_draw_key(
    Canvas* canvas,
    HidNumpadModel* model,
    uint8_t x,
    uint8_t y,
    HidNumpadKey key,
    bool selected) {
    if(!key.width || !key.height) return;

    canvas_set_color(canvas, ColorBlack);
    uint8_t keyWidth = KEY_WIDTH * key.width + KEY_PADDING * (key.width - 1);
    uint8_t keyHeight = KEY_HEIGHT * key.height + KEY_PADDING * (key.height - 1);
    if(selected) {
        elements_slightly_rounded_box(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING),
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING),
            keyWidth,
            keyHeight);
        canvas_set_color(canvas, ColorWhite);
    } else {
        elements_slightly_rounded_frame(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING),
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING),
            keyWidth,
            keyHeight);
    }
    if(key.icon != NULL) {
        canvas_draw_icon(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING) + keyWidth / 2 - key.icon->width / 2,
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING) + keyHeight / 2 - key.icon->height / 2,
            key.icon);
    } else {
        strcpy(model->key_string, key.key);
        canvas_draw_str_aligned(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING) + keyWidth / 2 + 1,
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING) + keyHeight / 2 + 1,
            AlignCenter,
            AlignCenter,
            model->key_string);
    }
}

static void hid_numpad_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidNumpadModel* model = context;

    // Header
    canvas_set_font(canvas, FontPrimary);
    if(model->transport == HidTransportBle) {
        if(model->connected) {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
        } else {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
            elements_multiline_text_aligned(
                canvas, 7, 60, AlignLeft, AlignBottom, "Waiting for\nConnection...");
        }
        elements_multiline_text_aligned(canvas, 20, 3, AlignLeft, AlignTop, "Numpad");

    } else {
        elements_multiline_text_aligned(canvas, 12, 3, AlignLeft, AlignTop, "Numpad");
    }

    canvas_draw_icon(canvas, 3, 18, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 15, 19, AlignLeft, AlignTop, "Hold to exit");

    if(!model->connected && (model->transport == HidTransportBle)) {
        return;
    }

    canvas_set_font(canvas, FontKeyboard);
    uint8_t initY = 0; // = model->y == 0 ? 0 : 1;

    // if(model->y > ROW_COUNT) {
    //     initY = model->y - (ROW_COUNT - 1);
    // }

    for(uint8_t y = initY; y < ROW_COUNT; y++) {
        const HidNumpadKey* numpadKeyRow = hid_numpad_keyset[y];
        uint8_t x = 0;
        for(uint8_t i = 0; i < COLUMN_COUNT; i++) {
            HidNumpadKey key = numpadKeyRow[i];
            bool keySelected = (x <= model->x && model->x < (x + key.width)) && y == model->y;
            bool backSelected = model->back_pressed && key.value == HID_KEYBOARD_DELETE;
            hid_numpad_draw_key(
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

static uint8_t hid_numpad_get_selected_key(HidNumpadModel* model) {
    HidNumpadKey key = hid_numpad_keyset[model->y][model->x];
    return key.value;
}

static void hid_numpad_get_select_key(HidNumpadModel* model, HidNumpadPoint delta) {
    do {
        const int delta_sum = model->y + delta.y;
        model->y = delta_sum < 0 ? ROW_COUNT - 1 : delta_sum % ROW_COUNT;
    } while(delta.y != 0 && hid_numpad_keyset[model->y][model->x].value == 0);

    do {
        const int delta_sum = model->x + delta.x;
        model->x = delta_sum < 0 ? COLUMN_COUNT - 1 : delta_sum % COLUMN_COUNT;
    } while(delta.x != 0 && hid_numpad_keyset[model->y][model->x].width == 0);
}

static void hid_numpad_process(HidNumpad* hid_numpad, InputEvent* event) {
    with_view_model(
        hid_numpad->view,
        HidNumpadModel * model,
        {
            if(event->key == InputKeyOk) {
                if(event->type == InputTypePress) {
                    model->ok_pressed = true;
                } else if(event->type == InputTypeLong || event->type == InputTypeShort) {
                    model->last_key_code = hid_numpad_get_selected_key(model);
                    hid_hal_keyboard_press(
                        hid_numpad->hid, model->modifier_code | model->last_key_code);
                } else if(event->type == InputTypeRelease) {
                    hid_hal_keyboard_release(
                        hid_numpad->hid, model->modifier_code | model->last_key_code);
                    model->ok_pressed = false;
                }
            } else if(event->key == InputKeyBack) {
                if(event->type == InputTypePress) {
                    model->back_pressed = true;
                } else if(event->type == InputTypeShort) {
                    hid_hal_keyboard_press(hid_numpad->hid, HID_KEYBOARD_DELETE);
                    hid_hal_keyboard_release(hid_numpad->hid, HID_KEYBOARD_DELETE);
                } else if(event->type == InputTypeRelease) {
                    model->back_pressed = false;
                }
            } else if(event->type == InputTypePress || event->type == InputTypeRepeat) {
                if(event->key == InputKeyUp) {
                    hid_numpad_get_select_key(model, (HidNumpadPoint){.x = 0, .y = -1});
                } else if(event->key == InputKeyDown) {
                    hid_numpad_get_select_key(model, (HidNumpadPoint){.x = 0, .y = 1});
                } else if(event->key == InputKeyLeft) {
                    if(model->last_x == 2 && model->last_y == 2 && model->y == 1 &&
                       model->x == 3) {
                        model->x = model->last_x;
                        model->y = model->last_y;
                    } else if(
                        model->last_x == 2 && model->last_y == 4 && model->y == 3 &&
                        model->x == 3) {
                        model->x = model->last_x;
                        model->y = model->last_y;
                    } else
                        hid_numpad_get_select_key(model, (HidNumpadPoint){.x = -1, .y = 0});
                    model->last_x = 0;
                    model->last_y = 0;
                } else if(event->key == InputKeyRight) {
                    if(model->x == 2 && model->y == 2) {
                        model->last_x = model->x;
                        model->last_y = model->y;
                        hid_numpad_get_select_key(model, (HidNumpadPoint){.x = 1, .y = -1});
                    } else if(model->x == 2 && model->y == 4) {
                        model->last_x = model->x;
                        model->last_y = model->y;
                        hid_numpad_get_select_key(model, (HidNumpadPoint){.x = 1, .y = -1});
                    } else {
                        hid_numpad_get_select_key(model, (HidNumpadPoint){.x = 1, .y = 0});
                    }
                }
            }
        },
        true);
}

static bool hid_numpad_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidNumpad* hid_numpad = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        hid_hal_keyboard_release_all(hid_numpad->hid);
    } else {
        hid_numpad_process(hid_numpad, event);
        consumed = true;
    }

    return consumed;
}

HidNumpad* hid_numpad_alloc(Hid* bt_hid) {
    HidNumpad* hid_numpad = malloc(sizeof(HidNumpad));
    hid_numpad->view = view_alloc();
    hid_numpad->hid = bt_hid;
    view_set_context(hid_numpad->view, hid_numpad);
    view_allocate_model(hid_numpad->view, ViewModelTypeLocking, sizeof(HidNumpadModel));
    view_set_orientation(hid_numpad->view, ViewOrientationVertical);
    view_set_draw_callback(hid_numpad->view, hid_numpad_draw_callback);
    view_set_input_callback(hid_numpad->view, hid_numpad_input_callback);

    with_view_model(
        hid_numpad->view,
        HidNumpadModel * model,
        {
            model->transport = bt_hid->transport;
            model->y = 0;
        },
        true);

    return hid_numpad;
}

void hid_numpad_free(HidNumpad* hid_numpad) {
    furi_assert(hid_numpad);
    view_free(hid_numpad->view);
    free(hid_numpad);
}

View* hid_numpad_get_view(HidNumpad* hid_numpad) {
    furi_assert(hid_numpad);
    return hid_numpad->view;
}

void hid_numpad_set_connected_status(HidNumpad* hid_numpad, bool connected) {
    furi_assert(hid_numpad);
    with_view_model(
        hid_numpad->view, HidNumpadModel * model, { model->connected = connected; }, true);
}
