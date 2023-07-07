#include "../subghz_i.h"
#include <dolphin/dolphin.h>

#include <lib/subghz/protocols/raw.h>

enum SubmenuIndex {
    SubmenuIndexRead = 10,
    SubmenuIndexSaved,
    SubmenuIndexAddManually,
    SubmenuIndexFrequencyAnalyzer,
    SubmenuIndexReadRAW,
    SubmenuIndexExtSettings,
};

void subghz_scene_start_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

void subghz_scene_start_on_enter(void* context) {
    SubGhz* subghz = context;
    if(subghz->state_notifications == SubGhzNotificationStateStarting) {
        subghz->state_notifications = SubGhzNotificationStateIDLE;
    }

    submenu_add_item(
        subghz->submenu, "Read", SubmenuIndexRead, subghz_scene_start_submenu_callback, subghz);
    submenu_add_item(
        subghz->submenu,
        "Read RAW",
        SubmenuIndexReadRAW,
        subghz_scene_start_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu, "Saved", SubmenuIndexSaved, subghz_scene_start_submenu_callback, subghz);
    submenu_add_item(
        subghz->submenu,
        "Add Manually",
        SubmenuIndexAddManually,
        subghz_scene_start_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Frequency Analyzer",
        SubmenuIndexFrequencyAnalyzer,
        subghz_scene_start_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Radio Settings",
        SubmenuIndexExtSettings,
        subghz_scene_start_submenu_callback,
        subghz);
    submenu_set_selected_item(
        subghz->submenu, scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneStart));

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdMenu);
}

bool subghz_scene_start_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeBack) {
        //exit app
        scene_manager_stop(subghz->scene_manager);
        view_dispatcher_stop(subghz->view_dispatcher);
        return true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexExtSettings) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexExtSettings);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneExtModuleSettings);
            return true;
        } else if(event.event == SubmenuIndexAddManually) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexAddManually);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetType);
            return true;
        } else {
            furi_hal_subghz_enable_ext_power();

            if(!furi_hal_subghz_check_radio()) {
                furi_hal_subghz_select_radio_type(SubGhzRadioInternal);
                furi_hal_subghz_init_radio_type(SubGhzRadioInternal);
                subghz->last_settings->external_module_enabled = false;
                furi_string_set(subghz->error_str, "Please connect\nexternal radio");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowErrorSub);
                return true;
            } else if(event.event == SubmenuIndexReadRAW) {
                scene_manager_set_scene_state(
                    subghz->scene_manager, SubGhzSceneStart, SubmenuIndexReadRAW);
                subghz_rx_key_state_set(subghz, SubGhzRxKeyStateIDLE);
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReadRAW);
                return true;
            } else if(event.event == SubmenuIndexRead) {
                scene_manager_set_scene_state(
                    subghz->scene_manager, SubGhzSceneStart, SubmenuIndexRead);
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiver);
                return true;
            } else if(event.event == SubmenuIndexSaved) {
                scene_manager_set_scene_state(
                    subghz->scene_manager, SubGhzSceneStart, SubmenuIndexSaved);
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaved);
                return true;
            } else if(event.event == SubmenuIndexFrequencyAnalyzer) {
                scene_manager_set_scene_state(
                    subghz->scene_manager, SubGhzSceneStart, SubmenuIndexFrequencyAnalyzer);
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneFrequencyAnalyzer);
                dolphin_deed(DolphinDeedSubGhzFrequencyAnalyzer);
                return true;
            }
        }
    }
    return false;
}

void subghz_scene_start_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_reset(subghz->submenu);
}
