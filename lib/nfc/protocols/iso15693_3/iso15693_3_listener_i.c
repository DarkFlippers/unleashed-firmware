#include "iso15693_3_listener_i.h"

#include <nfc/helpers/iso13239_crc.h>

#define TAG "Iso15693_3Listener"

typedef Iso15693_3Error (*Iso15693_3RequestHandler)(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags);

typedef struct {
    Iso15693_3RequestHandler mandatory[ISO15693_3_MANDATORY_COUNT];
    Iso15693_3RequestHandler optional[ISO15693_3_OPTIONAL_COUNT];
} Iso15693_3ListenerHandlerTable;

static Iso15693_3Error
    iso15693_3_listener_extension_handler(Iso15693_3Listener* instance, uint32_t command, ...) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        if(instance->extension_table == NULL) break;

        Iso15693_3ExtensionHandler handler = NULL;

        if(command < ISO15693_3_CMD_MANDATORY_RFU) {
            const Iso15693_3ExtensionHandler* mandatory = instance->extension_table->mandatory;
            handler = mandatory[command - ISO15693_3_CMD_MANDATORY_START];
        } else if(command >= ISO15693_3_CMD_OPTIONAL_START && command < ISO15693_3_CMD_OPTIONAL_RFU) {
            const Iso15693_3ExtensionHandler* optional = instance->extension_table->optional;
            handler = optional[command - ISO15693_3_CMD_OPTIONAL_START];
        }

        if(handler == NULL) break;

        va_list args;
        va_start(args, command);

        error = handler(instance->extension_context, args);

        va_end(args);

    } while(false);
    return error;
}

static Iso15693_3Error iso15693_3_listener_inventory_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        const bool afi_flag = flags & ISO15693_3_REQ_FLAG_T5_AFI_PRESENT;
        const size_t data_size_min = sizeof(uint8_t) * (afi_flag ? 2 : 1);

        if(data_size < data_size_min) {
            error = Iso15693_3ErrorFormat;
            break;
        }

        if(afi_flag) {
            const uint8_t afi = *data++;
            // When AFI flag is set, ignore non-matching requests
            if(afi != 0) {
                if(afi != instance->data->system_info.afi) break;
            }
        }

        const uint8_t mask_len = *data++;
        const size_t data_size_required = data_size_min + mask_len;

        if(data_size != data_size_required) {
            error = Iso15693_3ErrorFormat;
            break;
        }

        if(mask_len != 0) {
            // TODO FL-3633: Take mask_len and mask_value into account (if present)
        }

        error = iso15693_3_listener_extension_handler(instance, ISO15693_3_CMD_INVENTORY);
        if(error != Iso15693_3ErrorNone) break;

        bit_buffer_append_byte(instance->tx_buffer, instance->data->system_info.dsfid); // DSFID
        iso15693_3_append_uid(instance->data, instance->tx_buffer); // UID
    } while(false);

    return error;
}

static Iso15693_3Error iso15693_3_listener_stay_quiet_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(data);
    UNUSED(data_size);
    UNUSED(flags);

    instance->state = Iso15693_3ListenerStateQuiet;
    return Iso15693_3ErrorIgnore;
}

static Iso15693_3Error iso15693_3_listener_read_block_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        typedef struct {
            uint8_t block_num;
        } Iso15693_3ReadBlockRequestLayout;

        const Iso15693_3ReadBlockRequestLayout* request =
            (const Iso15693_3ReadBlockRequestLayout*)data;

        if(data_size != sizeof(Iso15693_3ReadBlockRequestLayout)) {
            error = Iso15693_3ErrorFormat;
            break;
        }

        const uint32_t block_index = request->block_num;
        const uint32_t block_count_max = instance->data->system_info.block_count;

        if(block_index >= block_count_max) {
            error = Iso15693_3ErrorInternal;
            break;
        }

        error = iso15693_3_listener_extension_handler(
            instance, ISO15693_3_CMD_READ_BLOCK, block_index);
        if(error != Iso15693_3ErrorNone) break;

        if(flags & ISO15693_3_REQ_FLAG_T4_OPTION) {
            iso15693_3_append_block_security(
                instance->data, block_index, instance->tx_buffer); // Block security (optional)
        }

        iso15693_3_append_block(instance->data, block_index, instance->tx_buffer); // Block data
    } while(false);

    return error;
}

