#include "../nfc_app_i.h"
#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"
#include "../helpers/protocol_support/nfc_protocol_support_unlock_helper.h"
#include "../views/dict_attack.h"

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

    // Dictionary attack view. A shared piece of app UI, not protocol logic: four protocols drive
    // the same screen, so it stays in the app and the plugins reach it through here.
    API_METHOD(dict_attack_set_callback, void, (DictAttack*, DictAttackCallback, void*)),
    API_METHOD(dict_attack_set_header, void, (DictAttack*, const char*)),
    API_METHOD(dict_attack_set_card_state, void, (DictAttack*, bool)),
    API_METHOD(dict_attack_set_total_dict_keys, void, (DictAttack*, size_t)),
    API_METHOD(dict_attack_set_current_dict_key, void, (DictAttack*, size_t)),
    API_METHOD(dict_attack_set_type, void, (DictAttack*, DictAttackType)),
    API_METHOD(dict_attack_set_pages_total, void, (DictAttack*, uint8_t)),
    API_METHOD(dict_attack_set_pages_read, void, (DictAttack*, uint8_t)),
    API_METHOD(dict_attack_set_key_found, void, (DictAttack*, bool))));
