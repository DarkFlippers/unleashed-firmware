#include "../subbrute_i.h"
#include "../subbrute_custom_event.h"
#include "../views/subbrute_attack_view.h"

#define TAG "SubBruteSceneSetupAttack"

static void subbrute_scene_setup_attack_callback(SubBruteCustomEvent event, void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
}

void subbrute_scene_setup_attack_on_enter(void* context) {
    furi_assert(context);
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteAttackView* view = instance->view_attack;

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "Enter Attack: %d", instance->device->attack);
#endif

    subbrute_attack_view_init_values(
        view,
        instance->device->attack,
        instance->device->max_value,
        instance->device->key_index,
        false);

    if(!subbrute_worker_init_manual_transmit(
           instance->worker,
           instance->device->frequency,
           instance->device->preset,
           string_get_cstr(instance->device->protocol_name))) {
        FURI_LOG_W(TAG, "Worker init failed!");
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
    subbrute_worker_manual_transmit_stop(instance->worker);
    notification_message(instance->notifications, &sequence_blink_stop);
}

bool subbrute_scene_setup_attack_on_event(void* context, SceneManagerEvent event) {
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteAttackView* view = instance->view_attack;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubBruteCustomEventTypeTransmitStarted) {
            subbrute_worker_set_continuous_worker(instance->worker, false);
            subbrute_attack_view_set_worker_type(view, false);
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneRunAttack);
        } else if(event.event == SubBruteCustomEventTypeTransmitContinuousStarted) {
            // Setting different type of worker
            subbrute_worker_set_continuous_worker(instance->worker, true);
            subbrute_attack_view_set_worker_type(view, true);
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneRunAttack);
        } else if(event.event == SubBruteCustomEventTypeSaveFile) {
            subbrute_worker_manual_transmit_stop(instance->worker);

            subbrute_attack_view_init_values(
                view,
                instance->device->attack,
                instance->device->max_value,
                instance->device->key_index,
                false);
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneSaveName);
        } else if(event.event == SubBruteCustomEventTypeBackPressed) {
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "SubBruteCustomEventTypeBackPressed");
#endif
            instance->device->key_index = 0x00;
            //subbrute_attack_view_stop_worker(view);
            subbrute_attack_view_init_values(
                view,
                instance->device->attack,
                instance->device->max_value,
                instance->device->key_index,
                false);
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneStart);
        } else if(event.event == SubBruteCustomEventTypeChangeStepUp) {
            // +1
            if((instance->device->key_index + 1) - instance->device->max_value == 1) {
                instance->device->key_index = 0x00;
            } else {
                uint64_t value = instance->device->key_index + 1;
                if(value == instance->device->max_value) {
                    instance->device->key_index = value;
                } else {
                    instance->device->key_index = value % instance->device->max_value;
                }
            }
            subbrute_attack_view_set_current_step(view, instance->device->key_index);
        } else if(event.event == SubBruteCustomEventTypeChangeStepUpMore) {
            // +50
            uint64_t value = instance->device->key_index + 50;
            if(value == instance->device->max_value) {
                instance->device->key_index += value;
            } else {
                instance->device->key_index = value % instance->device->max_value;
            }
            subbrute_attack_view_set_current_step(view, instance->device->key_index);
        } else if(event.event == SubBruteCustomEventTypeChangeStepDown) {
            // -1
            if(instance->device->key_index - 1 == 0) {
                instance->device->key_index = 0x00;
            } else if(instance->device->key_index == 0) {
                instance->device->key_index = instance->device->max_value;
            } else {
                uint64_t value = ((instance->device->key_index - 1) + instance->device->max_value);
                if(value == instance->device->max_value) {
                    instance->device->key_index = value;
                } else {
                    instance->device->key_index = value % instance->device->max_value;
                }
            }
            subbrute_attack_view_set_current_step(view, instance->device->key_index);
        } else if(event.event == SubBruteCustomEventTypeChangeStepDownMore) {
            // -50
            uint64_t value = ((instance->device->key_index - 50) + instance->device->max_value);
            if(value == instance->device->max_value) {
                instance->device->key_index = value;
            } else {
                instance->device->key_index = value % instance->device->max_value;
            }
            subbrute_attack_view_set_current_step(view, instance->device->key_index);
        } else if(event.event == SubBruteCustomEventTypeTransmitCustom) {
            if(subbrute_worker_can_manual_transmit(instance->worker, true)) {
                // Blink
                notification_message(instance->notifications, &sequence_blink_green_100);

                //                if(!subbrute_attack_view_is_worker_running(view)) {
                //                    subbrute_attack_view_start_worker(
                //                        view,
                //                        instance->device->frequency,
                //                        instance->device->preset,
                //                        string_get_cstr(instance->device->protocol_name));
                //                }
                subbrute_device_create_packet_parsed(
                    instance->device, instance->device->key_index, false);
                subbrute_worker_manual_transmit(instance->worker, instance->device->payload);

                // Stop
                notification_message(instance->notifications, &sequence_blink_stop);
            }
        }

        consumed = true;
    }

    //    if(event.type == SceneManagerEventTypeCustom) {
    //        switch(event.event) {
    //        case SubBruteCustomEventTypeMenuSelected:
    //            with_view_model(
    //                view, (SubBruteMainViewModel * model) {
    //                    instance->menu_index = model->index;
    //                    return false;
    //                });
    //            scene_manager_next_scene(instance->scene_manager, SubBruteSceneLoadFile);
    //            consumed = true;
    //            break;
    //        case SubBruteCustomEventTypeLoadFile:
    //            with_view_model(
    //                view, (SubBruteMainViewModel * model) {
    //                    instance->menu_index = model->index;
    //                    return false;
    //                });
    //            scene_manager_next_scene(instance->scene_manager, SubBruteSceneSetupAttack);
    //            consumed = true;
    //            break;
    //        }
    //    }

    return consumed;
}