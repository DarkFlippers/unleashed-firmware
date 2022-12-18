#include "../subbrute_i.h"
#include "../subbrute_custom_event.h"
#include "../views/subbrute_attack_view.h"
#include "../helpers/subbrute_worker.h"

#define TAG "SubBruteSceneRunAttack"

static void subbrute_scene_run_attack_callback(SubBruteCustomEvent event, void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
}

//static void subbrute_scene_run_attack_worker_callback(void* context) {
//    SubBruteState* instance = (SubBruteState*)context;
//
//    if(instance->locked || instance->device->key_index + 1 > instance->device->max_value) {
//        return;
//    }
//    instance->locked = true;
//
//    if(subbrute_worker_can_manual_transmit(instance->worker)) {
//        // Blink
//        notification_message(instance->notifications, &sequence_blink_yellow_100);
//        subbrute_device_create_packet_parsed(instance->device, instance->device->key_index, true);
//
//#ifdef FURI_DEBUG
//        FURI_LOG_I(TAG, "subbrute_worker_manual_transmit");
//#endif
//        if(subbrute_worker_manual_transmit(instance->worker, instance->device->payload)) {
//#ifdef FURI_DEBUG
//            FURI_LOG_I(TAG, "transmit ok");
//#endif
//            // Make payload for new iteration or exit
//            if(instance->device->key_index + 1 <= instance->device->max_value) {
//                instance->device->key_index++;
//            } else {
//                view_dispatcher_send_custom_event(
//                    instance->view_dispatcher, SubBruteCustomEventTypeTransmitFinished);
//            }
//        }
//
//        // Stop
//        notification_message(instance->notifications, &sequence_blink_stop);
//    }
//
//    instance->locked = false;
//    subbrute_attack_view_set_current_step(instance->view_attack, instance->device->key_index);
//}

void subbrute_scene_run_attack_on_exit(void* context) {
    furi_assert(context);
    SubBruteState* instance = (SubBruteState*)context;
    //    SubBruteAttackState* state = (SubBruteAttackState*)scene_manager_get_scene_state(
    //        instance->scene_manager, SubBruteSceneRunAttack);
    //    furi_assert(state);
    //
    //    furi_timer_free(state->timer);
    //    free(state);

    if(subbrute_worker_get_continuous_worker(instance->worker)) {
        subbrute_worker_stop(instance->worker);
    }

    notification_message(instance->notifications, &sequence_blink_stop);
}

void subbrute_scene_run_attack_on_enter(void* context) {
    furi_assert(context);
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteAttackView* view = instance->view_attack;
    //
    //    SubBruteAttackState* state = malloc(sizeof(SubBruteAttackState));
    //    scene_manager_set_scene_state(
    //        instance->scene_manager, SubBruteSceneRunAttack, (uint32_t)state);

    instance->current_view = SubBruteViewAttack;
    subbrute_attack_view_set_callback(view, subbrute_scene_run_attack_callback, instance);
    view_dispatcher_switch_to_view(instance->view_dispatcher, instance->current_view);

    subbrute_attack_view_init_values(
        view,
        (uint8_t)instance->device->attack,
        instance->device->max_value,
        instance->device->key_index,
        true);

    if(subbrute_worker_get_continuous_worker(instance->worker)) {
        // Init Continuous worker with values!
        if(!subbrute_worker_start(
               instance->worker,
               instance->device->frequency,
               instance->device->preset,
               string_get_cstr(instance->device->protocol_name))) {
            FURI_LOG_W(TAG, "Worker Continuous init failed!");
        }
    } else {
        // Init worker with values
        if(!subbrute_worker_init_manual_transmit(
               instance->worker,
               instance->device->frequency,
               instance->device->preset,
               string_get_cstr(instance->device->protocol_name))) {
            FURI_LOG_W(TAG, "Worker init failed!");
        }

        //    state->timer = furi_timer_alloc(
        //        subbrute_scene_run_attack_worker_callback, FuriTimerTypePeriodic, instance);
        //    furi_timer_start(state->timer, pdMS_TO_TICKS(100)); // 20 ms
    }
}

bool subbrute_scene_run_attack_on_event(void* context, SceneManagerEvent event) {
    SubBruteState* instance = (SubBruteState*)context;
    //    SubBruteAttackState* state = (SubBruteAttackState*)scene_manager_get_scene_state(
    //        instance->scene_manager, SubBruteSceneRunAttack);
    //    furi_assert(state);

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        SubBruteAttackView* view = instance->view_attack;

        if(event.event == SubBruteCustomEventTypeTransmitNotStarted ||
           event.event == SubBruteCustomEventTypeTransmitFinished ||
           event.event == SubBruteCustomEventTypeBackPressed) {
            //            furi_timer_stop(state->timer);
            // Stop transmit
            notification_message(instance->notifications, &sequence_display_backlight_on);
            notification_message(instance->notifications, &sequence_single_vibro);
            subbrute_attack_view_set_current_step(view, instance->device->key_index);
            scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, SubBruteSceneSetupAttack);
            consumed = true;
        } else if(event.event == SubBruteCustomEventTypeUpdateView) {
            subbrute_attack_view_set_current_step(view, instance->device->key_index);
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(subbrute_worker_get_continuous_worker(instance->worker)) {
            if(subbrute_worker_can_transmit(instance->worker)) {
                // Blink
                notification_message(instance->notifications, &sequence_blink_yellow_100);

                subbrute_device_create_packet_parsed(
                    instance->device, instance->device->key_index, true);

                if(subbrute_worker_transmit(instance->worker, instance->device->payload)) {
                    // Make payload for new iteration or exit
                    if(instance->device->key_index + 1 > instance->device->max_value) {
                        // End of list
                        view_dispatcher_send_custom_event(
                            instance->view_dispatcher, SubBruteCustomEventTypeTransmitFinished);
                    } else {
                        instance->device->key_index++;
                        view_dispatcher_send_custom_event(
                            instance->view_dispatcher, SubBruteCustomEventTypeUpdateView);
                        //subbrute_attack_view_set_current_step(view, instance->device->key_index);
                    }
                }

                // Stop
                notification_message(instance->notifications, &sequence_blink_stop);
            }
        } else {
            if(subbrute_worker_can_manual_transmit(instance->worker, false)) {
                // Blink
                notification_message(instance->notifications, &sequence_blink_yellow_100);

                subbrute_device_create_packet_parsed(
                    instance->device, instance->device->key_index, true);

                if(subbrute_worker_manual_transmit(instance->worker, instance->device->payload)) {
                    // Make payload for new iteration or exit
                    if(instance->device->key_index + 1 > instance->device->max_value) {
                        // End of list
                        view_dispatcher_send_custom_event(
                            instance->view_dispatcher, SubBruteCustomEventTypeTransmitFinished);
                    } else {
                        instance->device->key_index++;
                        view_dispatcher_send_custom_event(
                            instance->view_dispatcher, SubBruteCustomEventTypeUpdateView);
                        //subbrute_attack_view_set_current_step(view, instance->device->key_index);
                    }
                }

                // Stop
                notification_message(instance->notifications, &sequence_blink_stop);
            }
        }

        consumed = true;
    }

    return consumed;
}
