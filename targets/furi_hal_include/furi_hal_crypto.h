/**
 * @file furi_hal_crypto.h
 *
 * Cryptography HAL API
 *
 * !!! READ THIS FIRST !!!
 *
 * Flipper was never designed to be secure, nor it passed cryptography audit.
 * Despite of the fact that keys are stored in secure enclave there are some
 * types of attack that can be performed against AES engine to recover
 * keys(theoretical). Also there is no way to securely deliver user keys to
 * device and never will be. In addition device is fully open and there is no
 * way to guarantee safety of your data, it can be easily dumped with debugger
 * or modified code.
 *
 * Secure enclave on WB series is implemented on core2 FUS side and can be used
 * only if core2 alive. Enclave is responsible for storing, loading and
 * unloading keys to and from enclave/AES in secure manner(AES engine key
 * registers will be locked when key from enclave loaded)
 *
 * There are 11 keys that we provision at factory:
 * - 0 - Master key for secure key delivery. Impossible to use for anything but
 *   key provisioning. We don't plan to use it too.
 * - 1 - 10 - Keys used by firmware. All devices got the same set of keys. You
 *   also can use them in your applications.
 *
 * Also there is a slot 11 that we use for device unique key. This slot is
 * intentionally left blank till the moment of first use, so you can ensure that
 * we don't know your unique key. Also you can provision this key by your self
 * with crypto cli or API.
 *
 * Other slots can be used for your needs. But since enclave is sequential
 * append only, we can not guarantee you that slots you want are free. NEVER USE
 * THEM FOR PUBLIC APPLICATIONS.
 *
 * Also you can directly load raw keys into AES engine and use it for your
 * needs.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Factory provisioned master key slot. Should never be used. */
#define FURI_HAL_CRYPTO_ENCLAVE_MASTER_KEY_SLOT (0u)

/** Factory provisioned keys slot range. All of them are exactly same on all flippers. */
#define FURI_HAL_CRYPTO_ENCLAVE_FACTORY_KEY_SLOT_START (1u)
#define FURI_HAL_CRYPTO_ENCLAVE_FACTORY_KEY_SLOT_END (10u)

/** Device unique key slot. This key generated on first use or provisioned by user. Use furi_hal_crypto_enclave_ensure_key before using this slot. */
#define FURI_HAL_CRYPTO_ENCLAVE_UNIQUE_KEY_SLOT (11u)

/** User key slot range. This slots can be used for your needs, but never use them in public apps. */
#define FURI_HAL_CRYPTO_ENCLAVE_USER_KEY_SLOT_START (12u)
#define FURI_HAL_CRYPTO_ENCLAVE_USER_KEY_SLOT_END (100u)

/** [Deprecated] Indicates availability of advanced crypto functions, will be dropped before v1.0 */
#define FURI_HAL_CRYPTO_ADVANCED_AVAIL 1

/** FuriHalCryptoKey Type */
typedef enum {
    FuriHalCryptoKeyTypeMaster, /**< Master key */
    FuriHalCryptoKeyTypeSimple, /**< Simple unencrypted key */
    FuriHalCryptoKeyTypeEncrypted, /**< Encrypted with Master key */
} FuriHalCryptoKeyType;

/** FuriHalCryptoKey Size in bits */
typedef enum {
    FuriHalCryptoKeySize128,
    FuriHalCryptoKeySize256,
} FuriHalCryptoKeySize;

/** FuriHalCryptoKey */
typedef struct {
    FuriHalCryptoKeyType type;
    FuriHalCryptoKeySize size;
    uint8_t* data;
} FuriHalCryptoKey;

/** FuriHalCryptoGCMState Result of a GCM operation */
typedef enum {
    FuriHalCryptoGCMStateOk, /**< operation successful */
    FuriHalCryptoGCMStateError, /**< error during encryption/decryption */
    FuriHalCryptoGCMStateAuthFailure, /**< tags do not match, auth failed */
} FuriHalCryptoGCMState;

/** Initialize cryptography layer(includes AES engines, PKA and RNG) */
void furi_hal_crypto_init();

/** Verify factory provisioned keys
 *
 * @param      keys_nb        The keys number of
 * @param      valid_keys_nb  The valid keys number of
 *
 * @return     true if all enclave keys are intact, false otherwise
 */
bool furi_hal_crypto_enclave_verify(uint8_t* keys_nb, uint8_t* valid_keys_nb);

/** Ensure that requested slot and slots before this slot contains keys.
 *
 * This function is used to provision FURI_HAL_CRYPTO_ENCLAVE_UNIQUE_KEY_SLOT. Also you
 * may want to use it to generate some unique keys in user key slot range.
 *
 * @warning    Because of the sequential nature of the secure enclave this
 *             method will generate key for all slots from
 *             FURI_HAL_CRYPTO_ENCLAVE_FACTORY_KEY_SLOT_END to the slot your requested.
 *             Keys are generated using on-chip RNG.
 *
 * @param[in]  key_slot  The key slot to enclave
 *
 * @return     true if key exists or created, false if enclave corrupted
 */
bool furi_hal_crypto_enclave_ensure_key(uint8_t key_slot);

/** Store key in crypto enclave
 *
 * @param      key   FuriHalCryptoKey to be stored
 * @param      slot  pointer to int where enclave slot will be stored
 *
 * @return     true on success
 */
bool furi_hal_crypto_enclave_store_key(FuriHalCryptoKey* key, uint8_t* slot);