static Iso15693_3Error iso15693_3_listener_write_block_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        typedef struct {
            uint8_t block_num;
            uint8_t block_data[];
        } Iso15693_3WriteBlockRequestLayout;

        const Iso15693_3WriteBlockRequestLayout* request =
            (const Iso15693_3WriteBlockRequestLayout*)data;

        instance->session_state.wait_for_eof = flags & ISO15693_3_REQ_FLAG_T4_OPTION;

        if(data_size <= sizeof(Iso15693_3WriteBlockRequestLayout)) {
            error = Iso15693_3ErrorFormat;
            break;
        }

        const uint32_t block_index = request->block_num;
        const uint32_t block_count_max = instance->data->system_info.block_count;
        const uint32_t block_size_max = instance->data->system_info.block_size;
        const size_t block_size_received = data_size - sizeof(Iso15693_3WriteBlockRequestLayout);

        if(block_index >= block_count_max) {
            error = Iso15693_3ErrorInternal;
            break;
        } else if(block_size_received != block_size_max) {
            error = Iso15693_3ErrorInternal;
            break;
        } else if(iso15693_3_is_block_locked(instance->data, block_index)) {
            error = Iso15693_3ErrorInternal;
            break;
        }

        error = iso15693_3_listener_extension_handler(
            instance, ISO15693_3_CMD_WRITE_BLOCK, block_index, request->block_data);
        if(error != Iso15693_3ErrorNone) break;

        iso15693_3_set_block_data(
            instance->data, block_index, request->block_data, block_size_received);
    } while(false);

    return error;
}

static Iso15693_3Error iso15693_3_listener_lock_block_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        typedef struct {
            uint8_t block_num;
        } Iso15693_3LockBlockRequestLayout;

        const Iso15693_3LockBlockRequestLayout* request =
            (const Iso15693_3LockBlockRequestLayout*)data;

        instance->session_state.wait_for_eof = flags & ISO15693_3_REQ_FLAG_T4_OPTION;

        if(data_size != sizeof(Iso15693_3LockBlockRequestLayout)) {
            error = Iso15693_3ErrorFormat;
            break;
        }

        const uint32_t block_index = request->block_num;
        const uint32_t block_count_max = instance->data->system_info.block_count;

        if(block_index >= block_count_max) {
            error = Iso15693_3ErrorInternal;
            break;
        } else if(iso15693_3_is_block_locked(instance->data, block_index)) {
            error = Iso15693_3ErrorInternal;
            break;
        }

        error = iso15693_3_listener_extension_handler(
            instance, ISO15693_3_CMD_LOCK_BLOCK, block_index);
        if(error != Iso15693_3ErrorNone) break;

        iso15693_3_set_block_locked(instance->data, block_index, true);
    } while(false);

    return error;
}

static Iso15693_3Error iso15693_3_listener_read_multi_blocks_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        typedef struct {
            uint8_t first_block_num;
            uint8_t block_count;
        } Iso15693_3ReadMultiBlocksRequestLayout;

        const Iso15693_3ReadMultiBlocksRequestLayout* request =
            (const Iso15693_3ReadMultiBlocksRequestLayout*)data;

        if(data_size != sizeof(Iso15693_3ReadMultiBlocksRequestLayout)) {
            error = Iso15693_3ErrorFormat;
            break;
        }

        const uint32_t block_index_start = request->first_block_num;
        const uint32_t block_index_end =
            MIN((block_index_start + request->block_count + 1),
                ((uint32_t)instance->data->system_info.block_count - 1));

        error = iso15693_3_listener_extension_handler(
            instance,
            ISO15693_3_CMD_READ_MULTI_BLOCKS,
            (uint32_t)block_index_start,
            (uint32_t)block_index_end);
        if(error != Iso15693_3ErrorNone) break;

        for(uint32_t i = block_index_start; i <= block_index_end; ++i) {
            if(flags & ISO15693_3_REQ_FLAG_T4_OPTION) {
                iso15693_3_append_block_security(
                    instance->data, i, instance->tx_buffer); // Block security (optional)
            }
            iso15693_3_append_block(instance->data, i, instance->tx_buffer); // Block data
        }
    } while(false);

    return error;
}

