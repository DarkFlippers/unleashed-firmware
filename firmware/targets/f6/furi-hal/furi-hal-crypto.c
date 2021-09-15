#include <furi-hal-crypto.h>
#include <furi.h>
#include <shci.h>

CRYP_HandleTypeDef crypt;

void furi_hal_crypto_init() {
    FURI_LOG_I("FuriHalCrypto", "Init OK");
}

bool furi_hal_crypto_store_add_key(FuriHalCryptoKey* key, uint8_t* slot) {
    furi_assert(key);
    furi_assert(slot);

    SHCI_C2_FUS_StoreUsrKey_Cmd_Param_t pParam;

    if (key->type == FuriHalCryptoKeyTypeMaster) {
        pParam.KeyType = KEYTYPE_MASTER;
    } else if (key->type == FuriHalCryptoKeyTypeSimple) {
        pParam.KeyType = KEYTYPE_SIMPLE;
    } else if (key->type == FuriHalCryptoKeyTypeEncrypted) {
        pParam.KeyType = KEYTYPE_ENCRYPTED;
    } else {
        furi_crash("Incorrect key type");
    }

    if (key->size == FuriHalCryptoKeySize128) {
        pParam.KeySize = KEYSIZE_16;
    } else if (key->size == FuriHalCryptoKeySize256) {
        pParam.KeySize = KEYSIZE_32;
    } else {
        furi_crash("Incorrect key size");
    }

    return SHCI_C2_FUS_StoreUsrKey(&pParam, slot) == SHCI_Success;
}

bool furi_hal_crypto_store_load_key(uint8_t slot, const uint8_t* iv) {
    furi_assert(slot > 0 && slot <= 100);

    crypt.Instance = AES1;
    crypt.Init.DataType = CRYP_DATATYPE_32B;
    crypt.Init.KeySize = CRYP_KEYSIZE_256B;
    crypt.Init.Algorithm = CRYP_AES_CBC;
    crypt.Init.pInitVect = (uint32_t*)iv;
    crypt.Init.pKey = NULL;

    furi_check(HAL_CRYP_Init(&crypt) == HAL_OK);

    if (SHCI_C2_FUS_LoadUsrKey(slot) == SHCI_Success) {
        return true;
    } else {
        furi_check(HAL_CRYP_DeInit(&crypt) == HAL_OK);
        return false;
    }
}

bool furi_hal_crypto_store_unload_key(uint8_t slot) {
    furi_check(HAL_CRYP_DeInit(&crypt) == HAL_OK);
    return SHCI_C2_FUS_UnloadUsrKey(slot) == SHCI_Success;
}

bool furi_hal_crypto_encrypt(const uint8_t *input, uint8_t *output, size_t size) {
    return HAL_CRYP_Encrypt(&crypt, (uint32_t*)input, size/4, (uint32_t*)output, 1000) == HAL_OK;
}

bool furi_hal_crypto_decrypt(const uint8_t *input, uint8_t *output, size_t size) {
    return HAL_CRYP_Decrypt(&crypt, (uint32_t*)input, size/4, (uint32_t*)output, 1000) == HAL_OK;
}
