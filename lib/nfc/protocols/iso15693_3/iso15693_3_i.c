#include "iso15693_3_i.h"

bool iso15693_3_error_response_parse(Iso15693_3Error* error, const BitBuffer* buf) {
    furi_assert(error);

    if(bit_buffer_get_size_bytes(buf) == 0) {
        // YEET!
        *error = Iso15693_3ErrorBufferEmpty;
        return true;
    }

    typedef struct {
        uint8_t flags;
        uint8_t error;
    } ErrorResponseLayout;

    const ErrorResponseLayout* resp = (const ErrorResponseLayout*)bit_buffer_get_data(buf);

    if((resp->flags & ISO15693_3_RESP_FLAG_ERROR) == 0) {
        // No error flag is set, the data does not contain an error frame
        return false;
    } else if(bit_buffer_get_size_bytes(buf) < sizeof(ErrorResponseLayout)) {
        // Error bit is set, but not enough data to determine the error
        *error = Iso15693_3ErrorUnexpectedResponse;
        return true;
    } else if(
        resp->error >= ISO15693_3_RESP_ERROR_CUSTOM_START &&
        resp->error <= ISO15693_3_RESP_ERROR_CUSTOM_END) {
        // Custom vendor-specific error, must be checked in the respective protocol implementation
        *error = Iso15693_3ErrorCustom;
        return true;
    }

    switch(resp->error) {
    case ISO15693_3_RESP_ERROR_NOT_SUPPORTED:
    case ISO15693_3_RESP_ERROR_OPTION:
        *error = Iso15693_3ErrorNotSupported;
        break;
    case ISO15693_3_RESP_ERROR_FORMAT:
        *error = Iso15693_3ErrorFormat;
        break;
    case ISO15693_3_RESP_ERROR_BLOCK_UNAVAILABLE:
    case ISO15693_3_RESP_ERROR_BLOCK_ALREADY_LOCKED:
    case ISO15693_3_RESP_ERROR_BLOCK_LOCKED:
    case ISO15693_3_RESP_ERROR_BLOCK_WRITE:
    case ISO15693_3_RESP_ERROR_BLOCK_LOCK:
        *error = Iso15693_3ErrorInternal;
        break;
    case ISO15693_3_RESP_ERROR_UNKNOWN:
    default:
        *error = Iso15693_3ErrorUnknown;
    }

    return true;
}

Iso15693_3Error iso15693_3_inventory_response_parse(uint8_t* data, const BitBuffer* buf) {
    furi_assert(data);

    Iso15693_3Error ret = Iso15693_3ErrorNone;

    do {
        if(iso15693_3_error_response_parse(&ret, buf)) break;

        typedef struct {
            uint8_t flags;
            uint8_t dsfid;
            uint8_t uid[ISO15693_3_UID_SIZE];
        } InventoryResponseLayout;

        if(bit_buffer_get_size_bytes(buf) != sizeof(InventoryResponseLayout)) {
            ret = Iso15693_3ErrorUnexpectedResponse;
            break;
        }

        const InventoryResponseLayout* resp =
            (const InventoryResponseLayout*)bit_buffer_get_data(buf);
        // Reverse UID for backward compatibility
        for(uint32_t i = 0; i < ISO15693_3_UID_SIZE; ++i) {
            data[i] = resp->uid[ISO15693_3_UID_SIZE - i - 1];
        }

    } while(false);

    return ret;
}

Iso15693_3Error
    iso15693_3_system_info_response_parse(Iso15693_3SystemInfo* data, const BitBuffer* buf) {
    furi_assert(data);

    Iso15693_3Error ret = Iso15693_3ErrorNone;

    do {
        if(iso15693_3_error_response_parse(&ret, buf)) break;

        typedef struct {
            uint8_t flags;
            uint8_t info_flags;
            uint8_t uid[ISO15693_3_UID_SIZE];
            uint8_t extra[];
        } SystemInfoResponseLayout;

        if(bit_buffer_get_size_bytes(buf) < sizeof(SystemInfoResponseLayout)) {
            ret = Iso15693_3ErrorUnexpectedResponse;
            break;
        }

        const SystemInfoResponseLayout* resp =
            (const SystemInfoResponseLayout*)bit_buffer_get_data(buf);

        const uint8_t* extra = resp->extra;
        const size_t extra_size = (resp->info_flags & ISO15693_3_SYSINFO_FLAG_DSFID ? 1 : 0) +
                                  (resp->info_flags & ISO15693_3_SYSINFO_FLAG_AFI ? 1 : 0) +
                                  (resp->info_flags & ISO15693_3_SYSINFO_FLAG_MEMORY ? 2 : 0) +
                                  (resp->info_flags & ISO15693_3_SYSINFO_FLAG_IC_REF ? 1 : 0);

        if(extra_size != bit_buffer_get_size_bytes(buf) - sizeof(SystemInfoResponseLayout)) {
            ret = Iso15693_3ErrorUnexpectedResponse;
            break;
        }

        data->flags = resp->info_flags;

        if(data->flags & ISO15693_3_SYSINFO_FLAG_DSFID) {
            data->dsfid = *extra++;
        }

        if(data->flags & ISO15693_3_SYSINFO_FLAG_AFI) {
            data->afi = *extra++;
        }

        if(data->flags & ISO15693_3_SYSINFO_FLAG_MEMORY) {
            // Add 1 to get actual values
            data->block_count = *extra++ + 1;
            data->block_size = (*extra++ & 0x1F) + 1;
        }

        if(data->flags & ISO15693_3_SYSINFO_FLAG_IC_REF) {
            data->ic_ref = *extra;
        }

    } while(false);

    return ret;
}

