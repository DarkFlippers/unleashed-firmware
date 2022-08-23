#include <furi_hal_crypto.h>
#include <furi_hal_bt.h>
#include <furi_hal_random.h>
#include <stm32wbxx_ll_cortex.h>
#include <stm32wbxx_ll_bus.h>
#include <furi.h>
#include <interface/patterns/ble_thread/shci/shci.h>

#define TAG "FuriHalCrypto"

#define ENCLAVE_FACTORY_KEY_SLOTS 10
#define ENCLAVE_SIGNATURE_SIZE 16

#define CRYPTO_BLK_LEN (4 * sizeof(uint32_t))
#define CRYPTO_TIMEOUT (1000)

#define CRYPTO_MODE_ENCRYPT 0U
#define CRYPTO_MODE_INIT (AES_CR_MODE_0)
#define CRYPTO_MODE_DECRYPT (AES_CR_MODE_1)
#define CRYPTO_MODE_DECRYPT_INIT (AES_CR_MODE_0 | AES_CR_MODE_1)

#define CRYPTO_DATATYPE_32B 0U
#define CRYPTO_KEYSIZE_256B (AES_CR_KEYSIZE)
#define CRYPTO_AES_CBC (AES_CR_CHMOD_0)

static FuriMutex* furi_hal_crypto_mutex = NULL;
static bool furi_hal_crypto_mode_init_done = false;

static const uint8_t enclave_signature_iv[ENCLAVE_FACTORY_KEY_SLOTS][16] = {
    {0xac, 0x5d, 0x68, 0xb8, 0x79, 0x74, 0xfc, 0x7f, 0x45, 0x02, 0x82, 0xf1, 0x48, 0x7e, 0x75, 0x8a},
    {0x38, 0xe6, 0x6a, 0x90, 0x5e, 0x5b, 0x8a, 0xa6, 0x70, 0x30, 0x04, 0x72, 0xc2, 0x42, 0xea, 0xaf},
    {0x73, 0xd5, 0x8e, 0xfb, 0x0f, 0x4b, 0xa9, 0x79, 0x0f, 0xde, 0x0e, 0x53, 0x44, 0x7d, 0xaa, 0xfd},
    {0x3c, 0x9a, 0xf4, 0x43, 0x2b, 0xfe, 0xea, 0xae, 0x8c, 0xc6, 0xd1, 0x60, 0xd2, 0x96, 0x64, 0xa9},
    {0x10, 0xac, 0x7b, 0x63, 0x03, 0x7f, 0x43, 0x18, 0xec, 0x9d, 0x9c, 0xc4, 0x01, 0xdc, 0x35, 0xa7},
    {0x26, 0x21, 0x64, 0xe6, 0xd0, 0xf2, 0x47, 0x49, 0xdc, 0x36, 0xcd, 0x68, 0x0c, 0x91, 0x03, 0x44},
    {0x7a, 0xbd, 0xce, 0x9c, 0x24, 0x7a, 0x2a, 0xb1, 0x3c, 0x4f, 0x5a, 0x7d, 0x80, 0x3e, 0xfc, 0x0d},
    {0xcd, 0xdd, 0xd3, 0x02, 0x85, 0x65, 0x43, 0x83, 0xf9, 0xac, 0x75, 0x2f, 0x21, 0xef, 0x28, 0x6b},
    {0xab, 0x73, 0x70, 0xe8, 0xe2, 0x56, 0x0f, 0x58, 0xab, 0x29, 0xa5, 0xb1, 0x13, 0x47, 0x5e, 0xe8},
    {0x4f, 0x3c, 0x43, 0x77, 0xde, 0xed, 0x79, 0xa1, 0x8d, 0x4c, 0x1f, 0xfd, 0xdb, 0x96, 0x87, 0x2e},
};

