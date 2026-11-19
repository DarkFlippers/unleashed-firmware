#include "../sim800_app_i.h"

#define SCENE_STATE_FRESH   0
#define SCENE_STATE_RESUMED 1

static void sim800_call_button_callback(Sim800CallButton button, void* context) {
    Sim800App* app = context;
    uint32_t event = Sim800CustomEventButtonCenter;

    if(button == Sim800CallButtonLeft) {
        event = Sim800CustomEventButtonLeft;
    } else if(button == Sim800CallButtonRight) {
        event = Sim800CustomEventButtonRight;
    }

    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

static void sim800_call_task_dial(Sim800App* app) {
    FuriString* cmd = furi_string_alloc_printf("ATD%s;", furi_string_get_cstr(app->number));
    app->task_result =
        sim800_uart_command_call(app->uart, furi_string_get_cstr(cmd), 8000, app->task_response);
    furi_string_free(cmd);

    if(app->task_result != Sim800AtResultOk) {
        sim800_call_view_set_state(app->call_view, Sim800CallStateFailed);
        sim800_call_view_set_detail(app->call_view, sim800_at_result_text(app->task_result));
        return;
    }

    // ATD with a semicolon answers as soon as the call is placed, so reading these now delays
    // nothing and fills the header well before the first poll comes round
    app->net_rssi = sim800_at_rssi(app->uart);
    sim800_call_view_set_rssi(app->call_view, app->net_rssi);
    sim800_at_operator(app->uart, app->net_operator);
    sim800_call_view_set_operator(app->call_view, furi_string_get_cstr(app->net_operator));
}

static void sim800_call_task_answer(Sim800App* app) {
    app->task_result = sim800_uart_command_call(app->uart, "ATA", 8000, NULL);
    if(app->task_result != Sim800AtResultOk) {
        sim800_call_view_set_state(app->call_view, Sim800CallStateEnded);
    }
}

static void sim800_call_task_hangup(Sim800App* app) {
    sim800_uart_command(app->uart, "ATH", 4000, NULL);
    sim800_call_view_set_state(app->call_view, Sim800CallStateEnded);
    sim800_call_view_set_detail(app->call_view, "");
}

static void sim800_call_task_poll(Sim800App* app) {
    FuriString* response = furi_string_alloc();
    FuriString* value = furi_string_alloc();
    FuriString* field = furi_string_alloc();

    Sim800CallState state = sim800_call_view_get_state(app->call_view);
    uint32_t run = app->call_run++;

    // At 2400 baud one AT exchange costs a few hundred milliseconds of wire time, so each of
    // these is a real delay on the screen. Signal and carrier barely move during a call.
    if((run % 3) == 0) {
        app->net_rssi = sim800_at_rssi(app->uart);
        sim800_call_view_set_rssi(app->call_view, app->net_rssi);
    }
    if((state != Sim800CallStateActive) && ((run == 0) || ((run % 5) == 0))) {
        sim800_at_operator(app->uart, value);
        furi_string_set(app->net_operator, value);
        sim800_call_view_set_operator(app->call_view, furi_string_get_cstr(value));
    }

    if(sim800_uart_command(app->uart, "AT+CLCC", 2000, response) != Sim800AtResultOk) {
        // No answer is no news: the call is whatever it already was
        goto done;
    }

    if(sim800_response_field(response, "+CLCC", value) &&
       sim800_csv_field(furi_string_get_cstr(value), 2, field)) {
        app->call_idle_polls = 0;
        switch(strtoul(furi_string_get_cstr(field), NULL, 10)) {
        case 0:
            sim800_call_view_set_state(app->call_view, Sim800CallStateActive);
            break;
        case 2:
            sim800_call_view_set_state(app->call_view, Sim800CallStateDialing);
            break;
        case 3:
            sim800_call_view_set_state(app->call_view, Sim800CallStateRinging);
            break;
        case 4:
        case 5:
            sim800_call_view_set_state(app->call_view, Sim800CallStateIncoming);
            break;
        default:
            break;
        }
    } else if(
        state == Sim800CallStateActive || state == Sim800CallStateDialing ||
        state == Sim800CallStateRinging || state == Sim800CallStateIncoming) {
        // A bare OK is how the module reports no calls, but one of them is not proof: a URC
        // landing mid-response or a slow link reads the same way, and hanging up a live call
        // over that is the worse mistake. NO CARRIER is what ends a call promptly.
        if(++app->call_idle_polls >= 2) {
            sim800_call_view_set_state(app->call_view, Sim800CallStateEnded);
        }
    }

done:
    furi_string_free(response);
    furi_string_free(value);
    furi_string_free(field);
}

static void sim800_call_update_timer(Sim800App* app) {
    Sim800CallState state = sim800_call_view_get_state(app->call_view);

    if(state == Sim800CallStateActive) {
        if(app->call_start_tick == 0) app->call_start_tick = furi_get_tick();
        uint32_t elapsed =
            (furi_get_tick() - app->call_start_tick) / furi_kernel_get_tick_frequency();
        sim800_call_view_set_duration(app->call_view, elapsed);
    } else {
        app->call_start_tick = 0;
        if(state == Sim800CallStateEnded || state == Sim800CallStateFailed) app->redial = true;
    }

    // Once per transition. Polling completes every second, and firing here on state alone
    // buzzed the user on every one of them for as long as the screen stayed open.
    if(state != app->call_last_state) {
        if(state == Sim800CallStateFailed) sim800_notify_result(app, false);
        app->call_last_state = state;
    }
}

void sim800_scene_call_on_enter(void* context) {
    Sim800App* app = context;

    sim800_call_view_set_callback(app->call_view, sim800_call_button_callback, app);
    sim800_call_view_set_number(app->call_view, furi_string_get_cstr(app->number));
    sim800_call_view_set_saved(app->call_view, app->number_saved);

    uint32_t state = scene_manager_get_scene_state(app->scene_manager, Sim800SceneCall);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewCall);

    if(state == SCENE_STATE_RESUMED) return;

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneCall, SCENE_STATE_RESUMED);
    app->call_start_tick = 0;
    app->call_poll = 0;
    app->call_run = 0;
    app->call_idle_polls = 0;
    app->call_last_state = Sim800CallStateDialing;
    sim800_call_view_set_duration(app->call_view, 0);
    sim800_call_view_set_detail(app->call_view, "");
    // Open with whatever was last seen rather than a blank, then let the poll correct it
    sim800_call_view_set_rssi(app->call_view, app->net_rssi);
    sim800_call_view_set_operator(
        app->call_view,
        (furi_string_size(app->net_operator) > 0) ? furi_string_get_cstr(app->net_operator) :
                                                    "...");

    if(app->call_incoming) {
        app->call_incoming = false;
        sim800_call_view_set_state(app->call_view, Sim800CallStateIncoming);
        if(app->settings.notify) {
            notification_message(app->notifications, &sim800_sequence_ring);
        }
    } else {
        sim800_call_view_set_state(app->call_view, Sim800CallStateDialing);
        sim800_task_start(app, sim800_call_task_dial);
    }
}

