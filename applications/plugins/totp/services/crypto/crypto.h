#pragma once

#include "../../types/plugin_state.h"

uint8_t* totp_crypto_encrypt(
    const uint8_t* plain_data,
    const uint8_t plain_data_length,
    const uint8_t* iv,
    uint8_t* encrypted_data_length);
uint8_t* totp_crypto_decrypt(
    const uint8_t* encrypted_data,
    const uint8_t encrypted_data_length,
    const uint8_t* iv,
    uint8_t* decrypted_data_length);
void totp_crypto_seed_iv(PluginState* plugin_state, uint8_t* pin, uint8_t pin_length);
bool totp_crypto_verify_key(const PluginState* plugin_state);