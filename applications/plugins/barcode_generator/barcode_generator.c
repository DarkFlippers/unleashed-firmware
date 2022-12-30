#include "barcode_generator.h"

static BarcodeType* barcodeTypes[NUMBER_OF_BARCODE_TYPES];

void init_types() {
    BarcodeType* upcA = malloc(sizeof(BarcodeType));
    upcA->name = "UPC-A";
    upcA->numberOfDigits = 12;
    upcA->startPos = 19;
    upcA->bartype = BarTypeUPCA;
    barcodeTypes[0] = upcA;

    BarcodeType* ean8 = malloc(sizeof(BarcodeType));
    ean8->name = "EAN-8";
    ean8->numberOfDigits = 8;
    ean8->startPos = 33;
    ean8->bartype = BarTypeEAN8;
    barcodeTypes[1] = ean8;

    BarcodeType* ean13 = malloc(sizeof(BarcodeType));
    ean13->name = "EAN-13";
    ean13->numberOfDigits = 13;
    ean13->startPos = 19;
    ean13->bartype = BarTypeEAN13;
    barcodeTypes[2] = ean13;
}

void draw_digit(
    Canvas* canvas,
    int digit,
    BarEncodingType rightHand,
    int startingPosition,
    bool drawlines) {
    char digitStr[2];
    snprintf(digitStr, 2, "%u", digit);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(
        canvas, startingPosition, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, digitStr);

    if(drawlines) {
        switch(rightHand) {
        case BarEncodingTypeLeft:
        case BarEncodingTypeRight:
            canvas_set_color(
                canvas, (rightHand == BarEncodingTypeRight) ? ColorBlack : ColorWhite);
            //int count = 0;
            for(int i = 0; i < 4; i++) {
                canvas_draw_box(
                    canvas, startingPosition, BARCODE_Y_START, DIGITS[digit][i], BARCODE_HEIGHT);
                canvas_invert_color(canvas);
                startingPosition += DIGITS[digit][i];
            }
            break;
        case BarEncodingTypeG:
            canvas_set_color(canvas, ColorWhite);
            //int count = 0;
            for(int i = 3; i >= 0; i--) {
                canvas_draw_box(
                    canvas, startingPosition, BARCODE_Y_START, DIGITS[digit][i], BARCODE_HEIGHT);
                canvas_invert_color(canvas);
                startingPosition += DIGITS[digit][i];
            }
            break;
        default:
            break;
        }
    }
}

int get_digit_position(int index, BarcodeType* type) {
    int pos = 0;
    switch(type->bartype) {
    case BarTypeEAN8:
    case BarTypeUPCA:
        pos = type->startPos + index * 7;
        if(index >= type->numberOfDigits / 2) {
            pos += 5;
        }
        break;
    case BarTypeEAN13:
        if(index == 0)
            pos = type->startPos - 10;
        else {
            pos = type->startPos + (index - 1) * 7;
            if((index - 1) >= type->numberOfDigits / 2) {
                pos += 5;
            }
        }
        break;
    default:
        break;
    }
    return pos;
}

int get_menu_text_location(int index) {
    return 20 + 10 * index;
}

int get_barcode_max_index(PluginState* plugin_state) {
    return plugin_state->barcode_state.doParityCalculation ?
               barcodeTypes[plugin_state->barcode_state.barcodeTypeIndex]->numberOfDigits - 1 :
               barcodeTypes[plugin_state->barcode_state.barcodeTypeIndex]->numberOfDigits;
}

