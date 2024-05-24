#include "slix_listener_i.h"

#include <nfc/protocols/iso15693_3/iso15693_3_listener_i.h>

#include <furi_hal_random.h>

#define TAG "SlixListener"

typedef SlixError (*SlixRequestHandler)(
    SlixListener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags);

// Helper functions

static bool
    slix_listener_is_password_lock_enabled(SlixListener* instance, SlixPasswordType password_type) {
    return !instance->session_state.password_match[password_type];
}

static SlixPasswordType slix_listener_get_password_type_by_id(uint8_t id) {
    uint32_t type;

    for(type = 0; type < SlixPasswordTypeCount; ++type) {
        if(id >> type == 0x01U) break;
    }

    return type;
}

static SlixPassword
    slix_listener_unxor_password(const SlixPassword password_xored, uint16_t random) {
    return REVERSE_BYTES_U32(password_xored ^ ((SlixPassword)random << 16 | random));
}

static SlixError slix_listener_set_password(
    SlixListener* instance,
    SlixPasswordType password_type,
    SlixPassword password) {
    SlixError error = SlixErrorNone;

    do {
        if(password_type >= SlixPasswordTypeCount) {
            error = SlixErrorInternal;
            break;
        }

        SlixData* slix_data = instance->data;

        if(!slix_type_supports_password(slix_get_type(slix_data), password_type)) {
            error = SlixErrorNotSupported;
            break;
        }

        SlixListenerSessionState* session_state = &instance->session_state;

        // With AcceptAllPassword capability set skip password validation
        if(instance->data->capabilities == SlixCapabilitiesAcceptAllPasswords) {
            session_state->password_match[password_type] = true;
            break;
        }

        session_state->password_match[password_type] =
            (password == slix_get_password(slix_data, password_type));

        if(!session_state->password_match[password_type]) {
            error = SlixErrorWrongPassword;
            break;
        }
    } while(false);

    return error;
}

static SlixError slix_listener_write_password(
    SlixListener* instance,
    SlixPasswordType password_type,
    SlixPassword password) {
    SlixError error = SlixErrorNone;

    do {
        if(password_type >= SlixPasswordTypeCount) {
            error = SlixErrorInternal;
            break;
        }

        SlixData* slix_data = instance->data;

        if(!slix_type_supports_password(slix_get_type(slix_data), password_type)) {
            error = SlixErrorNotSupported;
            break;
        }

        SlixListenerSessionState* session_state = &instance->session_state;

        if(session_state->password_match[password_type]) {
            // TODO FL-3634: check for password lock
            slix_set_password(slix_data, password_type, password);
            // Require another SET_PASSWORD command with the new password
            session_state->password_match[password_type] = false;
        } else {
            error = SlixErrorWrongPassword;
            break;
        }
    } while(false);

    return error;
}

// Custom SLIX request handlers
static SlixError slix_listener_default_handler(
    SlixListener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(instance);
    UNUSED(data);
    UNUSED(data_size);
    UNUSED(flags);

    // Empty placeholder handler
    return SlixErrorNotSupported;
}

static SlixError slix_listener_get_nxp_system_info_handler(
    SlixListener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(data);
    UNUSED(data_size);
    UNUSED(flags);

    const SlixData* slix_data = instance->data;
    const Iso15693_3Data* iso15693_data = instance->data->iso15693_3_data;

    const SlixProtection* protection = &slix_data->system_info.protection;
    bit_buffer_append_byte(instance->tx_buffer, protection->pointer);
    bit_buffer_append_byte(instance->tx_buffer, protection->condition);

    uint8_t lock_bits = 0;
    if(iso15693_data->settings.lock_bits.dsfid) {
        lock_bits |= SLIX_LOCK_BITS_DSFID;
    }
    if(iso15693_data->settings.lock_bits.afi) {
        lock_bits |= SLIX_LOCK_BITS_AFI;
    }
    if(slix_data->system_info.lock_bits.eas) {
        lock_bits |= SLIX_LOCK_BITS_EAS;
    }
    if(slix_data->system_info.lock_bits.ppl) {
        lock_bits |= SLIX_LOCK_BITS_PPL;
    }
    bit_buffer_append_byte(instance->tx_buffer, lock_bits);

    const uint32_t feature_flags = SLIX2_FEATURE_FLAGS;
    bit_buffer_append_bytes(instance->tx_buffer, (uint8_t*)&feature_flags, sizeof(uint32_t));

    return SlixErrorNone;
}

