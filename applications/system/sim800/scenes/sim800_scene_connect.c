#include "../sim800_app_i.h"

// Opening the port, locking the rate and running the init commands all take real time at a low
// baud, and a module that is still off adds a PWRKEY pulse plus its boot. Do it behind a screen
// instead of a frozen menu.
static void sim800_connect_task(Sim800App* app) {
    app->last_ok = sim800_app_connect(app, 3);

    if(!app->last_ok && (sim800_pwrkey_state(app) == Sim800PwrkeyReady)) {
        sim800_pwrkey_pulse(app);
        furi_delay_ms(SIM800_PWRKEY_BOOT_MS);
        app->last_ok = sim800_app_connect(app, 10);
    }

    if(app->last_ok) {
        // One sample here means every later screen opens with a carrier already on it
        app->net_rssi = sim800_at_rssi(app->uart);
        sim800_at_operator(app->uart, app->net_operator);
    }

    app->task_result = app->last_ok ? Sim800AtResultOk : Sim800AtResultTimeout;
}

void sim800_scene_connect_on_enter(void* context) {
    Sim800App* app = context;

    // A module that has to be started costs a PWRKEY pulse plus its boot on top of the connect
    uint32_t total_ms = (sim800_pwrkey_state(app) == Sim800PwrkeyReady) ? 9000 : 3000;

    sim800_connect_view_start(app->connect_view, app->settings.baudrate, total_ms);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewConnect);

    sim800_task_start(app, sim800_connect_task);
}

bool sim800_scene_connect_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    // Back would leave the task running into a torn-down scene
    if(event.type == SceneManagerEventTypeBack) return true;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != Sim800CustomEventTaskDone) return false;

    scene_manager_previous_scene(app->scene_manager);
    return true;
}

void sim800_scene_connect_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
}
