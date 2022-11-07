#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

#include "barcode_generator.h"

static BarcodeType* barcodeTypes[NUMBER_OF_BARCODE_TYPES];

void init_types() {
    BarcodeType* upcA = malloc(sizeof(BarcodeType));
    upcA->name = "UPC-A";
    upcA->numberOfDigits = 12;
    upcA->startPos = 19;
    barcodeTypes[0] = upcA;

    BarcodeType* ean8 = malloc(sizeof(BarcodeType));
    ean8->name = "EAN-8";
    ean8->numberOfDigits = 8;
    ean8->startPos = 33;
    barcodeTypes[1] = ean8;
}

void draw_digit(Canvas* canvas, int digit, bool rightHand, int startingPosition) {
    char digitStr[2];
    snprintf(digitStr, 2, "%u", digit);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(
        canvas, startingPosition, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, digitStr);
    if(rightHand) {
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_color(canvas, ColorWhite);
    }

    int count = 0;
    for(int i = 0; i < 4; i++) {
        canvas_draw_box(
            canvas, startingPosition + count, BARCODE_Y_START, DIGITS[digit][i], BARCODE_HEIGHT);
        canvas_invert_color(canvas);
        count += DIGITS[digit][i];
    }
}

int get_digit_position(int index, BarcodeType* type) {
    int pos = type->startPos + index * 7;
    if(index >= type->numberOfDigits / 2) {
        pos += 5;
    }
    return pos;
}

int get_menu_text_location(int index) {
    return 20 + 10 * index;
}

int calculate_check_digit(PluginState* plugin_state, BarcodeType* type) {
    int checkDigit = 0;
    //add all odd positions. Confusing because 0index
    for(int i = 0; i < type->numberOfDigits - 1; i += 2) {
        checkDigit += plugin_state->barcodeNumeral[i];
    }

    checkDigit = checkDigit * 3; //times 3

    //add all even positions to above. Confusing because 0index
    for(int i = 1; i < type->numberOfDigits - 1; i += 2) {
        checkDigit += plugin_state->barcodeNumeral[i];
    }

    checkDigit = checkDigit % 10; //mod 10

    //if m = 0 then x12 = 0, otherwise x12 is 10 - m
    return (10 - checkDigit) % 10;
}

