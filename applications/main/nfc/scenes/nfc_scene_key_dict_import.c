#include "../nfc_app_i.h"

#include <dolphin/dolphin.h>

#define TAG "NfcKeyDictImport"

static void nfc_scene_key_dict_import_show_result(
    NfcApp* instance,
    const NfcKeyDictImportStats* stats,
    size_t key_count) {
    Popup* popup = instance->popup;

    if(stats->write_failed) {
        notification_message(instance->notifications, &sequence_error);
        popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
        popup_set_header(popup, "Save Failed", 64, 3, AlignCenter, AlignTop);
        nfc_text_store_set(instance, "SD card full\nor read-only");
    } else {
        // The two counts are the point of the screen: they say whether this card's keys were
        // worth carrying in a dictionary every future attack has to walk.
        popup_set_header(
            popup,
            stats->added > 0 ? "Keys Saved" : "Nothing to Add",
            64,
            3,
            AlignCenter,
            AlignTop);
        nfc_text_store_set(
            instance,
            "New keys: %u\nAlready known: %u",
            (unsigned)stats->added,
            (unsigned)(key_count - stats->added));
    }

    popup_set_text(popup, instance->text_store, 4, 26, AlignLeft, AlignTop);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);
}

void nfc_scene_key_dict_import_on_enter(void* context) {
    NfcApp* instance = context;
    const NfcKeyDict* dict = nfc_key_dict(instance->key_dict_type);

    // Both dictionaries are read in full, and keys_dict_alloc() walks each one once more just to
    // count - the Classic system dictionary alone is ~67 KB read twice. Put the animated view up
    // first so the wait is not a frozen copy of the menu we came from.
    nfc_show_loading_label_popup(instance, "Saving keys to\nuser dictionary", true);

    uint8_t* keys = malloc(NFC_KEY_DICT_DEVICE_KEYS_MAX * dict->key_size);
    const size_t key_count = nfc_key_dict_collect_from_device(
        instance->key_dict_type, instance->nfc_device, keys, NFC_KEY_DICT_DEVICE_KEYS_MAX);

    NfcKeyDictImportStats stats = {};
    nfc_key_dict_import(instance->key_dict_type, keys, key_count, &stats);
    free(keys);

    nfc_show_loading_label_popup(instance, NULL, false);

    FURI_LOG_I(
        TAG,
        "%s: %u of %u keys added (%u in system dict, %u in user dict)",
        dict->title,
        (unsigned)stats.added,
        (unsigned)key_count,
        (unsigned)stats.known_system,
        (unsigned)stats.known_user);

    if(stats.added > 0) dolphin_deed(DolphinDeedNfcKeyAdd);
    nfc_scene_key_dict_import_show_result(instance, &stats, key_count);
}

bool nfc_scene_key_dict_import_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);

    // Nothing to consume: the work is done on enter, and Back returns to the menu that sent us.
    return false;
}

void nfc_scene_key_dict_import_on_exit(void* context) {
    NfcApp* instance = context;

    popup_reset(instance->popup);
}
