#include "mifare_desfire.h"
#include <furi.h>
#include <furi_hal_nfc.h>

#define TAG "MifareDESFire"

void mf_df_clear(MifareDesfireData* data) {
    free(data->free_memory);
    if(data->master_key_settings) {
        MifareDesfireKeyVersion* key_version = data->master_key_settings->key_version_head;
        while(key_version) {
            MifareDesfireKeyVersion* next_key_version = key_version->next;
            free(key_version);
            key_version = next_key_version;
        }
    }
    free(data->master_key_settings);
    MifareDesfireApplication* app = data->app_head;
    while(app) {
        MifareDesfireApplication* next_app = app->next;
        if(app->key_settings) {
            MifareDesfireKeyVersion* key_version = app->key_settings->key_version_head;
            while(key_version) {
                MifareDesfireKeyVersion* next_key_version = key_version->next;
                free(key_version);
                key_version = next_key_version;
            }
        }
        free(app->key_settings);
        MifareDesfireFile* file = app->file_head;
        while(file) {
            MifareDesfireFile* next_file = file->next;
            free(file->contents);
            free(file);
            file = next_file;
        }
        free(app);
        app = next_app;
    }
    data->free_memory = NULL;
    data->master_key_settings = NULL;
    data->app_head = NULL;
}

void mf_df_cat_data(MifareDesfireData* data, string_t out) {
    mf_df_cat_card_info(data, out);
    for(MifareDesfireApplication* app = data->app_head; app; app = app->next) {
        mf_df_cat_application(app, out);
    }
}

void mf_df_cat_card_info(MifareDesfireData* data, string_t out) {
    mf_df_cat_version(&data->version, out);
    if(data->free_memory) {
        mf_df_cat_free_mem(data->free_memory, out);
    }
    if(data->master_key_settings) {
        mf_df_cat_key_settings(data->master_key_settings, out);
    }
}

void mf_df_cat_version(MifareDesfireVersion* version, string_t out) {
    string_cat_printf(
        out,
        "%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
        version->uid[0],
        version->uid[1],
        version->uid[2],
        version->uid[3],
        version->uid[4],
        version->uid[5],
        version->uid[6]);
    string_cat_printf(
        out,
        "hw %02x type %02x sub %02x\n"
        " maj %02x min %02x\n"
        " size %02x proto %02x\n",
        version->hw_vendor,
        version->hw_type,
        version->hw_subtype,
        version->hw_major,
        version->hw_minor,
        version->hw_storage,
        version->hw_proto);
    string_cat_printf(
        out,
        "sw %02x type %02x sub %02x\n"
        " maj %02x min %02x\n"
        " size %02x proto %02x\n",
        version->sw_vendor,
        version->sw_type,
        version->sw_subtype,
        version->sw_major,
        version->sw_minor,
        version->sw_storage,
        version->sw_proto);
    string_cat_printf(
        out,
        "batch %02x:%02x:%02x:%02x:%02x\n"
        "week %d year %d\n",
        version->batch[0],
        version->batch[1],
        version->batch[2],
        version->batch[3],
        version->batch[4],
        version->prod_week,
        version->prod_year);
}

void mf_df_cat_free_mem(MifareDesfireFreeMemory* free_mem, string_t out) {
    string_cat_printf(out, "freeMem %d\n", free_mem->bytes);
}

void mf_df_cat_key_settings(MifareDesfireKeySettings* ks, string_t out) {
    string_cat_printf(out, "changeKeyID %d\n", ks->change_key_id);
    string_cat_printf(out, "configChangeable %d\n", ks->config_changeable);
    string_cat_printf(out, "freeCreateDelete %d\n", ks->free_create_delete);
    string_cat_printf(out, "freeDirectoryList %d\n", ks->free_directory_list);
    string_cat_printf(out, "masterChangeable %d\n", ks->master_key_changeable);
    if(ks->flags) {
        string_cat_printf(out, "flags %d\n", ks->flags);
    }
    string_cat_printf(out, "maxKeys %d\n", ks->max_keys);
    for(MifareDesfireKeyVersion* kv = ks->key_version_head; kv; kv = kv->next) {
        string_cat_printf(out, "key %d version %d\n", kv->id, kv->version);
    }
}

