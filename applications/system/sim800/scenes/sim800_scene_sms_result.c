#include "../sim800_app_i.h"

#define SCENE_STATE_SENDING 0
#define SCENE_STATE_DONE    1

static void sim800_sms_send_task(Sim800App* app) {
    FuriString* cmd = furi_string_alloc();

    sim800_uart_command(app->uart, "AT+CMGF=1", 1000, NULL);
    sim800_uart_command(app->uart, "AT+CSCS=\"GSM\"", 1000, NULL);

    furi_string_printf(cmd, "AT+CMGS=\"%s\"", furi_string_get_cstr(app->number));
    app->task_result = sim800_uart_command_prompt(
        app->uart,
        furi_string_get_cstr(cmd),
        furi_string_get_cstr(app->sms_text),
        20000,
        app->task_response);

    furi_string_free(cmd);
}

static void sim800_scene_sms_result_show(Sim800App* app) {
    bool sent = (app->task_result == Sim800AtResultOk);

    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 64, 10, AlignCenter, AlignBottom, FontPrimary, sent ? "SMS sent" : "Not sent");
    widget_add_string_element(
        app->widget,
        64,
        24,
        AlignCenter,
        AlignBottom,
        FontSecondary,
        furi_string_get_cstr(app->number));

    if(!sent) {
        FuriString* detail = furi_string_alloc_set(app->task_response);
        furi_string_trim(detail);
        if(furi_string_size(detail) == 0) {
            furi_string_set_str(detail, sim800_at_result_text(app->task_result));
        }
        widget_add_text_box_element(
            app->widget, 2, 28, 124, 24, AlignCenter, AlignTop, furi_string_get_cstr(detail), true);
        furi_string_free(detail);
    }

    if(!app->number_saved) {
        widget_add_button_element(
            app->widget, GuiButtonTypeLeft, "Save No.", sim800_widget_button_callback, app);
    }
    widget_add_button_element(
        app->widget, GuiButtonTypeRight, "Preset", sim800_widget_button_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewWidget);
}

void sim800_scene_sms_result_on_enter(void* context) {
    Sim800App* app = context;

    if(scene_manager_get_scene_state(app->scene_manager, Sim800SceneSmsResult) ==
       SCENE_STATE_DONE) {
        sim800_scene_sms_result_show(app);
        return;
    }

    sim800_show_busy(app, "Sending", furi_string_get_cstr(app->number));
    sim800_task_start(app, sim800_sms_send_task);
}

bool sim800_scene_sms_result_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    switch(event.event) {
    case Sim800CustomEventTaskDone:
        scene_manager_set_scene_state(app->scene_manager, Sim800SceneSmsResult, SCENE_STATE_DONE);
        sim800_notify_result(app, app->task_result == Sim800AtResultOk);

        sim800_scene_sms_result_show(app);
        return true;

    case Sim800CustomEventButtonLeft:
        app->save_target = Sim800SaveTargetNumber;
        scene_manager_next_scene(app->scene_manager, Sim800SceneSaveName);
        return true;

    case Sim800CustomEventButtonRight:
        app->save_target = Sim800SaveTargetPreset;
        scene_manager_next_scene(app->scene_manager, Sim800SceneSaveName);
        return true;

    default:
        return false;
    }
}

void sim800_scene_sms_result_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
    widget_reset(app->widget);
    popup_reset(app->popup);
}
