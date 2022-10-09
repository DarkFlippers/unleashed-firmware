#include "../subbrute_i.h"
#include "subbrute_scene.h"

#define TAG "SubBruteSceneStart"

void subbrute_scene_load_select_callback(SubBruteCustomEvent event, void* context) {
    furi_assert(context);

    SubBruteState* instance = (SubBruteState*)context;
    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
}

void subbrute_scene_load_select_on_enter(void* context) {
    furi_assert(context);
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "subbrute_scene_load_select_on_enter");
#endif
    SubBruteState* instance = (SubBruteState*)context;
    SubBruteMainView* view = instance->view_main;

    instance->current_view = SubBruteViewMain;
    subbrute_main_view_set_callback(view, subbrute_scene_load_select_callback, instance);
    subbrute_main_view_set_index(view, 7, true, instance->device->file_key);

    view_dispatcher_switch_to_view(instance->view_dispatcher, instance->current_view);
}

void subbrute_scene_load_select_on_exit(void* context) {
    UNUSED(context);
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "subbrute_scene_load_select_on_exit");
#endif
}

bool subbrute_scene_load_select_on_event(void* context, SceneManagerEvent event) {
    SubBruteState* instance = (SubBruteState*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubBruteCustomEventTypeIndexSelected) {
            instance->device->load_index = subbrute_main_view_get_index(instance->view_main);
            if(!subbrute_worker_init_file_attack(
                   instance->worker,
                   instance->device->key_index,
                   instance->device->load_index,
                   instance->device->file_key,
                   instance->device->file_protocol_info)) {
                furi_crash("Invalid attack set!");
            }
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneSetupAttack);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(!scene_manager_search_and_switch_to_previous_scene(
               instance->scene_manager, SubBruteSceneStart)) {
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneStart);
        }
        consumed = true;
    }

    return consumed;
}