void mf_df_cat_application_info(MifareDesfireApplication* app, string_t out) {
    string_cat_printf(out, "Application %02x%02x%02x\n", app->id[0], app->id[1], app->id[2]);
    if(app->key_settings) {
        mf_df_cat_key_settings(app->key_settings, out);
    }
}

void mf_df_cat_application(MifareDesfireApplication* app, string_t out) {
    mf_df_cat_application_info(app, out);
    for(MifareDesfireFile* file = app->file_head; file; file = file->next) {
        mf_df_cat_file(file, out);
    }
}

void mf_df_cat_file(MifareDesfireFile* file, string_t out) {
    char* type = "unknown";
    switch(file->type) {
    case MifareDesfireFileTypeStandard:
        type = "standard";
        break;
    case MifareDesfireFileTypeBackup:
        type = "backup";
        break;
    case MifareDesfireFileTypeValue:
        type = "value";
        break;
    case MifareDesfireFileTypeLinearRecord:
        type = "linear";
        break;
    case MifareDesfireFileTypeCyclicRecord:
        type = "cyclic";
        break;
    }
    char* comm = "unknown";
    switch(file->comm) {
    case MifareDesfireFileCommunicationSettingsPlaintext:
        comm = "plain";
        break;
    case MifareDesfireFileCommunicationSettingsAuthenticated:
        comm = "auth";
        break;
    case MifareDesfireFileCommunicationSettingsEnciphered:
        comm = "enciphered";
        break;
    }
    string_cat_printf(out, "File %d\n", file->id);
    string_cat_printf(out, "%s %s\n", type, comm);
    string_cat_printf(
        out,
        "r %d w %d rw %d c %d\n",
        file->access_rights >> 12 & 0xF,
        file->access_rights >> 8 & 0xF,
        file->access_rights >> 4 & 0xF,
        file->access_rights & 0xF);
    uint16_t size = 0;
    uint16_t num = 1;
    switch(file->type) {
    case MifareDesfireFileTypeStandard:
    case MifareDesfireFileTypeBackup:
        size = file->settings.data.size;
        string_cat_printf(out, "size %d\n", size);
        break;
    case MifareDesfireFileTypeValue:
        size = 4;
        string_cat_printf(
            out, "lo %d hi %d\n", file->settings.value.lo_limit, file->settings.value.hi_limit);
        string_cat_printf(
            out,
            "limit %d enabled %d\n",
            file->settings.value.limited_credit_value,
            file->settings.value.limited_credit_enabled);
        break;
    case MifareDesfireFileTypeLinearRecord:
    case MifareDesfireFileTypeCyclicRecord:
        size = file->settings.record.size;
        num = file->settings.record.cur;
        string_cat_printf(out, "size %d\n", size);
        string_cat_printf(out, "num %d max %d\n", num, file->settings.record.max);
        break;
    }
    uint8_t* data = file->contents;
    if(data) {
        for(int rec = 0; rec < num; rec++) {
            string_cat_printf(out, "record %d\n", rec);
            for(int ch = 0; ch < size; ch += 4) {
                string_cat_printf(out, "%03x|", ch);
                for(int i = 0; i < 4; i++) {
                    if(ch + i < size) {
                        string_cat_printf(out, "%02x ", data[rec * size + ch + i]);
                    } else {
                        string_cat_printf(out, "   ");
                    }
                }
                for(int i = 0; i < 4 && ch + i < size; i++) {
                    if(isprint(data[rec * size + ch + i])) {
                        string_cat_printf(out, "%c", data[rec * size + ch + i]);
                    } else {
                        string_cat_printf(out, ".");
                    }
                }
                string_cat_printf(out, "\n");
            }
            string_cat_printf(out, " \n");
        }
    }
}

bool mf_df_check_card_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK) {
    return ATQA0 == 0x44 && ATQA1 == 0x03 && SAK == 0x20;
}

