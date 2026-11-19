#include "../sim800_app_i.h"

static void sim800_inbox_task(Sim800App* app) {
    sim800_inbox_reset(&app->inbox);

    if(!sim800_uart_is_open(app->uart) || !sim800_uart_probe(app->uart)) {
        app->task_result = Sim800AtResultNotOpen;
        return;
    }

    sim800_uart_command(app->uart, "AT+CMGF=1", 1000, NULL);
    app->task_result =
        sim800_uart_command(app->uart, "AT+CMGL=\"ALL\"", 15000, app->task_response);

    if(app->task_result == Sim800AtResultOk) {
        sim800_inbox_parse(&app->inbox, app->task_response);
    }
}

static void sim800_inbox_show(Sim800App* app) {
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "SMS inbox");

    if(app->inbox.count == 0) {
        submenu_add_item(
            app->submenu,
            (app->task_result == Sim800AtResultOk) ? "Inbox is empty" : "Read failed",
            0xFF,
            sim800_submenu_callback,
            app);
    } else {
        FuriString* label = furi_string_alloc();
        for(size_t i = 0; i < app->inbox.count; i++) {
            furi_string_printf(
                label,
                "%s: %s",
                furi_string_get_cstr(app->inbox.items[i].from),
                furi_string_get_cstr(app->inbox.items[i].body));
            furi_string_replace_all_str(label, "\n", " ");
            submenu_add_item(
                app->submenu, furi_string_get_cstr(label), i, sim800_submenu_callback, app);
        }
        furi_string_free(label);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

void sim800_scene_inbox_on_enter(void* context) {
    Sim800App* app = context;
    sim800_show_busy(app, "SMS inbox", "Reading...");
    sim800_task_start(app, sim800_inbox_task);
}

bool sim800_scene_inbox_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == Sim800CustomEventTaskDone) {
        if(app->task_result != Sim800AtResultOk) sim800_notify_result(app, false);
        sim800_inbox_show(app);
        return true;
    }

    if(event.event < app->inbox.count) {
        app->selected_sms = event.event;
        scene_manager_next_scene(app->scene_manager, Sim800SceneSmsRead);
        return true;
    }

    return false;
}

void sim800_scene_inbox_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
    submenu_reset(app->submenu);
    popup_reset(app->popup);
}