Iso15693_3Error
    iso15693_3_read_block_response_parse(uint8_t* data, uint8_t block_size, const BitBuffer* buf) {
    furi_assert(data);

    Iso15693_3Error ret = Iso15693_3ErrorNone;

    do {
        if(iso15693_3_error_response_parse(&ret, buf)) break;

        typedef struct {
            uint8_t flags;
            uint8_t block_data[];
        } ReadBlockResponseLayout;

        const size_t buf_size = bit_buffer_get_size_bytes(buf);
        const size_t received_block_size = buf_size - sizeof(ReadBlockResponseLayout);

        if(buf_size <= sizeof(ReadBlockResponseLayout) || received_block_size != block_size) {
            ret = Iso15693_3ErrorUnexpectedResponse;
            break;
        }

        const ReadBlockResponseLayout* resp =
            (const ReadBlockResponseLayout*)bit_buffer_get_data(buf);
        memcpy(data, resp->block_data, received_block_size);

    } while(false);

    return ret;
}

Iso15693_3Error iso15693_3_get_block_security_response_parse(
    uint8_t* data,
    uint16_t block_count,
    const BitBuffer* buf) {
    furi_assert(data);
    furi_assert(block_count);
    Iso15693_3Error ret = Iso15693_3ErrorNone;

    do {
        if(iso15693_3_error_response_parse(&ret, buf)) break;

        typedef struct {
            uint8_t flags;
            uint8_t block_security[];
        } GetBlockSecurityResponseLayout;

        const size_t buf_size = bit_buffer_get_size_bytes(buf);
        const size_t received_block_count = buf_size - sizeof(GetBlockSecurityResponseLayout);

        if(buf_size <= sizeof(GetBlockSecurityResponseLayout) ||
           received_block_count != block_count) {
            ret = Iso15693_3ErrorUnexpectedResponse;
            break;
        }

        const GetBlockSecurityResponseLayout* resp =
            (const GetBlockSecurityResponseLayout*)bit_buffer_get_data(buf);

        memcpy(data, resp->block_security, received_block_count);

    } while(false);

    return ret;
}

void iso15693_3_append_uid(const Iso15693_3Data* data, BitBuffer* buf) {
    for(size_t i = 0; i < ISO15693_3_UID_SIZE; ++i) {
        // Reverse the UID
        bit_buffer_append_byte(buf, data->uid[ISO15693_3_UID_SIZE - i - 1]);
    }
}

void iso15693_3_append_block(const Iso15693_3Data* data, uint8_t block_num, BitBuffer* buf) {
    furi_assert(block_num < data->system_info.block_count);

    const uint32_t block_offset = block_num * data->system_info.block_size;
    const uint8_t* block_data = simple_array_cget(data->block_data, block_offset);

    bit_buffer_append_bytes(buf, block_data, data->system_info.block_size);
}

void iso15693_3_set_block_locked(Iso15693_3Data* data, uint8_t block_index, bool locked) {
    furi_assert(data);
    furi_assert(block_index < data->system_info.block_count);

    *(uint8_t*)simple_array_get(data->block_security, block_index) = locked ? 1 : 0;
}

void iso15693_3_set_block_data(
    Iso15693_3Data* data,
    uint8_t block_num,
    const uint8_t* block_data,
    size_t block_data_size) {
    furi_assert(block_num < data->system_info.block_count);
    furi_assert(block_data_size == data->system_info.block_size);

    const uint32_t block_offset = block_num * data->system_info.block_size;
    uint8_t* block = simple_array_get(data->block_data, block_offset);

    memcpy(block, block_data, block_data_size);
}

void iso15693_3_append_block_security(
    const Iso15693_3Data* data,
    uint8_t block_num,
    BitBuffer* buf) {
    bit_buffer_append_byte(buf, *(uint8_t*)simple_array_cget(data->block_security, block_num));
}

bool iso15693_3_is_equal_uid(const Iso15693_3Data* data, const uint8_t* uid) {
    for(size_t i = 0; i < ISO15693_3_UID_SIZE; ++i) {
        if(data->uid[i] != uid[ISO15693_3_UID_SIZE - i - 1]) return false;
    }
    return true;
}
