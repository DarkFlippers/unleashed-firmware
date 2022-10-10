#include "../subbrute_i.h"
#include "subbrute_scene.h"

#define TAG "SubBruteSceneStart"

void subbrute_scene_start_callback(SubBruteCustomEvent event, void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_scene_start_callback");
#endif
    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
}

void subbrute_scene_start_on_enter(void* context) {
    furi_assert(context);
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "subbrute_scene_start_on_enter");
#endif
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteMainView* view = instance->view_main;

    instance->current_view = SubBruteViewMain;
    subbrute_main_view_set_callback(view, subbrute_scene_start_callback, instance);
    subbrute_main_view_set_index(view, instance->device->attack, false, NULL);

    view_dispatcher_switch_to_view(instance->view_dispatcher, instance->current_view);
}

void subbrute_scene_start_on_exit(void* context) {
    UNUSED(context);
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "subbrute_scene_start_on_exit");
#endif
}

bool subbrute_scene_start_on_event(void* context, SceneManagerEvent event) {
    SubBruteState* instance = (SubBruteState*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
#ifdef FURI_DEBUG
        FURI_LOG_D(
            TAG,
            "Event: %ld, SubBruteCustomEventTypeMenuSelected: %s, SubBruteCustomEventTypeLoadFile: %s",
            event.event,
            event.event == SubBruteCustomEventTypeMenuSelected ? "true" : "false",
            event.event == SubBruteCustomEventTypeLoadFile ? "true" : "false");
#endif
        if(event.event == SubBruteCustomEventTypeMenuSelected) {
            SubBruteAttacks attack = subbrute_main_view_get_index(instance->view_main);

            if(subbrute_device_attack_set(instance->device, attack) != SubBruteFileResultOk ||
               !subbrute_worker_init_default_attack(
                   instance->worker,
                   attack,
                   instance->device->key_index,
                   instance->device->protocol_info)) {
                furi_crash("Invalid attack set!");
            }
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneSetupAttack);

            consumed = true;
        } else if(event.event == SubBruteCustomEventTypeLoadFile) {
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneLoadFile);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        //exit app
        scene_manager_stop(instance->scene_manager);
        view_dispatcher_stop(instance->view_dispatcher);
        consumed = true;
    }

    return consumed;
}