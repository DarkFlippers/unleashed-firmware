#include "../subghz_i.h"
#include "../views/subghz_receiver.h"

void subghz_scene_receiver_callback(SubghzReceverEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

const void subghz_scene_receiver_on_enter(void* context) {
    SubGhz* subghz = context;
    SubghzReceiver* subghz_receiver = subghz->subghz_receiver;

    subghz_receiver_set_callback(subghz_receiver, subghz_scene_receiver_callback, subghz);

    subghz_receiver_set_protocol(subghz_receiver, subghz->protocol_result);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewReceiver);
}

const bool subghz_scene_receiver_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubghzReceverEventSave) {
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
            return true;
        } else if(event.event == SubghzReceverEventBack) {
            scene_manager_previous_scene(subghz->scene_manager);
            return true;
        }
    }
    return false;
}

const void subghz_scene_receiver_on_exit(void* context) {
    // SubGhz* subghz = context;
}
