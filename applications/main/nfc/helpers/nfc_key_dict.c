#include "nfc_key_dict.h"

#include "../nfc_app_i.h"

#include <nfc/protocols/mf_classic/mf_classic.h>
#include <nfc/protocols/mf_plus/mf_plus.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight.h>

static const NfcKeyDict nfc_key_dicts[NfcKeyDictTypeNum] = {
    [NfcKeyDictTypeMfClassic] =
        {
            .title = "MIFARE Classic Keys",
            .system_path = NFC_APP_MF_CLASSIC_DICT_SYSTEM_PATH,
            .user_path = NFC_APP_MF_CLASSIC_DICT_USER_PATH,
            .key_size = sizeof(MfClassicKey),
        },
    [NfcKeyDictTypeMfPlus] =
        {
            .title = "MIFARE Plus Keys",
            .system_path = NFC_APP_MF_PLUS_DICT_SYSTEM_PATH,
            .user_path = NFC_APP_MF_PLUS_DICT_USER_PATH,
            .key_size = sizeof(MfPlusKey),
        },
    [NfcKeyDictTypeMfUltralightC] =
        {
            .title = "MIFARE Ultralight C Keys",
            .system_path = NFC_APP_MF_ULTRALIGHT_C_DICT_SYSTEM_PATH,
            .user_path = NFC_APP_MF_ULTRALIGHT_C_DICT_USER_PATH,
            .key_size = sizeof(MfUltralightC3DesAuthKey),
        },
    [NfcKeyDictTypeMfUltralightAes] =
        {
            .title = "MIFARE UL AES Keys",
            .system_path = NFC_APP_MF_ULTRALIGHT_AES_DICT_SYSTEM_PATH,
            .user_path = NFC_APP_MF_ULTRALIGHT_AES_DICT_USER_PATH,
            .key_size = sizeof(MfUltralightAesKey),
        },
};

const NfcKeyDict* nfc_key_dict(NfcKeyDictType type) {
    furi_check(type > NfcKeyDictTypeNone && type < NfcKeyDictTypeNum);

    const NfcKeyDict* dict = &nfc_key_dicts[type];
    // Checked per row rather than per key type, so an enumerator added without a row (zero
    // size) or with a key longer than the byte input store cannot reach the scenes, which
    // use key_size as a write length into uint8_t[NFC_BYTE_INPUT_STORE_SIZE] buffers.
    furi_check(dict->key_size > 0 && dict->key_size <= NFC_BYTE_INPUT_STORE_SIZE);

    return dict;
}
