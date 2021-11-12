#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>

#define TAG "KeypadTest"

typedef struct {
    bool press[5];
    uint16_t up;
    uint16_t down;
    uint16_t left;
    uint16_t right;
    uint16_t ok;
} KeypadTestState;

typedef enum {
    EventTypeInput,
} EventType;

typedef struct {
    union {
        InputEvent input;
    };
    EventType type;
} KeypadTestEvent;

static void keypad_test_reset_state(KeypadTestState* state) {
    state->left = 0;
    state->right = 0;
    state->up = 0;
    state->down = 0;
    state->ok = 0;
}

static void keypad_test_render_callback(Canvas* canvas, void* ctx) {
    KeypadTestState* state = (KeypadTestState*)acquire_mutex((ValueMutex*)ctx, 25);
    canvas_clear(canvas);
    char strings[5][20];

    sprintf(strings[0], "Ok: %d", state->ok);
    sprintf(strings[1], "L: %d", state->left);
    sprintf(strings[2], "R: %d", state->right);
    sprintf(strings[3], "U: %d", state->up);
    sprintf(strings[4], "D: %d", state->down);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "Keypad test");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 24, strings[1]);
    canvas_draw_str(canvas, 35, 24, strings[2]);
    canvas_draw_str(canvas, 0, 36, strings[3]);
    canvas_draw_str(canvas, 35, 36, strings[4]);
    canvas_draw_str(canvas, 0, 48, strings[0]);
    canvas_draw_circle(canvas, 100, 26, 25);

    if(state->press[0]) canvas_draw_disc(canvas, 118, 26, 5);
    if(state->press[1]) canvas_draw_disc(canvas, 82, 26, 5);
    if(state->press[2]) canvas_draw_disc(canvas, 100, 8, 5);
    if(state->press[3]) canvas_draw_disc(canvas, 100, 44, 5);
    if(state->press[4]) canvas_draw_disc(canvas, 100, 26, 5);

    canvas_draw_str(canvas, 10, 63, "[back] - reset, hold to exit");

    release_mutex((ValueMutex*)ctx, state);
}

static void keypad_test_input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = ctx;

    KeypadTestEvent event;
    event.type = EventTypeInput;
    event.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

int32_t keypad_test_app(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(32, sizeof(KeypadTestEvent), NULL);
    furi_check(event_queue);

    KeypadTestState _state = {{false, false, false, false, false}, 0, 0, 0, 0, 0};

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, &_state, sizeof(KeypadTestState))) {
        FURI_LOG_E(TAG, "cannot create mutex");
        return 0;
    }

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, keypad_test_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, keypad_test_input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    KeypadTestEvent event;
    while(1) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, osWaitForever);
        KeypadTestState* state = (KeypadTestState*)acquire_mutex_block(&state_mutex);

        if(event_status == osOK) {
            if(event.type == EventTypeInput) {
                FURI_LOG_I(
                    TAG,
                    "key: %s type: %s",
                    input_get_key_name(event.input.key),
                    input_get_type_name(event.input.type));

                if(event.input.type == InputTypeLong && event.input.key == InputKeyBack) {
                    release_mutex(&state_mutex, state);
                    break;
                }

                if(event.input.type == InputTypeShort && event.input.key == InputKeyBack) {
                    keypad_test_reset_state(state);
                }

                if(event.input.key == InputKeyRight) {
                    if(event.input.type == InputTypePress) {
                        state->press[0] = true;
                    } else if(event.input.type == InputTypeRelease) {
                        state->press[0] = false;
                    }

                    if(event.input.type == InputTypeShort) {
                        ++state->right;
                    }
                }

                if(event.input.key == InputKeyLeft) {
                    if(event.input.type == InputTypePress) {
                        state->press[1] = true;
                    } else if(event.input.type == InputTypeRelease) {
                        state->press[1] = false;
                    }

                    if(event.input.type == InputTypeShort) {
                        ++state->left;
                    }
                }

                if(event.input.key == InputKeyUp) {
                    if(event.input.type == InputTypePress) {
                        state->press[2] = true;
                    } else if(event.input.type == InputTypeRelease) {
                        state->press[2] = false;
                    }

                    if(event.input.type == InputTypeShort) {
                        ++state->up;
                    }
                }

                if(event.input.key == InputKeyDown) {
                    if(event.input.type == InputTypePress) {
                        state->press[3] = true;
                    } else if(event.input.type == InputTypeRelease) {
                        state->press[3] = false;
                    }

                    if(event.input.type == InputTypeShort) {
                        ++state->down;
                    }
                }

                if(event.input.key == InputKeyOk) {
                    if(event.input.type == InputTypePress) {
                        state->press[4] = true;
                    } else if(event.input.type == InputTypeRelease) {
                        state->press[4] = false;
                    }

                    if(event.input.type == InputTypeShort) {
                        ++state->ok;
                    }
                }
            }
        }
        view_port_update(view_port);
        release_mutex(&state_mutex, state);
    }
    // remove & free all stuff created by app
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);
    delete_mutex(&state_mutex);

    return 0;
}
