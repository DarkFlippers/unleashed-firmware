#include "../sim800_app_i.h"

static void sim800_call_dtmf_task(Sim800App* app) {
    char cmd[16];
    snprintf(cmd, sizeof(cmd), "AT+VTS=%c", app->dtmf_digit);
    app->last_ok = (sim800_uart_command(app->uart, cmd, 3000, NULL) == Sim800AtResultOk);
    app->task_result = app->last_ok ? Sim800AtResultOk : Sim800AtResultError;
}

/** The call screen stops polling while the pad is up, so the pad has to watch for the end. */
static void sim800_call_dtmf_watch(Sim800App* app) {
    FuriString* response = furi_string_alloc();
    FuriString* value = furi_string_alloc();

    if(sim800_uart_command(app->uart, "AT+CLCC", 2000, response) == Sim800AtResultOk) {
        if(sim800_response_field(response, "+CLCC", value)) {
            app->call_idle_polls = 0;
        } else if(++app->call_idle_polls >= 2) {
            // Same rule as the call screen: one empty answer is not proof
            sim800_call_view_set_state(app->call_view, Sim800CallStateEnded);
        }
    }

    furi_string_free(response);
    furi_string_free(value);
    app->task_result = Sim800AtResultOk;
}

static void sim800_call_dtmf_callback(char digit, void* context) {
    Sim800App* app = context;

    // One tone at a time: the module answers each AT+VTS before it will take the next
    if(sim800_task_is_busy(app)) return;

    app->dtmf_digit = digit;
    app->dtmf_watch = false;

    size_t len = strlen(app->dtmf_sent);
    if(len + 1 < sizeof(app->dtmf_sent)) {
        app->dtmf_sent[len] = digit;
        app->dtmf_sent[len + 1] = '\0';
    }
    sim800_dtmf_view_set_sent(app->dtmf_view, app->dtmf_sent);

    sim800_task_start(app, sim800_call_dtmf_task);
}

void sim800_scene_call_dtmf_on_enter(void* context) {
    Sim800App* app = context;

    app->dtmf_sent[0] = '\0';
    app->dtmf_watch = false;
    app->call_idle_polls = 0;

    sim800_dtmf_view_set_sent(app->dtmf_view, app->dtmf_sent);
    sim800_dtmf_view_set_callback(app->dtmf_view, sim800_call_dtmf_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewDtmf);
}

/** Hand the call screen back once there is no call left to send tones into. */
static void sim800_call_dtmf_leave(Sim800App* app) {
    app->redial = true;
    scene_manager_previous_scene(app->scene_manager);
}

bool sim800_scene_call_dtmf_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeTick) {
        if(!sim800_task_is_busy(app)) {
            app->dtmf_watch = true;
            sim800_task_start(app, sim800_call_dtmf_watch);
        }
        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;

    // NO CARRIER is the prompt one; the watch above is the backstop if it goes missing
    if(event.event == Sim800CustomEventCallEnded) {
        sim800_call_view_set_state(app->call_view, Sim800CallStateEnded);
        sim800_call_dtmf_leave(app);
        return true;
    }

    if(event.event == Sim800CustomEventTaskDone) {
        if(!app->dtmf_watch && !app->last_ok) sim800_notify_result(app, false);
        if(sim800_call_view_get_state(app->call_view) != Sim800CallStateActive) {
            sim800_call_dtmf_leave(app);
        }
        return true;
    }

    return false;
}

void sim800_scene_call_dtmf_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
}