static Iso15693_3Error iso15693_3_listener_write_multi_blocks_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        typedef struct {
            uint8_t first_block_num;
            uint8_t block_count;
            uint8_t block_data[];
        } Iso15693_3WriteMultiBlocksRequestLayout;

        const Iso15693_3WriteMultiBlocksRequestLayout* request =
            (const Iso15693_3WriteMultiBlocksRequestLayout*)data;

        instance->session_state.wait_for_eof = flags & ISO15693_3_REQ_FLAG_T4_OPTION;

        if(data_size <= sizeof(Iso15693_3WriteMultiBlocksRequestLayout)) {
            error = Iso15693_3ErrorFormat;
            break;
        }

        const uint32_t block_index_start = request->first_block_num;
        const uint32_t block_index_end = block_index_start + request->block_count;

        const uint32_t block_count = request->block_count + 1;
        const uint32_t block_count_max = instance->data->system_info.block_count;
        const uint32_t block_count_available = block_count_max - block_index_start;

        const size_t block_data_size = data_size - sizeof(Iso15693_3WriteMultiBlocksRequestLayout);
        const size_t block_size = block_data_size / block_count;
        const size_t block_size_max = instance->data->system_info.block_size;

        if(block_count > block_count_available) {
            error = Iso15693_3ErrorInternal;
            break;
        } else if(block_size != block_size_max) {
            error = Iso15693_3ErrorInternal;
            break;
        }

        error = iso15693_3_listener_extension_handler(
            instance, ISO15693_3_CMD_WRITE_MULTI_BLOCKS, block_index_start, block_index_end);
        if(error != Iso15693_3ErrorNone) break;

        for(uint32_t i = block_index_start; i <= block_index_end; ++i) {
            if(iso15693_3_is_block_locked(instance->data, i)) {
                error = Iso15693_3ErrorInternal;
                break;
            }
        }

        if(error != Iso15693_3ErrorNone) break;

        for(uint32_t i = block_index_start; i < block_count + request->first_block_num; ++i) {
            const uint8_t* block_data = &request->block_data[block_size * i];
            iso15693_3_set_block_data(instance->data, i, block_data, block_size);
        }
    } while(false);

    return error;
}

static Iso15693_3Error iso15693_3_listener_select_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(data);
    UNUSED(data_size);

    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        if(!(flags & ISO15693_3_REQ_FLAG_T4_ADDRESSED)) {
            error = Iso15693_3ErrorFormat;
            break;
        }

        instance->state = Iso15693_3ListenerStateSelected;
    } while(false);

    return error;
}

static Iso15693_3Error iso15693_3_listener_reset_to_ready_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(data);
    UNUSED(data_size);
    UNUSED(flags);

    instance->state = Iso15693_3ListenerStateReady;
    return Iso15693_3ErrorNone;
}

static Iso15693_3Error iso15693_3_listener_write_afi_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        typedef struct {
            uint8_t afi;
        } Iso15693_3WriteAfiRequestLayout;

        const Iso15693_3WriteAfiRequestLayout* request =
            (const Iso15693_3WriteAfiRequestLayout*)data;

        instance->session_state.wait_for_eof = flags & ISO15693_3_REQ_FLAG_T4_OPTION;

        if(data_size <= sizeof(Iso15693_3WriteAfiRequestLayout)) {
            error = Iso15693_3ErrorFormat;
            break;
        } else if(instance->data->settings.lock_bits.afi) {
            error = Iso15693_3ErrorInternal;
            break;
        }

        error = iso15693_3_listener_extension_handler(instance, ISO15693_3_CMD_WRITE_AFI);
        if(error != Iso15693_3ErrorNone) break;

        instance->data->system_info.afi = request->afi;
    } while(false);

    return error;
}

static Iso15693_3Error iso15693_3_listener_lock_afi_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(data);
    UNUSED(data_size);

    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        instance->session_state.wait_for_eof = flags & ISO15693_3_REQ_FLAG_T4_OPTION;

        Iso15693_3LockBits* lock_bits = &instance->data->settings.lock_bits;

        if(lock_bits->afi) {
            error = Iso15693_3ErrorInternal;
            break;
        }

        error = iso15693_3_listener_extension_handler(instance, ISO15693_3_CMD_LOCK_AFI);
        if(error != Iso15693_3ErrorNone) break;

        lock_bits->afi = true;
    } while(false);

    return error;
}