static SlixError slix_listener_get_random_number_handler(
    SlixListener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(data);
    UNUSED(data_size);
    UNUSED(flags);

    SlixListenerSessionState* session_state = &instance->session_state;
    session_state->random = furi_hal_random_get();
    bit_buffer_append_bytes(
        instance->tx_buffer, (uint8_t*)&session_state->random, sizeof(uint16_t));

    return SlixErrorNone;
}

static SlixError slix_listener_set_password_handler(
    SlixListener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(flags);
    SlixError error = SlixErrorNone;

    do {
#pragma pack(push, 1)
        typedef struct {
            uint8_t password_id;
            SlixPassword password_xored;
        } SlixSetPasswordRequestLayout;
#pragma pack(pop)

        if(data_size != sizeof(SlixSetPasswordRequestLayout)) {
            error = SlixErrorFormat;
            break;
        }

        const SlixSetPasswordRequestLayout* request = (const SlixSetPasswordRequestLayout*)data;
        const SlixPasswordType password_type =
            slix_listener_get_password_type_by_id(request->password_id);
        const SlixPassword password_received =
            slix_listener_unxor_password(request->password_xored, instance->session_state.random);

        error = slix_listener_set_password(instance, password_type, password_received);
        if(error != SlixErrorNone) break;

        if(password_type == SlixPasswordTypePrivacy) {
            slix_set_privacy_mode(instance->data, false);
        }
    } while(false);

    return error;
}

static SlixError slix_listener_write_password_handler(
    SlixListener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(flags);
    SlixError error = SlixErrorNone;

    do {
#pragma pack(push, 1)
        typedef struct {
            uint8_t password_id;
            SlixPassword password;
        } SlixWritePasswordRequestLayout;
#pragma pack(pop)

        if(data_size != sizeof(SlixWritePasswordRequestLayout)) {
            error = SlixErrorFormat;
            break;
        }

        const SlixWritePasswordRequestLayout* request =
            (const SlixWritePasswordRequestLayout*)data;
        const SlixPasswordType password_type =
            slix_listener_get_password_type_by_id(request->password_id);

        error = slix_listener_write_password(instance, password_type, request->password);
        if(error != SlixErrorNone) break;

    } while(false);

    return error;
}

static SlixError slix_listener_protect_page_handler(
    SlixListener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(flags);
    SlixError error = SlixErrorNone;

    do {
        typedef struct {
            uint8_t pointer;
            uint8_t condition;
        } SlixProtectPageRequestLayout;

        if(data_size != sizeof(SlixProtectPageRequestLayout)) {
            error = SlixErrorFormat;
            break;
        }

        SlixData* slix_data = instance->data;

        if(slix_data->system_info.lock_bits.ppl) {
            error = SlixErrorInternal;
            break;
        }

        const SlixListenerSessionState* session_state = &instance->session_state;
        if(!session_state->password_match[SlixPasswordTypeRead] ||
           !session_state->password_match[SlixPasswordTypeWrite]) {
            error = SlixErrorInternal;
            break;
        }

        const SlixProtectPageRequestLayout* request = (const SlixProtectPageRequestLayout*)data;

        if(request->pointer >= SLIX_COUNTER_BLOCK_NUM) {
            error = SlixErrorInternal;
            break;
        }

        SlixProtection* protection = &slix_data->system_info.protection;

        protection->pointer = request->pointer;
        protection->condition = request->condition;
    } while(false);

    return error;
}

static SlixError slix_listener_enable_privacy_handler(
    SlixListener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(flags);
    SlixError error = SlixErrorNone;

    do {
        typedef struct {
            SlixPassword password_xored;
        } SlixEnablePrivacyRequestLayout;

        if(data_size != sizeof(SlixEnablePrivacyRequestLayout)) {
            error = SlixErrorFormat;
            break;
        }

        const SlixEnablePrivacyRequestLayout* request =
            (const SlixEnablePrivacyRequestLayout*)data;

        const SlixPassword password_received =
            slix_listener_unxor_password(request->password_xored, instance->session_state.random);

        error = slix_listener_set_password(instance, SlixPasswordTypePrivacy, password_received);
        if(error != SlixErrorNone) break;

        slix_set_privacy_mode(instance->data, true);
    } while(false);

    return error;
}

