#include "../sim800_app_i.h"

typedef enum {
    SubmenuIndexPwrkey,
    SubmenuIndexReset,
    SubmenuIndexFlightOn,
    SubmenuIndexFlightOff,
    SubmenuIndexPowerOff,
    SubmenuIndexFactory,
    SubmenuIndexStore,
} SubmenuIndex;

#define SCENE_STATE_MENU   0
#define SCENE_STATE_RESULT 1

static const char* const power_commands[] = {
    [SubmenuIndexPwrkey] = NULL, // not an AT command, handled before the table
    [SubmenuIndexReset] = "AT+CFUN=1,1",
    [SubmenuIndexFlightOn] = "AT+CFUN=4",
    [SubmenuIndexFlightOff] = "AT+CFUN=1",
    [SubmenuIndexPowerOff] = "AT+CPOWD=1",
    [SubmenuIndexFactory] = "AT&F",
    [SubmenuIndexStore] = "AT&W",
};

static void sim800_pwrkey_task(Sim800App* app) {
    sim800_pwrkey_pulse(app);
    furi_delay_ms(SIM800_PWRKEY_BOOT_MS);
    app->task_result = sim800_app_connect(app, 10) ? Sim800AtResultOk : Sim800AtResultTimeout;
}

static void sim800_power_task(Sim800App* app) {
    app->task_result = sim800_uart_command(app->uart, app->text_store, 10000, NULL);
}

static void sim800_power_show_menu(Sim800App* app) {
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Power & reset");
    submenu_add_item(
        app->submenu, "Power on (PWRKEY)", SubmenuIndexPwrkey, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Reset module", SubmenuIndexReset, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Flight mode on", SubmenuIndexFlightOn, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Flight mode off", SubmenuIndexFlightOff, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Power off", SubmenuIndexPowerOff, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Factory defaults", SubmenuIndexFactory, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Store settings", SubmenuIndexStore, sim800_submenu_callback, app);

    scene_manager_set_scene_state(app->scene_manager, Sim800ScenePower, SCENE_STATE_MENU);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

void sim800_scene_power_on_enter(void* context) {
    sim800_power_show_menu(context);
}

bool sim800_scene_power_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeBack &&
       scene_manager_get_scene_state(app->scene_manager, Sim800ScenePower) == SCENE_STATE_RESULT) {
        sim800_power_show_menu(app);
        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == SubmenuIndexPwrkey) {
        Sim800PwrkeyState state = sim800_pwrkey_state(app);
        if(state != Sim800PwrkeyReady) {
            bool no_pin = (state == Sim800PwrkeyNoPin);
            sim800_notify_result(app, false);
            popup_reset(app->popup);
            popup_set_header(
                app->popup, no_pin ? "No PWRKEY pin" : "Pin in use", 64, 16, AlignCenter, AlignTop);
            popup_set_text(
                app->popup,
                no_pin ? "Assign one in Settings" : "That pin is the UART's",
                64,
                34,
                AlignCenter,
                AlignTop);
            scene_manager_set_scene_state(
                app->scene_manager, Sim800ScenePower, SCENE_STATE_RESULT);
            view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewPopup);
            return true;
        }
        sim800_show_busy(app, "PWRKEY", "Holding low...");
        scene_manager_set_scene_state(app->scene_manager, Sim800ScenePower, SCENE_STATE_RESULT);
        sim800_task_start(app, sim800_pwrkey_task);
        return true;
    }

    if(event.event < COUNT_OF(power_commands)) {
        strlcpy(app->text_store, power_commands[event.event], SIM800_AT_CMD_MAX);
        sim800_show_busy(app, "Running", app->text_store);
        scene_manager_set_scene_state(app->scene_manager, Sim800ScenePower, SCENE_STATE_RESULT);
        sim800_task_start(app, sim800_power_task);
        return true;
    }

    if(event.event == Sim800CustomEventTaskDone) {
        sim800_notify_result(app, app->task_result == Sim800AtResultOk);
        popup_reset(app->popup);
        popup_set_header(
            app->popup,
            (app->task_result == Sim800AtResultOk) ? "Done" : "Failed",
            64,
            20,
            AlignCenter,
            AlignTop);
        popup_set_text(
            app->popup, sim800_at_result_text(app->task_result), 64, 38, AlignCenter, AlignTop);
        view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewPopup);
        return true;
    }

    return false;
}

void sim800_scene_power_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
    submenu_reset(app->submenu);
    popup_reset(app->popup);
}