/** Init AES engine and load key from crypto enclave
 *
 * @warning    Use only with furi_hal_crypto_enclave_unload_key()
 *
 * @param      slot  enclave slot
 * @param[in]  iv    pointer to 16 bytes Initialization Vector data
 *
 * @return     true on success
 */
bool furi_hal_crypto_enclave_load_key(uint8_t slot, const uint8_t* iv);

/** Unload key and deinit AES engine
 *
 * @warning    Use only with furi_hal_crypto_enclave_load_key()
 *
 * @param      slot  enclave slot
 *
 * @return     true on success
 */
bool furi_hal_crypto_enclave_unload_key(uint8_t slot);

/** Init AES engine and load supplied key
 *
 * @warning    Use only with furi_hal_crypto_unload_key()
 *
 * @param[in]  key   pointer to 32 bytes key data
 * @param[in]  iv    pointer to 16 bytes Initialization Vector data
 *
 * @return     true on success
 */
bool furi_hal_crypto_load_key(const uint8_t* key, const uint8_t* iv);

/** Unload key and de-init AES engine
 *
 * @warning    Use this function only with furi_hal_crypto_load_key()
 *
 * @return     true on success
 */
bool furi_hal_crypto_unload_key(void);

/** Encrypt data
 *
 * @param      input   pointer to input data
 * @param      output  pointer to output data
 * @param      size    input/output buffer size in bytes
 *
 * @return     true on success
 */
bool furi_hal_crypto_encrypt(const uint8_t* input, uint8_t* output, size_t size);

/** Decrypt data
 *
 * @param      input   pointer to input data
 * @param      output  pointer to output data
 * @param      size    input/output buffer size in bytes
 *
 * @return     true on success
 */
bool furi_hal_crypto_decrypt(const uint8_t* input, uint8_t* output, size_t size);

/** Encrypt the input using AES-CTR
 *
 * Decryption can be performed by supplying the ciphertext as input. Inits and
 * deinits the AES engine internally.
 *
 * @param[in]  key     pointer to 32 bytes key data
 * @param[in]  iv      pointer to 12 bytes Initialization Vector data
 * @param[in]  input   pointer to input data
 * @param[out] output  pointer to output data
 * @param      length  length of the input and output in bytes
 *
 * @return     true on success
 */
bool furi_hal_crypto_ctr(
    const uint8_t* key,
    const uint8_t* iv,
    const uint8_t* input,
    uint8_t* output,
    size_t length);

/** Encrypt/decrypt the input using AES-GCM
 *
 * When decrypting the tag generated needs to be compared to the tag attached to
 * the ciphertext in a constant-time fashion. If the tags are not equal, the
 * decryption failed and the plaintext returned needs to be discarded. Inits and
 * deinits the AES engine internally.
 *
 * @param[in]  key         pointer to 32 bytes key data
 * @param[in]  iv          pointer to 12 bytes Initialization Vector data
 * @param[in]  aad         pointer to additional authentication data
 * @param      aad_length  length of the additional authentication data in bytes
 * @param[in]  input       pointer to input data
 * @param[out] output      pointer to output data
 * @param      length      length of the input and output in bytes
 * @param[out] tag         pointer to 16 bytes space for the tag
 * @param      decrypt     true for decryption, false otherwise
 *
 * @return     true on success
 */
bool furi_hal_crypto_gcm(
    const uint8_t* key,
    const uint8_t* iv,
    const uint8_t* aad,
    size_t aad_length,
    const uint8_t* input,
    uint8_t* output,
    size_t length,
    uint8_t* tag,
    bool decrypt);

/** Encrypt the input using AES-GCM and generate a tag
 *
 * Inits and deinits the AES engine internally.
 *
 * @param[in]  key         pointer to 32 bytes key data
 * @param[in]  iv          pointer to 12 bytes Initialization Vector data
 * @param[in]  aad         pointer to additional authentication data
 * @param      aad_length  length of the additional authentication data in bytes
 * @param[in]  input       pointer to input data
 * @param[out] output      pointer to output data
 * @param      length      length of the input and output in bytes
 * @param[out] tag         pointer to 16 bytes space for the tag
 *
 * @return     FuriHalCryptoGCMStateOk on success, FuriHalCryptoGCMStateError on
 *             failure
 */
FuriHalCryptoGCMState furi_hal_crypto_gcm_encrypt_and_tag(
    const uint8_t* key,
    const uint8_t* iv,
    const uint8_t* aad,
    size_t aad_length,
    const uint8_t* input,
    uint8_t* output,
    size_t length,
    uint8_t* tag);

/** Decrypt the input using AES-GCM and verify the provided tag
 *
 * Inits and deinits the AES engine internally.
 *
 * @param[in]  key         pointer to 32 bytes key data
 * @param[in]  iv          pointer to 12 bytes Initialization Vector data
 * @param[in]  aad         pointer to additional authentication data
 * @param      aad_length  length of the additional authentication data in bytes
 * @param[in]  input       pointer to input data
 * @param[out] output      pointer to output data
 * @param      length      length of the input and output in bytes
 * @param[out] tag         pointer to 16 bytes tag
 *
 * @return     FuriHalCryptoGCMStateOk on success, FuriHalCryptoGCMStateError on
 *             failure, FuriHalCryptoGCMStateAuthFailure if the tag does not
 *             match
 */
FuriHalCryptoGCMState furi_hal_crypto_gcm_decrypt_and_verify(
    const uint8_t* key,
    const uint8_t* iv,
    const uint8_t* aad,
    size_t aad_length,
    const uint8_t* input,
    uint8_t* output,
    size_t length,
    const uint8_t* tag);

#ifdef __cplusplus
}
#endif
