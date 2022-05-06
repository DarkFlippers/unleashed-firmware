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
    osMessageQueuePut(event_queue, input_event, 0, osWaitForever);
}

int32_t keypad_test_app(void* p) {
    UNUSED(p);
    osMessageQueueId_t event_queue = osMessageQueueNew(32, sizeof(InputEvent), NULL);
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

    InputEvent event;
    while(osMessageQueueGet(event_queue, &event, NULL, osWaitForever) == osOK) {
        KeypadTestState* state = (KeypadTestState*)acquire_mutex_block(&state_mutex);
        FURI_LOG_I(
            TAG,
            "key: %s type: %s",
            input_get_key_name(event.key),
            input_get_type_name(event.type));

        if(event.key == InputKeyRight) {
            if(event.type == InputTypePress) {
                state->press[0] = true;
            } else if(event.type == InputTypeRelease) {
                state->press[0] = false;
            } else if(event.type == InputTypeShort) {
                ++state->right;
            }
        } else if(event.key == InputKeyLeft) {
            if(event.type == InputTypePress) {
                state->press[1] = true;
            } else if(event.type == InputTypeRelease) {
                state->press[1] = false;
            } else if(event.type == InputTypeShort) {
                ++state->left;
            }
        } else if(event.key == InputKeyUp) {
            if(event.type == InputTypePress) {
                state->press[2] = true;
            } else if(event.type == InputTypeRelease) {
                state->press[2] = false;
            } else if(event.type == InputTypeShort) {
                ++state->up;
            }
        } else if(event.key == InputKeyDown) {
            if(event.type == InputTypePress) {
                state->press[3] = true;
            } else if(event.type == InputTypeRelease) {
                state->press[3] = false;
            } else if(event.type == InputTypeShort) {
                ++state->down;
            }
        } else if(event.key == InputKeyOk) {
            if(event.type == InputTypePress) {
                state->press[4] = true;
            } else if(event.type == InputTypeRelease) {
                state->press[4] = false;
            } else if(event.type == InputTypeShort) {
                ++state->ok;
            }
        } else if(event.key == InputKeyBack) {
            if(event.type == InputTypeLong) {
                release_mutex(&state_mutex, state);
                break;
            } else if(event.type == InputTypeShort) {
                keypad_test_reset_state(state);
            }
        }

        release_mutex(&state_mutex, state);
        view_port_update(view_port);
    }

    // remove & free all stuff created by app
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);
    delete_mutex(&state_mutex);

    furi_record_close("gui");

    return 0;
}
