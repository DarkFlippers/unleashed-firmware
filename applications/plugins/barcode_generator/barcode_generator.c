#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

#define BARCODE_STARTING_POS 16
#define BARCODE_HEIGHT 50
#define BARCODE_Y_START 3
#define BARCODE_TEXT_OFFSET 9

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    int barcodeNumeral[12]; //The current barcode number
    int editingIndex; //The index of the editing symbol
    int menuIndex; //The index of the menu cursor
    int modeIndex; //Set to 0 for view, 1 for edit, 2 for menu
    bool doParityCalculation; //Should do parity check?
} PluginState;

void number_0(
    Canvas* canvas,
    bool rightHand,
    int startingPosition) { //UPC Code for #0 on left is OOOIIOI
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(
        canvas, startingPosition, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, "0");
    if(rightHand) {
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_box(canvas, startingPosition, BARCODE_Y_START, 3, BARCODE_HEIGHT); //OOO
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 3, BARCODE_Y_START, 2, BARCODE_HEIGHT); //II
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 5, BARCODE_Y_START, 1, BARCODE_HEIGHT); //O
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 6, BARCODE_Y_START, 1, BARCODE_HEIGHT); //I
}
void number_1(
    Canvas* canvas,
    bool rightHand,
    int startingPosition) { //UPC Code for #1 on left is OOIIOOI
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(
        canvas, startingPosition, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, "1");

    if(rightHand) {
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_box(canvas, startingPosition, BARCODE_Y_START, 2, BARCODE_HEIGHT); //OO
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 2, BARCODE_Y_START, 2, BARCODE_HEIGHT); //II
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 4, BARCODE_Y_START, 2, BARCODE_HEIGHT); //OO
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 6, BARCODE_Y_START, 1, BARCODE_HEIGHT); //I
}
void number_2(
    Canvas* canvas,
    bool rightHand,
    int startingPosition) { //UPC Code for #2 on left is OOIOOII
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(
        canvas, startingPosition, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, "2");
    if(rightHand) {
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_box(canvas, startingPosition, BARCODE_Y_START, 2, BARCODE_HEIGHT); //OO
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 2, BARCODE_Y_START, 1, BARCODE_HEIGHT); //I
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 3, BARCODE_Y_START, 2, BARCODE_HEIGHT); //OO
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 5, BARCODE_Y_START, 2, BARCODE_HEIGHT); //II
}
void number_3(
    Canvas* canvas,
    bool rightHand,
    int startingPosition) { //UPC Code for #3 on left is OIIIIOI
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(
        canvas, startingPosition, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, "3");
    if(rightHand) {
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_box(canvas, startingPosition, BARCODE_Y_START, 1, BARCODE_HEIGHT); //O
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 1, BARCODE_Y_START, 4, BARCODE_HEIGHT); //IIII
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 5, BARCODE_Y_START, 1, BARCODE_HEIGHT); //O
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 6, BARCODE_Y_START, 1, BARCODE_HEIGHT); //I
}
void number_4(
    Canvas* canvas,
    bool rightHand,
    int startingPosition) { //UPC Code for #4 on left is OIOOOII
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(
        canvas, startingPosition, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, "4");
    if(rightHand) {
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_box(canvas, startingPosition, BARCODE_Y_START, 1, BARCODE_HEIGHT); //O
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 1, BARCODE_Y_START, 1, BARCODE_HEIGHT); //I
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 2, BARCODE_Y_START, 3, BARCODE_HEIGHT); //OOO
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 5, BARCODE_Y_START, 2, BARCODE_HEIGHT); //II
}
void number_5(
    Canvas* canvas,
    bool rightHand,
    int startingPosition) { //UPC Code for #5 on left is OIIOOOI
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(
        canvas, startingPosition, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, "5");
    if(rightHand) {
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_box(canvas, startingPosition, BARCODE_Y_START, 1, BARCODE_HEIGHT); //O
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 1, BARCODE_Y_START, 2, BARCODE_HEIGHT); //II
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 3, BARCODE_Y_START, 3, BARCODE_HEIGHT); //OOO
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 6, BARCODE_Y_START, 1, BARCODE_HEIGHT); //I
}
void number_6(
    Canvas* canvas,
    bool rightHand,
    int startingPosition) { //UPC Code for #6 on left is OIOIIII
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(
        canvas, startingPosition, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, "6");
    if(rightHand) {
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_box(canvas, startingPosition, BARCODE_Y_START, 1, BARCODE_HEIGHT); //O
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 1, BARCODE_Y_START, 1, BARCODE_HEIGHT); //I
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 2, BARCODE_Y_START, 1, BARCODE_HEIGHT); //O
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 3, BARCODE_Y_START, 4, BARCODE_HEIGHT); //IIII
}
void number_7(
    Canvas* canvas,
    bool rightHand,
    int startingPosition) { //UPC Code for #7 on left is OIIIOII
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(
        canvas, startingPosition, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, "7");
    if(rightHand) {
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_box(canvas, startingPosition, BARCODE_Y_START, 1, BARCODE_HEIGHT); //O
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 1, BARCODE_Y_START, 3, BARCODE_HEIGHT); //III
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 4, BARCODE_Y_START, 1, BARCODE_HEIGHT); //O
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 5, BARCODE_Y_START, 2, BARCODE_HEIGHT); //II
}
void number_8(
    Canvas* canvas,
    bool rightHand,
    int startingPosition) { //UPC Code for #8 on left is OIIOIII
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(
        canvas, startingPosition, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, "8");
    if(rightHand) {
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_box(canvas, startingPosition, BARCODE_Y_START, 1, BARCODE_HEIGHT); //O
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 1, BARCODE_Y_START, 2, BARCODE_HEIGHT); //II
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 3, BARCODE_Y_START, 1, BARCODE_HEIGHT); //O
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 4, BARCODE_Y_START, 3, BARCODE_HEIGHT); //III
}
void number_9(
    Canvas* canvas,
    bool rightHand,
    int startingPosition) { //UPC Code for #9 on left is OOOIOII
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(
        canvas, startingPosition, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, "9");
    if(rightHand) {
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_box(canvas, startingPosition, BARCODE_Y_START, 3, BARCODE_HEIGHT); //OOO
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 3, BARCODE_Y_START, 1, BARCODE_HEIGHT); //I
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 4, BARCODE_Y_START, 1, BARCODE_HEIGHT); //O
    canvas_invert_color(canvas);
    canvas_draw_box(canvas, startingPosition + 5, BARCODE_Y_START, 2, BARCODE_HEIGHT); //II
}

