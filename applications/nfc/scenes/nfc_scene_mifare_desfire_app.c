#include "../nfc_i.h"

#define TAG "NfcSceneMifareDesfireApp"

enum SubmenuIndex {
    SubmenuIndexAppInfo,
    SubmenuIndexDynamic, // dynamic indexes start here
};

MifareDesfireApplication* nfc_scene_mifare_desfire_app_get_app(Nfc* nfc) {
    uint32_t app_idx =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMifareDesfireApp) >> 1;
    MifareDesfireApplication* app = nfc->dev->dev_data.mf_df_data.app_head;
    for(int i = 0; i < app_idx && app; i++) {
        app = app->next;
    }
    return app;
}

void nfc_scene_mifare_desfire_app_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_mifare_desfire_app_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    Submenu* submenu = nfc->submenu;
    MifareDesfireApplication* app = nfc_scene_mifare_desfire_app_get_app(nfc);
    if(!app) {
        popup_set_icon(nfc->popup, 5, 5, &I_WarningDolphin_45x42);
        popup_set_header(nfc->popup, "Internal Error!", 55, 12, AlignLeft, AlignBottom);
        popup_set_text(
            nfc->popup,
            "No app selected.\nThis should\nnever happen,\nplease file a bug.",
            55,
            15,
            AlignLeft,
            AlignTop);
        view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
        FURI_LOG_E(TAG, "Bad state. No app selected?");
        return;
    }

    text_box_set_font(nfc->text_box, TextBoxFontHex);

    submenu_add_item(
        submenu,
        "App info",
        SubmenuIndexAppInfo,
        nfc_scene_mifare_desfire_app_submenu_callback,
        nfc);

    uint16_t cap = NFC_TEXT_STORE_SIZE;
    char* buf = nfc->text_store;
    int idx = SubmenuIndexDynamic;
    for(MifareDesfireFile* file = app->file_head; file; file = file->next) {
        int size = snprintf(buf, cap, "File %d", file->id);
        if(size < 0 || size >= cap) {
            FURI_LOG_W(
                TAG,
                "Exceeded NFC_TEXT_STORE_SIZE when preparing file id strings; menu truncated");
            break;
        }
        char* label = buf;
        cap -= size + 1;
        buf += size + 1;
        submenu_add_item(
            submenu, label, idx++, nfc_scene_mifare_desfire_app_submenu_callback, nfc);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_mifare_desfire_app_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;
    uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMifareDesfireApp);

    if(event.type == SceneManagerEventTypeCustom) {
        MifareDesfireApplication* app = nfc_scene_mifare_desfire_app_get_app(nfc);
        TextBox* text_box = nfc->text_box;
        string_reset(nfc->text_box_store);
        if(event.event == SubmenuIndexAppInfo) {
            mf_df_cat_application_info(app, nfc->text_box_store);
        } else {
            uint16_t index = event.event - SubmenuIndexDynamic;
            MifareDesfireFile* file = app->file_head;
            for(int i = 0; file && i < index; i++) {
                file = file->next;
            }
            if(!file) {
                return false;
            }
            mf_df_cat_file(file, nfc->text_box_store);
        }
        text_box_set_text(text_box, string_get_cstr(nfc->text_box_store));
        scene_manager_set_scene_state(nfc->scene_manager, NfcSceneMifareDesfireApp, state | 1);
        view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
        return true;
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state & 1) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMifareDesfireApp, state & ~1);
            return true;
        }
    }

    return false;
}

void nfc_scene_mifare_desfire_app_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    text_box_reset(nfc->text_box);
    string_reset(nfc->text_box_store);

    submenu_reset(nfc->submenu);
}
