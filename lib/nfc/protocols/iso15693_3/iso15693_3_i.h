#pragma once

#include "iso15693_3.h"

#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if the buffer contains an error frame and if it does, determine
 * the error type.
 * NOTE: No changes are done to the result if no error is present.
 *
 * @param [out] data Pointer to the resulting error value.
 * @param [in] buf Data buffer to be checked
 *
 * @return True if data contains an error frame or is empty, false otherwise
 */
bool iso15693_3_error_response_parse(Iso15693_3Error* error, const BitBuffer* buf);

Iso15693_3Error iso15693_3_inventory_response_parse(uint8_t* data, const BitBuffer* buf);

Iso15693_3Error
    iso15693_3_system_info_response_parse(Iso15693_3SystemInfo* data, const BitBuffer* buf);

Iso15693_3Error
    iso15693_3_read_block_response_parse(uint8_t* data, uint8_t block_size, const BitBuffer* buf);

Iso15693_3Error iso15693_3_get_block_security_response_parse(
    uint8_t* data,
    uint16_t block_count,
    const BitBuffer* buf);

void iso15693_3_append_uid(const Iso15693_3Data* data, BitBuffer* buf);

void iso15693_3_append_block(const Iso15693_3Data* data, uint8_t block_num, BitBuffer* buf);

void iso15693_3_set_block_locked(Iso15693_3Data* data, uint8_t block_index, bool locked);

void iso15693_3_set_block_data(
    Iso15693_3Data* data,
    uint8_t block_num,
    const uint8_t* block_data,
    size_t block_data_size);

void iso15693_3_append_block_security(
    const Iso15693_3Data* data,
    uint8_t block_num,
    BitBuffer* buf);

// NOTE: the uid parameter has reversed byte order with respect to data
bool iso15693_3_is_equal_uid(const Iso15693_3Data* data, const uint8_t* uid);

#ifdef __cplusplus
}
#endif
