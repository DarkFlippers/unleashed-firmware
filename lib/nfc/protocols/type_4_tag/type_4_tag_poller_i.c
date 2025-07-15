#include "type_4_tag_poller_i.h"
#include "type_4_tag_i.h"

#include <bit_lib/bit_lib.h>

#include <nfc/nfc_device.h>
#include <nfc/protocols/ntag4xx/ntag4xx_poller.h>
#include <nfc/protocols/ntag4xx/ntag4xx_poller_defs.h>
#include <nfc/protocols/mf_desfire/mf_desfire_poller.h>
#include <nfc/protocols/mf_desfire/mf_desfire_poller_defs.h>

#define TAG "Type4TagPoller"

static const MfDesfireApplicationId mf_des_picc_app_id = {.data = {0x00, 0x00, 0x00}};
static const MfDesfireApplicationId mf_des_t4t_app_id = {.data = {0x10, 0xEE, 0xEE}};
static const MfDesfireKeySettings mf_des_t4t_app_key_settings = {
    .is_master_key_changeable = true,
    .is_free_directory_list = true,
    .is_free_create_delete = true,
    .is_config_changeable = true,
    .change_key_id = 0,
    .max_keys = 1,
    .flags = 0,
};
#define MF_DES_T4T_CC_FILE_ID (0x01)
static const MfDesfireFileSettings mf_des_t4t_cc_file = {
    .type = MfDesfireFileTypeStandard,
    .comm = MfDesfireFileCommunicationSettingsPlaintext,
    .access_rights[0] = 0xEEEE,
    .access_rights_len = 1,
    .data.size = TYPE_4_TAG_T4T_CC_MIN_SIZE,
};
#define MF_DES_T4T_NDEF_FILE_ID (0x02)
static const MfDesfireFileSettings mf_des_t4t_ndef_file_default = {
    .type = MfDesfireFileTypeStandard,
    .comm = MfDesfireFileCommunicationSettingsPlaintext,
    .access_rights[0] = 0xEEE0,
    .access_rights_len = 1,
};

Type4TagError type_4_tag_apdu_trx(Type4TagPoller* instance, BitBuffer* tx_buf, BitBuffer* rx_buf) {
    furi_check(instance);

    bit_buffer_reset(rx_buf);

    Iso14443_4aError iso14443_4a_error =
        iso14443_4a_poller_send_block(instance->iso14443_4a_poller, tx_buf, rx_buf);

    bit_buffer_reset(tx_buf);

    if(iso14443_4a_error != Iso14443_4aErrorNone) {
        return type_4_tag_process_error(iso14443_4a_error);
    }

    size_t response_len = bit_buffer_get_size_bytes(rx_buf);
    if(response_len < TYPE_4_TAG_ISO_STATUS_LEN) {
        return Type4TagErrorWrongFormat;
    }

    static const uint8_t success[TYPE_4_TAG_ISO_STATUS_LEN] = {TYPE_4_TAG_ISO_STATUS_SUCCESS};
    uint8_t status[TYPE_4_TAG_ISO_STATUS_LEN] = {
        bit_buffer_get_byte(rx_buf, response_len - 2),
        bit_buffer_get_byte(rx_buf, response_len - 1),
    };
    bit_buffer_set_size_bytes(rx_buf, response_len - 2);

    if(memcmp(status, success, sizeof(status)) == 0) {
        return Type4TagErrorNone;
    } else {
        FURI_LOG_E(TAG, "APDU failed: %02X%02X", status[0], status[1]);
        return Type4TagErrorApduFailed;
    }
}

static Type4TagError type_4_tag_poller_iso_select_name(
    Type4TagPoller* instance,
    const uint8_t* name,
    uint8_t name_len) {
    static const uint8_t type_4_tag_iso_select_name_apdu[] = {
        TYPE_4_TAG_ISO_SELECT_CMD,
        TYPE_4_TAG_ISO_SELECT_P1_BY_NAME,
        TYPE_4_TAG_ISO_SELECT_P2_EMPTY,
    };

    bit_buffer_append_bytes(
        instance->tx_buffer,
        type_4_tag_iso_select_name_apdu,
        sizeof(type_4_tag_iso_select_name_apdu));
    bit_buffer_append_byte(instance->tx_buffer, name_len);
    bit_buffer_append_bytes(instance->tx_buffer, name, name_len);

    Type4TagError error = type_4_tag_apdu_trx(instance, instance->tx_buffer, instance->rx_buffer);
    if(error == Type4TagErrorApduFailed) error = Type4TagErrorCardUnformatted;

    return error;
}