uint16_t mf_df_prepare_get_version(uint8_t* dest) {
    dest[0] = MF_DF_GET_VERSION;
    return 1;
}

bool mf_df_parse_get_version_response(uint8_t* buf, uint16_t len, MifareDesfireVersion* out) {
    if(len < 1 || *buf) {
        return false;
    }
    len--;
    buf++;
    if(len < sizeof(MifareDesfireVersion)) {
        return false;
    }
    memcpy(out, buf, sizeof(MifareDesfireVersion));
    return true;
}

uint16_t mf_df_prepare_get_free_memory(uint8_t* dest) {
    dest[0] = MF_DF_GET_FREE_MEMORY;
    return 1;
}

bool mf_df_parse_get_free_memory_response(uint8_t* buf, uint16_t len, MifareDesfireFreeMemory* out) {
    if(len < 1 || *buf) {
        return false;
    }
    len--;
    buf++;
    if(len != 3) {
        return false;
    }
    out->bytes = buf[0] | (buf[1] << 8) | (buf[2] << 16);
    return true;
}

uint16_t mf_df_prepare_get_key_settings(uint8_t* dest) {
    dest[0] = MF_DF_GET_KEY_SETTINGS;
    return 1;
}

bool mf_df_parse_get_key_settings_response(
    uint8_t* buf,
    uint16_t len,
    MifareDesfireKeySettings* out) {
    if(len < 1 || *buf) {
        return false;
    }
    len--;
    buf++;
    if(len < 2) {
        return false;
    }
    out->change_key_id = buf[0] >> 4;
    out->config_changeable = (buf[0] & 0x8) != 0;
    out->free_create_delete = (buf[0] & 0x4) != 0;
    out->free_directory_list = (buf[0] & 0x2) != 0;
    out->master_key_changeable = (buf[0] & 0x1) != 0;
    out->flags = buf[1] >> 4;
    out->max_keys = buf[1] & 0xF;
    return true;
}

uint16_t mf_df_prepare_get_key_version(uint8_t* dest, uint8_t key_id) {
    dest[0] = MF_DF_GET_KEY_VERSION;
    dest[1] = key_id;
    return 2;
}

bool mf_df_parse_get_key_version_response(uint8_t* buf, uint16_t len, MifareDesfireKeyVersion* out) {
    if(len != 2 || *buf) {
        return false;
    }
    out->version = buf[1];
    return true;
}

uint16_t mf_df_prepare_get_application_ids(uint8_t* dest) {
    dest[0] = MF_DF_GET_APPLICATION_IDS;
    return 1;
}

bool mf_df_parse_get_application_ids_response(
    uint8_t* buf,
    uint16_t len,
    MifareDesfireApplication** app_head) {
    if(len < 1 || *buf) {
        return false;
    }
    len--;
    buf++;
    if(len % 3 != 0) {
        return false;
    }
    while(len) {
        MifareDesfireApplication* app = malloc(sizeof(MifareDesfireApplication));
        memset(app, 0, sizeof(MifareDesfireApplication));
        memcpy(app->id, buf, 3);
        len -= 3;
        buf += 3;
        *app_head = app;
        app_head = &app->next;
    }
    return true;
}

uint16_t mf_df_prepare_select_application(uint8_t* dest, uint8_t id[3]) {
    dest[0] = MF_DF_SELECT_APPLICATION;
    dest[1] = id[0];
    dest[2] = id[1];
    dest[3] = id[2];
    return 4;
}

bool mf_df_parse_select_application_response(uint8_t* buf, uint16_t len) {
    return len == 1 && !*buf;
}

uint16_t mf_df_prepare_get_file_ids(uint8_t* dest) {
    dest[0] = MF_DF_GET_FILE_IDS;
    return 1;
}

