/**
 * @file nfc_key_dict.h
 * @brief Description of a user key dictionary, shared by the key management scenes.
 *
 * MIFARE Classic, Plus, Ultralight C and Ultralight AES all offer the same
 * add/list/delete flow over their own dictionary file. The scenes are generic;
 * this is the only thing that differs between them.
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <nfc/nfc_device.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Upper bound on the keys one card can hand to the user dictionary.
 *
 * MIFARE Classic 4K: 40 sectors, key A and key B each. Checked against the protocol's own
 * maximum in nfc_key_dict.c so this cannot drift.
 */
#define NFC_KEY_DICT_DEVICE_KEYS_MAX (80)

typedef enum {
    /**
     * @brief Reserved: NfcApp is zero-filled, so an unset selector lands here.
     *
     * Without it a scene entered without setting the type would silently act on the MIFARE
     * Classic dictionary - including Delete.
     */
    NfcKeyDictTypeNone,

    NfcKeyDictTypeMfClassic,
    NfcKeyDictTypeMfPlus,
    NfcKeyDictTypeMfUltralightC,
    NfcKeyDictTypeMfUltralightAes,

    NfcKeyDictTypeNum,
} NfcKeyDictType;

typedef struct {
    const char* title; /**< Header shown on the dictionary screen. */
    const char* system_path; /**< Read-only dictionary shipped with the firmware. */
    const char* user_path; /**< User dictionary, the one add/delete operate on. */
    size_t key_size; /**< Key length in bytes, bounded by NFC_BYTE_INPUT_STORE_SIZE
                          (enforced in nfc_key_dict.c). */
} NfcKeyDict;

const NfcKeyDict* nfc_key_dict(NfcKeyDictType type);

/**
 * @brief Outcome of nfc_key_dict_import(), for the screen that reports it.
 *
 * added + known == candidates, unless write_failed - the append loop stops at the first
 * failure, leaving the rest counted nowhere.
 */
typedef struct {
    size_t candidates; /**< Keys offered, i.e. the key_count passed in. */
    size_t added; /**< Keys appended to the user dictionary. */
    size_t known; /**< Keys skipped: either dictionary already held them. */
    bool write_failed; /**< An append failed (full SD, missing folder, read-only card). */
} NfcKeyDictImportStats;

/**
 * @brief Collect the distinct keys a loaded device actually recovered.
 *
 * Only keys the device reports as found are collected - an unrecovered slot holds zero fill,
 * not a key. Duplicates are dropped here, because sectors commonly share a key and every
 * duplicate would otherwise cost a full dictionary comparison downstream.
 *
 * @param type      Dictionary the keys belong to; the device must hold that protocol (checked).
 * @param device    Loaded device to read the keys from.
 * @param keys      Output buffer of at least keys_max * nfc_key_dict(type)->key_size bytes.
 * @param keys_max  Capacity of the buffer in keys, at most NFC_KEY_DICT_DEVICE_KEYS_MAX.
 *
 * @return Number of keys written to the buffer.
 */
size_t nfc_key_dict_collect_from_device(
    NfcKeyDictType type,
    const NfcDevice* device,
    uint8_t* keys,
    size_t keys_max);

/**
 * @brief Append the keys that neither dictionary already holds to the user dictionary.
 *
 * One pass per dictionary rather than keys_dict_is_key_present() per key, which rescans the
 * whole file every time. Both files are read in full, so callers should put up the loading
 * view first.
 *
 * @param type       Dictionary to import into.
 * @param keys       Keys to import, packed at nfc_key_dict(type)->key_size bytes each and
 *                   free of duplicates - a repeated key would be appended twice.
 * @param key_count  Number of keys, at most NFC_KEY_DICT_DEVICE_KEYS_MAX.
 * @param stats      Filled in with what happened; never NULL.
 */
void nfc_key_dict_import(
    NfcKeyDictType type,
    const uint8_t* keys,
    size_t key_count,
    NfcKeyDictImportStats* stats);

#ifdef __cplusplus
}
#endif
