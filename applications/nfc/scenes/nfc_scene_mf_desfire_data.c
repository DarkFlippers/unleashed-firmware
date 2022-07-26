#include "../nfc_i.h"

#define TAG "NfcSceneMfDesfireData"

enum {
    MifareDesfireDataStateMenu,
    MifareDesfireDataStateItem, // MUST be last, states >= this correspond with submenu index
};

enum SubmenuIndex {
    SubmenuIndexCardInfo,
    SubmenuIndexDynamic, // dynamic indexes start here
};

void nfc_scene_mf_desfire_data_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_mf_desfire_data_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;
    uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfDesfireData);
    MifareDesfireData* data = &nfc->dev->dev_data.mf_df_data;

    text_box_set_font(nfc->text_box, TextBoxFontHex);

    submenu_add_item(
        submenu,
        "Card info",
        SubmenuIndexCardInfo,
        nfc_scene_mf_desfire_data_submenu_callback,
        nfc);

    uint16_t cap = NFC_TEXT_STORE_SIZE;
    char* buf = nfc->text_store;
    int idx = SubmenuIndexDynamic;
    for(MifareDesfireApplication* app = data->app_head; app; app = app->next) {
        int size = snprintf(buf, cap, "App %02x%02x%02x", app->id[0], app->id[1], app->id[2]);
        if(size < 0 || size >= cap) {
            FURI_LOG_W(
                TAG, "Exceeded NFC_TEXT_STORE_SIZE when preparing app id strings; menu truncated");
            break;
        }
        char* label = buf;
        cap -= size + 1;
        buf += size + 1;
        submenu_add_item(submenu, label, idx++, nfc_scene_mf_desfire_data_submenu_callback, nfc);
    }

    if(state >= MifareDesfireDataStateItem) {
        submenu_set_selected_item(
            nfc->submenu, state - MifareDesfireDataStateItem + SubmenuIndexDynamic);
        scene_manager_set_scene_state(
            nfc->scene_manager, NfcSceneMfDesfireData, MifareDesfireDataStateMenu);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_mf_desfire_data_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;
    uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfDesfireData);
    MifareDesfireData* data = &nfc->dev->dev_data.mf_df_data;

    if(event.type == SceneManagerEventTypeCustom) {
        TextBox* text_box = nfc->text_box;
        string_reset(nfc->text_box_store);
        if(event.event == SubmenuIndexCardInfo) {
            mf_df_cat_card_info(data, nfc->text_box_store);
            text_box_set_text(text_box, string_get_cstr(nfc->text_box_store));
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
            scene_manager_set_scene_state(
                nfc->scene_manager,
                NfcSceneMfDesfireData,
                MifareDesfireDataStateItem + SubmenuIndexCardInfo);
            consumed = true;
        } else {
            uint16_t index = event.event - SubmenuIndexDynamic;
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMfDesfireData, MifareDesfireDataStateItem + index);
            scene_manager_set_scene_state(nfc->scene_manager, NfcSceneMfDesfireApp, index << 1);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfDesfireApp);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state >= MifareDesfireDataStateItem) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMfDesfireData, MifareDesfireDataStateMenu);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_mf_desfire_data_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear views
    text_box_reset(nfc->text_box);
    string_reset(nfc->text_box_store);
    submenu_reset(nfc->submenu);
}
