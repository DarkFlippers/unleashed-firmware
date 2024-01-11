#include "../nfc_app_i.h"

#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"
#include "../helpers/protocol_support/emv/emv_render.h"

enum {
    EmvMoreInfoStateMenu,
    EmvMoreInfoStateItem, // MUST be last, states >= this correspond with submenu index
};

enum SubmenuIndex {
    SubmenuIndexCardInfo,
    SubmenuIndexDynamic, // dynamic indices start here
};

void nfc_scene_emv_more_info_on_enter(void* context) {
    NfcApp* nfc = context;
    Submenu* submenu = nfc->submenu;

    text_box_set_font(nfc->text_box, TextBoxFontHex);

    submenu_add_item(
        submenu,
        "Card info",
        SubmenuIndexCardInfo,
        nfc_protocol_support_common_submenu_callback,
        nfc);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_emv_more_info_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    bool consumed = false;

    const uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneEmvMoreInfo);
    const EmvData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolEmv);

    if(event.type == SceneManagerEventTypeCustom) {
        TextBox* text_box = nfc->text_box;
        furi_string_reset(nfc->text_box_store);

        if(event.event == SubmenuIndexCardInfo) {
            nfc_render_emv_data(data, nfc->text_box_store);
            text_box_set_text(text_box, furi_string_get_cstr(nfc->text_box_store));
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
            scene_manager_set_scene_state(
                nfc->scene_manager,
                NfcSceneEmvMoreInfo,
                EmvMoreInfoStateItem + SubmenuIndexCardInfo);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state >= EmvMoreInfoStateItem) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneEmvMoreInfo, EmvMoreInfoStateMenu);
        } else {
            // Return directly to the Info scene
            scene_manager_search_and_switch_to_previous_scene(nfc->scene_manager, NfcSceneInfo);
        }
        consumed = true;
    }

    return consumed;
}

void nfc_scene_emv_more_info_on_exit(void* context) {
    NfcApp* nfc = context;

    // Clear views
    text_box_reset(nfc->text_box);
    furi_string_reset(nfc->text_box_store);
    submenu_reset(nfc->submenu);
}
