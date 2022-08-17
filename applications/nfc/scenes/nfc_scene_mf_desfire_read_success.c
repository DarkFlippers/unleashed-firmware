#include "../nfc_i.h"
#include <dolphin/dolphin.h>

void nfc_scene_mf_desfire_read_success_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    Nfc* nfc = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_mf_desfire_read_success_on_enter(void* context) {
    Nfc* nfc = context;

    FuriHalNfcDevData* nfc_data = &nfc->dev->dev_data.nfc_data;
    MifareDesfireData* data = &nfc->dev->dev_data.mf_df_data;
    Widget* widget = nfc->widget;

    // Prepare string for data display
    string_t temp_str;
    string_init_printf(temp_str, "\e#MIFARE DESfire\n");
    string_cat_printf(temp_str, "UID:");
    for(size_t i = 0; i < nfc_data->uid_len; i++) {
        string_cat_printf(temp_str, " %02X", nfc_data->uid[i]);
    }

    uint32_t bytes_total = 1 << (data->version.sw_storage >> 1);
    uint32_t bytes_free = data->free_memory ? data->free_memory->bytes : 0;
    string_cat_printf(temp_str, "\n%d", bytes_total);
    if(data->version.sw_storage & 1) {
        string_push_back(temp_str, '+');
    }
    string_cat_printf(temp_str, " bytes, %d bytes free\n", bytes_free);

    uint16_t n_apps = 0;
    uint16_t n_files = 0;
    for(MifareDesfireApplication* app = data->app_head; app; app = app->next) {
        n_apps++;
        for(MifareDesfireFile* file = app->file_head; file; file = file->next) {
            n_files++;
        }
    }
    string_cat_printf(temp_str, "%d Application", n_apps);
    if(n_apps != 1) {
        string_push_back(temp_str, 's');
    }
    string_cat_printf(temp_str, ", %d file", n_files);
    if(n_files != 1) {
        string_push_back(temp_str, 's');
    }

    // Add text scroll element
    widget_add_text_scroll_element(widget, 0, 0, 128, 52, string_get_cstr(temp_str));
    string_clear(temp_str);

    // Add button elements
    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", nfc_scene_mf_desfire_read_success_widget_callback, nfc);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "More", nfc_scene_mf_desfire_read_success_widget_callback, nfc);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_desfire_read_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRetryConfirm);
            consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfDesfireMenu);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(nfc->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }

    return consumed;
}

void nfc_scene_mf_desfire_read_success_on_exit(void* context) {
    Nfc* nfc = context;

    // Clean dialog
    widget_reset(nfc->widget);
}
