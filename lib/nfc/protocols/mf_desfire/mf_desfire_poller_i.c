#include "mf_desfire_poller_i.h"

#include <furi.h>

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

MfDesfireError mf_desfire_send_chunks(
    MfDesfirePoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    furi_assert(instance);
    furi_assert(instance->iso14443_4a_poller);
    furi_assert(instance->tx_buffer);
    furi_assert(instance->rx_buffer);
    furi_assert(tx_buffer);
    furi_assert(rx_buffer);

    MfDesfireError error = MfDesfireErrorNone;

    do {
        Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block(
            instance->iso14443_4a_poller, tx_buffer, instance->rx_buffer);

        if(iso14443_4a_error != Iso14443_4aErrorNone) {
            error = mf_desfire_process_error(iso14443_4a_error);
            break;
        }

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_append_byte(instance->tx_buffer, MF_DESFIRE_FLAG_HAS_NEXT);

        if(bit_buffer_get_size_bytes(instance->rx_buffer) > sizeof(uint8_t)) {
            bit_buffer_copy_right(rx_buffer, instance->rx_buffer, sizeof(uint8_t));
        } else {
            bit_buffer_reset(rx_buffer);
        }

        while(bit_buffer_starts_with_byte(instance->rx_buffer, MF_DESFIRE_FLAG_HAS_NEXT)) {
            Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block(
                instance->iso14443_4a_poller, instance->tx_buffer, instance->rx_buffer);

            if(iso14443_4a_error != Iso14443_4aErrorNone) {
                error = mf_desfire_process_error(iso14443_4a_error);
                break;
            }

            const size_t rx_size = bit_buffer_get_size_bytes(instance->rx_buffer);
            const size_t rx_capacity_remaining =
                bit_buffer_get_capacity_bytes(rx_buffer) - bit_buffer_get_size_bytes(rx_buffer);

            if(rx_size <= rx_capacity_remaining) {
                bit_buffer_append_right(rx_buffer, instance->rx_buffer, sizeof(uint8_t));
            } else {
                FURI_LOG_W(TAG, "RX buffer overflow: ignoring %zu bytes", rx_size);
            }
        }
    } while(false);

    return error;
}

MfDesfireError mf_desfire_poller_read_version(MfDesfirePoller* instance, MfDesfireVersion* data) {
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_VERSION);

    MfDesfireError error;

    do {
        error = mf_desfire_send_chunks(instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        if(!mf_desfire_version_parse(data, instance->result_buffer)) {
            error = MfDesfireErrorProtocol;
        }
    } while(false);

    return error;
}

MfDesfireError
    mf_desfire_poller_read_free_memory(MfDesfirePoller* instance, MfDesfireFreeMemory* data) {
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_FREE_MEMORY);

    MfDesfireError error;

    do {
        error = mf_desfire_send_chunks(instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        if(!mf_desfire_free_memory_parse(data, instance->result_buffer)) {
            error = MfDesfireErrorProtocol;
        }
    } while(false);

    return error;
}

MfDesfireError
    mf_desfire_poller_read_key_settings(MfDesfirePoller* instance, MfDesfireKeySettings* data) {
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_KEY_SETTINGS);

    MfDesfireError error;

    do {
        error = mf_desfire_send_chunks(instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        if(!mf_desfire_key_settings_parse(data, instance->result_buffer)) {
            error = MfDesfireErrorProtocol;
        }
    } while(false);

    return error;
}

MfDesfireError mf_desfire_poller_read_key_versions(
    MfDesfirePoller* instance,
    SimpleArray* data,
    uint32_t count) {
    furi_assert(instance);
    furi_assert(count > 0);

    simple_array_init(data, count);

    bit_buffer_set_size_bytes(instance->input_buffer, sizeof(uint8_t) * 2);
    bit_buffer_set_byte(instance->input_buffer, 0, MF_DESFIRE_CMD_GET_KEY_VERSION);

    MfDesfireError error = MfDesfireErrorNone;

    for(uint32_t i = 0; i < count; ++i) {
        bit_buffer_set_byte(instance->input_buffer, 1, i);

        error = mf_desfire_send_chunks(instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        if(!mf_desfire_key_version_parse(simple_array_get(data, i), instance->result_buffer)) {
            error = MfDesfireErrorProtocol;
            break;
        }
    }

    return error;
}

MfDesfireError
    mf_desfire_poller_read_application_ids(MfDesfirePoller* instance, SimpleArray* data) {
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_APPLICATION_IDS);

    MfDesfireError error;

    do {
        error = mf_desfire_send_chunks(instance, instance->input_buffer, instance->result_buffer);

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
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_SELECT_APPLICATION);
    bit_buffer_append_bytes(
        instance->input_buffer, (const uint8_t*)id, sizeof(MfDesfireApplicationId));

    MfDesfireError error =
        mf_desfire_send_chunks(instance, instance->input_buffer, instance->result_buffer);

    return error;
}

