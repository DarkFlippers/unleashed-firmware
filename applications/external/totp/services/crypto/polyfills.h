#pragma once

#include <furi_hal_crypto.h>

#ifndef FURI_HAL_CRYPTO_ENCLAVE_USER_KEY_SLOT_START

// FW Crypto API is outdated, let's polyfill it
#define FURI_HAL_CRYPTO_ENCLAVE_USER_KEY_SLOT_START (12u)
#define FURI_HAL_CRYPTO_ENCLAVE_USER_KEY_SLOT_END (100u)
#define furi_hal_crypto_enclave_ensure_key furi_hal_crypto_verify_key
#define furi_hal_crypto_enclave_load_key furi_hal_crypto_store_load_key
#define furi_hal_crypto_enclave_unload_key furi_hal_crypto_store_unload_key

#endif