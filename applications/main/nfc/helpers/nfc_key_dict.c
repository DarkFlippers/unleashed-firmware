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

// The key scenes read into uint8_t[NFC_BYTE_INPUT_STORE_SIZE] stack buffers using key_size as the
// length, and byte input writes into a store of the same size. Adding a longer key below - a
// 24-byte 3K3DES, say - must break the build here rather than smash a stack at runtime.
static_assert(sizeof(MfClassicKey) <= NFC_BYTE_INPUT_STORE_SIZE, "MfClassicKey too long");
static_assert(sizeof(MfPlusKey) <= NFC_BYTE_INPUT_STORE_SIZE, "MfPlusKey too long");
static_assert(
    sizeof(MfUltralightC3DesAuthKey) <= NFC_BYTE_INPUT_STORE_SIZE,
    "MfUltralightC3DesAuthKey too long");
static_assert(
    sizeof(MfUltralightAesKey) <= NFC_BYTE_INPUT_STORE_SIZE,
    "MfUltralightAesKey too long");

const NfcKeyDict* nfc_key_dict(NfcKeyDictType type) {
    furi_check(type > NfcKeyDictTypeNone && type < NfcKeyDictTypeNum);

    return &nfc_key_dicts[type];
}