MfDesfireError mf_desfire_poller_read_file_ids(MfDesfirePoller* instance, SimpleArray* data) {
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_FILE_IDS);

    MfDesfireError error;

    do {
        error = mf_desfire_send_chunks(instance, instance->input_buffer, instance->result_buffer);

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
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_FILE_SETTINGS);
    bit_buffer_append_byte(instance->input_buffer, id);

    MfDesfireError error;

    do {
        error = mf_desfire_send_chunks(instance, instance->input_buffer, instance->result_buffer);

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
    furi_assert(instance);

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

MfDesfireError mf_desfire_poller_read_file_data(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    uint32_t offset,
    size_t size,
    MfDesfireFileData* data) {
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_READ_DATA);
    bit_buffer_append_byte(instance->input_buffer, id);
    bit_buffer_append_bytes(instance->input_buffer, (const uint8_t*)&offset, 3);
    bit_buffer_append_bytes(instance->input_buffer, (const uint8_t*)&size, 3);

    MfDesfireError error;

    do {
        error = mf_desfire_send_chunks(instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        if(!mf_desfire_file_data_parse(data, instance->result_buffer)) {
            error = MfDesfireErrorProtocol;
        }
    } while(false);

    return error;
}

MfDesfireError mf_desfire_poller_read_file_value(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    MfDesfireFileData* data) {
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_GET_VALUE);
    bit_buffer_append_byte(instance->input_buffer, id);

    MfDesfireError error;

    do {
        error = mf_desfire_send_chunks(instance, instance->input_buffer, instance->result_buffer);

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
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_DESFIRE_CMD_READ_RECORDS);
    bit_buffer_append_byte(instance->input_buffer, id);
    bit_buffer_append_bytes(instance->input_buffer, (const uint8_t*)&offset, 3);
    bit_buffer_append_bytes(instance->input_buffer, (const uint8_t*)&size, 3);

    MfDesfireError error;

    do {
        error = mf_desfire_send_chunks(instance, instance->input_buffer, instance->result_buffer);

        if(error != MfDesfireErrorNone) break;

        if(!mf_desfire_file_data_parse(data, instance->result_buffer)) {
            error = MfDesfireErrorProtocol;
        }
    } while(false);

    return error;
}

MfDesfireError mf_desfire_poller_read_file_data_multi(
    MfDesfirePoller* instance,
    const SimpleArray* file_ids,
    const SimpleArray* file_settings,
    SimpleArray* data) {
    furi_assert(instance);
    furi_assert(simple_array_get_count(file_ids) == simple_array_get_count(file_settings));

    MfDesfireError error = MfDesfireErrorNone;

    const uint32_t file_id_count = simple_array_get_count(file_ids);
    if(file_id_count > 0) {
        simple_array_init(data, file_id_count);
    }

    for(uint32_t i = 0; i < file_id_count; ++i) {
        const MfDesfireFileId file_id = *(const MfDesfireFileId*)simple_array_cget(file_ids, i);
        const MfDesfireFileSettings* file_settings_cur = simple_array_cget(file_settings, i);
        const MfDesfireFileType file_type = file_settings_cur->type;

        MfDesfireFileData* file_data = simple_array_get(data, i);

        if(file_type == MfDesfireFileTypeStandard || file_type == MfDesfireFileTypeBackup) {
            error = mf_desfire_poller_read_file_data(
                instance, file_id, 0, file_settings_cur->data.size, file_data);
        } else if(file_type == MfDesfireFileTypeValue) {
            error = mf_desfire_poller_read_file_value(instance, file_id, file_data);
        } else if(
            file_type == MfDesfireFileTypeLinearRecord ||
            file_type == MfDesfireFileTypeCyclicRecord) {
            error = mf_desfire_poller_read_file_records(
                instance, file_id, 0, file_settings_cur->data.size, file_data);
        }

        if(error != MfDesfireErrorNone) break;
    }

    return error;
}

MfDesfireError
    mf_desfire_poller_read_application(MfDesfirePoller* instance, MfDesfireApplication* data) {
    furi_assert(instance);
    furi_assert(data);

    MfDesfireError error;

    do {
        error = mf_desfire_poller_read_key_settings(instance, &data->key_settings);
        if(error != MfDesfireErrorNone) break;

        error = mf_desfire_poller_read_key_versions(
            instance, data->key_versions, data->key_settings.max_keys);
        if(error != MfDesfireErrorNone) break;

        error = mf_desfire_poller_read_file_ids(instance, data->file_ids);
        if(error != MfDesfireErrorNone) break;

        error = mf_desfire_poller_read_file_settings_multi(
            instance, data->file_ids, data->file_settings);
        if(error != MfDesfireErrorNone) break;

        error = mf_desfire_poller_read_file_data_multi(
            instance, data->file_ids, data->file_settings, data->file_data);
        if(error != MfDesfireErrorNone) break;

    } while(false);

    return error;
}

MfDesfireError mf_desfire_poller_read_applications(
    MfDesfirePoller* instance,
    const SimpleArray* app_ids,
    SimpleArray* data) {
    furi_assert(instance);

    MfDesfireError error = MfDesfireErrorNone;

    const uint32_t app_id_count = simple_array_get_count(app_ids);
    if(app_id_count > 0) {
        simple_array_init(data, app_id_count);
    }

    for(uint32_t i = 0; i < app_id_count; ++i) {
        do {
            error = mf_desfire_poller_select_application(instance, simple_array_cget(app_ids, i));
            if(error != MfDesfireErrorNone) break;

            MfDesfireApplication* current_app = simple_array_get(data, i);
            error = mf_desfire_poller_read_application(instance, current_app);

        } while(false);
    }

    return error;
}
