#include <lib/subghz/protocols/registry.h>
#include "../subbrute_i.h"
#include "../subbrute_custom_event.h"
#include "../views/subbrute_main_view.h"

//void subbrute_scene_load_file_callback(SubBruteCustomEvent event, void* context) {
////    furi_assert(context);
////
////    SubBruteState* instance = (SubBruteState*)context;
////    view_dispatcher_send_custom_event(instance->view_dispatcher, event);
//}

void subbrute_scene_load_file_on_enter(void* context) {
    furi_assert(context);
    SubBruteState* instance = (SubBruteState*)context;
    string_t file_path;
    string_init(file_path);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, SUBBRUTE_FILE_EXT, &I_sub1_10px);

    // Input events and views are managed by file_select
    bool res = dialog_file_browser_show(
        instance->dialogs,
        instance->device->load_path,
        instance->device->load_path,
        &browser_options);

    if(res) {
        SubBruteFileResult load_result = subbrute_device_load_protocol_from_file(instance->device);
        if(load_result == SubBruteFileResultOk) {
            scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, SubBruteSceneSetupAttack);
        } else {
            string_t dialog_msg;
            string_init(dialog_msg);
            string_cat_printf(
                dialog_msg, "Cannot parse\nfile: %s", subbrute_device_error_get_desc(load_result));
            dialog_message_show_storage_error(instance->dialogs, string_get_cstr(dialog_msg));
            string_clear(dialog_msg);
            res = false;
        }
    }

    string_clear(file_path);

    if(!res) {
        scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, SubBruteSceneLoadFile);
    }
}

void subbrute_scene_load_file_on_exit(void* context) {
    UNUSED(context);
}

bool subbrute_scene_load_file_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}