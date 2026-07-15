#include "type_4_tag_listener_i.h"
#include "type_4_tag_i.h"

#include <bit_lib/bit_lib.h>

#define TAG "Type4TagListener"

typedef Type4TagError (*Type4TagListenerApduHandler)(
    Type4TagListener* instance,
    uint8_t p1,
    uint8_t p2,
    size_t lc,
    const uint8_t* data,
    size_t le);

typedef struct {
    uint8_t cla_ins[2];
    Type4TagListenerApduHandler handler;
} Type4TagListenerApduCommand;

static const uint8_t type_4_tag_success_apdu[] = {TYPE_4_TAG_ISO_STATUS_SUCCESS};
static const uint8_t type_4_tag_offset_error_apdu[] = {TYPE_4_TAG_ISO_STATUS_OFFSET_ERR};
static const uint8_t type_4_tag_not_found_apdu[] = {TYPE_4_TAG_ISO_STATUS_NOT_FOUND};
static const uint8_t type_4_tag_no_support_apdu[] = {TYPE_4_TAG_ISO_STATUS_NO_SUPPORT};
static const uint8_t type_4_tag_bad_params_apdu[] = {TYPE_4_TAG_ISO_STATUS_BAD_PARAMS};
static const uint8_t type_4_tag_no_cmd_apdu[] = {TYPE_4_TAG_ISO_STATUS_NO_CMD};

static Type4TagError type_4_tag_listener_iso_select(
    Type4TagListener* instance,
    uint8_t p1,
    uint8_t p2,
    size_t lc,
    const uint8_t* data,
    size_t le) {
    UNUSED(p2);
    UNUSED(le);

    if(p1 == TYPE_4_TAG_ISO_SELECT_P1_BY_NAME && lc == TYPE_4_TAG_ISO_NAME_LEN) {
        if(memcmp(type_4_tag_iso_mf_name, data, sizeof(type_4_tag_iso_mf_name)) == 0) {
            instance->state = Type4TagListenerStateSelectedPicc;
            bit_buffer_append_bytes(
                instance->tx_buffer, type_4_tag_success_apdu, sizeof(type_4_tag_success_apdu));
            return Type4TagErrorNone;
        }

        if(memcmp(type_4_tag_iso_df_name, data, sizeof(type_4_tag_iso_df_name)) == 0) {
            instance->state = Type4TagListenerStateSelectedApplication;
            bit_buffer_append_bytes(
                instance->tx_buffer, type_4_tag_success_apdu, sizeof(type_4_tag_success_apdu));
            return Type4TagErrorNone;
        }

    } else if(
        (p1 == TYPE_4_TAG_ISO_SELECT_P1_BY_ID || p1 == TYPE_4_TAG_ISO_SELECT_P1_BY_EF_ID) &&
        lc == TYPE_4_TAG_ISO_ID_LEN) {
        uint16_t id = bit_lib_bytes_to_num_be(data, sizeof(uint16_t));

        if(p1 == TYPE_4_TAG_ISO_SELECT_P1_BY_ID) {
            if(id == TYPE_4_TAG_ISO_MF_ID) {
                instance->state = Type4TagListenerStateSelectedPicc;
                bit_buffer_append_bytes(
                    instance->tx_buffer, type_4_tag_success_apdu, sizeof(type_4_tag_success_apdu));
                return Type4TagErrorNone;
            }

            if(id == TYPE_4_TAG_ISO_DF_ID) {
                instance->state = Type4TagListenerStateSelectedApplication;
                bit_buffer_append_bytes(
                    instance->tx_buffer, type_4_tag_success_apdu, sizeof(type_4_tag_success_apdu));
                return Type4TagErrorNone;
            }
        }

        if(instance->state >= Type4TagListenerStateSelectedApplication) {
            if(id == TYPE_4_TAG_T4T_CC_EF_ID) {
                instance->state = Type4TagListenerStateSelectedCapabilityContainer;
                bit_buffer_append_bytes(
                    instance->tx_buffer, type_4_tag_success_apdu, sizeof(type_4_tag_success_apdu));
                return Type4TagErrorNone;
            }

            if(id == (instance->data->is_tag_specific ? instance->data->ndef_file_id :
                                                        TYPE_4_TAG_T4T_NDEF_EF_ID)) {
                instance->state = Type4TagListenerStateSelectedNdefMessage;
                bit_buffer_append_bytes(
                    instance->tx_buffer, type_4_tag_success_apdu, sizeof(type_4_tag_success_apdu));
                return Type4TagErrorNone;
            }
        }
    }

    bit_buffer_append_bytes(
        instance->tx_buffer, type_4_tag_not_found_apdu, sizeof(type_4_tag_not_found_apdu));
    return Type4TagErrorCustomCommand;
}

