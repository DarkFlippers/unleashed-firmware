// Also brings in the views and helpers whose symbols are exported below.
#include "../nfc_app_i.h"
#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"
#include "../helpers/protocol_support/nfc_protocol_support_unlock_helper.h"

/*
 * A list of app's private functions and objects to expose for plugins.
 * It is used to generate a table of symbols for import resolver to use.
 * TBD: automatically generate this table from app's header files
 *
 * Keep this list small: anything exported here is pinned into the app's
 * resident image for good, since the resolver counts as a reference. Card
 * parsing helpers belong in the plugins that use them (see application.fam),
 * where they are only in RAM while that plugin is loaded.
 */
static constexpr auto nfc_app_api_table = sort(create_array_t<sym_entry>(
    API_METHOD(
        nfc_append_filename_string_when_present,
        void,
        (NfcApp * instance, FuriString* string)),
    API_METHOD(nfc_protocol_support_common_submenu_callback, void, (void* context, uint32_t index)),
    API_METHOD(
        nfc_protocol_support_common_widget_callback,
        void,
        (GuiButtonType result, InputType type, void* context)),
    API_METHOD(nfc_protocol_support_common_on_enter_empty, void, (NfcApp * instance)),
    API_METHOD(
        nfc_protocol_support_common_on_event_empty,
        bool,
        (NfcApp * instance, SceneManagerEvent event)),
    API_METHOD(nfc_unlock_helper_setup_from_state, void, (NfcApp * instance)),
    API_METHOD(nfc_unlock_helper_card_detected_handler, void, (NfcApp * instance)),

    // Dictionary attack view. A shared piece of app UI, not protocol logic: four scenes across
    // three protocol plugins drive the same screen, so it stays in the app and they reach it here.
    API_METHOD(dict_attack_reset, void, (DictAttack*)),
    API_METHOD(dict_attack_set_callback, void, (DictAttack*, DictAttackCallback, void*)),
    API_METHOD(dict_attack_set_header, void, (DictAttack*, const char*)),
    API_METHOD(dict_attack_set_card_state, void, (DictAttack*, bool)),
    API_METHOD(dict_attack_set_total_dict_keys, void, (DictAttack*, size_t)),
    API_METHOD(dict_attack_set_current_dict_key, void, (DictAttack*, size_t)),
    API_METHOD(dict_attack_set_type, void, (DictAttack*, DictAttackType)),
    API_METHOD(dict_attack_set_pages_total, void, (DictAttack*, uint8_t)),
    API_METHOD(dict_attack_set_pages_read, void, (DictAttack*, uint8_t)),
    API_METHOD(dict_attack_set_key_found, void, (DictAttack*, bool)),
    // Sector-shaped setters, used by the Classic and Plus attacks.
    API_METHOD(dict_attack_set_sectors_total, void, (DictAttack*, uint8_t)),
    API_METHOD(dict_attack_set_sectors_read, void, (DictAttack*, uint8_t)),
    API_METHOD(dict_attack_set_keys_found, void, (DictAttack*, uint8_t)),
    API_METHOD(dict_attack_set_current_sector, void, (DictAttack*, uint8_t)),
    API_METHOD(dict_attack_set_key_attack, void, (DictAttack*, uint8_t)),
    API_METHOD(dict_attack_reset_key_attack, void, (DictAttack*)),
    API_METHOD(dict_attack_set_nested_phase, void, (DictAttack*, MfClassicNestedPhase)),
    API_METHOD(dict_attack_set_prng_type, void, (DictAttack*, MfClassicPrngType)),
    API_METHOD(dict_attack_set_backdoor, void, (DictAttack*, MfClassicBackdoor)),
    API_METHOD(dict_attack_set_nested_target_key, void, (DictAttack*, uint16_t)),
    API_METHOD(dict_attack_set_msb_count, void, (DictAttack*, uint16_t)),

    // Detect reader view, driven only by the Classic detect-reader scene.
    API_METHOD(detect_reader_reset, void, (DetectReader*)),
    API_METHOD(detect_reader_set_callback, void, (DetectReader*, DetectReaderDoneCallback, void*)),
    API_METHOD(detect_reader_set_nonces_max, void, (DetectReader*, uint16_t)),
    API_METHOD(detect_reader_set_nonces_collected, void, (DetectReader*, uint16_t)),
    API_METHOD(detect_reader_set_state, void, (DetectReader*, DetectReaderState)),

    // App-level services the moved scenes call: LED feedback, the shared text store, shadow-file
    // saving, the loading popup, launching mfkey, and the detected-protocol list.
    API_METHOD(nfc_blink_read_start, void, (NfcApp*)),
    API_METHOD(nfc_blink_emulate_start, void, (NfcApp*)),
    API_METHOD(nfc_blink_stop, void, (NfcApp*)),
    API_METHOD(nfc_text_store_set, void, (NfcApp*, const char*, ...)),
    API_METHOD(nfc_text_store_clear, void, (NfcApp*)),
    API_METHOD(nfc_save_shadow_file, bool, (NfcApp*)),
    API_METHOD(nfc_show_loading_label_popup, void, (void*, const char*, bool)),
    API_METHOD(nfc_app_run_external, void, (NfcApp*, const char*)),
    API_METHOD(
        nfc_detected_protocols_set,
        void,
        (NfcDetectedProtocols*, const NfcProtocol*, uint32_t)),

    // Icons the moved scenes draw. Exported rather than linked into each plugin, which would
    // duplicate the bitmap into every .fal that draws it - the app already carries them.
    API_VARIABLE(A_Loading_24, const Icon),
    API_VARIABLE(I_MFKey_qr_25x25, const Icon),
    API_VARIABLE(I_NFC_manual_60x50, const Icon),
    API_VARIABLE(I_WarningDolphin_45x42, const Icon),
    API_VARIABLE(I_WarningDolphinFlip_45x42, const Icon),
    API_VARIABLE(I_Modern_reader_18x34, const Icon),
    API_VARIABLE(I_Move_flipper_26x39, const Icon)));
