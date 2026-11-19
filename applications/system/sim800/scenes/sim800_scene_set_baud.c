#include "../sim800_app_i.h"

#define SCENE_STATE_MENU   0
#define SCENE_STATE_RESULT 1

/** Move the module itself to another rate with AT+IPR, then store it so it survives a restart.
 *
 * The module changes rate as soon as it has answered, so the port has to follow it over before
 * anything else can be sent. If it does not come back, the old rate is put back and nothing is
 * written to the module or to our own settings.
 */
static void sim800_set_baud_task(Sim800App* app) {
    uint32_t previous = app->settings.baudrate;
    char cmd[24];

    app->last_ok = false;
    snprintf(cmd, sizeof(cmd), "AT+IPR=%lu", app->pending_baud);

    // Plenty of boards are strapped to one rate and turn AT+IPR down flat
    Sim800AtResult result = sim800_uart_command(app->uart, cmd, 3000, NULL);
    if(result != Sim800AtResultOk) {
        snprintf(
            app->popup_text, sizeof(app->popup_text), "AT+IPR %s", sim800_at_result_text(result));
        app->task_result = result;
        return;
    }

    // The module answers first and reconfigures its UART after, so do not talk over the change
    furi_delay_ms(300);

    app->settings.baudrate = app->pending_baud;
    if(!sim800_app_connect(app, 10)) {
        app->settings.baudrate = previous;
        sim800_app_connect(app, 3);
        snprintf(app->popup_text, sizeof(app->popup_text), "No answer at %lu", app->pending_baud);
        app->task_result = Sim800AtResultTimeout;
        return;
    }

    // It answered on the new rate, so the app has to remember it whatever AT&W then does
    sim800_settings_save(&app->settings);
    app->last_ok = true;
    app->task_result = Sim800AtResultOk;

    if(sim800_uart_command(app->uart, "AT&W", 3000, NULL) == Sim800AtResultOk) {
        snprintf(app->popup_text, sizeof(app->popup_text), "Now at %lu", app->pending_baud);
    } else {
        // Set and working, but the module will not come back on it by itself
        snprintf(app->popup_text, sizeof(app->popup_text), "%lu, not stored", app->pending_baud);
    }
}

static void sim800_set_baud_show_menu(Sim800App* app) {
    char label[16];

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Set module baud");
    for(size_t i = 0; i < SIM800_BAUDRATE_COUNT; i++) {
        snprintf(label, sizeof(label), "%s", sim800_settings_baudrate_names[i]);
        submenu_add_item(app->submenu, label, i, sim800_submenu_callback, app);
    }

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneSetBaud, SCENE_STATE_MENU);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

void sim800_scene_set_baud_on_enter(void* context) {
    sim800_set_baud_show_menu(context);
}

bool sim800_scene_set_baud_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeBack &&
       scene_manager_get_scene_state(app->scene_manager, Sim800SceneSetBaud) ==
           SCENE_STATE_RESULT) {
        sim800_set_baud_show_menu(app);
        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event < SIM800_BAUDRATE_COUNT) {
        app->pending_baud = sim800_settings_baudrates[event.event];
        sim800_show_busy(app, "Set baud", sim800_settings_baudrate_names[event.event]);
        scene_manager_set_scene_state(app->scene_manager, Sim800SceneSetBaud, SCENE_STATE_RESULT);
        sim800_task_start(app, sim800_set_baud_task);
        return true;
    }

    if(event.event == Sim800CustomEventTaskDone) {
        sim800_notify_result(app, app->last_ok);

        popup_reset(app->popup);
        popup_set_header(
            app->popup, app->last_ok ? "Done" : "Failed", 64, 18, AlignCenter, AlignTop);
        popup_set_text(app->popup, app->popup_text, 64, 36, AlignCenter, AlignTop);
        view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewPopup);
        return true;
    }

    return false;
}

void sim800_scene_set_baud_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
    submenu_reset(app->submenu);
    popup_reset(app->popup);
}