bool mf_df_parse_get_file_ids_response(uint8_t* buf, uint16_t len, MifareDesfireFile** file_head) {
    if(len < 1 || *buf) {
        return false;
    }
    len--;
    buf++;
    while(len) {
        MifareDesfireFile* file = malloc(sizeof(MifareDesfireFile));
        memset(file, 0, sizeof(MifareDesfireFile));
        file->id = *buf;
        len--;
        buf++;
        *file_head = file;
        file_head = &file->next;
    }
    return true;
}

uint16_t mf_df_prepare_get_file_settings(uint8_t* dest, uint8_t file_id) {
    dest[0] = MF_DF_GET_FILE_SETTINGS;
    dest[1] = file_id;
    return 2;
}

bool mf_df_parse_get_file_settings_response(uint8_t* buf, uint16_t len, MifareDesfireFile* out) {
    if(len < 5 || *buf) {
        return false;
    }
    len--;
    buf++;
    out->type = buf[0];
    out->comm = buf[1];
    out->access_rights = buf[2] | (buf[3] << 8);
    switch(out->type) {
    case MifareDesfireFileTypeStandard:
    case MifareDesfireFileTypeBackup:
        if(len != 7) {
            return false;
        }
        out->settings.data.size = buf[4] | (buf[5] << 8) | (buf[6] << 16);
        break;
    case MifareDesfireFileTypeValue:
        if(len != 17) {
            return false;
        }
        out->settings.value.lo_limit = buf[4] | (buf[5] << 8) | (buf[6] << 16) | (buf[7] << 24);
        out->settings.value.hi_limit = buf[8] | (buf[9] << 8) | (buf[10] << 16) | (buf[11] << 24);
        out->settings.value.limited_credit_value = buf[12] | (buf[13] << 8) | (buf[14] << 16) |
                                                   (buf[15] << 24);
        out->settings.value.limited_credit_enabled = buf[16];
        break;
    case MifareDesfireFileTypeLinearRecord:
    case MifareDesfireFileTypeCyclicRecord:
        if(len != 13) {
            return false;
        }
        out->settings.record.size = buf[4] | (buf[5] << 8) | (buf[6] << 16);
        out->settings.record.max = buf[7] | (buf[8] << 8) | (buf[9] << 16);
        out->settings.record.cur = buf[10] | (buf[11] << 8) | (buf[12] << 16);
        break;
    default:
        return false;
    }
    return true;
}

uint16_t mf_df_prepare_read_data(uint8_t* dest, uint8_t file_id, uint32_t offset, uint32_t len) {
    dest[0] = MF_DF_READ_DATA;
    dest[1] = file_id;
    dest[2] = offset;
    dest[3] = offset >> 8;
    dest[4] = offset >> 16;
    dest[5] = len;
    dest[6] = len >> 8;
    dest[7] = len >> 16;
    return 8;
}

uint16_t mf_df_prepare_get_value(uint8_t* dest, uint8_t file_id) {
    dest[0] = MF_DF_GET_VALUE;
    dest[1] = file_id;
    return 2;
}

uint16_t
    mf_df_prepare_read_records(uint8_t* dest, uint8_t file_id, uint32_t offset, uint32_t len) {
    dest[0] = MF_DF_READ_RECORDS;
    dest[1] = file_id;
    dest[2] = offset;
    dest[3] = offset >> 8;
    dest[4] = offset >> 16;
    dest[5] = len;
    dest[6] = len >> 8;
    dest[7] = len >> 16;
    return 8;
}

bool mf_df_parse_read_data_response(uint8_t* buf, uint16_t len, MifareDesfireFile* out) {
    if(len < 1 || *buf) {
        return false;
    }
    len--;
    buf++;
    out->contents = malloc(len);
    memcpy(out->contents, buf, len);
    return true;
}