static Type4TagError type_4_tag_listener_iso_read(
    Type4TagListener* instance,
    uint8_t p1,
    uint8_t p2,
    size_t lc,
    const uint8_t* data,
    size_t le) {
    UNUSED(lc);
    UNUSED(data);

    size_t offset;
    if(p1 & TYPE_4_TAG_ISO_READ_P1_ID_MASK) {
        bit_buffer_append_bytes(
            instance->tx_buffer, type_4_tag_no_support_apdu, sizeof(type_4_tag_no_support_apdu));
        return Type4TagErrorCustomCommand;
    } else {
        offset = (p1 << 8) + p2;
    }

    if(instance->state == Type4TagListenerStateSelectedCapabilityContainer) {
        uint8_t cc_buf[TYPE_4_TAG_T4T_CC_MIN_SIZE];
        if(offset >= sizeof(cc_buf)) {
            bit_buffer_append_bytes(
                instance->tx_buffer,
                type_4_tag_offset_error_apdu,
                sizeof(type_4_tag_offset_error_apdu));
            return Type4TagErrorWrongFormat;
        }
        type_4_tag_cc_dump(instance->data, cc_buf, sizeof(cc_buf));

        bit_buffer_append_bytes(
            instance->tx_buffer, cc_buf + offset, MIN(sizeof(cc_buf) - offset, le));
        bit_buffer_append_bytes(
            instance->tx_buffer, type_4_tag_success_apdu, sizeof(type_4_tag_success_apdu));
        return Type4TagErrorNone;
    }

    if(instance->state == Type4TagListenerStateSelectedNdefMessage) {
        size_t ndef_file_len = simple_array_get_count(instance->data->ndef_data);
        bool included_len = false;
        if(offset < sizeof(uint16_t)) {
            uint8_t ndef_file_len_be[sizeof(uint16_t)];
            bit_lib_num_to_bytes_be(ndef_file_len, sizeof(ndef_file_len_be), ndef_file_len_be);
            uint8_t read_len = MIN(sizeof(ndef_file_len_be) - offset, le);
            bit_buffer_append_bytes(instance->tx_buffer, &ndef_file_len_be[offset], read_len);
            included_len = true;
            offset = sizeof(uint16_t);
            le -= read_len;
        }
        offset -= sizeof(uint16_t);

        if(offset >= ndef_file_len) {
            if(included_len) {
                bit_buffer_append_bytes(
                    instance->tx_buffer, type_4_tag_success_apdu, sizeof(type_4_tag_success_apdu));
                return Type4TagErrorNone;
            } else {
                bit_buffer_append_bytes(
                    instance->tx_buffer,
                    type_4_tag_offset_error_apdu,
                    sizeof(type_4_tag_offset_error_apdu));
                return Type4TagErrorWrongFormat;
            }
        }
        const uint8_t* ndef_data = simple_array_cget_data(instance->data->ndef_data);
        bit_buffer_append_bytes(
            instance->tx_buffer, &ndef_data[offset], MIN(ndef_file_len - offset, le));
        bit_buffer_append_bytes(
            instance->tx_buffer, type_4_tag_success_apdu, sizeof(type_4_tag_success_apdu));
        return Type4TagErrorNone;
    }

    bit_buffer_append_bytes(
        instance->tx_buffer, type_4_tag_not_found_apdu, sizeof(type_4_tag_not_found_apdu));
    return Type4TagErrorCustomCommand;
}

