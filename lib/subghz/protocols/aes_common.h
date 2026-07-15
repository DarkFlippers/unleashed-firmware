#pragma once

#include "base.h"

#include <furi.h>

void reverse_bits_in_bytes(uint8_t* data, uint8_t len);
void aes128_decrypt(const uint8_t* expanded_key, uint8_t* data);
void aes128_encrypt(const uint8_t* expanded_key, uint8_t* data);
void aes_key_expansion(const uint8_t* key, uint8_t* round_keys);