bool mf_df_read_card(FuriHalNfcTxRxContext* tx_rx, MifareDesfireData* data) {
    furi_assert(tx_rx);
    furi_assert(data);

    bool card_read = false;
    do {
        // Get version
        tx_rx->tx_bits = 8 * mf_df_prepare_get_version(tx_rx->tx_data);
        if(!furi_hal_nfc_tx_rx_full(tx_rx)) {
            FURI_LOG_W(TAG, "Bad exchange getting version");
            break;
        }
        if(!mf_df_parse_get_version_response(tx_rx->rx_data, tx_rx->rx_bits / 8, &data->version)) {
            FURI_LOG_W(TAG, "Bad DESFire GET_VERSION responce");
        }

        // Get free memory
        tx_rx->tx_bits = 8 * mf_df_prepare_get_free_memory(tx_rx->tx_data);
        if(furi_hal_nfc_tx_rx_full(tx_rx)) {
            data->free_memory = malloc(sizeof(MifareDesfireFreeMemory));
            if(!mf_df_parse_get_free_memory_response(
                   tx_rx->rx_data, tx_rx->rx_bits / 8, data->free_memory)) {
                FURI_LOG_D(TAG, "Bad DESFire GET_FREE_MEMORY response (normal for pre-EV1 cards)");
                free(data->free_memory);
                data->free_memory = NULL;
            }
        }

        // Get key settings
        tx_rx->tx_bits = 8 * mf_df_prepare_get_key_settings(tx_rx->tx_data);
        if(!furi_hal_nfc_tx_rx_full(tx_rx)) {
            FURI_LOG_D(TAG, "Bad exchange getting key settings");
        } else {
            data->master_key_settings = malloc(sizeof(MifareDesfireKeySettings));
            if(!mf_df_parse_get_key_settings_response(
                   tx_rx->rx_data, tx_rx->rx_bits / 8, data->master_key_settings)) {
                FURI_LOG_W(TAG, "Bad DESFire GET_KEY_SETTINGS response");
                free(data->master_key_settings);
                data->master_key_settings = NULL;
            } else {
                MifareDesfireKeyVersion** key_version_head =
                    &data->master_key_settings->key_version_head;
                for(uint8_t key_id = 0; key_id < data->master_key_settings->max_keys; key_id++) {
                    tx_rx->tx_bits = 8 * mf_df_prepare_get_key_version(tx_rx->tx_data, key_id);
                    if(!furi_hal_nfc_tx_rx_full(tx_rx)) {
                        FURI_LOG_W(TAG, "Bad exchange getting key version");
                        continue;
                    }
                    MifareDesfireKeyVersion* key_version = malloc(sizeof(MifareDesfireKeyVersion));
                    memset(key_version, 0, sizeof(MifareDesfireKeyVersion));
                    key_version->id = key_id;
                    if(!mf_df_parse_get_key_version_response(
                           tx_rx->rx_data, tx_rx->rx_bits / 8, key_version)) {
                        FURI_LOG_W(TAG, "Bad DESFire GET_KEY_VERSION response");
                        free(key_version);
                        continue;
                    }
                    *key_version_head = key_version;
                    key_version_head = &key_version->next;
                }
            }
        }

        // Get application IDs
        tx_rx->tx_bits = 8 * mf_df_prepare_get_application_ids(tx_rx->tx_data);
        if(!furi_hal_nfc_tx_rx_full(tx_rx)) {
            FURI_LOG_W(TAG, "Bad exchange getting application IDs");
            break;
        } else {
            if(!mf_df_parse_get_application_ids_response(
                   tx_rx->rx_data, tx_rx->rx_bits / 8, &data->app_head)) {
                FURI_LOG_W(TAG, "Bad DESFire GET_APPLICATION_IDS response");
                break;
            }
        }

        for(MifareDesfireApplication* app = data->app_head; app; app = app->next) {
            tx_rx->tx_bits = 8 * mf_df_prepare_select_application(tx_rx->tx_data, app->id);
            if(!furi_hal_nfc_tx_rx_full(tx_rx) ||
               !mf_df_parse_select_application_response(tx_rx->rx_data, tx_rx->rx_bits / 8)) {
                FURI_LOG_W(TAG, "Bad exchange selecting application");
                continue;
            }
            tx_rx->tx_bits = 8 * mf_df_prepare_get_key_settings(tx_rx->tx_data);
            if(!furi_hal_nfc_tx_rx_full(tx_rx)) {
                FURI_LOG_W(TAG, "Bad exchange getting key settings");
            } else {
                app->key_settings = malloc(sizeof(MifareDesfireKeySettings));
                memset(app->key_settings, 0, sizeof(MifareDesfireKeySettings));
                if(!mf_df_parse_get_key_settings_response(
                       tx_rx->rx_data, tx_rx->rx_bits / 8, app->key_settings)) {
                    FURI_LOG_W(TAG, "Bad DESFire GET_KEY_SETTINGS response");
                    free(app->key_settings);
                    app->key_settings = NULL;
                    continue;
                }

                MifareDesfireKeyVersion** key_version_head = &app->key_settings->key_version_head;
                for(uint8_t key_id = 0; key_id < app->key_settings->max_keys; key_id++) {
                    tx_rx->tx_bits = 8 * mf_df_prepare_get_key_version(tx_rx->tx_data, key_id);
                    if(!furi_hal_nfc_tx_rx_full(tx_rx)) {
                        FURI_LOG_W(TAG, "Bad exchange getting key version");
                        continue;
                    }
                    MifareDesfireKeyVersion* key_version = malloc(sizeof(MifareDesfireKeyVersion));
                    memset(key_version, 0, sizeof(MifareDesfireKeyVersion));
                    key_version->id = key_id;
                    if(!mf_df_parse_get_key_version_response(
                           tx_rx->rx_data, tx_rx->rx_bits / 8, key_version)) {
                        FURI_LOG_W(TAG, "Bad DESFire GET_KEY_VERSION response");
                        free(key_version);
                        continue;
                    }
                    *key_version_head = key_version;
                    key_version_head = &key_version->next;
                }
            }

            tx_rx->tx_bits = 8 * mf_df_prepare_get_file_ids(tx_rx->tx_data);
            if(!furi_hal_nfc_tx_rx_full(tx_rx)) {
                FURI_LOG_W(TAG, "Bad exchange getting file IDs");
            } else {
                if(!mf_df_parse_get_file_ids_response(
                       tx_rx->rx_data, tx_rx->rx_bits / 8, &app->file_head)) {
                    FURI_LOG_W(TAG, "Bad DESFire GET_FILE_IDS response");
                }
            }

            for(MifareDesfireFile* file = app->file_head; file; file = file->next) {
                tx_rx->tx_bits = 8 * mf_df_prepare_get_file_settings(tx_rx->tx_data, file->id);
                if(!furi_hal_nfc_tx_rx_full(tx_rx)) {
                    FURI_LOG_W(TAG, "Bad exchange getting file settings");
                    continue;
                }
                if(!mf_df_parse_get_file_settings_response(
                       tx_rx->rx_data, tx_rx->rx_bits / 8, file)) {
                    FURI_LOG_W(TAG, "Bad DESFire GET_FILE_SETTINGS response");
                    continue;
                }
                switch(file->type) {
                case MifareDesfireFileTypeStandard:
                case MifareDesfireFileTypeBackup:
                    tx_rx->tx_bits = 8 * mf_df_prepare_read_data(tx_rx->tx_data, file->id, 0, 0);
                    break;
                case MifareDesfireFileTypeValue:
                    tx_rx->tx_bits = 8 * mf_df_prepare_get_value(tx_rx->tx_data, file->id);
                    break;
                case MifareDesfireFileTypeLinearRecord:
                case MifareDesfireFileTypeCyclicRecord:
                    tx_rx->tx_bits =
                        8 * mf_df_prepare_read_records(tx_rx->tx_data, file->id, 0, 0);
                    break;
                }
                if(!furi_hal_nfc_tx_rx_full(tx_rx)) {
                    FURI_LOG_W(TAG, "Bad exchange reading file %d", file->id);
                    continue;
                }
                if(!mf_df_parse_read_data_response(tx_rx->rx_data, tx_rx->rx_bits / 8, file)) {
                    FURI_LOG_W(TAG, "Bad response reading file %d", file->id);
                    continue;
                }
            }
        }

        card_read = true;
    } while(false);

    return card_read;
}