static SlixError slix_listener_read_signature_handler(
    SlixListener* instance,
    const uint8_t* data,
    size_t data_size,
    uint8_t flags) {
    UNUSED(data);
    UNUSED(data_size);
    UNUSED(flags);

    bit_buffer_append_bytes(instance->tx_buffer, instance->data->signature, sizeof(SlixSignature));
    return SlixErrorNone;
}

// Custom SLIX commands handler table
static const SlixRequestHandler slix_request_handler_table[SLIX_CMD_CUSTOM_COUNT] = {
    slix_listener_default_handler, // SLIX_CMD_SET_EAS (0xA2U)
    slix_listener_default_handler, // SLIX_CMD_RESET_EAS (0xA3U)
    slix_listener_default_handler, // SLIX_CMD_LOCK_EAS (0xA4U)
    slix_listener_default_handler, // SLIX_CMD_EAS_ALARM (0xA5U)
    slix_listener_default_handler, // SLIX_CMD_PASSWORD_PROTECT_EAS_AFI (0xA6U)
    slix_listener_default_handler, // SLIX_CMD_WRITE_EAS_ID (0xA7U)
    slix_listener_default_handler, // UNUSED (0xA8U)
    slix_listener_default_handler, // UNUSED (0xA9U)
    slix_listener_default_handler, // UNUSED (0xAAU)
    slix_listener_get_nxp_system_info_handler,
    slix_listener_default_handler, // UNUSED (0xACU)
    slix_listener_default_handler, // UNUSED (0xADU)
    slix_listener_default_handler, // UNUSED (0xAEU)
    slix_listener_default_handler, // UNUSED (0xAFU)
    slix_listener_default_handler, // SLIX_CMD_INVENTORY_PAGE_READ (0xB0U)
    slix_listener_default_handler, // SLIX_CMD_INVENTORY_PAGE_READ_FAST (0xB1U)
    slix_listener_get_random_number_handler,
    slix_listener_set_password_handler,
    slix_listener_write_password_handler,
    slix_listener_default_handler, // SLIX_CMD_64_BIT_PASSWORD_PROTECTION (0xB5U)
    slix_listener_protect_page_handler,
    slix_listener_default_handler, // SLIX_CMD_LOCK_PAGE_PROTECTION_CONDITION (0xB7U)
    slix_listener_default_handler, // UNUSED (0xB8U)
    slix_listener_default_handler, // SLIX_CMD_DESTROY (0xB9U)
    slix_listener_enable_privacy_handler,
    slix_listener_default_handler, // UNUSED (0xBBU)
    slix_listener_default_handler, // SLIX_CMD_STAY_QUIET_PERSISTENT (0xBCU)
    slix_listener_read_signature_handler,
};

// ISO15693-3 Protocol extension handlers

static Iso15693_3Error
    slix_listener_iso15693_3_inventory_extension_handler(SlixListener* instance, va_list args) {
    UNUSED(args);

    return instance->data->privacy ? Iso15693_3ErrorIgnore : Iso15693_3ErrorNone;
}

static Iso15693_3Error
    slix_iso15693_3_read_block_extension_handler(SlixListener* instance, va_list args) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        const uint32_t block_num = va_arg(args, uint32_t);
        // SLIX Counter has no read protection
        if(block_num == SLIX_COUNTER_BLOCK_NUM) break;

        if(slix_is_block_protected(instance->data, SlixPasswordTypeRead, block_num)) {
            if(slix_listener_is_password_lock_enabled(instance, SlixPasswordTypeRead)) {
                error = Iso15693_3ErrorInternal;
                break;
            }
        }
    } while(false);

    return error;
}