static const uint8_t enclave_signature_input[ENCLAVE_FACTORY_KEY_SLOTS][ENCLAVE_SIGNATURE_SIZE] = {
    {0x9f, 0x5c, 0xb1, 0x43, 0x17, 0x53, 0x18, 0x8c, 0x66, 0x3d, 0x39, 0x45, 0x90, 0x13, 0xa9, 0xde},
    {0xc5, 0x98, 0xe9, 0x17, 0xb8, 0x97, 0x9e, 0x03, 0x33, 0x14, 0x13, 0x8f, 0xce, 0x74, 0x0d, 0x54},
    {0x34, 0xba, 0x99, 0x59, 0x9f, 0x70, 0x67, 0xe9, 0x09, 0xee, 0x64, 0x0e, 0xb3, 0xba, 0xfb, 0x75},
    {0xdc, 0xfa, 0x6c, 0x9a, 0x6f, 0x0a, 0x3e, 0xdc, 0x42, 0xf6, 0xae, 0x0d, 0x3c, 0xf7, 0x83, 0xaf},
    {0xea, 0x2d, 0xe3, 0x1f, 0x02, 0x99, 0x1a, 0x7e, 0x6d, 0x93, 0x4c, 0xb5, 0x42, 0xf0, 0x7a, 0x9b},
    {0x53, 0x5e, 0x04, 0xa2, 0x49, 0xa0, 0x73, 0x49, 0x56, 0xb0, 0x88, 0x8c, 0x12, 0xa0, 0xe4, 0x18},
    {0x7d, 0xa7, 0xc5, 0x21, 0x7f, 0x12, 0x95, 0xdd, 0x4d, 0x77, 0x01, 0xfa, 0x71, 0x88, 0x2b, 0x7f},
    {0xdc, 0x9b, 0xc5, 0xa7, 0x6b, 0x84, 0x5c, 0x37, 0x7c, 0xec, 0x05, 0xa1, 0x9f, 0x91, 0x17, 0x3b},
    {0xea, 0xcf, 0xd9, 0x9b, 0x86, 0xcd, 0x2b, 0x43, 0x54, 0x45, 0x82, 0xc6, 0xfe, 0x73, 0x1a, 0x1a},
    {0x77, 0xb8, 0x1b, 0x90, 0xb4, 0xb7, 0x32, 0x76, 0x8f, 0x8a, 0x57, 0x06, 0xc7, 0xdd, 0x08, 0x90},
};

static const uint8_t enclave_signature_expected[ENCLAVE_FACTORY_KEY_SLOTS][ENCLAVE_SIGNATURE_SIZE] = {
    {0xe9, 0x9a, 0xce, 0xe9, 0x4d, 0xe1, 0x7f, 0x55, 0xcb, 0x8a, 0xbf, 0xf2, 0x4d, 0x98, 0x27, 0x67},
    {0x34, 0x27, 0xa7, 0xea, 0xa8, 0x98, 0x66, 0x9b, 0xed, 0x43, 0xd3, 0x93, 0xb5, 0xa2, 0x87, 0x8e},
    {0x6c, 0xf3, 0x01, 0x78, 0x53, 0x1b, 0x11, 0x32, 0xf0, 0x27, 0x2f, 0xe3, 0x7d, 0xa6, 0xe2, 0xfd},
    {0xdf, 0x7f, 0x37, 0x65, 0x2f, 0xdb, 0x7c, 0xcf, 0x5b, 0xb6, 0xe4, 0x9c, 0x63, 0xc5, 0x0f, 0xe0},
    {0x9b, 0x5c, 0xee, 0x44, 0x0e, 0xd1, 0xcb, 0x5f, 0x28, 0x9f, 0x12, 0x17, 0x59, 0x64, 0x40, 0xbb},
    {0x94, 0xc2, 0x09, 0x98, 0x62, 0xa7, 0x2b, 0x93, 0xed, 0x36, 0x1f, 0x10, 0xbc, 0x26, 0xbd, 0x41},
    {0x4d, 0xb2, 0x2b, 0xc5, 0x96, 0x47, 0x61, 0xf4, 0x16, 0xe0, 0x81, 0xc3, 0x8e, 0xb9, 0x9c, 0x9b},
    {0xc3, 0x6b, 0x83, 0x55, 0x90, 0x38, 0x0f, 0xea, 0xd1, 0x65, 0xbf, 0x32, 0x4f, 0x8e, 0x62, 0x5b},
    {0x8d, 0x5e, 0x27, 0xbc, 0x14, 0x4f, 0x08, 0xa8, 0x2b, 0x14, 0x89, 0x5e, 0xdf, 0x77, 0x04, 0x31},
    {0xc9, 0xf7, 0x03, 0xf1, 0x6c, 0x65, 0xad, 0x49, 0x74, 0xbe, 0x00, 0x54, 0xfd, 0xa6, 0x9c, 0x32},
};

