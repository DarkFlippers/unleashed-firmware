#include "../subbrute_i.h"
#include "subbrute_scene.h"

#define TAG "SubBruteSceneRunAttack"

static void subbrute_scene_run_attack_callback(SubBruteCustomEvent event, void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
}

static void
    subbrute_scene_run_attack_device_state_changed(void* context, SubBruteWorkerState state) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;

    if(state == SubBruteWorkerStateIDLE) {
        // Can't be IDLE on this step!
        view_dispatcher_send_custom_event(instance->view_dispatcher, SubBruteCustomEventTypeError);
    } else if(state == SubBruteWorkerStateFinished) {
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, SubBruteCustomEventTypeTransmitFinished);
    }
}
void subbrute_scene_run_attack_on_exit(void* context) {
    furi_assert(context);
    SubBruteState* instance = (SubBruteState*)context;

    subbrute_worker_stop(instance->worker);

    notification_message(instance->notifications, &sequence_blink_stop);
}

void subbrute_scene_run_attack_on_enter(void* context) {
    furi_assert(context);
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteAttackView* view = instance->view_attack;

    instance->current_view = SubBruteViewAttack;
    subbrute_attack_view_set_callback(view, subbrute_scene_run_attack_callback, instance);
    view_dispatcher_switch_to_view(instance->view_dispatcher, instance->current_view);

    subbrute_worker_set_callback(
        instance->worker, subbrute_scene_run_attack_device_state_changed, instance);

    if(!subbrute_worker_is_running(instance->worker)) {
        subbrute_worker_set_step(instance->worker, instance->device->key_index);
        if(!subbrute_worker_start(instance->worker)) {
            view_dispatcher_send_custom_event(
                instance->view_dispatcher, SubBruteCustomEventTypeError);
        } else {
            notification_message(instance->notifications, &sequence_single_vibro);
            notification_message(instance->notifications, &sequence_blink_start_yellow);
        }
    }
}

bool subbrute_scene_run_attack_on_event(void* context, SceneManagerEvent event) {
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteAttackView* view = instance->view_attack;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        uint64_t step = subbrute_worker_get_step(instance->worker);
        instance->device->key_index = step;
        subbrute_attack_view_set_current_step(view, step);

        if(event.event == SubBruteCustomEventTypeTransmitFinished) {
            notification_message(instance->notifications, &sequence_display_backlight_on);
            notification_message(instance->notifications, &sequence_double_vibro);

            scene_manager_next_scene(instance->scene_manager, SubBruteSceneSetupAttack);
        } else if(
            event.event == SubBruteCustomEventTypeTransmitNotStarted ||
            event.event == SubBruteCustomEventTypeBackPressed) {
            if(subbrute_worker_is_running(instance->worker)) {
                // Notify
                notification_message(instance->notifications, &sequence_single_vibro);
            }
            // Stop transmit
            scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, SubBruteSceneSetupAttack);
        } else if(event.event == SubBruteCustomEventTypeError) {
            notification_message(instance->notifications, &sequence_error);

            // Stop transmit
            scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, SubBruteSceneSetupAttack);
        } else if(event.event == SubBruteCustomEventTypeUpdateView) {
            //subbrute_attack_view_set_current_step(view, instance->device->key_index);
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        uint64_t step = subbrute_worker_get_step(instance->worker);
        instance->device->key_index = step;
        subbrute_attack_view_set_current_step(view, step);

        consumed = true;
    }

    return consumed;
}
