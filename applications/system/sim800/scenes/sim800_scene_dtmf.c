#include "../sim800_app_i.h"

#define SCENE_STATE_INPUT  0
#define SCENE_STATE_RESULT 1

static void sim800_dtmf_task(Sim800App* app) {
    FuriString* cmd = furi_string_alloc();

    app->task_result = Sim800AtResultOk;
    for(const char* digit = app->text_store; *digit != '\0'; digit++) {
        if(*digit == ' ') continue;
        furi_string_printf(cmd, "AT+VTS=%c", *digit);
        Sim800AtResult result =
            sim800_uart_command(app->uart, furi_string_get_cstr(cmd), 3000, NULL);
        if(result != Sim800AtResultOk) {
            app->task_result = result;
            break;
        }
    }

    furi_string_free(cmd);
}

void sim800_scene_dtmf_on_enter(void* context) {
    Sim800App* app = context;

    app->text_store[0] = '\0';

    sim800_text_input_reset(app->text_input);
    sim800_text_input_set_header_text(app->text_input, "DTMF digits");
    sim800_text_input_set_minimum_length(app->text_input, 1);
    sim800_text_input_set_result_callback(
        app->text_input, sim800_input_done_callback, app, app->text_store, SIM800_DTMF_MAX, false);

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneDtmf, SCENE_STATE_INPUT);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewTextInput);
}

bool sim800_scene_dtmf_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeBack &&
       scene_manager_get_scene_state(app->scene_manager, Sim800SceneDtmf) == SCENE_STATE_RESULT) {
        sim800_scene_dtmf_on_enter(app);
        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == Sim800CustomEventTextInputDone) {
        sim800_show_busy(app, "DTMF", app->text_store);
        scene_manager_set_scene_state(app->scene_manager, Sim800SceneDtmf, SCENE_STATE_RESULT);
        sim800_task_start(app, sim800_dtmf_task);
        return true;
    }

    if(event.event == Sim800CustomEventTaskDone) {
        sim800_notify_result(app, app->task_result == Sim800AtResultOk);
        popup_reset(app->popup);
        popup_set_header(
            app->popup,
            (app->task_result == Sim800AtResultOk) ? "Sent" : "Failed",
            64,
            20,
            AlignCenter,
            AlignTop);
        popup_set_text(
            app->popup,
            (app->task_result == Sim800AtResultOk) ? "Tones need an active call" :
                                                     sim800_at_result_text(app->task_result),
            64,
            38,
            AlignCenter,
            AlignTop);
        view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewPopup);
        return true;
    }

    return false;
}

void sim800_scene_dtmf_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
    sim800_text_input_reset(app->text_input);
    popup_reset(app->popup);
}