static void render_callback(Canvas* const canvas, void* ctx) {
    PluginState* plugin_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(plugin_state == NULL) {
        return;
    }

    //I originally had all of these values being generated at runtime by math, but that kept giving me trouble.
    int editingMarkerPosition[12] = {
        19,
        26,
        33,
        40,
        47,
        54,
        66,
        73,
        80,
        87,
        94,
        101,
    };
    int menuTextLocations[6] = {
        20,
        30,
        40,
        50,
        60,
        70,
    };

    if(plugin_state->modeIndex == 2) { //if in the menu
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_str_aligned(canvas, 64, 6, AlignCenter, AlignCenter, "MENU");
        canvas_draw_frame(canvas, 50, 0, 29, 11); //box around Menu
        canvas_draw_str_aligned(
            canvas, 64, menuTextLocations[0], AlignCenter, AlignCenter, "View");
        canvas_draw_str_aligned(
            canvas, 64, menuTextLocations[1], AlignCenter, AlignCenter, "Edit");
        canvas_draw_str_aligned(
            canvas, 64, menuTextLocations[2], AlignCenter, AlignCenter, "Parity?");

        canvas_draw_frame(canvas, 81, menuTextLocations[2] - 2, 6, 6);
        if(plugin_state->doParityCalculation == true) {
            canvas_draw_box(canvas, 83, menuTextLocations[2], 2, 2);
        }
        canvas_draw_str_aligned(
            canvas, 64, menuTextLocations[3], AlignCenter, AlignCenter, "TODO");
        canvas_draw_disc(
            canvas, 40, menuTextLocations[plugin_state->menuIndex], 2); //draw menu cursor
    }

    if(plugin_state->modeIndex != 2) { //if not in the menu
        canvas_set_color(canvas, ColorBlack);
        //canvas_draw_glyph(canvas, 115, BARCODE_Y_START + BARCODE_HEIGHT + BARCODE_TEXT_OFFSET, 'M');
        canvas_draw_box(canvas, BARCODE_STARTING_POS, BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);
        //canvas_draw_box(canvas, BARCODE_STARTING_POS + 1, 1, 1, 50); //left blank on purpose
        canvas_draw_box(
            canvas,
            (BARCODE_STARTING_POS + 2),
            BARCODE_Y_START,
            1,
            BARCODE_HEIGHT + 2); //start saftey
        for(int index = 0; index < 12; index++) {
            bool isOnRight = false;
            if(index >= 6) {
                isOnRight = true;
            }
            if((index == 11) && (plugin_state->doParityCalculation)) { //calculate the check digit
                int checkDigit =
                    plugin_state->barcodeNumeral[0] + plugin_state->barcodeNumeral[2] +
                    plugin_state->barcodeNumeral[4] + plugin_state->barcodeNumeral[6] +
                    plugin_state->barcodeNumeral[8] + plugin_state->barcodeNumeral[10];
                //add all odd positions Confusing because 0index
                checkDigit = checkDigit * 3; //times 3
                checkDigit += plugin_state->barcodeNumeral[1] + plugin_state->barcodeNumeral[3] +
                              plugin_state->barcodeNumeral[5] + plugin_state->barcodeNumeral[7] +
                              plugin_state->barcodeNumeral[9];
                //add all even positions to above. Confusing because 0index
                checkDigit = checkDigit % 10; //mod 10
                //if m - 0 then x12 = 0, otherwise x12 is 10 - m
                if(checkDigit == 0) {
                    plugin_state->barcodeNumeral[11] = 0;
                } else {
                    checkDigit = 10 - checkDigit;
                    plugin_state->barcodeNumeral[11] = checkDigit;
                }
            }
            switch(plugin_state->barcodeNumeral[index]) {
            case 0:
                number_0(canvas, isOnRight, editingMarkerPosition[index]);
                break;
            case 1:
                number_1(canvas, isOnRight, editingMarkerPosition[index]);
                break;
            case 2:
                number_2(canvas, isOnRight, editingMarkerPosition[index]);
                break;
            case 3:
                number_3(canvas, isOnRight, editingMarkerPosition[index]);
                break;
            case 4:
                number_4(canvas, isOnRight, editingMarkerPosition[index]);
                break;
            case 5:
                number_5(canvas, isOnRight, editingMarkerPosition[index]);
                break;
            case 6:
                number_6(canvas, isOnRight, editingMarkerPosition[index]);
                break;
            case 7:
                number_7(canvas, isOnRight, editingMarkerPosition[index]);
                break;
            case 8:
                number_8(canvas, isOnRight, editingMarkerPosition[index]);
                break;
            case 9:
                number_9(canvas, isOnRight, editingMarkerPosition[index]);
                break;
            }
        }

        canvas_set_color(canvas, ColorBlack);
        //canvas_draw_box(canvas, BARCODE_STARTING_POS + 45, BARCODE_Y_START, 1, BARCODE_HEIGHT);
        canvas_draw_box(canvas, BARCODE_STARTING_POS + 46, BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);
        //canvas_draw_box(canvas, BARCODE_STARTING_POS + 47, BARCODE_Y_START, 1, BARCODE_HEIGHT);
        canvas_draw_box(canvas, BARCODE_STARTING_POS + 48, BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);
        //canvas_draw_box(canvas, BARCODE_STARTING_POS + 49, BARCODE_Y_START, 1, BARCODE_HEIGHT);

        if(plugin_state->modeIndex == 1) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(
                canvas,
                editingMarkerPosition[plugin_state->editingIndex],
                63,
                7,
                1); //draw editing cursor
        }

        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, BARCODE_STARTING_POS + 92, BARCODE_Y_START, 1, BARCODE_HEIGHT + 2);
        //canvas_draw_box(canvas, 14, 1, 1, 50); //left blank on purpose
        canvas_draw_box(
            canvas,
            (BARCODE_STARTING_POS + 2) + 92,
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

static void barcode_UPCA_generator_state_init(PluginState* const plugin_state) {
    int i;
    for(i = 0; i < 12; ++i) {
        if(i > 9) {
            plugin_state->barcodeNumeral[i] = i - 10;
        } else if(i < 10) {
            plugin_state->barcodeNumeral[i] = i;
        }
    }
    plugin_state->editingIndex = 0;
    plugin_state->modeIndex = 0;
    plugin_state->doParityCalculation = true;
    plugin_state->menuIndex = 0;
}

int32_t barcode_UPCA_generator_app(void* p) {
    UNUSED(p);
    //testing
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    PluginState* plugin_state = malloc(sizeof(PluginState));
    barcode_UPCA_generator_state_init(plugin_state);
    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, plugin_state, sizeof(PluginState))) {
        FURI_LOG_E("barcode_UPCA_generator", "cannot create mutex\r\n");
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
        int barcodeMaxIndex;
        if(plugin_state->doParityCalculation == true) {
            barcodeMaxIndex = 11;
        }
        if(plugin_state->doParityCalculation == false) {
            barcodeMaxIndex = 12;
        }

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if((event.input.type == InputTypePress) || (event.input.type == InputTypeRepeat)) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        if(plugin_state->modeIndex == 1) { //if edit mode
                            plugin_state->barcodeNumeral[plugin_state->editingIndex]++;
                        }
                        if(plugin_state->barcodeNumeral[plugin_state->editingIndex] > 9) {
                            plugin_state->barcodeNumeral[plugin_state->editingIndex] = 0;
                        }
                        if(plugin_state->modeIndex == 2) { //if menu mode
                            plugin_state->menuIndex--;
                        }
                        if(plugin_state->menuIndex < 0) {
                            plugin_state->menuIndex = 3;
                        }
                        break;
                    case InputKeyDown:
                        if(plugin_state->modeIndex == 1) {
                            plugin_state->barcodeNumeral[plugin_state->editingIndex]--;
                        }
                        if(plugin_state->barcodeNumeral[plugin_state->editingIndex] < 0) {
                            plugin_state->barcodeNumeral[plugin_state->editingIndex] = 9;
                        }
                        if(plugin_state->modeIndex == 2) { //if menu mode
                            plugin_state->menuIndex++;
                        }
                        if(plugin_state->menuIndex > 3) {
                            plugin_state->menuIndex = 0;
                        }
                        break;
                    case InputKeyRight:
                        if(plugin_state->modeIndex == 1) {
                            plugin_state->editingIndex++;
                        }
                        if(plugin_state->editingIndex >= barcodeMaxIndex) {
                            plugin_state->editingIndex = 0;
                        }
                        break;
                    case InputKeyLeft:
                        if(plugin_state->modeIndex == 1) {
                            plugin_state->editingIndex--;
                        }
                        if(plugin_state->editingIndex < 0) {
                            plugin_state->editingIndex = barcodeMaxIndex - 1;
                        }
                        break;
                    case InputKeyOk:
                        if((plugin_state->modeIndex == 0) ||
                           (plugin_state->modeIndex == 1)) { //if normal or edit more, open menu
                            plugin_state->modeIndex = 2;
                            break;
                        } else if(
                            (plugin_state->modeIndex == 2) &&
                            (plugin_state->menuIndex ==
                             1)) { //if hits select in menu, while index is 1. edit mode
                            plugin_state->modeIndex = 1;
                            break;
                        } else if(
                            (plugin_state->modeIndex == 2) &&
                            (plugin_state->menuIndex ==
                             0)) { //if hits select in menu, while index is 0. view mode
                            plugin_state->modeIndex = 0;
                            break;
                        } else if(
                            (plugin_state->modeIndex == 2) &&
                            (plugin_state->menuIndex ==
                             2)) { //if hits select in menu, while index is 2. Parity switch
                            plugin_state->doParityCalculation =
                                !plugin_state->doParityCalculation; //invert bool
                            break;
                        } else {
                            break;
                        }

                    case InputKeyBack:
                        if(plugin_state->modeIndex == 0) {
                            processing = false;
                        }
                        if(plugin_state->modeIndex == 2) {
                            plugin_state->modeIndex = 0;
                        }
                        break;
                    default:
                        break;
                    }
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

    return 0;
}
