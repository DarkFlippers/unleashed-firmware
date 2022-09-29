#include "../subbrute_i.h"
#include "../subbrute_custom_event.h"
#include <lib/subghz/protocols/registry.h>

#define TAG "SubBruteSceneLoadFile"

//void subbrute_scene_load_file_callback(SubBruteCustomEvent event, void* context) {
////    furi_assert(context);
////
////    SubBruteState* instance = (SubBruteState*)context;
////    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
//}

void subbrute_scene_load_file_on_enter(void* context) {
    furi_assert(context);
    SubBruteState* instance = (SubBruteState*)context;

    // Input events and views are managed by file_browser
    string_t app_folder;
    string_t load_path;
    string_init(load_path);
    string_init_set_str(app_folder, SUBBRUTE_PATH);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, SUBBRUTE_FILE_EXT, &I_sub1_10px);

    SubBruteFileResult load_result = SubBruteFileResultUnknown;
    bool res =
        dialog_file_browser_show(instance->dialogs, load_path, app_folder, &browser_options);
#ifdef FURI_DEBUG
    FURI_LOG_D(
        TAG,
        "load_path: %s, app_folder: %s",
        string_get_cstr(load_path),
        string_get_cstr(app_folder));
#endif
    if(res) {
        load_result = subbrute_device_load_from_file(instance->device, load_path);
        if(load_result == SubBruteFileResultOk) {
            load_result = subbrute_device_attack_set(instance->device, SubBruteAttackLoadFile);
            if(load_result == SubBruteFileResultOk) {
                // Ready to run!
                instance->device->state = SubBruteDeviceStateReady;
                FURI_LOG_I(TAG, "Ready to run");
                res = true;
            }
        }

        if(load_result == SubBruteFileResultOk) {
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneLoadSelect);
        } else {
            FURI_LOG_E(TAG, "Returned error: %d", load_result);

            string_t dialog_msg;
            string_init(dialog_msg);
            string_cat_printf(
                dialog_msg, "Cannot parse\nfile: %s", subbrute_device_error_get_desc(load_result));
            dialog_message_show_storage_error(instance->dialogs, string_get_cstr(dialog_msg));
            string_clear(dialog_msg);
            scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, SubBruteSceneStart);
        }
    } else {
        scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, SubBruteSceneStart);
    }

    string_clear(app_folder);
    string_clear(load_path);
}

void subbrute_scene_load_file_on_exit(void* context) {
    UNUSED(context);
}

bool subbrute_scene_load_file_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}