static Iso15693_3Error iso15693_3_listener_write_dsfid_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        typedef struct {
            uint8_t dsfid;
        } Iso15693_3WriteDsfidRequestLayout;

        const Iso15693_3WriteDsfidRequestLayout* request =
            (const Iso15693_3WriteDsfidRequestLayout*)data;

        instance->session_state.wait_for_eof = flags & ISO15693_3_REQ_FLAG_T4_OPTION;

        if(data_size <= sizeof(Iso15693_3WriteDsfidRequestLayout)) {
            error = Iso15693_3ErrorFormat;
            break;
        } else if(instance->data->settings.lock_bits.dsfid) {
            error = Iso15693_3ErrorInternal;
            break;
        }

        error = iso15693_3_listener_extension_handler(instance, ISO15693_3_CMD_WRITE_DSFID);
        if(error != Iso15693_3ErrorNone) break;

        instance->data->system_info.dsfid = request->dsfid;
    } while(false);

    return error;
}

static Iso15693_3Error iso15693_3_listener_lock_dsfid_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(data);
    UNUSED(data_size);

    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        instance->session_state.wait_for_eof = flags & ISO15693_3_REQ_FLAG_T4_OPTION;

        Iso15693_3LockBits* lock_bits = &instance->data->settings.lock_bits;

        if(lock_bits->dsfid) {
            error = Iso15693_3ErrorInternal;
            break;
        }

        error = iso15693_3_listener_extension_handler(instance, ISO15693_3_CMD_LOCK_DSFID);
        if(error != Iso15693_3ErrorNone) break;

        lock_bits->dsfid = true;
    } while(false);

    return error;
}

static Iso15693_3Error iso15693_3_listener_get_system_info_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(data);
    UNUSED(data_size);
    UNUSED(flags);

    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        const uint8_t system_flags = instance->data->system_info.flags;
        bit_buffer_append_byte(instance->tx_buffer, system_flags); // System info flags

        iso15693_3_append_uid(instance->data, instance->tx_buffer); // UID

        if(system_flags & ISO15693_3_SYSINFO_FLAG_DSFID) {
            bit_buffer_append_byte(instance->tx_buffer, instance->data->system_info.dsfid);
        }
        if(system_flags & ISO15693_3_SYSINFO_FLAG_AFI) {
            bit_buffer_append_byte(instance->tx_buffer, instance->data->system_info.afi);
        }
        if(system_flags & ISO15693_3_SYSINFO_FLAG_MEMORY) {
            const uint8_t memory_info[2] = {
                instance->data->system_info.block_count - 1,
                instance->data->system_info.block_size - 1,
            };
            bit_buffer_append_bytes(instance->tx_buffer, memory_info, COUNT_OF(memory_info));
        }
        if(system_flags & ISO15693_3_SYSINFO_FLAG_IC_REF) {
            bit_buffer_append_byte(instance->tx_buffer, instance->data->system_info.ic_ref);
        }

    } while(false);

    return error;
}

static Iso15693_3Error iso15693_3_listener_get_multi_blocks_security_handler(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(flags);

    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        typedef struct {
            uint8_t first_block_num;
            uint8_t block_count;
        } Iso15693_3GetMultiBlocksSecurityRequestLayout;

        const Iso15693_3GetMultiBlocksSecurityRequestLayout* request =
            (const Iso15693_3GetMultiBlocksSecurityRequestLayout*)data;

        if(data_size < sizeof(Iso15693_3GetMultiBlocksSecurityRequestLayout)) {
            error = Iso15693_3ErrorFormat;
            break;
        }

        const uint32_t block_index_start = request->first_block_num;
        const uint32_t block_index_end = block_index_start + request->block_count;

        const uint32_t block_count_max = instance->data->system_info.block_count;

        if(block_index_end >= block_count_max) {
            error = Iso15693_3ErrorInternal;
            break;
        }

        for(uint32_t i = block_index_start; i <= block_index_end; ++i) {
            bit_buffer_append_byte(
                instance->tx_buffer, iso15693_3_is_block_locked(instance->data, i) ? 1 : 0);
        }
    } while(false);

    return error;
}

const Iso15693_3ListenerHandlerTable iso15693_3_handler_table = {
    .mandatory =
        {
            iso15693_3_listener_inventory_handler,
            iso15693_3_listener_stay_quiet_handler,
        },
    .optional =
        {
            iso15693_3_listener_read_block_handler,
            iso15693_3_listener_write_block_handler,
            iso15693_3_listener_lock_block_handler,
            iso15693_3_listener_read_multi_blocks_handler,
            iso15693_3_listener_write_multi_blocks_handler,
            iso15693_3_listener_select_handler,
            iso15693_3_listener_reset_to_ready_handler,
            iso15693_3_listener_write_afi_handler,
            iso15693_3_listener_lock_afi_handler,
            iso15693_3_listener_write_dsfid_handler,
            iso15693_3_listener_lock_dsfid_handler,
            iso15693_3_listener_get_system_info_handler,
            iso15693_3_listener_get_multi_blocks_security_handler,
        },
};