static Type4TagError
    type_4_tag_poller_iso_select_file(Type4TagPoller* instance, uint16_t file_id) {
    static const uint8_t type_4_tag_iso_select_file_apdu[] = {
        TYPE_4_TAG_ISO_SELECT_CMD,
        TYPE_4_TAG_ISO_SELECT_P1_BY_EF_ID,
        TYPE_4_TAG_ISO_SELECT_P2_EMPTY,
        sizeof(file_id),
    };
    uint8_t file_id_be[sizeof(file_id)];
    bit_lib_num_to_bytes_be(file_id, sizeof(file_id), file_id_be);

    bit_buffer_append_bytes(
        instance->tx_buffer,
        type_4_tag_iso_select_file_apdu,
        sizeof(type_4_tag_iso_select_file_apdu));
    bit_buffer_append_bytes(instance->tx_buffer, file_id_be, sizeof(file_id_be));

    Type4TagError error = type_4_tag_apdu_trx(instance, instance->tx_buffer, instance->rx_buffer);
    if(error == Type4TagErrorApduFailed) error = Type4TagErrorCardUnformatted;

    return error;
}

static Type4TagError type_4_tag_poller_iso_read(
    Type4TagPoller* instance,
    uint16_t offset,
    uint16_t length,
    uint8_t* buffer) {
    const uint8_t chunk_max = instance->data->is_tag_specific ?
                                  MIN(instance->data->chunk_max_read, TYPE_4_TAG_CHUNK_LEN) :
                                  TYPE_4_TAG_CHUNK_LEN;
    if(offset + length > TYPE_4_TAG_ISO_READ_P_OFFSET_MAX + chunk_max - sizeof(length)) {
        FURI_LOG_E(TAG, "File too large: %zu bytes", length);
        return Type4TagErrorNotSupported;
    }

    static const uint8_t type_4_tag_iso_read_apdu[] = {
        TYPE_4_TAG_ISO_READ_CMD,
    };

    while(length > 0) {
        uint8_t chunk_len = MIN(length, chunk_max);
        uint8_t offset_be[sizeof(offset)];
        bit_lib_num_to_bytes_be(offset, sizeof(offset_be), offset_be);

        bit_buffer_append_bytes(
            instance->tx_buffer, type_4_tag_iso_read_apdu, sizeof(type_4_tag_iso_read_apdu));
        bit_buffer_append_bytes(instance->tx_buffer, offset_be, sizeof(offset_be));
        bit_buffer_append_byte(instance->tx_buffer, chunk_len);

        Type4TagError error =
            type_4_tag_apdu_trx(instance, instance->tx_buffer, instance->rx_buffer);
        if(error != Type4TagErrorNone) {
            return error;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != chunk_len) {
            FURI_LOG_E(
                TAG,
                "Wrong chunk len: %zu != %zu",
                bit_buffer_get_size_bytes(instance->rx_buffer),
                chunk_len);
            return Type4TagErrorWrongFormat;
        }

        memcpy(buffer, bit_buffer_get_data(instance->rx_buffer), chunk_len);
        buffer += chunk_len;
        offset += chunk_len;
        length -= chunk_len;
    }

    return Type4TagErrorNone;
}

static Type4TagError type_4_tag_poller_iso_write(
    Type4TagPoller* instance,
    uint16_t offset,
    uint16_t length,
    uint8_t* buffer) {
    const uint8_t chunk_max = instance->data->is_tag_specific ?
                                  MIN(instance->data->chunk_max_write, TYPE_4_TAG_CHUNK_LEN) :
                                  TYPE_4_TAG_CHUNK_LEN;
    if(offset + length > TYPE_4_TAG_ISO_READ_P_OFFSET_MAX + chunk_max - sizeof(length)) {
        FURI_LOG_E(TAG, "File too large: %zu bytes", length);
        return Type4TagErrorNotSupported;
    }

    static const uint8_t type_4_tag_iso_write_apdu[] = {
        TYPE_4_TAG_ISO_WRITE_CMD,
    };

    while(length > 0) {
        uint8_t chunk_len = MIN(length, chunk_max);
        uint8_t offset_be[sizeof(offset)];
        bit_lib_num_to_bytes_be(offset, sizeof(offset_be), offset_be);

        bit_buffer_append_bytes(
            instance->tx_buffer, type_4_tag_iso_write_apdu, sizeof(type_4_tag_iso_write_apdu));
        bit_buffer_append_bytes(instance->tx_buffer, offset_be, sizeof(offset_be));
        bit_buffer_append_byte(instance->tx_buffer, chunk_len);
        bit_buffer_append_bytes(instance->tx_buffer, buffer, chunk_len);

        Type4TagError error =
            type_4_tag_apdu_trx(instance, instance->tx_buffer, instance->rx_buffer);
        if(error == Type4TagErrorApduFailed) error = Type4TagErrorCardLocked;
        if(error != Type4TagErrorNone) {
            return error;
        }

        buffer += chunk_len;
        offset += chunk_len;
        length -= chunk_len;
    }

    return Type4TagErrorNone;
}

