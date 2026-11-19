#include "../sim800_app_i.h"

// Commonest rates first: a module that is fixed low costs the most to rule out
static const uint32_t detect_order[] =
    {115200, 9600, 19200, 38400, 57600, 230400, 4800, 2400, 1200};

static void sim800_detect_task(Sim800App* app) {
    FuriHalSerialId serial_id = (app->settings.uart_port == Sim800UartLpuart) ?
                                    FuriHalSerialIdLpuart :
                                    FuriHalSerialIdUsart;
    app->last_ok = false;

    for(size_t i = 0; i < COUNT_OF(detect_order); i++) {
        sim800_uart_close(app->uart);
        if(!sim800_uart_open(app->uart, serial_id, detect_order[i])) continue;

        if(sim800_uart_sync(app->uart, 3)) {
            app->settings.baudrate = detect_order[i];
            sim800_settings_save(&app->settings);
            app->last_ok = true;
            break;
        }
    }

    // Found or not, leave the port open on the configured rate with the module set up
    sim800_app_connect(app, 3);
    app->task_result = app->last_ok ? Sim800AtResultOk : Sim800AtResultTimeout;
}

void sim800_scene_detect_on_enter(void* context) {
    Sim800App* app = context;
    sim800_show_busy(app, "Detecting", "Trying baud rates...");
    sim800_task_start(app, sim800_detect_task);
}

bool sim800_scene_detect_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != Sim800CustomEventTaskDone) return false;

    sim800_notify_result(app, app->last_ok);

    snprintf(app->popup_text, sizeof(app->popup_text), "%lu baud", app->settings.baudrate);

    popup_reset(app->popup);
    popup_set_header(
        app->popup, app->last_ok ? "Found" : "No module", 64, 18, AlignCenter, AlignTop);
    popup_set_text(
        app->popup,
        app->last_ok ? app->popup_text : "Check wiring and power",
        64,
        36,
        AlignCenter,
        AlignTop);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewPopup);
    return true;
}

void sim800_scene_detect_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
    popup_reset(app->popup);
}
