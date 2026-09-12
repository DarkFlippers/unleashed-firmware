#include "nfc_key_dict.h"

#include "../nfc_app_i.h"

#include <furi.h>
#include <toolbox/keys_dict.h>

#include <nfc/protocols/mf_classic/mf_classic.h>
#include <nfc/protocols/mf_plus/mf_plus.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight.h>

#define TAG "NfcKeyDict"

_Static_assert(
    NFC_KEY_DICT_DEVICE_KEYS_MAX >= MF_CLASSIC_TOTAL_SECTORS_MAX * 2,
    "A 4K card can hand over two keys per sector");

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

/** Append `key` unless the buffer already holds it. Returns the new count. */
static size_t nfc_key_dict_push_unique(
    uint8_t* keys,
    size_t key_count,
    size_t keys_max,
    size_t key_size,
    const uint8_t* key) {
    if(key_count == keys_max) return key_count;

    for(size_t i = 0; i < key_count; i++) {
        if(memcmp(keys + i * key_size, key, key_size) == 0) return key_count;
    }

    memcpy(keys + key_count * key_size, key, key_size);
    return key_count + 1;
}

static size_t nfc_key_dict_collect_mf_classic(
    const NfcDevice* device,
    uint8_t* keys,
    size_t keys_max,
    size_t key_size) {
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    const uint8_t sector_num = mf_classic_get_total_sectors_num(data->type);

    size_t key_count = 0;
    for(uint8_t sector = 0; sector < sector_num; sector++) {
        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, sector);
        // Gated on the found masks, not on the bytes: an unrecovered slot is zero fill, while a
        // key that genuinely is all zeroes is a real key and has its mask bit set.
        if(FURI_BIT(data->key_a_mask, sector)) {
            key_count =
                nfc_key_dict_push_unique(keys, key_count, keys_max, key_size, sec_tr->key_a.data);
        }
        if(FURI_BIT(data->key_b_mask, sector)) {
            key_count =
                nfc_key_dict_push_unique(keys, key_count, keys_max, key_size, sec_tr->key_b.data);
        }
    }

    return key_count;
}

size_t nfc_key_dict_collect_from_device(
    NfcKeyDictType type,
    const NfcDevice* device,
    uint8_t* keys,
    size_t keys_max) {
    furi_check(device);
    furi_check(keys);
    furi_check(keys_max > 0 && keys_max <= NFC_KEY_DICT_DEVICE_KEYS_MAX);

    const NfcKeyDict* dict = nfc_key_dict(type);

    size_t key_count = 0;
    if(type == NfcKeyDictTypeMfClassic) {
        // The loaded device and the dictionary type are chosen independently, so check them
        // against each other here rather than leaving it to nfc_device_get_data(), which crashes
        // several frames deeper with nothing pointing back at the mismatched pair.
        furi_check(nfc_device_get_protocol(device) == NfcProtocolMfClassic);
        key_count = nfc_key_dict_collect_mf_classic(device, keys, keys_max, dict->key_size);
    } else {
        // Only the Classic dictionary has a card-side key store to read back. The other three
        // are fed by hand, so there is nothing to collect until a protocol grows one.
        FURI_LOG_W(TAG, "No key collector for %s", dict->title);
    }

    return key_count;
}

/**
 * Walk `dict` once, marking every candidate it already holds. Comparing the parsed bytes rather
 * than keys_dict_is_key_present()'s formatted strings also means a hand-edited lowercase-hex
 * line still counts as present.
 */
static void nfc_key_dict_mark_present(
    KeysDict* dict,
    const uint8_t* keys,
    size_t key_count,
    size_t key_size,
    bool* known,
    size_t* known_count) {
    // A missing or empty dictionary reports no keys either way, and there is nothing to
    // compare against - the missing one also leaves keys_dict_alloc()'s stream closed.
    if(keys_dict_get_total_keys(dict) == 0) return;

    uint8_t* dict_key = malloc(key_size);

    while(keys_dict_get_next_key(dict, dict_key, key_size)) {
        for(size_t i = 0; i < key_count; i++) {
            if(known[i]) continue;
            if(memcmp(keys + i * key_size, dict_key, key_size) != 0) continue;

            known[i] = true;
            (*known_count)++;
            break; // Candidates are deduplicated, so one match settles this dictionary line.
        }
    }

    free(dict_key);
}

void nfc_key_dict_import(
    NfcKeyDictType type,
    const uint8_t* keys,
    size_t key_count,
    NfcKeyDictImportStats* stats) {
    furi_check(keys);
    furi_check(key_count <= NFC_KEY_DICT_DEVICE_KEYS_MAX);
    furi_check(stats);

    const NfcKeyDict* dict = nfc_key_dict(type);
    const size_t key_size = dict->key_size;
    memset(stats, 0, sizeof(NfcKeyDictImportStats));
    stats->candidates = key_count;
    if(key_count == 0) return;

    bool known[NFC_KEY_DICT_DEVICE_KEYS_MAX] = {};

    // Filtering against the system dictionary is what stops a default-keyed card from doubling
    // the user dictionary, so a missing one is worth a word: it is not fatal, but it means a
    // broken install, and every factory key on the card is about to look new.
    if(!keys_dict_check_presence(dict->system_path)) {
        FURI_LOG_W(TAG, "System dictionary %s is missing", dict->system_path);
    }
    KeysDict* system_dict = keys_dict_alloc(dict->system_path, KeysDictModeOpenExisting, key_size);
    nfc_key_dict_mark_present(system_dict, keys, key_count, key_size, known, &stats->known);
    keys_dict_free(system_dict);

    // One handle for both the read and the append, so the file is not walked a third time.
    KeysDict* user_dict = keys_dict_alloc(dict->user_path, KeysDictModeOpenAlways, key_size);
    nfc_key_dict_mark_present(user_dict, keys, key_count, key_size, known, &stats->known);

    for(size_t i = 0; i < key_count; i++) {
        if(known[i]) continue;

        if(!keys_dict_add_key(user_dict, keys + i * key_size, key_size)) {
            // Stop at the first failure: the cause (full SD, read-only card) applies to the
            // rest as well, and the count already written stays accurate.
            FURI_LOG_E(TAG, "Failed to add a key to %s", dict->user_path);
            stats->write_failed = true;
            break;
        }
        stats->added++;
    }

    keys_dict_free(user_dict);
}