void furi_hal_crypto_init() {
    furi_hal_crypto_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    FURI_LOG_I(TAG, "Init OK");
}

static bool furi_hal_crypto_generate_unique_keys(uint8_t start_slot, uint8_t end_slot) {
    FuriHalCryptoKey key;
    uint8_t key_data[32];
    FURI_LOG_I(TAG, "Generating keys %u..%u", start_slot, end_slot);
    for(uint8_t slot = start_slot; slot <= end_slot; slot++) {
        key.type = FuriHalCryptoKeyTypeSimple;
        key.size = FuriHalCryptoKeySize256;
        key.data = key_data;
        furi_hal_random_fill_buf(key_data, 32);
        if(!furi_hal_crypto_store_add_key(&key, &slot)) {
            FURI_LOG_E(TAG, "Error writing key to slot %u", slot);
            return false;
        }
    }
    return true;
}

bool furi_hal_crypto_verify_key(uint8_t key_slot) {
    uint8_t keys_nb = 0;
    uint8_t valid_keys_nb = 0;
    uint8_t last_valid_slot = ENCLAVE_FACTORY_KEY_SLOTS;
    uint8_t empty_iv[16];
    furi_hal_crypto_verify_enclave(&keys_nb, &valid_keys_nb);
    if(key_slot <= ENCLAVE_FACTORY_KEY_SLOTS) { // It's a factory key
        if(key_slot > keys_nb) return false;
    } else { // Unique key
        if(keys_nb < ENCLAVE_FACTORY_KEY_SLOTS) // Some factory keys are missing
            return false;
        for(uint8_t i = key_slot; i > ENCLAVE_FACTORY_KEY_SLOTS; i--) {
            if(furi_hal_crypto_store_load_key(i, empty_iv)) {
                last_valid_slot = i;
                furi_hal_crypto_store_unload_key(i);
                break;
            }
        }
        if(last_valid_slot == key_slot)
            return true;
        else // Generate missing unique keys
            return furi_hal_crypto_generate_unique_keys(last_valid_slot + 1, key_slot);
    }
    return true;
}

bool furi_hal_crypto_verify_enclave(uint8_t* keys_nb, uint8_t* valid_keys_nb) {
    furi_assert(keys_nb);
    furi_assert(valid_keys_nb);
    uint8_t keys = 0;
    uint8_t keys_valid = 0;
    uint8_t buffer[ENCLAVE_SIGNATURE_SIZE];
    for(size_t key_slot = 0; key_slot < ENCLAVE_FACTORY_KEY_SLOTS; key_slot++) {
        if(furi_hal_crypto_store_load_key(key_slot + 1, enclave_signature_iv[key_slot])) {
            keys++;
            if(furi_hal_crypto_encrypt(
                   enclave_signature_input[key_slot], buffer, ENCLAVE_SIGNATURE_SIZE)) {
                keys_valid +=
                    memcmp(buffer, enclave_signature_expected[key_slot], ENCLAVE_SIGNATURE_SIZE) ==
                    0;
            }
            furi_hal_crypto_store_unload_key(key_slot + 1);
        }
    }
    *keys_nb = keys;
    *valid_keys_nb = keys_valid;
    if(*valid_keys_nb == ENCLAVE_FACTORY_KEY_SLOTS)
        return true;
    else
        return false;
}