static Iso15693_3Error
    slix_listener_iso15693_3_write_block_extension_handler(SlixListener* instance, va_list args) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        const uint32_t block_num = va_arg(args, uint32_t);

        if(block_num == SLIX_COUNTER_BLOCK_NUM) {
            const uint32_t counter = *(va_arg(args, uint32_t*));
            if(counter == 0x00000001U) {
                if(slix_is_counter_increment_protected(instance->data) &&
                   slix_listener_is_password_lock_enabled(instance, SlixPasswordTypeRead)) {
                    error = Iso15693_3ErrorInternal;
                    break;
                }
                slix_increment_counter(instance->data);
                error = Iso15693_3ErrorFullyHandled;
                break;
            }
        } else if(slix_is_block_protected(instance->data, SlixPasswordTypeRead, block_num)) {
            if(slix_listener_is_password_lock_enabled(instance, SlixPasswordTypeRead)) {
                error = Iso15693_3ErrorInternal;
                break;
            }
        }

        if(slix_is_block_protected(instance->data, SlixPasswordTypeWrite, block_num)) {
            if(slix_listener_is_password_lock_enabled(instance, SlixPasswordTypeWrite)) {
                error = Iso15693_3ErrorInternal;
                break;
            }
        }

    } while(false);

    return error;
}

static Iso15693_3Error
    slix_listener_iso15693_3_lock_block_extension_handler(SlixListener* instance, va_list args) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    do {
        const uint32_t block_num = va_arg(args, uint32_t);

        // SLIX counter cannot be locked
        if(block_num == SLIX_COUNTER_BLOCK_NUM) {
            error = Iso15693_3ErrorInternal;
            break;
        }

        if(slix_is_block_protected(instance->data, SlixPasswordTypeRead, block_num)) {
            if(slix_listener_is_password_lock_enabled(instance, SlixPasswordTypeRead)) {
                error = Iso15693_3ErrorInternal;
                break;
            }
        }

        if(slix_is_block_protected(instance->data, SlixPasswordTypeWrite, block_num)) {
            if(slix_listener_is_password_lock_enabled(instance, SlixPasswordTypeWrite)) {
                error = Iso15693_3ErrorInternal;
                break;
            }
        }

    } while(false);

    return error;
}

static Iso15693_3Error slix_listener_iso15693_3_read_multi_block_extension_handler(
    SlixListener* instance,
    va_list args) {
    Iso15693_3Error error = Iso15693_3ErrorNone;

    const uint32_t block_index_start = va_arg(args, uint32_t);
    const uint32_t block_index_end = va_arg(args, uint32_t);

    for(uint32_t i = block_index_start; i <= block_index_end; ++i) {
        // SLIX Counter has no read protection
        if(i == SLIX_COUNTER_BLOCK_NUM) continue;

        if(slix_is_block_protected(instance->data, SlixPasswordTypeRead, i)) {
            if(slix_listener_is_password_lock_enabled(instance, SlixPasswordTypeRead)) {
                error = Iso15693_3ErrorInternal;
                break;
            }
        }
    }

    return error;
}

static Iso15693_3Error slix_listener_iso15693_3_write_multi_block_extension_handler(
    SlixListener* instance,
    va_list args) {
    UNUSED(instance);
    UNUSED(args);
    // No mention of this command in SLIX docs, assuming not supported
    return Iso15693_3ErrorNotSupported;
}

static Iso15693_3Error slix_listener_iso15693_3_write_lock_afi_extension_handler(
    SlixListener* instance,
    va_list args) {
    UNUSED(args);

    return slix_listener_is_password_lock_enabled(instance, SlixPasswordTypeEasAfi) ?
               Iso15693_3ErrorInternal :
               Iso15693_3ErrorNone;
}