bool sim800_scene_call_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeTick) {
        sim800_call_update_timer(app);

        Sim800CallState tick_state = sim800_call_view_get_state(app->call_view);

        if(tick_state == Sim800CallStateIncoming) {
            // +CLIP can land after the first RING, so the number may only turn up now
            if((app->incoming_number[0] != '\0') &&
               (strcmp(furi_string_get_cstr(app->number), app->incoming_number) != 0)) {
                furi_string_set_str(app->number, app->incoming_number);
                sim800_call_view_set_number(app->call_view, app->incoming_number);
            }
            // A phone that alerts once and then sits silent is not much of a ring
            if(app->settings.notify && ((app->call_poll % 2) == 0)) {
                notification_message(app->notifications, &sim800_sequence_ring);
            }
        }

        // Once the call is over there is nothing left to watch, and polling a dead call was
        // most of the churn on the screen after a failed dial.
        if(tick_state == Sim800CallStateEnded || tick_state == Sim800CallStateFailed) {
            return true;
        }

        // Setting a call up is the only part worth watching closely. Once it is up the module
        // sends NO CARRIER by itself when it drops, so this is just a safety net.
        uint32_t period = (tick_state == Sim800CallStateActive) ? 5 : 2;
        if((app->call_poll++ % period) != 0) return true;

        if(!sim800_task_is_busy(app)) sim800_task_start(app, sim800_call_task_poll);
        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;

    Sim800CallState state = sim800_call_view_get_state(app->call_view);

    switch(event.event) {
    case Sim800CustomEventTaskDone:
        sim800_call_update_timer(app);
        return true;

    case Sim800CustomEventCallEnded:
        if(state != Sim800CallStateEnded && state != Sim800CallStateFailed) {
            sim800_call_view_set_state(app->call_view, Sim800CallStateEnded);
            app->redial = true;
        }
        return true;

    case Sim800CustomEventButtonCenter:
        if(sim800_task_is_busy(app)) return true;
        if(state == Sim800CallStateIncoming) {
            sim800_task_start(app, sim800_call_task_answer);
        } else if(state == Sim800CallStateEnded || state == Sim800CallStateFailed) {
            app->redial = false;
            app->call_start_tick = 0;
            app->call_idle_polls = 0;
            sim800_call_view_set_duration(app->call_view, 0);
            sim800_call_view_set_detail(app->call_view, "");
            sim800_call_view_set_state(app->call_view, Sim800CallStateDialing);
            sim800_task_start(app, sim800_call_task_dial);
        } else {
            sim800_task_start(app, sim800_call_task_hangup);
        }
        return true;

    case Sim800CustomEventButtonRight:
        if(state == Sim800CallStateActive) {
            // on_exit hangs up unless it is told this is a detour, not a departure
            app->call_keep = true;
            scene_manager_next_scene(app->scene_manager, Sim800SceneCallDtmf);
        }
        return true;

    case Sim800CustomEventButtonLeft:
        if(state == Sim800CallStateIncoming) {
            if(!sim800_task_is_busy(app)) sim800_task_start(app, sim800_call_task_hangup);
        } else if(!app->number_saved) {
            app->save_target = Sim800SaveTargetNumber;
            app->call_keep = true;
            scene_manager_next_scene(app->scene_manager, Sim800SceneSaveName);
        }
        return true;

    default:
        return false;
    }
}

void sim800_scene_call_on_exit(void* context) {
    Sim800App* app = context;

    // A push to the save-name scene also lands here; the call must survive it
    if(app->call_keep) {
        app->call_keep = false;
        return;
    }

    sim800_task_join(app);

    Sim800CallState state = sim800_call_view_get_state(app->call_view);
    if(state != Sim800CallStateEnded && state != Sim800CallStateFailed) {
        sim800_uart_command(app->uart, "ATH", 2000, NULL);
        app->redial = true;
    }
}
