#include "../nfc_i.h"

#define TAG "NfcSceneMfDesfireApp"

enum SubmenuIndex {
    SubmenuIndexAppInfo,
    SubmenuIndexDynamic, // dynamic indexes start here
};

void nfc_scene_mf_desfire_popup_callback(void* context) {
    furi_assert(context);

    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventViewExit);
}

MifareDesfireApplication* nfc_scene_mf_desfire_app_get_app(Nfc* nfc) {
    uint32_t app_idx = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfDesfireApp) >>
                       1;
    MifareDesfireApplication* app = nfc->dev->dev_data.mf_df_data.app_head;
    for(uint32_t i = 0; i < app_idx && app; i++) {
        app = app->next;
    }
    return app;
}

void nfc_scene_mf_desfire_app_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_mf_desfire_app_on_enter(void* context) {
    Nfc* nfc = context;
    MifareDesfireApplication* app = nfc_scene_mf_desfire_app_get_app(nfc);
    if(!app) {
        popup_set_icon(nfc->popup, 5, 5, &I_WarningDolphin_45x42);
        popup_set_header(nfc->popup, "Empty card!", 55, 12, AlignLeft, AlignBottom);
        popup_set_callback(nfc->popup, nfc_scene_mf_desfire_popup_callback);
        popup_set_context(nfc->popup, nfc);
        popup_set_timeout(nfc->popup, 3000);
        popup_enable_timeout(nfc->popup);
        popup_set_text(nfc->popup, "No application\nfound.", 55, 15, AlignLeft, AlignTop);
        view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
    } else {
        text_box_set_font(nfc->text_box, TextBoxFontHex);
        submenu_add_item(
            nfc->submenu,
            "App info",
            SubmenuIndexAppInfo,
            nfc_scene_mf_desfire_app_submenu_callback,
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
                nfc->submenu, label, idx++, nfc_scene_mf_desfire_app_submenu_callback, nfc);
        }

        view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
    }
}

bool nfc_scene_mf_desfire_app_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;
    uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfDesfireApp);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else {
            MifareDesfireApplication* app = nfc_scene_mf_desfire_app_get_app(nfc);
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
    Nfc* nfc = context;

    // Clear views
    popup_reset(nfc->popup);
    text_box_reset(nfc->text_box);
    string_reset(nfc->text_box_store);
    submenu_reset(nfc->submenu);
}
