#include "../nfc_app_i.h"

#include <bit_lib/bit_lib.h>
#include <dolphin/dolphin.h>

#define TAG "NfcMfClassicShowKeys"

void nfc_scene_mf_classic_show_keys_callback(GuiButtonType button, InputType type, void* context) {
    NfcApp* instance = context;
    if(button == GuiButtonTypeLeft && type == InputTypeShort) {
        scene_manager_previous_scene(instance->scene_manager);
    }
}

void nfc_scene_mf_classic_show_keys_on_enter(void* context) {
    NfcApp* instance = context;

    const MfClassicData* mfc_data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);

    furi_string_reset(instance->text_box_store);
    nfc_append_filename_string_when_present(instance, instance->text_box_store);

    furi_string_cat_printf(instance->text_box_store, "\e#Found MFC Keys:");

    uint8_t num_sectors = mf_classic_get_total_sectors_num(mfc_data->type);
    uint8_t found_keys_a = 0, found_keys_b = 0;
    for(uint8_t i = 0; i < num_sectors; i++) {
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(mfc_data, i);

        bool key_a = FURI_BIT(mfc_data->key_a_mask, i);
        bool key_b = FURI_BIT(mfc_data->key_b_mask, i);

        if(key_a || key_b) {
            furi_string_cat_printf(instance->text_box_store, "\n  -> Sector %d\n\e*AccBits:", i);
            for(uint8_t j = 0; j < MF_CLASSIC_ACCESS_BYTES_SIZE; j++) {
                furi_string_cat_printf(
                    instance->text_box_store, " %02X", sec_tr->access_bits.data[j]);
            }
        }

        if(key_a) {
            found_keys_a++;
            furi_string_cat_printf(instance->text_box_store, "\n\e*A:");
            for(uint8_t j = 0; j < MF_CLASSIC_KEY_SIZE; j++)
                furi_string_cat_printf(instance->text_box_store, " %02X", sec_tr->key_a.data[j]);
        }
        if(key_b) {
            found_keys_b++;
            furi_string_cat_printf(instance->text_box_store, "\n\e*B:");
            for(uint8_t j = 0; j < MF_CLASSIC_KEY_SIZE; j++)
                furi_string_cat_printf(instance->text_box_store, " %02X", sec_tr->key_b.data[j]);
        }
    }

    furi_string_cat_printf(
        instance->text_box_store,
        "\nTotal keys found:\n -> %d/%d A keys\n -> %d/%d B keys",
        found_keys_a,
        num_sectors,
        found_keys_b,
        num_sectors);

    widget_add_text_scroll_element(
        instance->widget, 2, 2, 124, 60, furi_string_get_cstr(instance->text_box_store));
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeLeft,
        "Back",
        nfc_scene_mf_classic_show_keys_callback,
        instance);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_classic_show_keys_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfc_scene_mf_classic_show_keys_on_exit(void* context) {
    NfcApp* instance = context;
    widget_reset(instance->widget);
    furi_string_reset(instance->text_box_store);
}