bool furi_hal_crypto_store_add_key(FuriHalCryptoKey* key, uint8_t* slot) {
    furi_assert(key);
    furi_assert(slot);

    furi_check(furi_mutex_acquire(furi_hal_crypto_mutex, FuriWaitForever) == FuriStatusOk);

    if(!furi_hal_bt_is_alive()) {
        return false;
    }

    SHCI_C2_FUS_StoreUsrKey_Cmd_Param_t pParam;
    size_t key_data_size = 0;

    if(key->type == FuriHalCryptoKeyTypeMaster) {
        pParam.KeyType = KEYTYPE_MASTER;
    } else if(key->type == FuriHalCryptoKeyTypeSimple) {
        pParam.KeyType = KEYTYPE_SIMPLE;
    } else if(key->type == FuriHalCryptoKeyTypeEncrypted) {
        pParam.KeyType = KEYTYPE_ENCRYPTED;
        key_data_size += 12;
    } else {
        furi_crash("Incorrect key type");
    }

    if(key->size == FuriHalCryptoKeySize128) {
        pParam.KeySize = KEYSIZE_16;
        key_data_size += 16;
    } else if(key->size == FuriHalCryptoKeySize256) {
        pParam.KeySize = KEYSIZE_32;
        key_data_size += 32;
    } else {
        furi_crash("Incorrect key size");
    }

    memcpy(pParam.KeyData, key->data, key_data_size);

    SHCI_CmdStatus_t shci_state = SHCI_C2_FUS_StoreUsrKey(&pParam, slot);
    furi_check(furi_mutex_release(furi_hal_crypto_mutex) == FuriStatusOk);
    return (shci_state == SHCI_Success);
}

static void crypto_key_init(uint32_t* key, uint32_t* iv) {
    CLEAR_BIT(AES1->CR, AES_CR_EN);
    MODIFY_REG(
        AES1->CR,
        AES_CR_DATATYPE | AES_CR_KEYSIZE | AES_CR_CHMOD,
        CRYPTO_DATATYPE_32B | CRYPTO_KEYSIZE_256B | CRYPTO_AES_CBC);

    if(key != NULL) {
        AES1->KEYR7 = key[0];
        AES1->KEYR6 = key[1];
        AES1->KEYR5 = key[2];
        AES1->KEYR4 = key[3];
        AES1->KEYR3 = key[4];
        AES1->KEYR2 = key[5];
        AES1->KEYR1 = key[6];
        AES1->KEYR0 = key[7];
    }

    AES1->IVR3 = iv[0];
    AES1->IVR2 = iv[1];
    AES1->IVR1 = iv[2];
    AES1->IVR0 = iv[3];
}

static bool crypto_process_block(uint32_t* in, uint32_t* out, uint8_t blk_len) {
    furi_check((blk_len <= 4) && (blk_len > 0));

    for(uint8_t i = 0; i < 4; i++) {
        if(i < blk_len) {
            AES1->DINR = in[i];
        } else {
            AES1->DINR = 0;
        }
    }

    uint32_t countdown = CRYPTO_TIMEOUT;
    while(!READ_BIT(AES1->SR, AES_SR_CCF)) {
        if(LL_SYSTICK_IsActiveCounterFlag()) {
            countdown--;
        }
        if(countdown == 0) {
            return false;
        }
    }

    SET_BIT(AES1->CR, AES_CR_CCFC);

    uint32_t out_temp[4];
    for(uint8_t i = 0; i < 4; i++) {
        out_temp[i] = AES1->DOUTR;
    }

    memcpy(out, out_temp, blk_len * sizeof(uint32_t));
    return true;
}

