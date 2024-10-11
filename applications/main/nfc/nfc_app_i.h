#pragma once

#include "nfc_app.h"

#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/view.h>
#include <assets_icons.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <cli/cli.h>
#include <notification/notification_messages.h>

#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/popup.h>
#include <gui/modules/loading.h>
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/text_box.h>
#include <gui/modules/widget.h>
#include "views/dict_attack.h"
#include "views/detect_reader.h"
#include "views/dict_attack.h"

#include <nfc/scenes/nfc_scene.h>
#include "helpers/nfc_detected_protocols.h"
#include "helpers/nfc_custom_event.h"
#include "helpers/mf_ultralight_auth.h"
#include "helpers/mf_user_dict.h"
#include "helpers/mfkey32_logger.h"
#include "helpers/mf_classic_key_cache.h"
#include "helpers/nfc_supported_cards.h"
#include "helpers/felica_auth.h"
#include "helpers/slix_unlock.h"

#include <dialogs/dialogs.h>
#include <storage/storage.h>
#include <toolbox/path.h>

#include "rpc/rpc_app.h"

#include <m-array.h>

#include <lib/nfc/nfc.h>
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a.h>
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_listener.h>
#include <lib/nfc/protocols/mf_ultralight/mf_ultralight_listener.h>

#include <nfc/nfc_poller.h>
#include <nfc/nfc_scanner.h>
#include <nfc/nfc_listener.h>

#include <nfc/nfc_device.h>
#include <nfc/helpers/nfc_data_generator.h>
#include <toolbox/keys_dict.h>

#include <gui/modules/validators.h>
#include <toolbox/path.h>
#include <toolbox/name_generator.h>
#include <dolphin/dolphin.h>

#define NFC_NAME_SIZE             22
#define NFC_TEXT_STORE_SIZE       128
#define NFC_BYTE_INPUT_STORE_SIZE 10
#define NFC_LOG_SIZE_MAX          (1024)
#define NFC_APP_FOLDER            EXT_PATH("nfc")
#define NFC_APP_EXTENSION         ".nfc"
#define NFC_APP_SHADOW_EXTENSION  ".shd"
#define NFC_APP_FILENAME_PREFIX   "NFC"

#define NFC_APP_MFKEY32_LOGS_FILE_NAME ".mfkey32.log"
#define NFC_APP_MFKEY32_LOGS_FILE_PATH (NFC_APP_FOLDER "/" NFC_APP_MFKEY32_LOGS_FILE_NAME)

#define NFC_APP_MF_CLASSIC_DICT_USER_PATH (NFC_APP_FOLDER "/assets/mf_classic_dict_user.nfc")
#define NFC_APP_MF_CLASSIC_DICT_USER_NESTED_PATH \
    (NFC_APP_FOLDER "/assets/mf_classic_dict_user_nested.nfc")
#define NFC_APP_MF_CLASSIC_DICT_SYSTEM_PATH (NFC_APP_FOLDER "/assets/mf_classic_dict.nfc")
#define NFC_APP_MF_CLASSIC_DICT_SYSTEM_NESTED_PATH \
    (NFC_APP_FOLDER "/assets/mf_classic_dict_nested.nfc")

typedef enum {
    NfcRpcStateIdle,
    NfcRpcStateEmulating,
} NfcRpcState;

typedef struct {
    KeysDict* dict;
    uint8_t sectors_total;
    uint8_t sectors_read;
    uint8_t current_sector;
    uint8_t keys_found;
    size_t dict_keys_total;
    size_t dict_keys_current;
    bool is_key_attack;
    uint8_t key_attack_current_sector;
    bool is_card_present;
    MfClassicNestedPhase nested_phase;
    MfClassicPrngType prng_type;
    MfClassicBackdoor backdoor;
    uint16_t nested_target_key;
    uint16_t msb_count;
    bool enhanced_dict;
} NfcMfClassicDictAttackContext;

struct NfcApp {
    DialogsApp* dialogs;
    Storage* storage;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    NotificationApp* notifications;
    SceneManager* scene_manager;

    char text_store[NFC_TEXT_STORE_SIZE + 1];
    FuriString* text_box_store;
    uint8_t byte_input_store[NFC_BYTE_INPUT_STORE_SIZE];

    NfcDetectedProtocols* detected_protocols;

    RpcAppSystem* rpc_ctx;
    NfcRpcState rpc_state;

    // Common Views
    Submenu* submenu;
    DialogEx* dialog_ex;
    Popup* popup;
    Loading* loading;
    TextInput* text_input;
    ByteInput* byte_input;
    TextBox* text_box;
    Widget* widget;
    DetectReader* detect_reader;
    DictAttack* dict_attack;

    Nfc* nfc;
    NfcPoller* poller;
    NfcScanner* scanner;
    NfcListener* listener;

    FelicaAuthenticationContext* felica_auth;
    MfUltralightAuth* mf_ul_auth;
    SlixUnlock* slix_unlock;
    NfcMfClassicDictAttackContext nfc_dict_context;
    Mfkey32Logger* mfkey32_logger;
    MfUserDict* mf_user_dict;
    MfClassicKeyCache* mfc_key_cache;
    NfcSupportedCards* nfc_supported_cards;

    NfcDevice* nfc_device;
    Iso14443_3aData* iso14443_3a_edit_data;
    FuriString* file_path;
    FuriString* file_name;
    FuriTimer* timer;
};

typedef enum {
    NfcViewMenu,
    NfcViewDialogEx,
    NfcViewPopup,
    NfcViewLoading,
    NfcViewTextInput,
    NfcViewByteInput,
    NfcViewTextBox,
    NfcViewWidget,
    NfcViewDictAttack,
    NfcViewDetectReader,
} NfcView;

int32_t nfc_task(void* p);

void nfc_text_store_set(NfcApp* nfc, const char* text, ...);

void nfc_text_store_clear(NfcApp* nfc);

void nfc_blink_read_start(NfcApp* nfc);

void nfc_blink_emulate_start(NfcApp* nfc);

void nfc_blink_detect_start(NfcApp* nfc);

void nfc_blink_stop(NfcApp* nfc);

void nfc_show_loading_popup(void* context, bool show);

bool nfc_has_shadow_file(NfcApp* instance);

bool nfc_save_shadow_file(NfcApp* instance);

bool nfc_delete_shadow_file(NfcApp* instance);

bool nfc_save(NfcApp* instance);

bool nfc_delete(NfcApp* instance);

bool nfc_load_from_file_select(NfcApp* instance);

bool nfc_load_file(NfcApp* instance, FuriString* path, bool show_dialog);

bool nfc_save_file(NfcApp* instance, FuriString* path);

void nfc_make_app_folder(NfcApp* instance);

void nfc_append_filename_string_when_present(NfcApp* instance, FuriString* string);
