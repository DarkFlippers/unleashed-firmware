#include "../sim800_app_i.h"

static void sim800_network_info_task(Sim800App* app) {
    FuriString* out = app->text_box_store;
    furi_string_reset(out);

    app->last_ok = sim800_uart_is_open(app->uart) && sim800_uart_probe(app->uart);
    if(!app->last_ok) {
        furi_string_set_str(out, "No module on the UART.\nCheck Settings.");
        return;
    }

    FuriString* value = furi_string_alloc();

    sim800_at_operator(app->uart, value);
    furi_string_cat_printf(out, "Carrier: %s\n", furi_string_get_cstr(value));

    uint8_t rssi = sim800_at_rssi(app->uart);
    if(rssi > 31) {
        furi_string_cat_str(out, "Signal: no reading\n");
    } else {
        furi_string_cat_printf(out, "Signal: %d dBm (%u)\n", -113 + (rssi * 2), rssi);
    }

    sim800_at_registration(app->uart, value);
    furi_string_cat_printf(out, "Network: %s\n", furi_string_get_cstr(value));

    if(sim800_at_query(app->uart, "AT+CCLK?", "+CCLK", value)) {
        sim800_unquote(value);
        furi_string_cat_printf(out, "Time: %s\n", furi_string_get_cstr(value));
    }

    if(sim800_at_query(app->uart, "AT+CNUM", "+CNUM", value)) {
        FuriString* number = furi_string_alloc();
        if(sim800_csv_field(furi_string_get_cstr(value), 1, number)) {
            furi_string_cat_printf(out, "Own number: %s\n", furi_string_get_cstr(number));
        }
        furi_string_free(number);
    }

    if(sim800_at_query(app->uart, "AT+CSCA?", "+CSCA", value)) {
        FuriString* centre = furi_string_alloc();
        if(sim800_csv_field(furi_string_get_cstr(value), 0, centre)) {
            furi_string_cat_printf(out, "SMS centre: %s\n", furi_string_get_cstr(centre));
        }
        furi_string_free(centre);
    }

    sim800_at_power(app->uart, value);
    furi_string_cat_printf(out, "Power: %s\n", furi_string_get_cstr(value));

    furi_string_free(value);
}

void sim800_scene_network_info_on_enter(void* context) {
    Sim800App* app = context;
    sim800_show_busy(app, "Network info", "Reading...");
    sim800_task_start(app, sim800_network_info_task);
}

bool sim800_scene_network_info_on_event(void* context, SceneManagerEvent event) {
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

void sim800_scene_network_info_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
    text_box_reset(app->text_box);
    popup_reset(app->popup);
}
