#include "../sim800_app_i.h"

static void
    sim800_info_row(FuriString* out, const char* label, Sim800Uart* uart, const char* cmd) {
    FuriString* value = furi_string_alloc();

    furi_string_cat_str(out, label);
    if(sim800_at_line(uart, cmd, value)) {
        furi_string_cat(out, value);
    } else {
        furi_string_cat_str(out, "--");
    }
    furi_string_cat_str(out, "\n");

    furi_string_free(value);
}

static void sim800_device_info_task(Sim800App* app) {
    FuriString* out = app->text_box_store;
    furi_string_reset(out);

    app->last_ok = sim800_uart_is_open(app->uart) && sim800_uart_probe(app->uart);
    if(!app->last_ok) {
        furi_string_set_str(out, "No module on the UART.\nCheck Settings.");
        return;
    }

    FuriString* value = furi_string_alloc();

    sim800_info_row(out, "Maker: ", app->uart, "AT+CGMI");
    sim800_info_row(out, "Model: ", app->uart, "AT+CGMM");
    sim800_info_row(out, "Firmware: ", app->uart, "AT+CGMR");
    sim800_info_row(out, "IMEI: ", app->uart, "AT+GSN");
    sim800_info_row(out, "ICCID: ", app->uart, "AT+CCID");
    sim800_info_row(out, "IMSI: ", app->uart, "AT+CIMI");

    sim800_at_sim_status(app->uart, value);
    furi_string_cat_printf(out, "SIM: %s\n", furi_string_get_cstr(value));

    sim800_at_power(app->uart, value);
    furi_string_cat_printf(out, "Power: %s\n", furi_string_get_cstr(value));

    furi_string_free(value);
}

void sim800_scene_device_info_on_enter(void* context) {
    Sim800App* app = context;
    sim800_show_busy(app, "Device info", "Reading...");
    sim800_task_start(app, sim800_device_info_task);
}

bool sim800_scene_device_info_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeCustom && event.event == Sim800CustomEventTaskDone) {
        if(!app->last_ok) sim800_notify_result(app, false);
        text_box_reset(app->text_box);
        text_box_set_font(app->text_box, TextBoxFontText);
        text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));
        view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewTextBox);
        return true;
    }
    return false;
}

void sim800_scene_device_info_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
    text_box_reset(app->text_box);
    popup_reset(app->popup);
}