Type4TagError type_4_tag_poller_detect_platform(Type4TagPoller* instance) {
    furi_check(instance);

    Iso14443_4aPollerEvent iso14443_4a_event = {
        .type = Iso14443_4aPollerEventTypeReady,
        .data = NULL,
    };
    NfcGenericEvent event = {
        .protocol = NfcProtocolIso14443_4a,
        .instance = instance->iso14443_4a_poller,
        .event_data = &iso14443_4a_event,
    };

    Type4TagPlatform platform = Type4TagPlatformUnknown;
    NfcDevice* device = nfc_device_alloc();

    do {
        FURI_LOG_D(TAG, "Detect NTAG4xx");
        Ntag4xxPoller* ntag4xx = ntag4xx_poller.alloc(instance->iso14443_4a_poller);
        if(ntag4xx_poller.detect(event, ntag4xx)) {
            platform = Type4TagPlatformNtag4xx;
            nfc_device_set_data(device, NfcProtocolNtag4xx, ntag4xx_poller.get_data(ntag4xx));
        }
        ntag4xx_poller.free(ntag4xx);
        if(platform != Type4TagPlatformUnknown) break;

        FURI_LOG_D(TAG, "Detect DESFire");
        MfDesfirePoller* mf_desfire = mf_desfire_poller.alloc(instance->iso14443_4a_poller);
        mf_desfire_poller_set_command_mode(mf_desfire, NxpNativeCommandModeIsoWrapped);
        if(mf_desfire_poller.detect(event, mf_desfire)) {
            platform = Type4TagPlatformMfDesfire;
            nfc_device_set_data(
                device, NfcProtocolMfDesfire, mf_desfire_poller.get_data(mf_desfire));
        }
        mf_desfire_poller.free(mf_desfire);
        if(platform != Type4TagPlatformUnknown) break;
    } while(false);

    Type4TagError error;
    if(platform != Type4TagPlatformUnknown) {
        furi_string_set(
            instance->data->platform_name, nfc_device_get_name(device, NfcDeviceNameTypeFull));
        error = Type4TagErrorNone;
    } else {
        furi_string_reset(instance->data->platform_name);
        error = Type4TagErrorNotSupported;
    }
    instance->data->platform = platform;
    nfc_device_free(device);

    return error;
}

Type4TagError type_4_tag_poller_select_app(Type4TagPoller* instance) {
    furi_check(instance);

    FURI_LOG_D(TAG, "Select application");
    return type_4_tag_poller_iso_select_name(
        instance, type_4_tag_iso_df_name, sizeof(type_4_tag_iso_df_name));
}

Type4TagError type_4_tag_poller_read_cc(Type4TagPoller* instance) {
    furi_check(instance);

    Type4TagError error;

    do {
        FURI_LOG_D(TAG, "Select CC");
        error = type_4_tag_poller_iso_select_file(instance, TYPE_4_TAG_T4T_CC_EF_ID);
        if(error != Type4TagErrorNone) break;

        FURI_LOG_D(TAG, "Read CC len");
        uint16_t cc_len;
        uint8_t cc_len_be[sizeof(cc_len)];
        error = type_4_tag_poller_iso_read(instance, 0, sizeof(cc_len_be), cc_len_be);
        if(error != Type4TagErrorNone) break;
        cc_len = bit_lib_bytes_to_num_be(cc_len_be, sizeof(cc_len_be));

        FURI_LOG_D(TAG, "Read CC");
        uint8_t cc_buf[cc_len];
        error = type_4_tag_poller_iso_read(instance, 0, sizeof(cc_buf), cc_buf);
        if(error != Type4TagErrorNone) break;

        error = type_4_tag_cc_parse(instance->data, cc_buf, sizeof(cc_buf));
        if(error != Type4TagErrorNone) break;
        instance->data->is_tag_specific = true;

        FURI_LOG_D(TAG, "Detected NDEF file ID %04X", instance->data->ndef_file_id);
    } while(false);

    return error;
}

