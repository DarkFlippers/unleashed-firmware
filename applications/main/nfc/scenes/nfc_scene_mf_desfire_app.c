#include "../nfc_app_i.h"

#include "../helpers/protocol_support/mf_desfire/mf_desfire_render.h"

enum SubmenuIndex {
    SubmenuIndexAppInfo,
    SubmenuIndexDynamic, // dynamic indexes start here
};

static void nfc_scene_mf_desfire_app_submenu_callback(void* context, uint32_t index) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_mf_desfire_app_on_enter(void* context) {
    NfcApp* nfc = context;

    text_box_set_font(nfc->text_box, TextBoxFontHex);
    submenu_add_item(
        nfc->submenu,
        "App info",
        SubmenuIndexAppInfo,
        nfc_scene_mf_desfire_app_submenu_callback,
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
            i + SubmenuIndexDynamic,
            nfc_scene_mf_desfire_app_submenu_callback,
            nfc);
    }

    furi_string_free(label);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_mf_desfire_app_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
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
            if(event.event == SubmenuIndexAppInfo) {
                const MfDesfireApplicationId* app_id =
                    simple_array_cget(data->application_ids, app_index);
                nfc_render_mf_desfire_application_id(app_id, nfc->text_box_store);
                nfc_render_mf_desfire_application(app, nfc->text_box_store);
            } else {
                const uint32_t file_index = event.event - SubmenuIndexDynamic;
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

void nfc_scene_mf_desfire_app_on_exit(void* context) {
    NfcApp* nfc = context;

    // Clear views
    text_box_reset(nfc->text_box);
    furi_string_reset(nfc->text_box_store);
    submenu_reset(nfc->submenu);
}
