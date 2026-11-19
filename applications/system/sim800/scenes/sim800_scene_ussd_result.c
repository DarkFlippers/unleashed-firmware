#include "../sim800_app_i.h"

static void sim800_ussd_task(Sim800App* app) {
    FuriString* out = app->text_box_store;
    FuriString* cmd = furi_string_alloc();
    FuriString* value = furi_string_alloc();

    furi_string_reset(out);
    sim800_arm_urc(app);

    furi_string_printf(cmd, "AT+CUSD=1,\"%s\"", app->text_store);
    app->task_result =
        sim800_uart_command(app->uart, furi_string_get_cstr(cmd), 10000, app->task_response);

    bool got = sim800_response_field(app->task_response, "+CUSD", value);
    // Most firmwares answer OK first and send the +CUSD line on its own a moment later
    if(!got && app->task_result == Sim800AtResultOk) {
        FuriString* line = furi_string_alloc();
        if(sim800_wait_urc(app, 15000, line)) {
            got = sim800_response_field(line, "+CUSD", value);
        }
        furi_string_free(line);
    }

    app->last_ok = got;
    if(got) {
        FuriString* text = furi_string_alloc();
        if(sim800_csv_field(furi_string_get_cstr(value), 1, text)) {
            furi_string_set(out, text);
        } else {
            furi_string_set(out, value);
        }
        furi_string_free(text);
    } else {
        furi_string_set_str(out, sim800_at_result_text(app->task_result));
    }

    furi_string_free(cmd);
    furi_string_free(value);
}

void sim800_scene_ussd_result_on_enter(void* context) {
    Sim800App* app = context;
    sim800_show_busy(app, "USSD", app->text_store);
    sim800_task_start(app, sim800_ussd_task);
}

bool sim800_scene_ussd_result_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeCustom && event.event == Sim800CustomEventTaskDone) {
        sim800_notify_result(app, app->last_ok);
        text_box_reset(app->text_box);
        text_box_set_font(app->text_box, TextBoxFontText);
        text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));
        view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewTextBox);
        return true;
    }
    return false;
}

void sim800_scene_ussd_result_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
    sim800_uart_command(app->uart, "AT+CUSD=2", 2000, NULL);
    text_box_reset(app->text_box);
    popup_reset(app->popup);
}
