#include "mf_desfire_extra_scenes.h"

#include "nfc/nfc_app_i.h"
#include "../nfc_protocol_support_gui_common.h"
#include "mf_desfire_render.h"

enum {
    MifareDesfireMoreInfoStateMenu,
    MifareDesfireMoreInfoStateItem, // MUST be last, states >= this correspond with submenu index
};

// ---- more_info ---------------------------------------------------------
enum MoreInfoSubmenuIndex {
    MoreInfoSubmenuIndexCardInfo,
    MoreInfoSubmenuIndexDynamic, // dynamic indices start here
};

static void mf_desfire_scene_more_info_on_enter(NfcApp* nfc) {
    Submenu* submenu = nfc->submenu;

    const uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfDesfireMoreInfo);
    const MfDesfireData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolMfDesfire);

    text_box_set_font(nfc->text_box, TextBoxFontHex);

    submenu_add_item(
        submenu,
        "Card info",
        MoreInfoSubmenuIndexCardInfo,
        nfc_protocol_support_common_submenu_callback,
        nfc);

    FuriString* label = furi_string_alloc();

    for(uint32_t i = 0; i < simple_array_get_count(data->application_ids); ++i) {
        const MfDesfireApplicationId* app_id = simple_array_cget(data->application_ids, i);
        furi_string_printf(
            label, "App %02x%02x%02x", app_id->data[2], app_id->data[1], app_id->data[0]);
        submenu_add_item(
            submenu,
            furi_string_get_cstr(label),
            i + MoreInfoSubmenuIndexDynamic,
            nfc_protocol_support_common_submenu_callback,
            nfc);
    }

    furi_string_free(label);

    if(state >= MifareDesfireMoreInfoStateItem) {
        submenu_set_selected_item(
            nfc->submenu, state - MifareDesfireMoreInfoStateItem + MoreInfoSubmenuIndexDynamic);
        scene_manager_set_scene_state(
            nfc->scene_manager, NfcSceneMfDesfireMoreInfo, MifareDesfireMoreInfoStateMenu);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

static bool mf_desfire_scene_more_info_on_event(NfcApp* nfc, SceneManagerEvent event) {
    bool consumed = false;

    const uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfDesfireMoreInfo);
    const MfDesfireData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolMfDesfire);

    if(event.type == SceneManagerEventTypeCustom) {
        TextBox* text_box = nfc->text_box;
        furi_string_reset(nfc->text_box_store);

        if(event.event == MoreInfoSubmenuIndexCardInfo) {
            nfc_render_mf_desfire_data(data, nfc->text_box_store);
            text_box_set_text(text_box, furi_string_get_cstr(nfc->text_box_store));
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
            scene_manager_set_scene_state(
                nfc->scene_manager,
                NfcSceneMfDesfireMoreInfo,
                MifareDesfireMoreInfoStateItem + MoreInfoSubmenuIndexCardInfo);
            consumed = true;
        } else {
            const uint32_t index = event.event - MoreInfoSubmenuIndexDynamic;
            scene_manager_set_scene_state(
                nfc->scene_manager,
                NfcSceneMfDesfireMoreInfo,
                MifareDesfireMoreInfoStateItem + index);
            scene_manager_set_scene_state(nfc->scene_manager, NfcSceneMfDesfireApp, index << 1);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfDesfireApp);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state >= MifareDesfireMoreInfoStateItem) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMfDesfireMoreInfo, MifareDesfireMoreInfoStateMenu);
        } else {
            // Return directly to the Info scene
            scene_manager_search_and_switch_to_previous_scene(nfc->scene_manager, NfcSceneInfo);
        }
        consumed = true;
    }

    return consumed;
}

static void mf_desfire_scene_more_info_on_exit(NfcApp* nfc) {
    // Clear views
    text_box_reset(nfc->text_box);
    furi_string_reset(nfc->text_box_store);
    submenu_reset(nfc->submenu);
}

// ---- app ---------------------------------------------------------------
enum AppSubmenuIndex {
    AppSubmenuIndexAppInfo,
    AppSubmenuIndexDynamic, // dynamic indexes start here
};

