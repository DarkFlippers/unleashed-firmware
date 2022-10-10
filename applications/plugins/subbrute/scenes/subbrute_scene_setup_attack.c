#include "../subbrute_i.h"
#include "subbrute_scene.h"

#define TAG "SubBruteSceneSetupAttack"

static void subbrute_scene_setup_attack_callback(SubBruteCustomEvent event, void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
}

static void
    subbrute_scene_setup_attack_device_state_changed(void* context, SubBruteWorkerState state) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;

    if(state == SubBruteWorkerStateIDLE) {
        // Can't be IDLE on this step!
        view_dispatcher_send_custom_event(instance->view_dispatcher, SubBruteCustomEventTypeError);
    }
}

void subbrute_scene_setup_attack_on_enter(void* context) {
    furi_assert(context);
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteAttackView* view = instance->view_attack;

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "Enter Attack: %d", instance->device->attack);
#endif

    subbrute_worker_set_callback(
        instance->worker, subbrute_scene_setup_attack_device_state_changed, context);

    if(subbrute_worker_is_running(instance->worker)) {
        instance->device->key_index = subbrute_worker_get_step(instance->worker);
        subbrute_worker_stop(instance->worker);
    }

    subbrute_attack_view_init_values(
        view,
        instance->device->attack,
        instance->device->max_value,
        instance->device->key_index,
        false);

    instance->current_view = SubBruteViewAttack;
    subbrute_attack_view_set_callback(view, subbrute_scene_setup_attack_callback, instance);
    view_dispatcher_switch_to_view(instance->view_dispatcher, instance->current_view);
}

void subbrute_scene_setup_attack_on_exit(void* context) {
    furi_assert(context);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_scene_setup_attack_on_exit");
#endif
    SubBruteState* instance = (SubBruteState*)context;
    subbrute_worker_stop(instance->worker);
    notification_message(instance->notifications, &sequence_blink_stop);
}

bool subbrute_scene_setup_attack_on_event(void* context, SceneManagerEvent event) {
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteAttackView* view = instance->view_attack;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubBruteCustomEventTypeTransmitStarted) {
            subbrute_attack_view_set_worker_type(view, false);
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneRunAttack);
        } else if(event.event == SubBruteCustomEventTypeSaveFile) {
            subbrute_attack_view_init_values(
                view,
                instance->device->attack,
                instance->device->max_value,
                instance->device->key_index,
                false);
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneSaveName);
        } else if(event.event == SubBruteCustomEventTypeBackPressed) {
            subbrute_attack_view_init_values(
                view,
                instance->device->attack,
                instance->device->max_value,
                instance->device->key_index,
                false);
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneStart);
        } else if(event.event == SubBruteCustomEventTypeError) {
            notification_message(instance->notifications, &sequence_error);
        } else if(event.event == SubBruteCustomEventTypeTransmitCustom) {
            // We can transmit only in not working states
            if(subbrute_worker_can_manual_transmit(instance->worker)) {
                // MANUAL Transmit!
                // Blink
                notification_message(instance->notifications, &sequence_blink_green_100);
                subbrute_worker_transmit_current_key(
                    instance->worker, instance->device->key_index);
                // Stop
                notification_message(instance->notifications, &sequence_blink_stop);
            }
        } else if(event.event == SubBruteCustomEventTypeChangeStepUp) {
            // +1
            uint64_t step = subbrute_device_add_step(instance->device, 1);
            subbrute_worker_set_step(instance->worker, step);
            subbrute_attack_view_set_current_step(view, step);
        } else if(event.event == SubBruteCustomEventTypeChangeStepUpMore) {
            // +50
            uint64_t step = subbrute_device_add_step(instance->device, 50);
            subbrute_worker_set_step(instance->worker, step);
            subbrute_attack_view_set_current_step(view, step);
        } else if(event.event == SubBruteCustomEventTypeChangeStepDown) {
            // -1
            uint64_t step = subbrute_device_add_step(instance->device, -1);
            subbrute_worker_set_step(instance->worker, step);
            subbrute_attack_view_set_current_step(view, step);
        } else if(event.event == SubBruteCustomEventTypeChangeStepDownMore) {
            // -50
            uint64_t step = subbrute_device_add_step(instance->device, -50);
            subbrute_worker_set_step(instance->worker, step);
            subbrute_attack_view_set_current_step(view, step);
        }

        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        if(subbrute_worker_is_running(instance->worker)) {
            instance->device->key_index = subbrute_worker_get_step(instance->worker);
        }
        subbrute_attack_view_set_current_step(view, instance->device->key_index);
        consumed = true;
    }

    return consumed;
}
