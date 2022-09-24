#include "../subbrute_i.h"
#include "../subbrute_custom_event.h"
#include "../views/subbrute_main_view.h"

void subbrute_scene_start_callback(SubBruteCustomEvent event, void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
}

void subbrute_scene_start_on_enter(void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
    SubBruteMainView* view = instance->view_main;

    instance->current_view = SubBruteViewMain;
    subbrute_main_view_set_callback(view,
                                    subbrute_scene_start_callback,
                                    instance);
    subbrute_main_view_set_index(view, (uint8_t)instance->device->attack);

    view_dispatcher_switch_to_view(instance->view_dispatcher, instance->current_view);
}

void subbrute_scene_start_on_exit(void* context) {
    UNUSED(context);
}

bool subbrute_scene_start_on_event(void* context, SceneManagerEvent event) {
    SubBruteState* instance = (SubBruteState*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom && event.event == SubBruteCustomEventTypeMenuSelected) {
        //subbrute_device_attack_set
        SubBruteAttacks attack = subbrute_main_view_get_index(instance->view_main);

        if (attack == SubBruteAttackLoadFile) {
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneLoadFile);
        } else {
            subbrute_device_attack_set(instance->device, attack, NULL);
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneSetupAttack);
        }

        consumed = true;
    }

    return consumed;
}