Type4TagError type_4_tag_poller_read_ndef(Type4TagPoller* instance) {
    furi_check(instance);

    Type4TagError error;

    do {
        FURI_LOG_D(TAG, "Select NDEF");
        error = type_4_tag_poller_iso_select_file(instance, instance->data->ndef_file_id);
        if(error != Type4TagErrorNone) break;

        FURI_LOG_D(TAG, "Read NDEF len");
        uint16_t ndef_len;
        uint8_t ndef_len_be[sizeof(ndef_len)];
        error = type_4_tag_poller_iso_read(instance, 0, sizeof(ndef_len_be), ndef_len_be);
        if(error != Type4TagErrorNone) break;
        ndef_len = bit_lib_bytes_to_num_be(ndef_len_be, sizeof(ndef_len_be));

        if(ndef_len == 0) {
            FURI_LOG_D(TAG, "NDEF file is empty");
            break;
        }

        FURI_LOG_D(TAG, "Read NDEF");
        simple_array_init(instance->data->ndef_data, ndef_len);
        uint8_t* ndef_buf = simple_array_get_data(instance->data->ndef_data);
        error = type_4_tag_poller_iso_read(instance, sizeof(ndef_len), ndef_len, ndef_buf);
        if(error != Type4TagErrorNone) break;

        FURI_LOG_D(
            TAG, "Read %hu bytes from NDEF file %04X", ndef_len, instance->data->ndef_file_id);
    } while(false);

    return error;
}

Type4TagError type_4_tag_poller_create_app(Type4TagPoller* instance) {
    Type4TagError error = Type4TagErrorNotSupported;

    if(instance->data->platform == Type4TagPlatformMfDesfire) {
        MfDesfirePoller* mf_des = mf_desfire_poller.alloc(instance->iso14443_4a_poller);
        mf_desfire_poller_set_command_mode(mf_des, NxpNativeCommandModeIsoWrapped);
        MfDesfireError mf_des_error;

        do {
            FURI_LOG_D(TAG, "Select DESFire PICC");
            mf_des_error = mf_desfire_poller_select_application(mf_des, &mf_des_picc_app_id);
            if(mf_des_error != MfDesfireErrorNone) {
                error = Type4TagErrorProtocol;
                break;
            }

            FURI_LOG_D(TAG, "Create DESFire T4T app");
            mf_des_error = mf_desfire_poller_create_application(
                mf_des,
                &mf_des_t4t_app_id,
                &mf_des_t4t_app_key_settings,
                TYPE_4_TAG_ISO_DF_ID,
                type_4_tag_iso_df_name,
                sizeof(type_4_tag_iso_df_name));
            if(mf_des_error != MfDesfireErrorNone) {
                if(mf_des_error != MfDesfireErrorNotPresent &&
                   mf_des_error != MfDesfireErrorTimeout) {
                    error = Type4TagErrorCardLocked;
                } else {
                    error = Type4TagErrorProtocol;
                }
                break;
            }

            error = Type4TagErrorNone;
        } while(false);

        mf_desfire_poller.free(mf_des);
    }

    return error;
}