static void render_callback(Canvas* const canvas, void* ctx) {
    PluginState* plugin_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(plugin_state == NULL) {
        return;
    }

    if(plugin_state->mode == MenuMode) {
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_str_aligned(canvas, 64, 6, AlignCenter, AlignCenter, "MENU");
        canvas_draw_frame(canvas, 50, 0, 29, 11); //box around Menu
        canvas_draw_str_aligned(
            canvas, 64, get_menu_text_location(0), AlignCenter, AlignCenter, "View");
        canvas_draw_str_aligned(
            canvas, 64, get_menu_text_location(1), AlignCenter, AlignCenter, "Edit");
        canvas_draw_str_aligned(
            canvas, 64, get_menu_text_location(2), AlignCenter, AlignCenter, "Parity?");

        canvas_draw_frame(canvas, 83, get_menu_text_location(2) - 3, 6, 6);
        if(plugin_state->doParityCalculation == true) {
            canvas_draw_box(canvas, 85, get_menu_text_location(2) - 1, 2, 2);
        }
        canvas_draw_str_aligned(
            canvas,
            64,
            get_menu_text_location(3),
            AlignCenter,
            AlignCenter,
            (barcodeTypes[plugin_state->barcodeTypeIndex])->name);
        canvas_draw_disc(
            canvas, 40, get_menu_text_location(plugin_state->menuIndex) - 1, 2); //draw menu cursor
    } else {
        BarcodeType* type = barcodeTypes[plugin_state->barcodeTypeIndex];

        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, type->startPos - 3, BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);
        canvas_draw_box(
            canvas,
            (type->startPos - 1),
            BARCODE_Y_START,
            1,
            BARCODE_HEIGHT + 2); //start saftey

        for(int index = 0; index < type->numberOfDigits; index++) {
            bool isOnRight = false;
            if(index >= type->numberOfDigits / 2) {
                isOnRight = true;
            }
            if((index == type->numberOfDigits - 1) &&
               (plugin_state->doParityCalculation)) { //calculate the check digit
                int checkDigit = calculate_check_digit(plugin_state, type);
                plugin_state->barcodeNumeral[type->numberOfDigits - 1] = checkDigit;
            }
            int digitPosition =
                get_digit_position(index, barcodeTypes[plugin_state->barcodeTypeIndex]);
            draw_digit(canvas, plugin_state->barcodeNumeral[index], isOnRight, digitPosition);
        }

        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, 62, BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);
        canvas_draw_box(canvas, 64, BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);

        if(plugin_state->mode == EditMode) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(
                canvas,
                get_digit_position(
                    plugin_state->editingIndex, barcodeTypes[plugin_state->barcodeTypeIndex]) -
                    1,
                63,
                7,
                1); //draw editing cursor
        }

        int endSafetyPosition = get_digit_position(type->numberOfDigits - 1, type) + 7;
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, endSafetyPosition, BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);
        canvas_draw_box(
            canvas,
            (endSafetyPosition + 2),
            BARCODE_Y_START,
            1,
            BARCODE_HEIGHT + 2); //end safety
    }

    release_mutex((ValueMutex*)ctx, plugin_state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void barcode_generator_state_init(PluginState* const plugin_state) {
    for(int i = 0; i < 12; ++i) {
        plugin_state->barcodeNumeral[i] = i % 10;
    }
    plugin_state->editingIndex = 0;
    plugin_state->mode = ViewMode;
    plugin_state->doParityCalculation = true;
    plugin_state->menuIndex = MENU_INDEX_VIEW;
    plugin_state->barcodeTypeIndex = 0;
}

static bool handle_key_press_view(InputKey key, PluginState* plugin_state) {
    switch(key) {
    case InputKeyOk:
    case InputKeyBack:
        plugin_state->mode = MenuMode;
        break;

    default:
        break;
    }

    return true;
}

static bool handle_key_press_edit(InputKey key, PluginState* plugin_state) {
    int barcodeMaxIndex = plugin_state->doParityCalculation ?
                              barcodeTypes[plugin_state->barcodeTypeIndex]->numberOfDigits - 1 :
                              barcodeTypes[plugin_state->barcodeTypeIndex]->numberOfDigits;

    switch(key) {
    case InputKeyUp:
        plugin_state->barcodeNumeral[plugin_state->editingIndex] =
            (plugin_state->barcodeNumeral[plugin_state->editingIndex] + 1) % 10;
        break;

    case InputKeyDown:
        plugin_state->barcodeNumeral[plugin_state->editingIndex] =
            (plugin_state->barcodeNumeral[plugin_state->editingIndex] == 0) ?
                9 :
                plugin_state->barcodeNumeral[plugin_state->editingIndex] - 1;
        break;

    case InputKeyRight:
        plugin_state->editingIndex = (plugin_state->editingIndex + 1) % barcodeMaxIndex;
        break;

    case InputKeyLeft:
        plugin_state->editingIndex = (plugin_state->editingIndex == 0) ?
                                         barcodeMaxIndex - 1 :
                                         plugin_state->editingIndex - 1;
        break;

    case InputKeyOk:
    case InputKeyBack:
        plugin_state->mode = MenuMode;
        break;

    default:
        break;
    }

    return true;
}

static bool handle_key_press_menu(InputKey key, PluginState* plugin_state) {
    switch(key) {
    case InputKeyUp:
        plugin_state->menuIndex = (plugin_state->menuIndex == MENU_INDEX_VIEW) ?
                                      MENU_INDEX_TYPE :
                                      plugin_state->menuIndex - 1;
        break;

    case InputKeyDown:
        plugin_state->menuIndex = (plugin_state->menuIndex + 1) % 4;
        break;

    case InputKeyRight:
        if(plugin_state->menuIndex == MENU_INDEX_TYPE) {
            plugin_state->barcodeTypeIndex =
                (plugin_state->barcodeTypeIndex == NUMBER_OF_BARCODE_TYPES - 1) ?
                    0 :
                    plugin_state->barcodeTypeIndex + 1;
        } else if(plugin_state->menuIndex == MENU_INDEX_PARITY) {
            plugin_state->doParityCalculation = !plugin_state->doParityCalculation;
        }
        break;
    case InputKeyLeft:
        if(plugin_state->menuIndex == MENU_INDEX_TYPE) {
            plugin_state->barcodeTypeIndex = (plugin_state->barcodeTypeIndex == 0) ?
                                                 NUMBER_OF_BARCODE_TYPES - 1 :
                                                 plugin_state->barcodeTypeIndex - 1;
        } else if(plugin_state->menuIndex == MENU_INDEX_PARITY) {
            plugin_state->doParityCalculation = !plugin_state->doParityCalculation;
        }
        break;

    case InputKeyOk:
        if(plugin_state->menuIndex == MENU_INDEX_VIEW) {
            plugin_state->mode = ViewMode;
        } else if(plugin_state->menuIndex == MENU_INDEX_EDIT) {
            plugin_state->mode = EditMode;
        } else if(plugin_state->menuIndex == MENU_INDEX_PARITY) {
            plugin_state->doParityCalculation = !plugin_state->doParityCalculation;
        } else if(plugin_state->menuIndex == MENU_INDEX_TYPE) {
            plugin_state->barcodeTypeIndex =
                (plugin_state->barcodeTypeIndex == NUMBER_OF_BARCODE_TYPES - 1) ?
                    0 :
                    plugin_state->barcodeTypeIndex + 1;
        }
        break;

    case InputKeyBack:
        return false;

    default:
        break;
    }

    return true;
}

int32_t barcode_generator_app(void* p) {
    UNUSED(p);

    init_types();

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    PluginState* plugin_state = malloc(sizeof(PluginState));
    barcode_generator_state_init(plugin_state);
    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, plugin_state, sizeof(PluginState))) {
        FURI_LOG_E("barcode_generator", "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(plugin_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        PluginState* plugin_state = (PluginState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey &&
               ((event.input.type == InputTypePress) || (event.input.type == InputTypeRepeat))) {
                switch(plugin_state->mode) {
                case ViewMode:
                    processing = handle_key_press_view(event.input.key, plugin_state);
                    break;
                case EditMode:
                    processing = handle_key_press_edit(event.input.key, plugin_state);
                    break;
                case MenuMode:
                    processing = handle_key_press_menu(event.input.key, plugin_state);
                    break;
                default:
                    break;
                }
            }
        } else {
            FURI_LOG_D("barcode_generator", "osMessageQueue: event timeout");
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    return 0;
}
