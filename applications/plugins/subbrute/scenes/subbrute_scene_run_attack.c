#include "../subbrute_i.h"
#include "../subbrute_custom_event.h"
#include "../views/subbrute_attack_view.h"
#include "../helpers/subbrute_worker.h"

static void subbrute_scene_run_attack_callback(SubBruteCustomEvent event, void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
}

void subbrute_scene_run_attack_on_exit(void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
    notification_message(instance->notifications, &sequence_blink_stop);
}

void subbrute_scene_run_attack_on_enter(void* context) {
    furi_assert(context);
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteAttackView* view = instance->view_attack;

    instance->current_view = SubBruteViewAttack;
    subbrute_attack_view_set_callback(view, subbrute_scene_run_attack_callback, instance);
    view_dispatcher_switch_to_view(instance->view_dispatcher, instance->current_view);

    subbrute_attack_view_init_values(
        view,
        (uint8_t)instance->device->attack,
        instance->device->max_value,
        instance->device->key_index,
        true);

    // Start worker if not started
    subbrute_worker_init_manual_transmit(
        instance->worker,
        instance->device->frequency,
        instance->device->preset,
        string_get_cstr(instance->device->protocol_name));
}

bool subbrute_scene_run_attack_on_event(void* context, SceneManagerEvent event) {
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteAttackView* view = instance->view_attack;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubBruteCustomEventTypeTransmitNotStarted ||
           event.event == SubBruteCustomEventTypeTransmitFinished ||
           event.event == SubBruteCustomEventTypeBackPressed) {
            // Stop transmit
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneSetupAttack);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(subbrute_worker_can_transmit(instance->worker)) {
            // Blink
            notification_message(instance->notifications, &sequence_blink_yellow_100);

            if(subbrute_worker_manual_transmit(instance->worker, instance->device->payload)) {
                // Make payload for new iteration or exit
                if(instance->device->key_index + 1 > instance->device->max_value) {
                    // End of list
                    scene_manager_next_scene(instance->scene_manager, SubBruteSceneSetupAttack);
                } else {
                    instance->device->key_index++;
                    subbrute_attack_view_set_current_step(view, instance->device->key_index);
                    subbrute_device_create_packet_parsed(
                        instance->device, instance->device->key_index);
                }
            }

            // Stop
            notification_message(instance->notifications, &sequence_blink_stop);
        }

        consumed = true;
    }

    return consumed;
}
