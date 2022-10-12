#include "../nfc_i.h"
#include "../helpers/nfc_emv_parser.h"
#include <dolphin/dolphin.h>

void nfc_scene_emv_read_success_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_emv_read_success_on_enter(void* context) {
    Nfc* nfc = context;
    EmvData* emv_data = &nfc->dev->dev_data.emv_data;
    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Setup Custom Widget view
    widget_add_button_element(
        nfc->widget, GuiButtonTypeLeft, "Retry", nfc_scene_emv_read_success_widget_callback, nfc);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeRight, "More", nfc_scene_emv_read_success_widget_callback, nfc);

    FuriString* temp_str;
    if(emv_data->name[0] != '\0') {
        temp_str = furi_string_alloc_printf("\e#%s\n", emv_data->name);
    } else {
        temp_str = furi_string_alloc_printf("\e#Unknown Bank Card\n");
    }
    if(emv_data->number_len) {
        for(uint8_t i = 0; i < emv_data->number_len; i += 2) {
            furi_string_cat_printf(
                temp_str, "%02X%02X ", emv_data->number[i], emv_data->number[i + 1]);
        }
        furi_string_trim(temp_str);
    } else if(emv_data->aid_len) {
        furi_string_cat_printf(temp_str, "Can't parse data from app\n");
        // Parse AID name
        FuriString* aid_name;
        aid_name = furi_string_alloc();
        if(nfc_emv_parser_get_aid_name(
               nfc->dev->storage, emv_data->aid, emv_data->aid_len, aid_name)) {
            furi_string_cat_printf(temp_str, "AID: %s", furi_string_get_cstr(aid_name));
        } else {
            furi_string_cat_printf(temp_str, "AID: ");
            for(uint8_t i = 0; i < emv_data->aid_len; i++) {
                furi_string_cat_printf(temp_str, "%02X", emv_data->aid[i]);
            }
        }
        furi_string_free(aid_name);
    }

    // Add expiration date
    if(emv_data->exp_mon) {
        furi_string_cat_printf(
            temp_str, "\nExp: %02X/%02X", emv_data->exp_mon, emv_data->exp_year);
    }
    // Parse currency code
    if((emv_data->currency_code)) {
        FuriString* currency_name;
        currency_name = furi_string_alloc();
        if(nfc_emv_parser_get_currency_name(
               nfc->dev->storage, emv_data->currency_code, currency_name)) {
            furi_string_cat_printf(temp_str, "\nCur: %s  ", furi_string_get_cstr(currency_name));
        }
        furi_string_free(currency_name);
    }
    // Parse country code
    if((emv_data->country_code)) {
        FuriString* country_name;
        country_name = furi_string_alloc();
        if(nfc_emv_parser_get_country_name(
               nfc->dev->storage, emv_data->country_code, country_name)) {
            furi_string_cat_printf(temp_str, "Reg: %s", furi_string_get_cstr(country_name));
        }
        furi_string_free(country_name);
    }

    notification_message_block(nfc->notifications, &sequence_set_green_255);

    widget_add_text_scroll_element(nfc->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_emv_read_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRetryConfirm);
            consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneEmvMenu);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(nfc->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }
    return consumed;
}

void nfc_scene_emv_read_success_on_exit(void* context) {
    Nfc* nfc = context;

    notification_message_block(nfc->notifications, &sequence_reset_green);

    // Clear view
    widget_reset(nfc->widget);
}