bool furi_hal_crypto_store_load_key(uint8_t slot, const uint8_t* iv) {
    furi_assert(slot > 0 && slot <= 100);
    furi_assert(furi_hal_crypto_mutex);
    furi_check(furi_mutex_acquire(furi_hal_crypto_mutex, FuriWaitForever) == FuriStatusOk);

    if(!furi_hal_bt_is_alive()) {
        return false;
    }

    furi_hal_crypto_mode_init_done = false;
    crypto_key_init(NULL, (uint32_t*)iv);

    if(SHCI_C2_FUS_LoadUsrKey(slot) == SHCI_Success) {
        return true;
    } else {
        CLEAR_BIT(AES1->CR, AES_CR_EN);
        furi_check(furi_mutex_release(furi_hal_crypto_mutex) == FuriStatusOk);
        return false;
    }
}

bool furi_hal_crypto_store_unload_key(uint8_t slot) {
    if(!furi_hal_bt_is_alive()) {
        return false;
    }

    CLEAR_BIT(AES1->CR, AES_CR_EN);

    SHCI_CmdStatus_t shci_state = SHCI_C2_FUS_UnloadUsrKey(slot);
    furi_assert(shci_state == SHCI_Success);

    FURI_CRITICAL_ENTER();
    LL_AHB2_GRP1_ForceReset(LL_AHB2_GRP1_PERIPH_AES1);
    LL_AHB2_GRP1_ReleaseReset(LL_AHB2_GRP1_PERIPH_AES1);
    FURI_CRITICAL_EXIT();

    furi_check(furi_mutex_release(furi_hal_crypto_mutex) == FuriStatusOk);
    return (shci_state == SHCI_Success);
}

bool furi_hal_crypto_encrypt(const uint8_t* input, uint8_t* output, size_t size) {
    bool state = false;

    SET_BIT(AES1->CR, AES_CR_EN);

    MODIFY_REG(AES1->CR, AES_CR_MODE, CRYPTO_MODE_ENCRYPT);

    for(size_t i = 0; i < size; i += CRYPTO_BLK_LEN) {
        size_t blk_len = size - i;
        if(blk_len > CRYPTO_BLK_LEN) {
            blk_len = CRYPTO_BLK_LEN;
        }
        state = crypto_process_block((uint32_t*)&input[i], (uint32_t*)&output[i], blk_len / 4);
        if(state == false) {
            break;
        }
    }

    CLEAR_BIT(AES1->CR, AES_CR_EN);

    return state;
}

bool furi_hal_crypto_decrypt(const uint8_t* input, uint8_t* output, size_t size) {
    bool state = false;

    if(!furi_hal_crypto_mode_init_done) {
        MODIFY_REG(AES1->CR, AES_CR_MODE, CRYPTO_MODE_INIT);

        SET_BIT(AES1->CR, AES_CR_EN);

        uint32_t countdown = CRYPTO_TIMEOUT;
        while(!READ_BIT(AES1->SR, AES_SR_CCF)) {
            if(LL_SYSTICK_IsActiveCounterFlag()) {
                countdown--;
            }
            if(countdown == 0) {
                return false;
            }
        }

        SET_BIT(AES1->CR, AES_CR_CCFC);

        furi_hal_crypto_mode_init_done = true;
    }

    MODIFY_REG(AES1->CR, AES_CR_MODE, CRYPTO_MODE_DECRYPT);
    SET_BIT(AES1->CR, AES_CR_EN);

    for(size_t i = 0; i < size; i += CRYPTO_BLK_LEN) {
        size_t blk_len = size - i;
        if(blk_len > CRYPTO_BLK_LEN) {
            blk_len = CRYPTO_BLK_LEN;
        }
        state = crypto_process_block((uint32_t*)&input[i], (uint32_t*)&output[i], blk_len / 4);
        if(state == false) {
            break;
        }
    }

    CLEAR_BIT(AES1->CR, AES_CR_EN);

    return state;
}
