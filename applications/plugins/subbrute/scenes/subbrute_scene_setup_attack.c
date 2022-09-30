#include "../subbrute_i.h"
#include "subbrute_scene.h"

#define TAG "SubBruteSceneSetupAttack"

static void subbrute_scene_setup_attack_callback(SubBruteCustomEvent event, void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
}

static void
    subbrute_scene_setup_attack_device_state_changed(void* context, SubBruteDeviceState state) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;

    if(state == SubBruteDeviceStateIDLE) {
        // Can't be IDLE on this step!
        view_dispatcher_send_custom_event(instance->view_dispatcher, SubBruteCustomEventTypeError);
    }
}

void subbrute_scene_setup_attack_on_enter(void* context) {
    furi_assert(context);
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteAttackView* view = instance->view_attack;

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "Enter Attack: %d", subbrute_device_get_attack(instance->device));
#endif

    subbrute_device_set_callback(
        instance->device, subbrute_scene_setup_attack_device_state_changed, context);

    if(subbrute_device_is_worker_running(instance->device)) {
        subbrute_worker_stop(instance->device);
    }

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
    subbrute_worker_stop(instance->device);
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
                subbrute_device_get_attack(instance->device),
                subbrute_device_get_max_value(instance->device),
                subbrute_device_get_step(instance->device),
                false);
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneSaveName);
        } else if(event.event == SubBruteCustomEventTypeBackPressed) {
            subbrute_device_reset_step(instance->device);
            subbrute_attack_view_init_values(
                view,
                subbrute_device_get_attack(instance->device),
                subbrute_device_get_max_value(instance->device),
                subbrute_device_get_step(instance->device),
                false);
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneStart);
        } else if(event.event == SubBruteCustomEventTypeError) {
            notification_message(instance->notifications, &sequence_error);
        } else if(event.event == SubBruteCustomEventTypeTransmitCustom) {
            // We can transmit only in not working states
            if(subbrute_device_can_manual_transmit(instance->device)) {
                // MANUAL Transmit!
                // Blink
                notification_message(instance->notifications, &sequence_blink_green_100);
                subbrute_device_transmit_current_key(instance->device);
                // Stop
                notification_message(instance->notifications, &sequence_blink_stop);
            }
        } else if(event.event == SubBruteCustomEventTypeChangeStepUp) {
            // +1
            uint64_t step = subbrute_device_add_step(instance->device, 1);
            subbrute_attack_view_set_current_step(view, step);
        } else if(event.event == SubBruteCustomEventTypeChangeStepUpMore) {
            // +50
            uint64_t step = subbrute_device_add_step(instance->device, 50);
            subbrute_attack_view_set_current_step(view, step);
        } else if(event.event == SubBruteCustomEventTypeChangeStepDown) {
            // -1
            uint64_t step = subbrute_device_add_step(instance->device, -1);
            subbrute_attack_view_set_current_step(view, step);
        } else if(event.event == SubBruteCustomEventTypeChangeStepDownMore) {
            // -50
            uint64_t step = subbrute_device_add_step(instance->device, -50);
            subbrute_attack_view_set_current_step(view, step);
        }

        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        subbrute_attack_view_set_current_step(view, subbrute_device_get_step(instance->device));
        consumed = true;
    }

    return consumed;
}
