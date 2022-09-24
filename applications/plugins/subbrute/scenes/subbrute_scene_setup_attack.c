#include "../subbrute_i.h"
#include "../subbrute_custom_event.h"
#include "../views/subbrute_attack_view.h"

static void subbrute_scene_setup_attack_callback(SubBruteCustomEvent event, void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
}

void subbrute_scene_setup_attack_on_enter(void* context) {
    furi_assert(context);
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteAttackView* view = instance->view_attack;

    instance->current_view = SubBruteViewAttack;
    subbrute_attack_view_set_callback(view, subbrute_scene_setup_attack_callback, instance);
    view_dispatcher_switch_to_view(instance->view_dispatcher, instance->current_view);

    instance->device->key_index = subbrute_attack_view_get_current_step(view);
    subbrute_attack_view_init_values(
        view,
        (uint8_t)instance->device->attack,
        instance->device->max_value,
        instance->device->key_index);

    // Run worker anyway
    subbrute_attack_view_start_worker(
        view,
        instance->device->frequency,
        instance->device->preset,
        instance->device->protocol_name);
}

void subbrute_scene_setup_attack_on_exit(void* context) {
    furi_assert(context);
    SubBruteState* instance = (SubBruteState*)context;
    notification_message(instance->notifications, &sequence_blink_stop);
}

bool subbrute_scene_setup_attack_on_event(void* context, SceneManagerEvent event) {
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteAttackView * view = instance->view_attack;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubBruteCustomEventTypeTransmitStarted) {
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneRunAttack);
        } else if (event.event == SubBruteCustomEventTypeBackPressed) {
            subbrute_attack_view_stop_worker(view);
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneStart);
        } else if (event.event == SubBruteCustomEventTypeChangeStep) {
            instance->device->key_index = subbrute_attack_view_get_current_step(view);
        } else if(event.event == SubBruteCustomEventTypeTransmitCustom) {
            if(subbrute_attack_view_can_send(view)) {
                // Blink
                notification_message(instance->notifications, &sequence_blink_green_100);

                subbrute_device_create_packet_parsed(
                    instance->device, instance->device->key_index);
                subbrute_attack_view_transmit(view, instance->device->payload);

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