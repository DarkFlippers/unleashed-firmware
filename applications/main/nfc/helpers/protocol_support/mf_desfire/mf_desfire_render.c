#include "mf_desfire_render.h"

#include "../iso14443_4a/iso14443_4a_render.h"

#define MF_DESFIRE_RENDER_MAX_RECORD_SIZE (256U)

void nfc_render_mf_desfire_info(
    const MfDesfireData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    nfc_render_iso14443_4a_brief(mf_desfire_get_base_data(data), str);

    const uint32_t bytes_total = 1UL << (data->version.sw_storage >> 1);
    const uint32_t bytes_free = data->free_memory.is_present ? data->free_memory.bytes_free : 0;

    if(data->master_key_settings.is_free_directory_list) {
        const uint32_t app_count = simple_array_get_count(data->applications);
        uint32_t file_count = 0;

        for(uint32_t i = 0; i < app_count; ++i) {
            const MfDesfireApplication* app = simple_array_cget(data->applications, i);
            if(app->key_settings.is_free_directory_list) {
                file_count += simple_array_get_count(app->file_ids);
            }
        }

        furi_string_cat_printf(str, "\n%lu Application%s", app_count, app_count != 1 ? "s" : "");
        furi_string_cat_printf(str, ", %lu File%s", file_count, file_count != 1 ? "s" : "");
    } else {
        furi_string_cat_printf(str, "\nAuth required to read apps!");
    }

    furi_string_cat_printf(str, "\n%lu", bytes_total);

    if(data->version.sw_storage & 1) {
        furi_string_push_back(str, '+');
    }
    furi_string_cat_printf(str, " bytes, %lu bytes free", bytes_free);

    if(format_type != NfcProtocolFormatTypeFull) return;

    furi_string_cat(str, "\n\e#ISO14443-4 data");
    nfc_render_iso14443_4a_extra(mf_desfire_get_base_data(data), str);
}

void nfc_render_mf_desfire_data(const MfDesfireData* data, FuriString* str) {
    nfc_render_mf_desfire_version(&data->version, str);
    nfc_render_mf_desfire_free_memory(&data->free_memory, str);
    nfc_render_mf_desfire_key_settings(&data->master_key_settings, str);

    for(uint32_t i = 0; i < simple_array_get_count(data->master_key_versions); ++i) {
        nfc_render_mf_desfire_key_version(simple_array_cget(data->master_key_versions, i), i, str);
    }
}

void nfc_render_mf_desfire_version(const MfDesfireVersion* data, FuriString* str) {
    furi_string_cat_printf(
        str,
        "%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
        data->uid[0],
        data->uid[1],
        data->uid[2],
        data->uid[3],
        data->uid[4],
        data->uid[5],
        data->uid[6]);
    furi_string_cat_printf(
        str,
        "hw %02x type %02x sub %02x\n"
        " maj %02x min %02x\n"
        " size %02x proto %02x\n",
        data->hw_vendor,
        data->hw_type,
        data->hw_subtype,
        data->hw_major,
        data->hw_minor,
        data->hw_storage,
        data->hw_proto);
    furi_string_cat_printf(
        str,
        "sw %02x type %02x sub %02x\n"
        " maj %02x min %02x\n"
        " size %02x proto %02x\n",
        data->sw_vendor,
        data->sw_type,
        data->sw_subtype,
        data->sw_major,
        data->sw_minor,
        data->sw_storage,
        data->sw_proto);
    furi_string_cat_printf(
        str,
        "batch %02x:%02x:%02x:%02x:%02x\n"
        "week %02x year 20%02x\n",
        data->batch[0],
        data->batch[1],
        data->batch[2],
        data->batch[3],
        data->batch[4],
        data->prod_week,
        data->prod_year);
}

void nfc_render_mf_desfire_free_memory(const MfDesfireFreeMemory* data, FuriString* str) {
    if(data->is_present) {
        furi_string_cat_printf(str, "freeMem %lu\n", data->bytes_free);
    }
}

void nfc_render_mf_desfire_key_settings(const MfDesfireKeySettings* data, FuriString* str) {
    if(data->is_free_directory_list) {
        furi_string_cat_printf(str, "changeKeyID %d\n", data->change_key_id);
        furi_string_cat_printf(str, "configChangeable %d\n", data->is_config_changeable);
        furi_string_cat_printf(str, "freeCreateDelete %d\n", data->is_free_create_delete);
        furi_string_cat_printf(str, "freeDirectoryList %d\n", data->is_free_directory_list);
        furi_string_cat_printf(str, "masterChangeable %d\n", data->is_master_key_changeable);
    } else {
        furi_string_cat_printf(str, "changeKeyID ??\n");
        furi_string_cat_printf(str, "configChangeable ??\n");
        furi_string_cat_printf(str, "freeCreateDelete ??\n");
        furi_string_cat_printf(str, "freeDirectoryList 0\n");
        furi_string_cat_printf(str, "masterChangeable ??\n");
    }

    if(data->flags) {
        furi_string_cat_printf(str, "flags %d\n", data->flags);
    }

    if(data->is_free_directory_list) {
        furi_string_cat_printf(str, "maxKeys %d\n", data->max_keys);
    } else {
        furi_string_cat_printf(str, "maxKeys ??\n");
    }
}

void nfc_render_mf_desfire_key_version(
    const MfDesfireKeyVersion* data,
    uint32_t index,
    FuriString* str) {
    furi_string_cat_printf(str, "key %lu version %u\n", index, *data);
}