int calculate_check_digit(PluginState* plugin_state, BarcodeType* type) {
    int checkDigit = 0;
    int checkDigitOdd = 0;
    int checkDigitEven = 0;
    //add all odd positions. Confusing because 0index
    for(int i = 0; i < type->numberOfDigits - 1; i += 2) {
        checkDigitOdd += plugin_state->barcode_state.barcodeNumeral[i];
    }

    //add all even positions to above. Confusing because 0index
    for(int i = 1; i < type->numberOfDigits - 1; i += 2) {
        checkDigitEven += plugin_state->barcode_state.barcodeNumeral[i];
    }

    if(type->bartype == BarTypeEAN13) {
        checkDigit = checkDigitEven * 3 + checkDigitOdd;
    } else {
        checkDigit = checkDigitOdd * 3 + checkDigitEven;
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
        if(plugin_state->barcode_state.doParityCalculation == true) {
            canvas_draw_box(canvas, 85, get_menu_text_location(2) - 1, 2, 2);
        }
        canvas_draw_str_aligned(
            canvas,
            64,
            get_menu_text_location(3),
            AlignCenter,
            AlignCenter,
            (barcodeTypes[plugin_state->barcode_state.barcodeTypeIndex])->name);
        canvas_draw_disc(
            canvas,
            40,
            get_menu_text_location(plugin_state->menuIndex) - 1,
            2); //draw menu cursor
    } else {
        BarcodeType* type = barcodeTypes[plugin_state->barcode_state.barcodeTypeIndex];

        //start saftey
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, type->startPos - 3, BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);
        canvas_draw_box(canvas, (type->startPos - 1), BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);

        int startpos = 0;
        int endpos = type->numberOfDigits;
        if(type->bartype == BarTypeEAN13) {
            startpos++;
            draw_digit(
                canvas,
                plugin_state->barcode_state.barcodeNumeral[0],
                BarEncodingTypeRight,
                get_digit_position(0, barcodeTypes[plugin_state->barcode_state.barcodeTypeIndex]),
                false);
        }
        if(plugin_state->barcode_state.doParityCalculation) { //calculate the check digit
            plugin_state->barcode_state.barcodeNumeral[type->numberOfDigits - 1] =
                calculate_check_digit(plugin_state, type);
        }
        for(int index = startpos; index < endpos; index++) {
            BarEncodingType barEncodingType = BarEncodingTypeLeft;
            if(type->bartype == BarTypeEAN13) {
                if(index - 1 >= (type->numberOfDigits - 1) / 2) {
                    barEncodingType = BarEncodingTypeRight;
                } else {
                    barEncodingType =
                        (FURI_BIT(
                            EAN13ENCODE[plugin_state->barcode_state.barcodeNumeral[0]],
                            index - 1)) ?
                            BarEncodingTypeG :
                            BarEncodingTypeLeft;
                }
            } else {
                if(index >= type->numberOfDigits / 2) {
                    barEncodingType = BarEncodingTypeRight;
                }
            }

            int digitPosition = get_digit_position(
                index, barcodeTypes[plugin_state->barcode_state.barcodeTypeIndex]);
            draw_digit(
                canvas,
                plugin_state->barcode_state.barcodeNumeral[index],
                barEncodingType,
                digitPosition,
                true);
        }

        //central separator
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, 62, BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);
        canvas_draw_box(canvas, 64, BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);

        if(plugin_state->mode == EditMode) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(
                canvas,
                get_digit_position(
                    plugin_state->editingIndex,
                    barcodeTypes[plugin_state->barcode_state.barcodeTypeIndex]) -
                    1,
                63,
                7,
                1); //draw editing cursor
        }

        //end safety
        int endSafetyPosition = get_digit_position(type->numberOfDigits - 1, type) + 7;
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, endSafetyPosition, BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);
        canvas_draw_box(canvas, (endSafetyPosition + 2), BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);
    }

    release_mutex((ValueMutex*)ctx, plugin_state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void barcode_generator_state_init(PluginState* plugin_state) {
    plugin_state->editingIndex = 0;
    plugin_state->mode = ViewMode;
    plugin_state->menuIndex = MENU_INDEX_VIEW;
    if(!LOAD_BARCODE_SETTINGS(&plugin_state->barcode_state)) {
        for(int i = 0; i < BARCODE_MAX_LENS; ++i) {
            plugin_state->barcode_state.barcodeNumeral[i] = i % 10;
        }
        plugin_state->barcode_state.doParityCalculation = true;
        plugin_state->barcode_state.barcodeTypeIndex = 0;
    }
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
    int barcodeMaxIndex = get_barcode_max_index(plugin_state);

    switch(key) {
    case InputKeyUp:
        plugin_state->barcode_state.barcodeNumeral[plugin_state->editingIndex] =
            (plugin_state->barcode_state.barcodeNumeral[plugin_state->editingIndex] + 1) % 10;
        break;

    case InputKeyDown:
        plugin_state->barcode_state.barcodeNumeral[plugin_state->editingIndex] =
            (plugin_state->barcode_state.barcodeNumeral[plugin_state->editingIndex] == 0) ?
                9 :
                plugin_state->barcode_state.barcodeNumeral[plugin_state->editingIndex] - 1;
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
            plugin_state->barcode_state.barcodeTypeIndex =
                (plugin_state->barcode_state.barcodeTypeIndex == NUMBER_OF_BARCODE_TYPES - 1) ?
                    0 :
                    plugin_state->barcode_state.barcodeTypeIndex + 1;
        } else if(plugin_state->menuIndex == MENU_INDEX_PARITY) {
            plugin_state->barcode_state.doParityCalculation =
                !plugin_state->barcode_state.doParityCalculation;
        }
        break;
    case InputKeyLeft:
        if(plugin_state->menuIndex == MENU_INDEX_TYPE) {
            plugin_state->barcode_state.barcodeTypeIndex =
                (plugin_state->barcode_state.barcodeTypeIndex == 0) ?
                    NUMBER_OF_BARCODE_TYPES - 1 :
                    plugin_state->barcode_state.barcodeTypeIndex - 1;
        } else if(plugin_state->menuIndex == MENU_INDEX_PARITY) {
            plugin_state->barcode_state.doParityCalculation =
                !plugin_state->barcode_state.doParityCalculation;
        }
        break;

    case InputKeyOk:
        if(plugin_state->menuIndex == MENU_INDEX_VIEW) {
            plugin_state->mode = ViewMode;
        } else if(plugin_state->menuIndex == MENU_INDEX_EDIT) {
            plugin_state->mode = EditMode;
        } else if(plugin_state->menuIndex == MENU_INDEX_PARITY) {
            plugin_state->barcode_state.doParityCalculation =
                !plugin_state->barcode_state.doParityCalculation;
        } else if(plugin_state->menuIndex == MENU_INDEX_TYPE) {
            plugin_state->barcode_state.barcodeTypeIndex =
                (plugin_state->barcode_state.barcodeTypeIndex == NUMBER_OF_BARCODE_TYPES - 1) ?
                    0 :
                    plugin_state->barcode_state.barcodeTypeIndex + 1;
        }
        break;

    case InputKeyBack:
        return false;

    default:
        break;
    }
    int barcodeMaxIndex = get_barcode_max_index(plugin_state);
    if(plugin_state->editingIndex >= barcodeMaxIndex)
        plugin_state->editingIndex = barcodeMaxIndex - 1;

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
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    // save settings
    SAVE_BARCODE_SETTINGS(&plugin_state->barcode_state);
    free(plugin_state);

    return 0;
}
