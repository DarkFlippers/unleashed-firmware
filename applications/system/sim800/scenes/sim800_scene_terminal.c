#include "../sim800_app_i.h"

#define SCENE_STATE_LOG   0
#define SCENE_STATE_INPUT 1
#define SCENE_STATE_FULL  2

#define TERMINAL_LINES 7

static void sim800_terminal_tail(Sim800App* app) {
    sim800_log_read(app, app->text_box_store);

    size_t breaks = 0;
    size_t size = furi_string_size(app->text_box_store);
    size_t cut = 0;

    for(size_t i = size; i > 0; i--) {
        if(furi_string_get_char(app->text_box_store, i - 1) == '\n') {
            breaks++;
            if(breaks > TERMINAL_LINES) {
                cut = i;
                break;
            }
        }
    }

    if(cut > 0) furi_string_right(app->text_box_store, cut);
    if(furi_string_size(app->text_box_store) == 0) {
        furi_string_set_str(app->text_box_store, "Nothing received yet.");
    }
}

static void sim800_terminal_show_log(Sim800App* app) {
    // The text box is pointed at text_box_store, which the tail is about to rewrite
    text_box_reset(app->text_box);
    sim800_terminal_tail(app);

    widget_reset(app->widget);
    widget_add_text_box_element(
        app->widget,
        0,
        0,
        128,
        51,
        AlignLeft,
        AlignBottom,
        furi_string_get_cstr(app->text_box_store),
        false);
    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Log", sim800_widget_button_callback, app);
    widget_add_button_element(
        app->widget, GuiButtonTypeCenter, "Send", sim800_widget_button_callback, app);

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneTerminal, SCENE_STATE_LOG);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewWidget);
}

/** The whole log in a text box, which is the only thing here that actually scrolls. */
static void sim800_terminal_show_full(Sim800App* app) {
    sim800_log_read(app, app->text_box_store);
    if(furi_string_size(app->text_box_store) == 0) {
        furi_string_set_str(app->text_box_store, "Nothing received yet.");
    }

    text_box_reset(app->text_box);
    text_box_set_font(app->text_box, TextBoxFontText);
    // Land on the newest output, then scroll back with up
    text_box_set_focus(app->text_box, TextBoxFocusEnd);
    text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneTerminal, SCENE_STATE_FULL);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewTextBox);
}

static void sim800_terminal_task(Sim800App* app) {
    sim800_uart_command(app->uart, app->text_store, 5000, NULL);
}

void sim800_scene_terminal_on_enter(void* context) {
    sim800_terminal_show_log(context);
}

bool sim800_scene_terminal_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;
    uint32_t state = scene_manager_get_scene_state(app->scene_manager, Sim800SceneTerminal);

    if(event.type == SceneManagerEventTypeBack &&
       (state == SCENE_STATE_INPUT || state == SCENE_STATE_FULL)) {
        sim800_terminal_show_log(app);
        return true;
    }

    if(event.type == SceneManagerEventTypeTick) {
        if(state == SCENE_STATE_LOG) sim800_terminal_show_log(app);
        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == Sim800CustomEventButtonLeft) {
        sim800_terminal_show_full(app);
        return true;
    }

    if(event.event == Sim800CustomEventButtonCenter) {
        strlcpy(app->text_store, "AT", SIM800_AT_CMD_MAX);
        sim800_text_input_reset(app->text_input);
        sim800_text_input_set_header_text(app->text_input, "AT command");
        sim800_text_input_set_minimum_length(app->text_input, 2);
        sim800_text_input_set_result_callback(
            app->text_input,
            sim800_input_done_callback,
            app,
            app->text_store,
            SIM800_AT_CMD_MAX,
            false);
        scene_manager_set_scene_state(app->scene_manager, Sim800SceneTerminal, SCENE_STATE_INPUT);
        view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewTextInput);
        return true;
    }

    if(event.event == Sim800CustomEventTextInputDone) {
        sim800_task_start(app, sim800_terminal_task);
        sim800_terminal_show_log(app);
        return true;
    }

    return false;
}

void sim800_scene_terminal_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
    text_box_reset(app->text_box);
    widget_reset(app->widget);
    sim800_text_input_reset(app->text_input);
}