static void mf_desfire_scene_app_submenu_callback(void* context, uint32_t index) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

static void mf_desfire_scene_app_on_enter(NfcApp* nfc) {
    text_box_set_font(nfc->text_box, TextBoxFontHex);
    submenu_add_item(
        nfc->submenu,
        "App info",
        AppSubmenuIndexAppInfo,
        mf_desfire_scene_app_submenu_callback,
        nfc);

    const uint32_t app_idx =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfDesfireApp) >> 1;

    const MfDesfireData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolMfDesfire);
    const MfDesfireApplication* app = simple_array_cget(data->applications, app_idx);

    FuriString* label = furi_string_alloc();

    for(uint32_t i = 0; i < simple_array_get_count(app->file_ids); ++i) {
        const MfDesfireFileId file_id =
            *(const MfDesfireFileId*)simple_array_cget(app->file_ids, i);
        furi_string_printf(label, "File %d", file_id);
        submenu_add_item(
            nfc->submenu,
            furi_string_get_cstr(label),
            i + AppSubmenuIndexDynamic,
            mf_desfire_scene_app_submenu_callback,
            nfc);
    }

    furi_string_free(label);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

static bool mf_desfire_scene_app_on_event(NfcApp* nfc, SceneManagerEvent event) {
    bool consumed = false;

    const uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfDesfireApp);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else {
            const MfDesfireData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolMfDesfire);

            const uint32_t app_index =
                scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfDesfireApp) >> 1;
            const MfDesfireApplication* app = simple_array_cget(data->applications, app_index);

            TextBox* text_box = nfc->text_box;
            furi_string_reset(nfc->text_box_store);
            if(event.event == AppSubmenuIndexAppInfo) {
                const MfDesfireApplicationId* app_id =
                    simple_array_cget(data->application_ids, app_index);
                nfc_render_mf_desfire_application_id(app_id, nfc->text_box_store);
                nfc_render_mf_desfire_application(app, nfc->text_box_store);
            } else {
                const uint32_t file_index = event.event - AppSubmenuIndexDynamic;
                const MfDesfireFileId* file_id = simple_array_cget(app->file_ids, file_index);
                const MfDesfireFileSettings* file_settings =
                    simple_array_cget(app->file_settings, file_index);
                const MfDesfireFileData* file_data = simple_array_cget(app->file_data, file_index);
                nfc_render_mf_desfire_file_id(file_id, nfc->text_box_store);
                nfc_render_mf_desfire_file_settings_data(
                    file_settings, file_data, nfc->text_box_store);
            }
            text_box_set_text(text_box, furi_string_get_cstr(nfc->text_box_store));
            scene_manager_set_scene_state(nfc->scene_manager, NfcSceneMfDesfireApp, state | 1);
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
            consumed = true;
        }

    } else if(event.type == SceneManagerEventTypeBack) {
        if(state & 1) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
            scene_manager_set_scene_state(nfc->scene_manager, NfcSceneMfDesfireApp, state & ~1);
            consumed = true;
        }
    }

    return consumed;
}

static void mf_desfire_scene_app_on_exit(NfcApp* nfc) {
    // Clear views
    text_box_reset(nfc->text_box);
    furi_string_reset(nfc->text_box_store);
    submenu_reset(nfc->submenu);
}

const NfcProtocolSupportExtraScene mf_desfire_extra_scenes[MfDesfireExtraSceneNum] = {
    [MfDesfireExtraSceneMoreInfo] =
        {
            .on_enter = mf_desfire_scene_more_info_on_enter,
            .on_event = mf_desfire_scene_more_info_on_event,
            .on_exit = mf_desfire_scene_more_info_on_exit,
        },
    [MfDesfireExtraSceneApp] =
        {
            .on_enter = mf_desfire_scene_app_on_enter,
            .on_event = mf_desfire_scene_app_on_event,
            .on_exit = mf_desfire_scene_app_on_exit,
        },
};
