#pragma once

#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Decodes Base-64 encoded bytes into plain bytes.
 * @param src Base-64 encoded bytes
 * @param len Base-64 encoded bytes count
 * @param[out] out_len decoded buffer length
 * @param[out] out_size decoded buffer allocated size
 * @return Decoded result buffer if successfully decoded; \c NULL otherwise
 */
uint8_t* base64_decode(const uint8_t* src, size_t len, size_t* out_len, size_t* out_size);