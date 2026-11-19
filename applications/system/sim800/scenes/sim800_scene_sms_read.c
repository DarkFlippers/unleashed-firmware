#include "../sim800_app_i.h"
#include "../helpers/sim800_history.h"

void sim800_scene_sms_read_on_enter(void* context) {
    Sim800App* app = context;
    Sim800SmsEntry* sms = &app->inbox.items[app->selected_sms];

    furi_string_printf(
        app->text_box_store,
        "\e#%s\e#\n%s\n%s",
        furi_string_get_cstr(sms->from),
        furi_string_get_cstr(sms->date),
        furi_string_get_cstr(sms->body));

    // Reading a message is the manual act that logs it; the key keeps a re-read from
    // writing the same one twice
    FuriString* key = furi_string_alloc_printf(
        "%s  %s", furi_string_get_cstr(sms->from), furi_string_get_cstr(sms->date));
    if(!sim800_history_contains(furi_string_get_cstr(key))) {
        sim800_history_add_sms(
            furi_string_get_cstr(sms->from),
            furi_string_get_cstr(sms->date),
            furi_string_get_cstr(sms->body));
    }
    furi_string_free(key);

    widget_reset(app->widget);
    widget_add_text_scroll_element(
        app->widget, 0, 0, 128, 50, furi_string_get_cstr(app->text_box_store));
    widget_add_button_element(
        app->widget, GuiButtonTypeRight, "Delete", sim800_widget_button_callback, app);
    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Reply", sim800_widget_button_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewWidget);
}

bool sim800_scene_sms_read_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;
    Sim800SmsEntry* sms = &app->inbox.items[app->selected_sms];

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == Sim800CustomEventButtonRight) {
        FuriString* cmd = furi_string_alloc_printf("AT+CMGD=%lu", (unsigned long)sms->index);
        Sim800AtResult result =
            sim800_uart_command(app->uart, furi_string_get_cstr(cmd), 5000, NULL);
        furi_string_free(cmd);
        sim800_notify_result(app, result == Sim800AtResultOk);
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }

    if(event.event == Sim800CustomEventButtonLeft) {
        furi_string_set(app->number, sms->from);
        app->mode = Sim800ModeSms;
        app->number_saved = false;
        furi_string_reset(app->sms_text);
        scene_manager_next_scene(app->scene_manager, Sim800SceneSmsText);
        return true;
    }

    return false;
}

void sim800_scene_sms_read_on_exit(void* context) {
    Sim800App* app = context;
    widget_reset(app->widget);
}