void nfc_render_mf_desfire_application_id(const MfDesfireApplicationId* data, FuriString* str) {
    const uint8_t* app_id = data->data;
    furi_string_cat_printf(str, "Application %02x%02x%02x\n", app_id[2], app_id[1], app_id[0]);
}

void nfc_render_mf_desfire_application(const MfDesfireApplication* data, FuriString* str) {
    nfc_render_mf_desfire_key_settings(&data->key_settings, str);

    if(data->key_settings.is_free_directory_list) {
        for(uint32_t i = 0; i < simple_array_get_count(data->key_versions); ++i) {
            nfc_render_mf_desfire_key_version(simple_array_cget(data->key_versions, i), i, str);
        }
    }
}

void nfc_render_mf_desfire_file_id(const MfDesfireFileId* data, FuriString* str) {
    furi_string_cat_printf(str, "File %d\n", *data);
}

void nfc_render_mf_desfire_file_settings_data(
    const MfDesfireFileSettings* settings,
    const MfDesfireFileData* data,
    FuriString* str) {
    const char* type;
    switch(settings->type) {
    case MfDesfireFileTypeStandard:
        type = "standard";
        break;
    case MfDesfireFileTypeBackup:
        type = "backup";
        break;
    case MfDesfireFileTypeValue:
        type = "value";
        break;
    case MfDesfireFileTypeLinearRecord:
        type = "linear";
        break;
    case MfDesfireFileTypeCyclicRecord:
        type = "cyclic";
        break;
    default:
        type = "unknown";
    }

    const char* comm;
    switch(settings->comm) {
    case MfDesfireFileCommunicationSettingsPlaintext:
        comm = "plain";
        break;
    case MfDesfireFileCommunicationSettingsAuthenticated:
        comm = "auth";
        break;
    case MfDesfireFileCommunicationSettingsEnciphered:
        comm = "enciphered";
        break;
    default:
        comm = "unknown";
    }

    furi_string_cat_printf(str, "%s %s\n", type, comm);

    for(size_t i = 0; i < settings->access_rights_len; i++) {
        furi_string_cat_printf(
            str,
            "r %d w %d rw %d c %d\n",
            settings->access_rights[i] >> 12 & 0xF,
            settings->access_rights[i] >> 8 & 0xF,
            settings->access_rights[i] >> 4 & 0xF,
            settings->access_rights[i] & 0xF);
    }

    uint32_t record_count = 1;
    uint32_t record_size = 0;

    switch(settings->type) {
    case MfDesfireFileTypeStandard:
    case MfDesfireFileTypeBackup:
        record_size = settings->data.size;
        furi_string_cat_printf(str, "size %lu\n", record_size);
        break;
    case MfDesfireFileTypeValue:
        record_size = MF_DESFIRE_VALUE_SIZE;
        furi_string_cat_printf(
            str, "lo %lu hi %lu\n", settings->value.lo_limit, settings->value.hi_limit);
        furi_string_cat_printf(
            str,
            "limit %lu enabled %d\n",
            settings->value.limited_credit_value,
            settings->value.limited_credit_enabled);
        break;
    case MfDesfireFileTypeLinearRecord:
    case MfDesfireFileTypeCyclicRecord:
        record_count = settings->record.cur;
        record_size = settings->record.size;
        furi_string_cat_printf(str, "size %lu\n", record_size);
        furi_string_cat_printf(str, "num %lu max %lu\n", record_count, settings->record.max);
        break;
    }

    bool is_auth_required = true;
    for(size_t i = 0; i < settings->access_rights_len; i++) {
        uint8_t read_rights = (settings->access_rights[i] >> 12) & 0x0f;
        uint8_t read_write_rights = (settings->access_rights[i] >> 4) & 0x0f;
        if((read_rights == 0x0e) || (read_write_rights == 0x0e)) {
            is_auth_required = false;
            break;
        }
    }
    if(is_auth_required) {
        furi_string_cat_printf(str, "Auth required to read file data\n");
        return;
    }

    if(simple_array_get_count(data->data) == 0) {
        return;
    }

    // Limit record size
    bool trim_data = record_size > MF_DESFIRE_RENDER_MAX_RECORD_SIZE;
    if(trim_data) {
        record_size = MF_DESFIRE_RENDER_MAX_RECORD_SIZE;
    }

    for(uint32_t rec = 0; rec < record_count; rec++) {
        furi_string_cat_printf(str, "record %lu\n", rec);

        for(uint32_t ch = 0; ch < record_size; ch += 4) {
            furi_string_cat_printf(str, "%03lx|", ch);

            for(uint32_t i = 0; i < 4; i++) {
                if(ch + i < record_size) {
                    const uint32_t data_index = rec * record_size + ch + i;
                    const uint8_t data_byte =
                        *(const uint8_t*)simple_array_cget(data->data, data_index);
                    furi_string_cat_printf(str, "%02x ", data_byte);
                } else {
                    furi_string_cat_printf(str, "   ");
                }
            }

            for(uint32_t i = 0; i < 4 && ch + i < record_size; i++) {
                const uint32_t data_index = rec * record_size + ch + i;
                const uint8_t data_byte =
                    *(const uint8_t*)simple_array_cget(data->data, data_index);
                if(isprint(data_byte)) {
                    furi_string_cat_printf(str, "%c", data_byte);
                } else {
                    furi_string_cat_printf(str, ".");
                }
            }

            furi_string_push_back(str, '\n');
        }
        if(trim_data) {
            furi_string_cat_str(str, "...");
        }

        furi_string_push_back(str, '\n');
    }
}