static Iso15693_3Error iso15693_3_listener_handle_standard_request(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t command,
    uint8_t flags) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        Iso15693_3RequestHandler handler = NULL;

        if(command < ISO15693_3_CMD_MANDATORY_RFU) {
            handler = iso15693_3_handler_table.mandatory[command - ISO15693_3_CMD_MANDATORY_START];
        } else if(command >= ISO15693_3_CMD_OPTIONAL_START && command < ISO15693_3_CMD_OPTIONAL_RFU) {
            handler = iso15693_3_handler_table.optional[command - ISO15693_3_CMD_OPTIONAL_START];
        }

        if(handler == NULL) {
            error = Iso15693_3ErrorNotSupported;
            break;
        }

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_append_byte(instance->tx_buffer, ISO15693_3_RESP_FLAG_NONE);

        error = handler(instance, data, data_size, flags);

        // The request was fully handled in the protocol extension, no further action necessary
        if(error == Iso15693_3ErrorFullyHandled) {
            error = Iso15693_3ErrorNone;
        }

        // Several commands may not require an answer
        if(error == Iso15693_3ErrorFormat || error == Iso15693_3ErrorIgnore) break;

        if(error != Iso15693_3ErrorNone) {
            bit_buffer_reset(instance->tx_buffer);
            bit_buffer_append_byte(instance->tx_buffer, ISO15693_3_RESP_FLAG_ERROR);
            bit_buffer_append_byte(instance->tx_buffer, ISO15693_3_RESP_ERROR_UNKNOWN);
        }

        Iso15693_3ListenerSessionState* session_state = &instance->session_state;

        if(!session_state->wait_for_eof) {
            error = iso15693_3_listener_send_frame(instance, instance->tx_buffer);
        }

    } while(false);

    return error;
}

static inline Iso15693_3Error iso15693_3_listener_handle_custom_request(
    Iso15693_3Listener* instance,
    const uint8_t* data,
    size_t data_size) {
    Iso15693_3Error error;

    do {
        typedef struct {
            uint8_t manufacturer;
            uint8_t extra[];
        } Iso15693_3CustomRequestLayout;

        if(data_size < sizeof(Iso15693_3CustomRequestLayout)) {
            error = Iso15693_3ErrorFormat;
            break;
        }

        const Iso15693_3CustomRequestLayout* request = (const Iso15693_3CustomRequestLayout*)data;

        if(request->manufacturer != iso15693_3_get_manufacturer_id(instance->data)) {
            error = Iso15693_3ErrorIgnore;
            break;
        }

        // This error code will trigger the CustomCommand listener event
        error = Iso15693_3ErrorNotSupported;
    } while(false);

    return error;
}

Iso15693_3Error iso15693_3_listener_set_extension_handler_table(
    Iso15693_3Listener* instance,
    const Iso15693_3ExtensionHandlerTable* table,
    void* context) {
    furi_assert(instance);
    furi_assert(context);

    instance->extension_table = table;
    instance->extension_context = context;
    return Iso15693_3ErrorNone;
}

Iso15693_3Error iso15693_3_listener_ready(Iso15693_3Listener* instance) {
    furi_assert(instance);
    instance->state = Iso15693_3ListenerStateReady;
    return Iso15693_3ErrorNone;
}

static Iso15693_3Error iso15693_3_listener_process_nfc_error(NfcError error) {
    Iso15693_3Error ret = Iso15693_3ErrorNone;

    if(error == NfcErrorNone) {
        ret = Iso15693_3ErrorNone;
    } else if(error == NfcErrorTimeout) {
        ret = Iso15693_3ErrorTimeout;
    } else {
        ret = Iso15693_3ErrorFieldOff;
    }

    return ret;
}

Iso15693_3Error
    iso15693_3_listener_send_frame(Iso15693_3Listener* instance, const BitBuffer* tx_buffer) {
    furi_assert(instance);
    furi_assert(tx_buffer);

    bit_buffer_copy(instance->tx_buffer, tx_buffer);
    iso13239_crc_append(Iso13239CrcTypeDefault, instance->tx_buffer);

    NfcError error = nfc_listener_tx(instance->nfc, instance->tx_buffer);
    return iso15693_3_listener_process_nfc_error(error);
}