Type4TagError type_4_tag_poller_create_cc(Type4TagPoller* instance) {
    Type4TagError error = Type4TagErrorNotSupported;

    if(instance->data->platform == Type4TagPlatformMfDesfire) {
        MfDesfirePoller* mf_des = mf_desfire_poller.alloc(instance->iso14443_4a_poller);
        mf_desfire_poller_set_command_mode(mf_des, NxpNativeCommandModeIsoWrapped);
        MfDesfireError mf_des_error;

        do {
            FURI_LOG_D(TAG, "Create DESFire CC");
            mf_des_error = mf_desfire_poller_create_file(
                mf_des, MF_DES_T4T_CC_FILE_ID, &mf_des_t4t_cc_file, TYPE_4_TAG_T4T_CC_EF_ID);
            if(mf_des_error != MfDesfireErrorNone) {
                if(mf_des_error != MfDesfireErrorNotPresent &&
                   mf_des_error != MfDesfireErrorTimeout) {
                    error = Type4TagErrorCardLocked;
                } else {
                    error = Type4TagErrorProtocol;
                }
                break;
            }

            FURI_LOG_D(TAG, "Select CC");
            error = type_4_tag_poller_iso_select_file(instance, TYPE_4_TAG_T4T_CC_EF_ID);
            if(error != Type4TagErrorNone) break;

            FURI_LOG_D(TAG, "Write DESFire CC");
            instance->data->t4t_version.value = TYPE_4_TAG_T4T_CC_VNO;
            instance->data->chunk_max_read = 0x3A;
            instance->data->chunk_max_write = 0x34;
            instance->data->ndef_file_id = TYPE_4_TAG_T4T_NDEF_EF_ID;
            instance->data->ndef_max_len = TYPE_4_TAG_MF_DESFIRE_NDEF_SIZE;
            instance->data->ndef_read_lock = TYPE_4_TAG_T4T_CC_RW_LOCK_NONE;
            instance->data->ndef_write_lock = TYPE_4_TAG_T4T_CC_RW_LOCK_NONE;
            instance->data->is_tag_specific = true;
            uint8_t cc_buf[TYPE_4_TAG_T4T_CC_MIN_SIZE];
            type_4_tag_cc_dump(instance->data, cc_buf, sizeof(cc_buf));
            error = type_4_tag_poller_iso_write(instance, 0, sizeof(cc_buf), cc_buf);
            if(error != Type4TagErrorNone) break;

            error = Type4TagErrorNone;
        } while(false);

        mf_desfire_poller.free(mf_des);
    }

    return error;
}

Type4TagError type_4_tag_poller_create_ndef(Type4TagPoller* instance) {
    Type4TagError error = Type4TagErrorNotSupported;

    if(instance->data->platform == Type4TagPlatformMfDesfire) {
        MfDesfirePoller* mf_des = mf_desfire_poller.alloc(instance->iso14443_4a_poller);
        mf_desfire_poller_set_command_mode(mf_des, NxpNativeCommandModeIsoWrapped);
        MfDesfireError mf_des_error;

        do {
            FURI_LOG_D(TAG, "Create DESFire NDEF");
            MfDesfireFileSettings mf_des_t4t_ndef_file = mf_des_t4t_ndef_file_default;
            mf_des_t4t_ndef_file.data.size = sizeof(uint16_t) + (instance->data->is_tag_specific ?
                                                                     instance->data->ndef_max_len :
                                                                     TYPE_4_TAG_DEFAULT_NDEF_SIZE);
            mf_des_error = mf_desfire_poller_create_file(
                mf_des, MF_DES_T4T_NDEF_FILE_ID, &mf_des_t4t_ndef_file, TYPE_4_TAG_T4T_NDEF_EF_ID);
            if(mf_des_error != MfDesfireErrorNone) {
                if(mf_des_error != MfDesfireErrorNotPresent &&
                   mf_des_error != MfDesfireErrorTimeout) {
                    error = Type4TagErrorCardLocked;
                } else {
                    error = Type4TagErrorProtocol;
                }
                break;
            }

            error = Type4TagErrorNone;
        } while(false);

        mf_desfire_poller.free(mf_des);
    }

    return error;
}

Type4TagError type_4_tag_poller_write_ndef(Type4TagPoller* instance) {
    furi_check(instance);

    Type4TagError error;

    do {
        FURI_LOG_D(TAG, "Select NDEF");
        error = type_4_tag_poller_iso_select_file(instance, instance->data->ndef_file_id);
        if(error != Type4TagErrorNone) break;

        FURI_LOG_D(TAG, "Write NDEF len");
        uint16_t ndef_len = simple_array_get_count(instance->data->ndef_data);
        uint8_t ndef_len_be[sizeof(ndef_len)];
        bit_lib_num_to_bytes_be(ndef_len, sizeof(ndef_len_be), ndef_len_be);
        error = type_4_tag_poller_iso_write(instance, 0, sizeof(ndef_len_be), ndef_len_be);
        if(error != Type4TagErrorNone) break;

        if(ndef_len == 0) {
            FURI_LOG_D(TAG, "NDEF file is empty");
            break;
        }

        FURI_LOG_D(TAG, "Write NDEF");
        uint8_t* ndef_buf = simple_array_get_data(instance->data->ndef_data);
        error = type_4_tag_poller_iso_write(instance, sizeof(ndef_len), ndef_len, ndef_buf);
        if(error != Type4TagErrorNone) break;

        FURI_LOG_D(
            TAG, "Wrote %hu bytes to NDEF file %04X", ndef_len, instance->data->ndef_file_id);
    } while(false);

    return error;
}
