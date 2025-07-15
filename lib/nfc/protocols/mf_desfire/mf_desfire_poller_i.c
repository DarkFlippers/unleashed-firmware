#include "mf_desfire_poller_i.h"

#include <furi.h>
#include <bit_lib/bit_lib.h>

#include "mf_desfire_i.h"

#define TAG "MfDesfirePoller"

MfDesfireError mf_desfire_process_error(Iso14443_4aError error) {
    switch(error) {
    case Iso14443_4aErrorNone:
        return MfDesfireErrorNone;
    case Iso14443_4aErrorNotPresent:
        return MfDesfireErrorNotPresent;
    case Iso14443_4aErrorTimeout:
        return MfDesfireErrorTimeout;
    default:
        return MfDesfireErrorProtocol;
    }
}

MfDesfireError mf_desfire_process_status_code(uint8_t status_code) {
    switch(status_code) {
    case NXP_NATIVE_COMMAND_STATUS_OPERATION_OK:
        return MfDesfireErrorNone;
    case NXP_NATIVE_COMMAND_STATUS_AUTHENTICATION_ERROR:
        return MfDesfireErrorAuthentication;
    case NXP_NATIVE_COMMAND_STATUS_ILLEGAL_COMMAND_CODE:
        return MfDesfireErrorCommandNotSupported;
    default:
        return MfDesfireErrorProtocol;
    }
}

void mf_desfire_poller_set_command_mode(
    MfDesfirePoller* instance,
    NxpNativeCommandMode command_mode) {
    furi_check(instance);
    furi_check(instance->state == MfDesfirePollerStateIdle);
    furi_check(command_mode < NxpNativeCommandModeMAX);

    instance->command_mode = command_mode;
}

MfDesfireError mf_desfire_poller_send_chunks(
    MfDesfirePoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    furi_check(instance);

    NxpNativeCommandStatus status_code = NXP_NATIVE_COMMAND_STATUS_OPERATION_OK;
    Iso14443_4aError iso14443_4a_error = nxp_native_command_iso14443_4a_poller(
        instance->iso14443_4a_poller,
        &status_code,
        tx_buffer,
        rx_buffer,
        instance->command_mode,
        instance->tx_buffer,
        instance->rx_buffer);

    if(iso14443_4a_error != Iso14443_4aErrorNone) {
        return mf_desfire_process_error(iso14443_4a_error);
    }

    return mf_desfire_process_status_code(status_code);
}

