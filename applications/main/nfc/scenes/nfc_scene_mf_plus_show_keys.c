#include "../nfc_app_i.h"

#include <nfc/protocols/mf_plus/mf_plus.h>

#define TAG "NfcMfPlusShowKeys"

void nfc_scene_mf_plus_show_keys_callback(GuiButtonType button, InputType type, void* context) {
    NfcApp* instance = context;
    if(button == GuiButtonTypeLeft && type == InputTypeShort) {
        scene_manager_previous_scene(instance->scene_manager);
    }
}

// Append a 16-byte AES key as compact uppercase hex (no separators -- 16 bytes are long enough that
// per-byte spaces would wrap the scroll line several times).
static void nfc_scene_mf_plus_show_keys_cat_key(FuriString* str, const MfPlusKey* key) {
    for(uint8_t i = 0; i < MF_PLUS_KEY_SIZE; i++) {
        furi_string_cat_printf(str, "%02X", key->data[i]);
    }
}

void nfc_scene_mf_plus_show_keys_on_enter(void* context) {
    NfcApp* instance = context;

    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);
    FuriString* str = instance->text_box_store;

    furi_string_reset(str);
    nfc_append_filename_string_when_present(instance, str);
    furi_string_cat_printf(str, "\e#Found MFP Keys:");

    // Sector keys: list only sectors with a recovered key, omitting unknown ones (as MIFARE Classic
    // does), so the screen stays compact on a partially-read card.
    const uint8_t num_sectors = mf_plus_get_sector_count(data->size);
    uint8_t found_a = 0, found_b = 0;
    for(uint8_t sector = 0; sector < num_sectors; sector++) {
        const bool key_a = mf_plus_is_key_found(data, sector, MfPlusKeyTypeA);
        const bool key_b = mf_plus_is_key_found(data, sector, MfPlusKeyTypeB);
        if(!key_a && !key_b) continue;

        furi_string_cat_printf(str, "\n  -> Sector %u", sector);
        if(key_a) {
            found_a++;
            furi_string_cat_printf(str, "\n\e*A: ");
            nfc_scene_mf_plus_show_keys_cat_key(str, &data->key_a[sector]);
        }
        if(key_b) {
            found_b++;
            furi_string_cat_printf(str, "\n\e*B: ");
            nfc_scene_mf_plus_show_keys_cat_key(str, &data->key_b[sector]);
        }
    }

    // Admin keys (Card Master / Card Config / L3 Switch / SL1 Card Auth), recovered by the same dict
    // attack. Shown under their own heading, again only the ones actually recovered.
    uint8_t found_admin = 0;
    for(uint8_t type = 0; type < MfPlusAdminKeyNum; type++) {
        if(!mf_plus_is_admin_key_found(data, type)) continue;
        if(found_admin == 0) furi_string_cat_printf(str, "\n\e*Admin Keys:");
        found_admin++;
        furi_string_cat_printf(str, "\n\e*%s: ", mf_plus_get_admin_key_name(type));
        nfc_scene_mf_plus_show_keys_cat_key(str, &data->admin_key[type]);
    }

    if(found_a == 0 && found_b == 0 && found_admin == 0) {
        furi_string_cat_printf(str, "\n\nNo keys recovered yet.");
    }

    furi_string_cat_printf(
        str,
        "\nTotal keys found:\n -> %u/%u A keys\n -> %u/%u B keys\n -> %u/%u admin keys",
        found_a,
        num_sectors,
        found_b,
        num_sectors,
        found_admin,
        (uint8_t)MfPlusAdminKeyNum);

    widget_add_text_scroll_element(instance->widget, 2, 2, 124, 60, furi_string_get_cstr(str));
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeLeft,
        "Back",
        nfc_scene_mf_plus_show_keys_callback,
        instance);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_plus_show_keys_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfc_scene_mf_plus_show_keys_on_exit(void* context) {
    NfcApp* instance = context;
    widget_reset(instance->widget);
    furi_string_reset(instance->text_box_store);
}