Iso15693_3Error
    iso15693_3_listener_process_request(Iso15693_3Listener* instance, const BitBuffer* rx_buffer) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        typedef struct {
            uint8_t flags;
            uint8_t command;
            uint8_t data[];
        } Iso15693_3RequestLayout;

        const size_t buf_size = bit_buffer_get_size_bytes(rx_buffer);
        const size_t buf_size_min = sizeof(Iso15693_3RequestLayout);

        if(buf_size < buf_size_min) {
            error = Iso15693_3ErrorFormat;
            break;
        }

        const Iso15693_3RequestLayout* request =
            (const Iso15693_3RequestLayout*)bit_buffer_get_data(rx_buffer);

        Iso15693_3ListenerSessionState* session_state = &instance->session_state;

        if((request->flags & ISO15693_3_REQ_FLAG_INVENTORY_T5) == 0) {
            session_state->selected = request->flags & ISO15693_3_REQ_FLAG_T4_SELECTED;
            session_state->addressed = request->flags & ISO15693_3_REQ_FLAG_T4_ADDRESSED;

            if(session_state->selected && session_state->addressed) {
                // A request mode can be either addressed or selected, but not both
                error = Iso15693_3ErrorUnknown;
                break;
            } else if(instance->state == Iso15693_3ListenerStateQuiet) {
                // If the card is quiet, ignore non-addressed commands
                if(session_state->addressed) {
                    error = Iso15693_3ErrorIgnore;
                    break;
                }
            } else if(instance->state != Iso15693_3ListenerStateSelected) {
                // If the card is not selected, ignore selected commands
                if(session_state->selected) {
                    error = Iso15693_3ErrorIgnore;
                    break;
                }
            }
        } else {
            // If the card is quiet, ignore inventory commands
            if(instance->state == Iso15693_3ListenerStateQuiet) {
                error = Iso15693_3ErrorIgnore;
                break;
            }

            session_state->selected = false;
            session_state->addressed = false;
        }

        if(request->command >= ISO15693_3_CMD_CUSTOM_START) {
            // Custom commands are properly handled in the protocol-specific top-level poller
            error = iso15693_3_listener_handle_custom_request(
                instance, request->data, buf_size - buf_size_min);
            break;
        }

        const uint8_t* data;
        size_t data_size;

        if(session_state->addressed) {
            // In addressed mode, UID must be included in each command
            const size_t buf_size_min_addr = buf_size_min + ISO15693_3_UID_SIZE;

            if(buf_size < buf_size_min_addr) {
                error = Iso15693_3ErrorFormat;
                break;
            } else if(!iso15693_3_is_equal_uid(instance->data, request->data)) {
                error = Iso15693_3ErrorUidMismatch;
                break;
            }

            data = &request->data[ISO15693_3_UID_SIZE];
            data_size = buf_size - buf_size_min_addr;

        } else {
            data = request->data;
            data_size = buf_size - buf_size_min;
        }

        error = iso15693_3_listener_handle_standard_request(
            instance, data, data_size, request->command, request->flags);

    } while(false);

    return error;
}

Iso15693_3Error iso15693_3_listener_process_single_eof(Iso15693_3Listener* instance) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        if(!instance->session_state.wait_for_eof) {
            error = Iso15693_3ErrorUnexpectedResponse;
            break;
        }

        instance->session_state.wait_for_eof = false;

        error = iso15693_3_listener_send_frame(instance, instance->tx_buffer);
    } while(false);

    return error;
}

Iso15693_3Error iso15693_3_listener_process_uid_mismatch(
    Iso15693_3Listener* instance,
    const BitBuffer* rx_buffer) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    // No checks, assuming they have been made beforehand
    typedef struct {
        uint8_t flags;
        uint8_t command;
    } Iso15693_3RequestLayout;

    const Iso15693_3RequestLayout* request =
        (const Iso15693_3RequestLayout*)bit_buffer_get_data(rx_buffer);

    if(request->command == ISO15693_3_CMD_SELECT) {
        if(instance->state == Iso15693_3ListenerStateSelected) {
            error = iso15693_3_listener_ready(instance);
        }
    }

    return error;
}