static Type4TagError type_4_tag_listener_iso_write(
    Type4TagListener* instance,
    uint8_t p1,
    uint8_t p2,
    size_t lc,
    const uint8_t* data,
    size_t le) {
    UNUSED(le);

    size_t offset;
    if(p1 & TYPE_4_TAG_ISO_WRITE_P1_ID_MASK) {
        bit_buffer_append_bytes(
            instance->tx_buffer, type_4_tag_no_support_apdu, sizeof(type_4_tag_no_support_apdu));
        return Type4TagErrorCustomCommand;
    } else {
        offset = (p1 << 8) + p2;
    }

    if(instance->state == Type4TagListenerStateSelectedCapabilityContainer) {
        bit_buffer_append_bytes(
            instance->tx_buffer, type_4_tag_no_support_apdu, sizeof(type_4_tag_no_support_apdu));
        return Type4TagErrorNotSupported;
    }

    if(instance->state == Type4TagListenerStateSelectedNdefMessage) {
        if(offset + lc > sizeof(uint16_t) + (instance->data->is_tag_specific ?
                                                 instance->data->ndef_max_len :
                                                 TYPE_4_TAG_DEFAULT_NDEF_SIZE)) {
            bit_buffer_append_bytes(
                instance->tx_buffer,
                type_4_tag_offset_error_apdu,
                sizeof(type_4_tag_offset_error_apdu));
            return Type4TagErrorWrongFormat;
        }

        const size_t ndef_file_len = simple_array_get_count(instance->data->ndef_data);
        size_t ndef_file_len_new = ndef_file_len;
        if(offset < sizeof(uint16_t)) {
            const uint8_t write_len = sizeof(uint16_t) - offset;
            ndef_file_len_new = bit_lib_bytes_to_num_be(data, write_len);
            offset = sizeof(uint16_t);
            data += offset;
            lc -= write_len;
        }
        offset -= sizeof(uint16_t);

        ndef_file_len_new = MAX(ndef_file_len_new, offset + lc);
        if(ndef_file_len_new != ndef_file_len) {
            SimpleArray* ndef_data_temp = simple_array_alloc(&simple_array_config_uint8_t);
            if(ndef_file_len_new > 0) {
                simple_array_init(ndef_data_temp, ndef_file_len_new);
                if(ndef_file_len > 0) {
                    memcpy(
                        simple_array_get_data(ndef_data_temp),
                        simple_array_get_data(instance->data->ndef_data),
                        MIN(ndef_file_len_new, ndef_file_len));
                }
            }
            simple_array_copy(instance->data->ndef_data, ndef_data_temp);
            simple_array_free(ndef_data_temp);
        }

        if(ndef_file_len_new > 0 && lc > 0) {
            uint8_t* ndef_data = simple_array_get_data(instance->data->ndef_data);
            memcpy(&ndef_data[offset], data, lc);
        }
        bit_buffer_append_bytes(
            instance->tx_buffer, type_4_tag_success_apdu, sizeof(type_4_tag_success_apdu));
        return Type4TagErrorNone;
    }

    bit_buffer_append_bytes(
        instance->tx_buffer, type_4_tag_not_found_apdu, sizeof(type_4_tag_not_found_apdu));
    return Type4TagErrorCustomCommand;
}

static const Type4TagListenerApduCommand type_4_tag_listener_commands[] = {
    {
        .cla_ins = {TYPE_4_TAG_ISO_SELECT_CMD},
        .handler = type_4_tag_listener_iso_select,
    },
    {
        .cla_ins = {TYPE_4_TAG_ISO_READ_CMD},
        .handler = type_4_tag_listener_iso_read,
    },
    {
        .cla_ins = {TYPE_4_TAG_ISO_WRITE_CMD},
        .handler = type_4_tag_listener_iso_write,
    },
};