// Extended ISO15693-3 standard commands handler table (NULL = no extension)
static const Iso15693_3ExtensionHandlerTable slix_iso15693_extension_handler_table = {
    .mandatory =
        {
            (Iso15693_3ExtensionHandler)slix_listener_iso15693_3_inventory_extension_handler,
            (Iso15693_3ExtensionHandler)NULL // ISO15693_3_CMD_STAY_QUIET (0x02U)
        },
    .optional =
        {
            (Iso15693_3ExtensionHandler)slix_iso15693_3_read_block_extension_handler,
            (Iso15693_3ExtensionHandler)slix_listener_iso15693_3_write_block_extension_handler,
            (Iso15693_3ExtensionHandler)slix_listener_iso15693_3_lock_block_extension_handler,
            (Iso15693_3ExtensionHandler)slix_listener_iso15693_3_read_multi_block_extension_handler,
            (Iso15693_3ExtensionHandler)
                slix_listener_iso15693_3_write_multi_block_extension_handler,
            (Iso15693_3ExtensionHandler)NULL, // ISO15693_3_CMD_SELECT (0x25U)
            (Iso15693_3ExtensionHandler)NULL, // ISO15693_3_CMD_RESET_TO_READY (0x26U)
            (Iso15693_3ExtensionHandler)slix_listener_iso15693_3_write_lock_afi_extension_handler,
            (Iso15693_3ExtensionHandler)slix_listener_iso15693_3_write_lock_afi_extension_handler,
            (Iso15693_3ExtensionHandler)NULL, // ISO15693_3_CMD_WRITE_DSFID (0x29U)
            (Iso15693_3ExtensionHandler)NULL, // ISO15693_3_CMD_LOCK_DSFID (0x2AU)
            (Iso15693_3ExtensionHandler)NULL, // ISO15693_3_CMD_GET_SYS_INFO (0x2BU)
            (Iso15693_3ExtensionHandler)NULL, // ISO15693_3_CMD_GET_BLOCKS_SECURITY (0x2CU)
        },
};

SlixError slix_listener_init_iso15693_3_extensions(SlixListener* instance) {
    iso15693_3_listener_set_extension_handler_table(
        instance->iso15693_3_listener, &slix_iso15693_extension_handler_table, instance);
    return SlixErrorNone;
}

SlixError slix_listener_process_request(SlixListener* instance, const BitBuffer* rx_buffer) {
    SlixError error = SlixErrorNone;

    do {
        typedef struct {
            uint8_t flags;
            uint8_t command;
            uint8_t manufacturer;
            uint8_t data[];
        } SlixRequestLayout;

        const size_t buf_size = bit_buffer_get_size_bytes(rx_buffer);

        if(buf_size < sizeof(SlixRequestLayout)) {
            error = SlixErrorFormat;
            break;
        }

        const SlixRequestLayout* request =
            (const SlixRequestLayout*)bit_buffer_get_data(rx_buffer);

        const bool addressed_mode = instance->iso15693_3_listener->session_state.addressed;

        const size_t uid_field_size = addressed_mode ? ISO15693_3_UID_SIZE : 0;
        const size_t buf_size_min = sizeof(SlixRequestLayout) + uid_field_size;

        if(buf_size < buf_size_min) {
            error = SlixErrorFormat;
            break;
        }

        if(addressed_mode) {
            if(!iso15693_3_is_equal_uid(instance->data->iso15693_3_data, request->data)) {
                error = SlixErrorUidMismatch;
                break;
            }
        }

        const uint8_t command = request->command;
        const bool is_valid_slix_command = command >= SLIX_CMD_CUSTOM_START &&
                                           command < SLIX_CMD_CUSTOM_END;
        if(!is_valid_slix_command) {
            error = SlixErrorNotSupported;
            break;
        }

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_append_byte(instance->tx_buffer, ISO15693_3_RESP_FLAG_NONE);

        const uint8_t* request_data = &request->data[uid_field_size];
        const size_t request_data_size = buf_size - buf_size_min;

        SlixRequestHandler handler = slix_request_handler_table[command - SLIX_CMD_CUSTOM_START];
        error = handler(instance, request_data, request_data_size, request->flags);

        // It's a trick! Send no reply.
        if(error == SlixErrorFormat || error == SlixErrorWrongPassword ||
           error == SlixErrorNotSupported)
            break;

        if(error != SlixErrorNone) {
            bit_buffer_reset(instance->tx_buffer);
            bit_buffer_append_byte(instance->tx_buffer, ISO15693_3_RESP_FLAG_ERROR);
            bit_buffer_append_byte(instance->tx_buffer, ISO15693_3_RESP_ERROR_UNKNOWN);
        }

        const Iso15693_3Error iso15693_error =
            iso15693_3_listener_send_frame(instance->iso15693_3_listener, instance->tx_buffer);
        error = slix_process_iso15693_3_error(iso15693_error);
    } while(false);

    return error;
}
