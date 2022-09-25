#include "../subbrute_i.h"
#include "../subbrute_custom_event.h"
#include "../views/subbrute_attack_view.h"

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
    subbrute_attack_view_start_worker(
        view,
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
        if(subbrute_attack_view_can_send(view)) {
            // Blink
            notification_message(instance->notifications, &sequence_blink_yellow_100);

            if(subbrute_attack_view_transmit(view, instance->device->payload)) {
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

    //    if(event.evt_type == EventTypeKey) {
    //        if(event.input_type == InputTypeShort) {
    //            switch(event.key) {
    //            case InputKeyDown:
    //                break;
    //            case InputKeyUp:
    //                if(!context->is_attacking) {
    //                    subbrute_send_packet_parsed(context);
    //                    string_clear(context->flipper_format_string);
    //                    toSave = true;
    //                    context->current_scene = SceneSaveName;
    //                }
    //                break;
    //            case InputKeyLeft:
    //                if(!context->is_attacking && context->payload > 0x00) {
    //                    context->payload--;
    //                    subbrute_send_packet(context);
    //                    notification_message(context->notify, &sequence_blink_blue_10);
    //                } else if(!context->is_attacking && context->payload == 0x00) {
    //                    context->payload = max_value;
    //                    subbrute_send_packet(context);
    //                    notification_message(context->notify, &sequence_blink_blue_10);
    //                }
    //                break;
    //            case InputKeyRight:
    //                if(!context->is_attacking && context->payload < max_value) {
    //                    context->payload++;
    //                    subbrute_send_packet(context);
    //                    notification_message(context->notify, &sequence_blink_blue_10);
    //                } else if(!context->is_attacking && context->payload == max_value) {
    //                    context->payload = 0x00;
    //                    subbrute_send_packet(context);
    //                    notification_message(context->notify, &sequence_blink_blue_10);
    //                }
    //                break;
    //            case InputKeyOk:
    //                if(!context->is_attacking) {
    //                    if(context->payload == max_value) {
    //                        context->payload = 0x00;
    //                        //subbrute_counter = 0;
    //                    }
    //                    context->is_attacking = true;
    //                    start_bruthread(context);
    //                    notification_message(context->notify, &sequence_blink_start_blue);
    //                } else {
    //                    context->is_attacking = false;
    //                    //context->close_thread_please = true;
    //                    if(context->is_thread_running && context->bruthread) {
    //                        furi_thread_join(context->bruthread); // wait until thread is finished
    //                    }
    //                    //context->close_thread_please = false;
    //                    notification_message(context->notify, &sequence_blink_stop);
    //                    notification_message(context->notify, &sequence_single_vibro);
    //                }
    //                break;
    //            case InputKeyBack:
    //                locked = false;
    //                //context->close_thread_please = true;
    //                context->is_attacking = false;
    //                if(context->is_thread_running && context->bruthread) {
    //                    furi_thread_join(context->bruthread); // wait until thread is finished
    //                }
    //                //context->close_thread_please = false;
    //                string_reset(context->notification_msg);
    //                context->payload = 0x00;
    //                //subbrute_counter = 0;
    //                notification_message(context->notify, &sequence_blink_stop);
    //                if(context->attack == SubBruteAttackLoadFile) {
    //                    context->current_scene = SceneSelectField;
    //                } else {
    //                    context->current_scene = SceneEntryPoint;
    //                }
    //                break;
    //            }
    //        }
    //    }
}
