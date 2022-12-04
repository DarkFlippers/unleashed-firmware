#include <furi.h>
#include <input/input.h>
#include <stdlib.h>

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/text_input.h>
#include <gui/modules/text_box.h>

#define TEXT_BUFFER_SIZE 256

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    ViewDispatcher* view_dispatcher;
    TextInput* text_input;
    TextBox* text_box;
    char input[TEXT_BUFFER_SIZE];
    char output[(TEXT_BUFFER_SIZE * 26) + (26)]; // linebreaks
} CaesarState;

static void string_to_uppercase(char* input) {
    int i;
    for(i = 0; input[i] != '\0'; i++) {
        if(input[i] >= 'a' && input[i] <= 'z') {
            input[i] = input[i] - 32;
        } else {
            input[i] = input[i];
        }
    }
}

static void build_output(char* input, char* output) {
    int out = 0;
    for(int rot = 1; rot < 26; rot++) {
        int in;
        for(in = 0; input[in] != '\0'; in++) {
            if(input[in] >= 'A' && input[in] <= 'Z') {
                output[out] = 65 + (((input[in] - 65) + rot) % 26);
            } else {
                output[out] = input[in];
            }
            out++;
        }
        output[out] = '\n';
        out++;
    }
    output[out] = '\0';
}

static void text_input_callback(void* ctx) {
    CaesarState* caesar_state = acquire_mutex((ValueMutex*)ctx, 25);
    FURI_LOG_D("caesar_cipher", "Input text: %s", caesar_state->input);
    // this is where we build the output.
    string_to_uppercase(caesar_state->input);
    FURI_LOG_D("caesar_cipher", "Upper text: %s", caesar_state->input);
    build_output(caesar_state->input, caesar_state->output);
    text_box_set_text(caesar_state->text_box, caesar_state->output);
    view_dispatcher_switch_to_view(caesar_state->view_dispatcher, 1);

    release_mutex((ValueMutex*)ctx, caesar_state);
}

static bool back_event_callback(void* ctx) {
    const CaesarState* caesar_state = acquire_mutex((ValueMutex*)ctx, 25);
    view_dispatcher_stop(caesar_state->view_dispatcher);
    release_mutex((ValueMutex*)ctx, caesar_state);
    return true;
}

static void caesar_cipher_state_init(CaesarState* const caesar_state) {
    caesar_state->view_dispatcher = view_dispatcher_alloc();
    caesar_state->text_input = text_input_alloc();
    caesar_state->text_box = text_box_alloc();
    text_box_set_font(caesar_state->text_box, TextBoxFontText);
}

static void caesar_cipher_state_free(CaesarState* const caesar_state) {
    text_input_free(caesar_state->text_input);
    text_box_free(caesar_state->text_box);
    view_dispatcher_remove_view(caesar_state->view_dispatcher, 0);
    view_dispatcher_remove_view(caesar_state->view_dispatcher, 1);
    view_dispatcher_free(caesar_state->view_dispatcher);
    free(caesar_state);
}

int32_t caesar_cipher_app() {
    CaesarState* caesar_state = malloc(sizeof(CaesarState));

    FURI_LOG_D("caesar_cipher", "Running caesar_cipher_state_init");
    caesar_cipher_state_init(caesar_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, caesar_state, sizeof(CaesarState))) {
        FURI_LOG_E("caesar_cipher", "cannot create mutex\r\n");
        free(caesar_state);
        return 255;
    }

    FURI_LOG_D("caesar_cipher", "Assigning text input callback");
    text_input_set_result_callback(
        caesar_state->text_input,
        text_input_callback,
        &state_mutex,
        caesar_state->input,
        TEXT_BUFFER_SIZE,
        //clear default text
        true);
    text_input_set_header_text(caesar_state->text_input, "Input");

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    //gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    FURI_LOG_D("caesar_cipher", "Enabling view dispatcher queue");
    view_dispatcher_enable_queue(caesar_state->view_dispatcher);

    FURI_LOG_D("caesar_cipher", "Adding text input view to dispatcher");
    view_dispatcher_add_view(
        caesar_state->view_dispatcher, 0, text_input_get_view(caesar_state->text_input));
    view_dispatcher_add_view(
        caesar_state->view_dispatcher, 1, text_box_get_view(caesar_state->text_box));
    FURI_LOG_D("caesar_cipher", "Attaching view dispatcher to GUI");
    view_dispatcher_attach_to_gui(
        caesar_state->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    FURI_LOG_D("ceasar_cipher", "starting view dispatcher");
    view_dispatcher_set_navigation_event_callback(
        caesar_state->view_dispatcher, back_event_callback);
    view_dispatcher_set_event_callback_context(caesar_state->view_dispatcher, &state_mutex);
    view_dispatcher_switch_to_view(caesar_state->view_dispatcher, 0);
    view_dispatcher_run(caesar_state->view_dispatcher);

    furi_record_close("gui");
    delete_mutex(&state_mutex);
    caesar_cipher_state_free(caesar_state);

    return 0;
}
