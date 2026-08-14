/**
 * @file nfc_key_dict.h
 * @brief Description of a user key dictionary, shared by the key management scenes.
 *
 * MIFARE Classic, Plus, Ultralight C and Ultralight AES all offer the same
 * add/list/delete flow over their own dictionary file. The scenes are generic;
 * this is the only thing that differs between them.
 */
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