MfDesfireError mf_desfire_poller_read_version(MfDesfirePoller* instance, MfDesfireVersion* data) {
    furi_check(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_VERSION);

    MfDesfireError error;

    do {
        error = mf_desfire_poller_send_chunks(
            instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        if(!mf_desfire_version_parse(data, instance->result_buffer)) {
            error = MfDesfireErrorProtocol;
        }
    } while(false);

    return error;
}

MfDesfireError
    mf_desfire_poller_read_free_memory(MfDesfirePoller* instance, MfDesfireFreeMemory* data) {
    furi_check(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_FREE_MEMORY);

    MfDesfireError error;

    do {
        error = mf_desfire_poller_send_chunks(
            instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        if(!mf_desfire_free_memory_parse(data, instance->result_buffer)) {
            error = MfDesfireErrorNotPresent;
        }
    } while(false);

    return error;
}

MfDesfireError
    mf_desfire_poller_read_key_settings(MfDesfirePoller* instance, MfDesfireKeySettings* data) {
    furi_check(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_KEY_SETTINGS);

    MfDesfireError error;

    do {
        error = mf_desfire_poller_send_chunks(
            instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        if(!mf_desfire_key_settings_parse(data, instance->result_buffer)) {
            error = MfDesfireErrorProtocol;
        }
    } while(false);

    return error;
}

MfDesfireError mf_desfire_poller_read_key_version(
    MfDesfirePoller* instance,
    uint8_t key_num,
    MfDesfireKeyVersion* data) {
    furi_check(instance);
    furi_check(data);

    bit_buffer_set_size_bytes(instance->input_buffer, sizeof(uint8_t) * 2);
    bit_buffer_set_byte(instance->input_buffer, 0, MF_DESFIRE_CMD_GET_KEY_VERSION);
    bit_buffer_set_byte(instance->input_buffer, 1, key_num);

    MfDesfireError error =
        mf_desfire_poller_send_chunks(instance, instance->input_buffer, instance->result_buffer);
    if(error == MfDesfireErrorNone) {
        if(!mf_desfire_key_version_parse(data, instance->result_buffer)) {
            error = MfDesfireErrorProtocol;
        }
    }

    return error;
}

MfDesfireError mf_desfire_poller_read_key_versions(
    MfDesfirePoller* instance,
    SimpleArray* data,
    uint32_t count) {
    furi_check(instance);
    furi_check(count > 0);

    simple_array_init(data, count);

    MfDesfireError error = MfDesfireErrorNone;

    for(uint32_t i = 0; i < count; ++i) {
        error = mf_desfire_poller_read_key_version(instance, i, simple_array_get(data, i));
        if(error != MfDesfireErrorNone) break;
    }

    return error;
}

MfDesfireError
    mf_desfire_poller_read_application_ids(MfDesfirePoller* instance, SimpleArray* data) {
    furi_check(instance);
    furi_check(data);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_APPLICATION_IDS);

    MfDesfireError error;

    do {
        error = mf_desfire_poller_send_chunks(
            instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        const uint32_t app_id_count =
            bit_buffer_get_size_bytes(instance->result_buffer) / sizeof(MfDesfireApplicationId);
        if(app_id_count == 0) break;

        simple_array_init(data, app_id_count);

        for(uint32_t i = 0; i < app_id_count; ++i) {
            if(!mf_desfire_application_id_parse(
                   simple_array_get(data, i), i, instance->result_buffer)) {
                error = MfDesfireErrorProtocol;
                break;
            }
        }
    } while(false);

    return error;
}

MfDesfireError mf_desfire_poller_select_application(
    MfDesfirePoller* instance,
    const MfDesfireApplicationId* id) {
    furi_check(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_SELECT_APPLICATION);
    bit_buffer_append_bytes(
        instance->input_buffer, (const uint8_t*)id, sizeof(MfDesfireApplicationId));

    MfDesfireError error =
        mf_desfire_poller_send_chunks(instance, instance->input_buffer, instance->result_buffer);

    return error;
}

MfDesfireError mf_desfire_poller_read_file_ids(MfDesfirePoller* instance, SimpleArray* data) {
    furi_check(instance);
    furi_check(data);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_FILE_IDS);

    MfDesfireError error;

    do {
        error = mf_desfire_poller_send_chunks(
            instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        const uint32_t id_count =
            bit_buffer_get_size_bytes(instance->result_buffer) / sizeof(MfDesfireFileId);

        if(id_count == 0) break;
        simple_array_init(data, id_count);

        for(uint32_t i = 0; i < id_count; ++i) {
            if(!mf_desfire_file_id_parse(simple_array_get(data, i), i, instance->result_buffer)) {
                error = MfDesfireErrorProtocol;
                break;
            }
        }
    } while(false);

    return error;
}

MfDesfireError mf_desfire_poller_read_file_settings(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    MfDesfireFileSettings* data) {
    furi_check(instance);
    furi_check(data);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_FILE_SETTINGS);
    bit_buffer_append_byte(instance->input_buffer, id);

    MfDesfireError error;

    do {
        error = mf_desfire_poller_send_chunks(
            instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        if(!mf_desfire_file_settings_parse(data, instance->result_buffer)) {
            error = MfDesfireErrorProtocol;
        }
    } while(false);

    return error;
}

MfDesfireError mf_desfire_poller_read_file_settings_multi(
    MfDesfirePoller* instance,
    const SimpleArray* file_ids,
    SimpleArray* data) {
    furi_check(instance);
    furi_check(file_ids);
    furi_check(data);

    MfDesfireError error = MfDesfireErrorNone;

    const uint32_t file_id_count = simple_array_get_count(file_ids);
    if(file_id_count > 0) {
        simple_array_init(data, file_id_count);
    }

    for(uint32_t i = 0; i < file_id_count; ++i) {
        const MfDesfireFileId file_id = *(const MfDesfireFileId*)simple_array_cget(file_ids, i);
        error = mf_desfire_poller_read_file_settings(instance, file_id, simple_array_get(data, i));
        if(error != MfDesfireErrorNone) break;
    }

    return error;
}

MfDesfireError mf_desfire_poller_create_application(
    MfDesfirePoller* instance,
    const MfDesfireApplicationId* id,
    const MfDesfireKeySettings* key_settings,
    uint16_t iso_df_id,
    const uint8_t* iso_df_name,
    uint8_t iso_df_name_len) {
    furi_check(instance);
    furi_check(key_settings);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_CREATE_APPLICATION);
    bit_buffer_append_bytes(
        instance->input_buffer, (const uint8_t*)id, sizeof(MfDesfireApplicationId));
    mf_desfire_key_settings_dump(key_settings, instance->input_buffer);

    if(iso_df_name && iso_df_name_len) {
        uint8_t ks2_pos = bit_buffer_get_size_bytes(instance->input_buffer) - 1;
        uint8_t ks2 = bit_buffer_get_byte(instance->input_buffer, ks2_pos);
        ks2 |= (1 << 5); // Mark file id present
        bit_buffer_set_byte(instance->input_buffer, ks2_pos, ks2);

        uint8_t iso_df_id_le[sizeof(iso_df_id)];
        bit_lib_num_to_bytes_le(iso_df_id, sizeof(iso_df_id_le), iso_df_id_le);
        bit_buffer_append_bytes(instance->input_buffer, iso_df_id_le, sizeof(iso_df_id_le));

        bit_buffer_append_bytes(instance->input_buffer, iso_df_name, iso_df_name_len);
    }

    MfDesfireError error =
        mf_desfire_poller_send_chunks(instance, instance->input_buffer, instance->result_buffer);

    return error;
}

MfDesfireError mf_desfire_poller_create_file(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    const MfDesfireFileSettings* data,
    uint16_t iso_ef_id) {
    furi_check(instance);
    furi_check(data);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(
        instance->input_buffer,
        data->type == MfDesfireFileTypeStandard     ? MF_DESFIRE_CMD_CREATE_STD_DATA_FILE :
        data->type == MfDesfireFileTypeBackup       ? MF_DESFIRE_CMD_CREATE_BACKUP_DATA_FILE :
        data->type == MfDesfireFileTypeValue        ? MF_DESFIRE_CMD_CREATE_VALUE_FILE :
        data->type == MfDesfireFileTypeLinearRecord ? MF_DESFIRE_CMD_CREATE_LINEAR_RECORD_FILE :
        data->type == MfDesfireFileTypeCyclicRecord ? MF_DESFIRE_CMD_CREATE_CYCLIC_RECORD_FILE :
                                                      0x00);
    bit_buffer_append_byte(instance->input_buffer, id);
    if(iso_ef_id) {
        uint8_t iso_ef_id_le[sizeof(iso_ef_id)];
        bit_lib_num_to_bytes_le(iso_ef_id, sizeof(iso_ef_id_le), iso_ef_id_le);
        bit_buffer_append_bytes(instance->input_buffer, iso_ef_id_le, sizeof(iso_ef_id_le));
    }
    bit_buffer_append_byte(instance->input_buffer, data->comm);
    bit_buffer_append_bytes(
        instance->input_buffer,
        (const uint8_t*)data->access_rights,
        sizeof(MfDesfireFileAccessRights) * data->access_rights_len);

    if(data->type == MfDesfireFileTypeStandard || data->type == MfDesfireFileTypeBackup) {
        uint8_t data_size_le[3 * sizeof(uint8_t)];
        bit_lib_num_to_bytes_le(data->data.size, sizeof(data_size_le), data_size_le);
        bit_buffer_append_bytes(instance->input_buffer, data_size_le, sizeof(data_size_le));

    } else if(data->type == MfDesfireFileTypeValue) {
        uint8_t lo_limit_le[sizeof(data->value.lo_limit)];
        bit_lib_num_to_bytes_le(data->value.lo_limit, sizeof(lo_limit_le), lo_limit_le);
        bit_buffer_append_bytes(instance->input_buffer, lo_limit_le, sizeof(lo_limit_le));

        uint8_t hi_limit_le[sizeof(data->value.hi_limit)];
        bit_lib_num_to_bytes_le(data->value.hi_limit, sizeof(hi_limit_le), hi_limit_le);
        bit_buffer_append_bytes(instance->input_buffer, hi_limit_le, sizeof(hi_limit_le));

        uint8_t value_le[sizeof(data->value.limited_credit_value)];
        bit_lib_num_to_bytes_le(data->value.limited_credit_value, sizeof(value_le), value_le);
        bit_buffer_append_bytes(instance->input_buffer, value_le, sizeof(value_le));

        bit_buffer_append_byte(instance->input_buffer, data->value.limited_credit_enabled);

    } else if(
        data->type == MfDesfireFileTypeLinearRecord ||
        data->type == MfDesfireFileTypeCyclicRecord) {
        uint8_t record_size_le[3 * sizeof(uint8_t)];
        bit_lib_num_to_bytes_le(data->record.size, sizeof(record_size_le), record_size_le);
        bit_buffer_append_bytes(instance->input_buffer, record_size_le, sizeof(record_size_le));

        uint8_t record_max_le[3 * sizeof(uint8_t)];
        bit_lib_num_to_bytes_le(data->record.max, sizeof(record_max_le), record_max_le);
        bit_buffer_append_bytes(instance->input_buffer, record_max_le, sizeof(record_max_le));
    }

    MfDesfireError error =
        mf_desfire_poller_send_chunks(instance, instance->input_buffer, instance->result_buffer);

    return error;
}

static MfDesfireError mf_desfire_poller_read_file(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    uint8_t read_cmd,
    uint32_t offset,
    size_t size,
    MfDesfireFileData* data) {
    furi_check(instance);
    furi_check(data);

    MfDesfireError error = MfDesfireErrorNone;
    simple_array_init(data->data, size);

    size_t buffer_capacity = bit_buffer_get_capacity_bytes(instance->result_buffer);
    uint32_t current_offset = offset;
    uint32_t bytes_read = 0;

    while(bytes_read < size) {
        size_t bytes_to_read = MIN(buffer_capacity, size - bytes_read);
        bit_buffer_reset(instance->input_buffer);
        bit_buffer_append_byte(instance->input_buffer, read_cmd);
        bit_buffer_append_byte(instance->input_buffer, id);
        bit_buffer_append_bytes(instance->input_buffer, (const uint8_t*)&current_offset, 3);
        bit_buffer_append_bytes(instance->input_buffer, (const uint8_t*)&bytes_to_read, 3);

        error = mf_desfire_poller_send_chunks(
            instance, instance->input_buffer, instance->result_buffer);
        if(error != MfDesfireErrorNone) break;

        size_t bytes_received = bit_buffer_get_size_bytes(instance->result_buffer);
        if(bytes_received != bytes_to_read) {
            FURI_LOG_W(TAG, "Read %zu out of %zu bytes", bytes_received, bytes_to_read);
            error = MfDesfireErrorProtocol;
            break;
        }

        uint8_t* file_data = simple_array_get_data(data->data);
        bit_buffer_write_bytes(instance->result_buffer, &file_data[current_offset], bytes_to_read);
        bytes_read += bytes_to_read;
        current_offset += bytes_to_read;
    }

    if(error != MfDesfireErrorNone) {
        simple_array_reset(data->data);
    }

    return error;
}

MfDesfireError mf_desfire_poller_read_file_data(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    uint32_t offset,
    size_t size,
    MfDesfireFileData* data) {
    return mf_desfire_poller_read_file(instance, id, MF_DESFIRE_CMD_READ_DATA, offset, size, data);
}

MfDesfireError mf_desfire_poller_read_file_value(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    MfDesfireFileData* data) {
    furi_check(instance);
    furi_check(data);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_VALUE);
    bit_buffer_append_byte(instance->input_buffer, id);

    MfDesfireError error;

    do {
        error = mf_desfire_poller_send_chunks(
            instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        if(!mf_desfire_file_data_parse(data, instance->result_buffer)) {
            error = MfDesfireErrorProtocol;
        }
    } while(false);

    return error;
}

MfDesfireError mf_desfire_poller_read_file_records(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    uint32_t offset,
    size_t size,
    MfDesfireFileData* data) {
    return mf_desfire_poller_read_file(
        instance, id, MF_DESFIRE_CMD_READ_RECORDS, offset, size, data);
}

MfDesfireError mf_desfire_poller_read_file_data_multi(
    MfDesfirePoller* instance,
    const SimpleArray* file_ids,
    const SimpleArray* file_settings,
    SimpleArray* data) {
    furi_check(instance);
    furi_check(file_ids);
    furi_check(file_settings);
    furi_check(data);
    furi_check(simple_array_get_count(file_ids) == simple_array_get_count(file_settings));

    MfDesfireError error = MfDesfireErrorNone;

    const uint32_t file_id_count = simple_array_get_count(file_ids);
    if(file_id_count > 0) {
        simple_array_init(data, file_id_count);
    }

    for(size_t i = 0; i < file_id_count; ++i) {
        const MfDesfireFileId file_id = *(const MfDesfireFileId*)simple_array_cget(file_ids, i);
        const MfDesfireFileSettings* file_settings_cur = simple_array_cget(file_settings, i);
        const MfDesfireFileType file_type = file_settings_cur->type;

        MfDesfireFileData* file_data = simple_array_get(data, i);

        bool can_read_data = false;
        for(size_t j = 0; j < file_settings_cur->access_rights_len; j++) {
            uint8_t read_access = (file_settings_cur->access_rights[j] >> 12) & 0x0f;
            uint8_t read_write_access = (file_settings_cur->access_rights[j] >> 4) & 0x0f;
            can_read_data = (read_access == 0x0e) || (read_write_access == 0x0e);
            if(can_read_data) break;
        }
        if(!can_read_data) {
            FURI_LOG_D(TAG, "Can't read file %zu data without authentication", i);
            continue;
        }

        if(file_type == MfDesfireFileTypeStandard || file_type == MfDesfireFileTypeBackup) {
            error = mf_desfire_poller_read_file_data(
                instance, file_id, 0, file_settings_cur->data.size, file_data);
        } else if(file_type == MfDesfireFileTypeValue) {
            error = mf_desfire_poller_read_file_value(instance, file_id, file_data);
        } else if(
            file_type == MfDesfireFileTypeLinearRecord ||
            file_type == MfDesfireFileTypeCyclicRecord) {
            error = mf_desfire_poller_read_file_records(
                instance, file_id, 0, file_settings_cur->record.size, file_data);
        }
    }

    return error;
}

MfDesfireError
    mf_desfire_poller_read_application(MfDesfirePoller* instance, MfDesfireApplication* data) {
    furi_check(instance);
    furi_check(data);

    MfDesfireError error;

    do {
        error = mf_desfire_poller_read_key_settings(instance, &data->key_settings);
        if(error == MfDesfireErrorAuthentication) {
            FURI_LOG_D(TAG, "Auth is required to read master key settings and app ids");
            data->key_settings.is_free_directory_list = false;
            error = MfDesfireErrorNone;
            break;
        }
        if(error != MfDesfireErrorNone) break;

        error = mf_desfire_poller_read_key_versions(
            instance, data->key_versions, data->key_settings.max_keys);
        if(error != MfDesfireErrorNone) {
            FURI_LOG_E(TAG, "Failed to read key version: %d", error);
            break;
        }

        error = mf_desfire_poller_read_file_ids(instance, data->file_ids);
        if(error != MfDesfireErrorNone) {
            FURI_LOG_E(TAG, "Failed to read file ids: %d", error);
            break;
        }

        error = mf_desfire_poller_read_file_settings_multi(
            instance, data->file_ids, data->file_settings);
        if(error != MfDesfireErrorNone) {
            FURI_LOG_E(TAG, "Failed to read file settings: %d", error);
            break;
        }

        error = mf_desfire_poller_read_file_data_multi(
            instance, data->file_ids, data->file_settings, data->file_data);

    } while(false);

    return error;
}

MfDesfireError mf_desfire_poller_read_applications(
    MfDesfirePoller* instance,
    const SimpleArray* app_ids,
    SimpleArray* data) {
    furi_check(instance);
    furi_check(data);

    MfDesfireError error = MfDesfireErrorNone;

    const uint32_t app_id_count = simple_array_get_count(app_ids);
    if(app_id_count > 0) {
        simple_array_init(data, app_id_count);
    }

    for(size_t i = 0; i < app_id_count; ++i) {
        do {
            FURI_LOG_D(TAG, "Selecting app %zu", i);
            error = mf_desfire_poller_select_application(instance, simple_array_cget(app_ids, i));
            if(error != MfDesfireErrorNone) break;

            FURI_LOG_D(TAG, "Reading app %zu", i);
            MfDesfireApplication* current_app = simple_array_get(data, i);
            error = mf_desfire_poller_read_application(instance, current_app);

        } while(false);
    }

    return error;
}
