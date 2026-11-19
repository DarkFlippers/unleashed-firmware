#include "../subghz_i.h"
#include <loader/loader.h>
#include <loader/firmware_api/firmware_api.h>
#include <flipper_application/flipper_application.h>

// Sub-GHz RAW Edit ships with the firmware, but lives on the SD card and can be deleted;
// 1.9+ opens a file passed as argument
#define SUBGHZ_RAW_EDITOR_PATH        EXT_PATH("apps/Sub-GHz/subghz_raw_edit.fap")
#define SUBGHZ_RAW_EDITOR_MIN_VERSION ((1 << 16) | 9)

enum SubmenuIndex {
    SubmenuIndexDecode,
    SubmenuIndexEdit,
    SubmenuIndexDelete,
    SubmenuIndexRawEditor,
};

void subghz_scene_more_raw_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

// Still on the card and recent enough to take a file argument
static bool subghz_scene_more_raw_editor_available(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperApplication* app = flipper_application_alloc(storage, firmware_api_interface);
    bool available = flipper_application_preload_manifest(app, SUBGHZ_RAW_EDITOR_PATH) ==
                         FlipperApplicationPreloadStatusSuccess &&
                     flipper_application_get_manifest(app)->app_version >=
                         SUBGHZ_RAW_EDITOR_MIN_VERSION;
    flipper_application_free(app);
    furi_record_close(RECORD_STORAGE);
    return available;
}

void subghz_scene_more_raw_on_enter(void* context) {
    SubGhz* subghz = context;

    submenu_add_item(
        subghz->submenu,
        "Decode",
        SubmenuIndexDecode,
        subghz_scene_more_raw_submenu_callback,
        subghz);

    if(subghz_scene_more_raw_editor_available()) {
        submenu_add_item(
            subghz->submenu,
            "Edit",
            SubmenuIndexRawEditor,
            subghz_scene_more_raw_submenu_callback,
            subghz);
    }

    submenu_add_item(
        subghz->submenu,
        "Rename",
        SubmenuIndexEdit,
        subghz_scene_more_raw_submenu_callback,
        subghz);

    submenu_add_item(
        subghz->submenu,
        "Delete",
        SubmenuIndexDelete,
        subghz_scene_more_raw_submenu_callback,
        subghz);

    submenu_set_selected_item(
        subghz->submenu, scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneMoreRAW));

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdMenu);
}

bool subghz_scene_more_raw_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(event.type != SceneManagerEventTypeCustom || event.event > SubmenuIndexRawEditor) {
        return false;
    }

    if(!subghz_file_available(subghz)) {
        if(!scene_manager_search_and_switch_to_previous_scene(
               subghz->scene_manager, SubGhzSceneStart)) {
            scene_manager_stop(subghz->scene_manager);
            view_dispatcher_stop(subghz->view_dispatcher);
        }
        return false;
    }

    if(event.event == SubmenuIndexRawEditor) {
        // Open the file in RAW Edit, then come back to Sub-GHz
        Loader* loader = furi_record_open(RECORD_LOADER);
        loader_enqueue_launch(
            loader,
            SUBGHZ_RAW_EDITOR_PATH,
            furi_string_get_cstr(subghz->file_path),
            LoaderDeferredLaunchFlagGui);
        loader_enqueue_launch(loader, "Sub-GHz", NULL, LoaderDeferredLaunchFlagNone);
        furi_record_close(RECORD_LOADER);

        scene_manager_stop(subghz->scene_manager);
        view_dispatcher_stop(subghz->view_dispatcher);
        return true;
    }

    scene_manager_set_scene_state(subghz->scene_manager, SubGhzSceneMoreRAW, event.event);
    if(event.event == SubmenuIndexDelete) {
        scene_manager_set_scene_state(
            subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerNoSet);
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneDeleteRAW);
    } else if(event.event == SubmenuIndexEdit) {
        furi_string_reset(subghz->file_path_tmp);
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
    } else {
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneDecodeRAW);
    }
    return true;
}

void subghz_scene_more_raw_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_reset(subghz->submenu);
}