Type4TagError
    type_4_tag_listener_handle_apdu(Type4TagListener* instance, const BitBuffer* rx_buffer) {
    Type4TagError error = Type4TagErrorNone;

    bit_buffer_reset(instance->tx_buffer);

    do {
        typedef struct {
            uint8_t cla_ins[2];
            uint8_t p1;
            uint8_t p2;
            uint8_t body[];
        } Type4TagApdu;

        const size_t buf_size = bit_buffer_get_size_bytes(rx_buffer);

        if(buf_size < sizeof(Type4TagApdu)) {
            error = Type4TagErrorWrongFormat;
            break;
        }

        const Type4TagApdu* apdu = (const Type4TagApdu*)bit_buffer_get_data(rx_buffer);

        Type4TagListenerApduHandler handler = NULL;
        for(size_t i = 0; i < COUNT_OF(type_4_tag_listener_commands); i++) {
            const Type4TagListenerApduCommand* command = &type_4_tag_listener_commands[i];
            if(memcmp(apdu->cla_ins, command->cla_ins, sizeof(apdu->cla_ins)) == 0) {
                handler = command->handler;
                break;
            }
        }
        if(!handler) {
            bit_buffer_append_bytes(
                instance->tx_buffer, type_4_tag_no_cmd_apdu, sizeof(type_4_tag_no_cmd_apdu));
            error = Type4TagErrorCustomCommand;
            break;
        }

        size_t body_size = buf_size - offsetof(Type4TagApdu, body);
        size_t lc;
        const uint8_t* data = apdu->body;
        size_t le;
        if(body_size == 0) {
            lc = 0;
            data = NULL;
            le = 0;
        } else if(body_size == 1) {
            lc = 0;
            data = NULL;
            le = apdu->body[0];
            if(le == 0) {
                le = 256;
            }
        } else if(body_size == 3 && apdu->body[0] == 0) {
            lc = 0;
            data = NULL;
            le = bit_lib_bytes_to_num_be(&apdu->body[1], sizeof(uint16_t));
            if(le == 0) {
                le = 65536;
            }
        } else {
            bool extended_lc = false;
            if(data[0] == 0) {
                extended_lc = true;
                lc = bit_lib_bytes_to_num_be(&data[1], sizeof(uint16_t));
                data += 1 + sizeof(uint16_t);
                body_size -= 1 + sizeof(uint16_t);
            } else {
                lc = data[0];
                data++;
                body_size--;
            }
            if(lc == 0 || body_size < lc) {
                bit_buffer_append_bytes(
                    instance->tx_buffer,
                    type_4_tag_bad_params_apdu,
                    sizeof(type_4_tag_bad_params_apdu));
                error = Type4TagErrorWrongFormat;
                break;
            }

            if(body_size == lc) {
                le = 0;
            } else if(!extended_lc && body_size - lc == 1) {
                le = data[lc];
                if(le == 0) {
                    le = 256;
                }
            } else if(extended_lc && body_size - lc == 2) {
                le = bit_lib_bytes_to_num_be(&data[lc], sizeof(uint16_t));
                if(le == 0) {
                    le = 65536;
                }
            } else {
                bit_buffer_append_bytes(
                    instance->tx_buffer,
                    type_4_tag_bad_params_apdu,
                    sizeof(type_4_tag_bad_params_apdu));
                error = Type4TagErrorWrongFormat;
                break;
            }
        }

        error = handler(instance, apdu->p1, apdu->p2, lc, data, le);
    } while(false);

    if(bit_buffer_get_size_bytes(instance->tx_buffer) > 0) {
        const Iso14443_4aError iso14443_4a_error =
            iso14443_4a_listener_send_block(instance->iso14443_4a_listener, instance->tx_buffer);

        // Keep error flag to show unknown command on screen
        if(error != Type4TagErrorCustomCommand) {
            error = type_4_tag_process_error(iso14443_4a_error);
        }
    }

    return error;
}
