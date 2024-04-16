#include "../nfc_app_i.h"

#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"
#include "../helpers/protocol_support/mf_desfire/mf_desfire_render.h"

enum {
    MifareDesfireMoreInfoStateMenu,
    MifareDesfireMoreInfoStateItem, // MUST be last, states >= this correspond with submenu index
};

enum SubmenuIndex {
    SubmenuIndexCardInfo,
    SubmenuIndexDynamic, // dynamic indices start here
};

void nfc_scene_mf_desfire_more_info_on_enter(void* context) {
    NfcApp* nfc = context;
    Submenu* submenu = nfc->submenu;

    const uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfDesfireMoreInfo);
    const MfDesfireData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolMfDesfire);

    text_box_set_font(nfc->text_box, TextBoxFontHex);

    submenu_add_item(
        submenu,
        "Card info",
        SubmenuIndexCardInfo,
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
            i + SubmenuIndexDynamic,
            nfc_protocol_support_common_submenu_callback,
            nfc);
    }

    furi_string_free(label);

    if(state >= MifareDesfireMoreInfoStateItem) {
        submenu_set_selected_item(
            nfc->submenu, state - MifareDesfireMoreInfoStateItem + SubmenuIndexDynamic);
        scene_manager_set_scene_state(
            nfc->scene_manager, NfcSceneMfDesfireMoreInfo, MifareDesfireMoreInfoStateMenu);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_mf_desfire_more_info_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    bool consumed = false;

    const uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfDesfireMoreInfo);
    const MfDesfireData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolMfDesfire);

    if(event.type == SceneManagerEventTypeCustom) {
        TextBox* text_box = nfc->text_box;
        furi_string_reset(nfc->text_box_store);

        if(event.event == SubmenuIndexCardInfo) {
            nfc_render_mf_desfire_data(data, nfc->text_box_store);
            text_box_set_text(text_box, furi_string_get_cstr(nfc->text_box_store));
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
            scene_manager_set_scene_state(
                nfc->scene_manager,
                NfcSceneMfDesfireMoreInfo,
                MifareDesfireMoreInfoStateItem + SubmenuIndexCardInfo);
            consumed = true;
        } else {
            const uint32_t index = event.event - SubmenuIndexDynamic;
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

void nfc_scene_mf_desfire_more_info_on_exit(void* context) {
    NfcApp* nfc = context;

    // Clear views
    text_box_reset(nfc->text_box);
    furi_string_reset(nfc->text_box_store);
    submenu_reset(nfc->submenu);
}
