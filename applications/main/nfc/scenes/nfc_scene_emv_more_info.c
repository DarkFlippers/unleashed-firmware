#include "../nfc_app_i.h"

#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"
#include "../helpers/protocol_support/emv/emv_render.h"

enum {
    EmvMoreInfoStateMenu,
    EmvMoreInfoStateItem, // MUST be last, states >= this correspond with submenu index
};

enum SubmenuIndex {
    SubmenuIndexTransactions,
    SubmenuIndexDynamic, // dynamic indices start here
};

void nfc_scene_emv_more_info_on_enter(void* context) {
    NfcApp* nfc = context;
    Submenu* submenu = nfc->submenu;

    text_box_set_font(nfc->text_box, TextBoxFontHex);

    submenu_add_item(
        submenu,
        "Transactions",
        SubmenuIndexTransactions,
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
        widget_reset(nfc->widget);

        if(event.event == SubmenuIndexTransactions) {
            FuriString* temp_str = furi_string_alloc();
            nfc_render_emv_transactions(&data->emv_application, temp_str);
            widget_add_text_scroll_element(
                nfc->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));
            furi_string_free(temp_str);
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
            scene_manager_set_scene_state(
                nfc->scene_manager,
                NfcSceneEmvMoreInfo,
                EmvMoreInfoStateItem + SubmenuIndexTransactions);
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
    widget_reset(nfc->widget);
    submenu_reset(nfc->submenu);
}
