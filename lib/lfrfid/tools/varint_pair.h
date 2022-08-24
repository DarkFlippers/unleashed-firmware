#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VarintPair VarintPair;

/**
 * @brief Allocate a new VarintPair instance
 * 
 * VarintPair is a buffer that holds pair of varint values
 * @return VarintPair* 
 */
VarintPair* varint_pair_alloc();

/**
 * @brief Free a VarintPair instance
 * 
 * @param pair 
 */
void varint_pair_free(VarintPair* pair);

/**
 * @brief Write varint pair to buffer
 * 
 * @param pair 
 * @param first 
 * @param value 
 * @return bool pair complete and needs to be written
 */
bool varint_pair_pack(VarintPair* pair, bool first, uint32_t value);

/**
 * @brief Get pointer to varint pair buffer
 * 
 * @param pair 
 * @return uint8_t* 
 */
uint8_t* varint_pair_get_data(VarintPair* pair);

/**
 * @brief Get size of varint pair buffer
 * 
 * @param pair 
 * @return size_t
 */
size_t varint_pair_get_size(VarintPair* pair);

/**
 * @brief Reset varint pair buffer
 * 
 * @param pair 
 */
void varint_pair_reset(VarintPair* pair);

/**
 * @brief Unpack varint pair to uint32_t pair from buffer
 * 
 * @param data 
 * @param data_length 
 * @param value_1 
 * @param value_2 
 * @param length 
 * @return bool 
 */
bool varint_pair_unpack(
    uint8_t* data,
    size_t data_length,
    uint32_t* value_1,
    uint32_t* value_2,
    size_t* length);

#ifdef __cplusplus
}
#endif