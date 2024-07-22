#pragma once
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Pack uint32 to varint
 * @param value value from UINT32_MIN to UINT32_MAX
 * @param output output array, need to be at least 5 bytes long
 * @return size_t 
 */
size_t varint_uint32_pack(uint32_t value, uint8_t* output);

size_t varint_uint32_unpack(uint32_t* value, const uint8_t* input, size_t input_size);

size_t varint_uint32_length(uint32_t value);

/**
 * Pack int32 to varint
 * @param value value from (INT32_MIN / 2 + 1) to INT32_MAX
 * @param output output array, need to be at least 5 bytes long
 * @return size_t 
 */
size_t varint_int32_pack(int32_t value, uint8_t* output);

size_t varint_int32_unpack(int32_t* value, const uint8_t* input, size_t input_size);

size_t varint_int32_length(int32_t value);

#ifdef __cplusplus
}
#endif
