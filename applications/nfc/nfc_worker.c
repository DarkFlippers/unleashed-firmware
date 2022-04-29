#include "nfc_worker_i.h"
#include <furi_hal.h>

#include <lib/nfc_protocols/nfc_util.h>
#include <lib/nfc_protocols/emv.h>
#include <lib/nfc_protocols/mifare_common.h>
#include <lib/nfc_protocols/mifare_ultralight.h>
#include <lib/nfc_protocols/mifare_classic.h>
#include <lib/nfc_protocols/mifare_desfire.h>

#include "helpers/nfc_mf_classic_dict.h"

#define TAG "NfcWorker"

/***************************** NFC Worker API *******************************/

NfcWorker* nfc_worker_alloc() {
    NfcWorker* nfc_worker = malloc(sizeof(NfcWorker));

    // Worker thread attributes
    nfc_worker->thread = furi_thread_alloc();
    furi_thread_set_name(nfc_worker->thread, "NfcWorker");
    furi_thread_set_stack_size(nfc_worker->thread, 8192);
    furi_thread_set_callback(nfc_worker->thread, nfc_worker_task);
    furi_thread_set_context(nfc_worker->thread, nfc_worker);

    nfc_worker->callback = NULL;
    nfc_worker->context = NULL;
    nfc_worker->storage = furi_record_open("storage");

    // Initialize rfal
    while(furi_hal_nfc_is_busy()) {
        osDelay(10);
    }
    nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);

    return nfc_worker;
}

void nfc_worker_free(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    furi_thread_free(nfc_worker->thread);
    furi_record_close("storage");
    free(nfc_worker);
}

NfcWorkerState nfc_worker_get_state(NfcWorker* nfc_worker) {
    return nfc_worker->state;
}

void nfc_worker_start(
    NfcWorker* nfc_worker,
    NfcWorkerState state,
    NfcDeviceData* dev_data,
    NfcWorkerCallback callback,
    void* context) {
    furi_assert(nfc_worker);
    furi_assert(dev_data);
    while(furi_hal_nfc_is_busy()) {
        osDelay(10);
    }

    nfc_worker->callback = callback;
    nfc_worker->context = context;
    nfc_worker->dev_data = dev_data;
    nfc_worker_change_state(nfc_worker, state);
    furi_thread_start(nfc_worker->thread);
}

void nfc_worker_stop(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    if(nfc_worker->state == NfcWorkerStateBroken || nfc_worker->state == NfcWorkerStateReady) {
        return;
    }
    furi_hal_nfc_stop();
    nfc_worker_change_state(nfc_worker, NfcWorkerStateStop);
    furi_thread_join(nfc_worker->thread);
}

void nfc_worker_change_state(NfcWorker* nfc_worker, NfcWorkerState state) {
    nfc_worker->state = state;
}

/***************************** NFC Worker Thread *******************************/

int32_t nfc_worker_task(void* context) {
    NfcWorker* nfc_worker = context;

    furi_hal_nfc_exit_sleep();

    if(nfc_worker->state == NfcWorkerStateDetect) {
        nfc_worker_detect(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulate) {
        nfc_worker_emulate(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadEMVApp) {
        nfc_worker_read_emv_app(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadEMVData) {
        nfc_worker_read_emv(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulateApdu) {
        nfc_worker_emulate_apdu(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadMifareUltralight) {
        nfc_worker_read_mifare_ultralight(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulateMifareUltralight) {
        nfc_worker_emulate_mifare_ul(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadMifareClassic) {
        nfc_worker_mifare_classic_dict_attack(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadMifareDesfire) {
        nfc_worker_read_mifare_desfire(nfc_worker);
    }
    furi_hal_nfc_sleep();
    nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);

    return 0;
}

void nfc_worker_detect(NfcWorker* nfc_worker) {
    nfc_device_data_clear(nfc_worker->dev_data);
    NfcDeviceData* dev_data = nfc_worker->dev_data;
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;

    while(nfc_worker->state == NfcWorkerStateDetect) {
        if(furi_hal_nfc_detect(nfc_data, 1000)) {
            // Process first found device
            if(nfc_data->type == FuriHalNfcTypeA) {
                if(mf_ul_check_card_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak)) {
                    dev_data->protocol = NfcDeviceProtocolMifareUl;
                } else if(mf_classic_check_card_type(
                              nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak)) {
                    dev_data->protocol = NfcDeviceProtocolMifareClassic;
                } else if(mf_df_check_card_type(
                              nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak)) {
                    dev_data->protocol = NfcDeviceProtocolMifareDesfire;
                } else if(nfc_data->interface == FuriHalNfcInterfaceIsoDep) {
                    dev_data->protocol = NfcDeviceProtocolEMV;
                } else {
                    dev_data->protocol = NfcDeviceProtocolUnknown;
                }
            }

            // Notify caller and exit
            if(nfc_worker->callback) {
                nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
            }
            break;
        }
        furi_hal_nfc_sleep();
        osDelay(100);
    }
}

void nfc_worker_emulate(NfcWorker* nfc_worker) {
    FuriHalNfcTxRxContext tx_rx = {};
    FuriHalNfcDevData* data = &nfc_worker->dev_data->nfc_data;
    NfcReaderRequestData* reader_data = &nfc_worker->dev_data->reader_data;

    while(nfc_worker->state == NfcWorkerStateEmulate) {
        if(furi_hal_nfc_listen(data->uid, data->uid_len, data->atqa, data->sak, true, 100)) {
            if(furi_hal_nfc_tx_rx(&tx_rx, 100)) {
                reader_data->size = tx_rx.rx_bits / 8;
                if(reader_data->size > 0) {
                    memcpy(reader_data->data, tx_rx.rx_data, reader_data->size);
                    if(nfc_worker->callback) {
                        nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
                    }
                }
            } else {
                FURI_LOG_E(TAG, "Failed to get reader commands");
            }
        }
    }
}

void nfc_worker_read_emv_app(NfcWorker* nfc_worker) {
    FuriHalNfcTxRxContext tx_rx = {};
    EmvApplication emv_app = {};
    NfcDeviceData* result = nfc_worker->dev_data;
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    nfc_device_data_clear(result);

    while(nfc_worker->state == NfcWorkerStateReadEMVApp) {
        if(furi_hal_nfc_detect(nfc_data, 1000)) {
            // Card was found. Check that it supports EMV
            if(nfc_data->interface == FuriHalNfcInterfaceIsoDep) {
                result->protocol = NfcDeviceProtocolEMV;
                if(emv_search_application(&tx_rx, &emv_app)) {
                    // Notify caller and exit
                    result->emv_data.aid_len = emv_app.aid_len;
                    memcpy(result->emv_data.aid, emv_app.aid, emv_app.aid_len);
                    if(nfc_worker->callback) {
                        nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
                    }
                }
            } else {
                FURI_LOG_W(TAG, "Card doesn't support EMV");
            }
        } else {
            FURI_LOG_D(TAG, "Can't find any cards");
        }
        furi_hal_nfc_sleep();
        osDelay(20);
    }
}

void nfc_worker_read_emv(NfcWorker* nfc_worker) {
    FuriHalNfcTxRxContext tx_rx = {};
    EmvApplication emv_app = {};
    NfcDeviceData* result = nfc_worker->dev_data;
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    nfc_device_data_clear(result);

    while(nfc_worker->state == NfcWorkerStateReadEMVData) {
        if(furi_hal_nfc_detect(nfc_data, 1000)) {
            // Card was found. Check that it supports EMV
            if(nfc_data->interface == FuriHalNfcInterfaceIsoDep) {
                result->protocol = NfcDeviceProtocolEMV;
                if(emv_read_bank_card(&tx_rx, &emv_app)) {
                    result->emv_data.number_len = emv_app.card_number_len;
                    memcpy(
                        result->emv_data.number, emv_app.card_number, result->emv_data.number_len);
                    result->emv_data.aid_len = emv_app.aid_len;
                    memcpy(result->emv_data.aid, emv_app.aid, emv_app.aid_len);
                    if(emv_app.name_found) {
                        memcpy(result->emv_data.name, emv_app.name, sizeof(emv_app.name));
                    }
                    if(emv_app.exp_month) {
                        result->emv_data.exp_mon = emv_app.exp_month;
                        result->emv_data.exp_year = emv_app.exp_year;
                    }
                    if(emv_app.country_code) {
                        result->emv_data.country_code = emv_app.country_code;
                    }
                    if(emv_app.currency_code) {
                        result->emv_data.currency_code = emv_app.currency_code;
                    }
                    // Notify caller and exit
                    if(nfc_worker->callback) {
                        nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
                    }
                    break;
                }
            } else {
                FURI_LOG_W(TAG, "Card doesn't support EMV");
            }
        } else {
            FURI_LOG_D(TAG, "Can't find any cards");
        }
        furi_hal_nfc_sleep();
        osDelay(20);
    }
}

void nfc_worker_emulate_apdu(NfcWorker* nfc_worker) {
    FuriHalNfcTxRxContext tx_rx = {};
    FuriHalNfcDevData params = {
        .uid = {0xCF, 0x72, 0xd4, 0x40},
        .uid_len = 4,
        .atqa = {0x00, 0x04},
        .sak = 0x20,
        .type = FuriHalNfcTypeA,
    };

    while(nfc_worker->state == NfcWorkerStateEmulateApdu) {
        if(furi_hal_nfc_listen(params.uid, params.uid_len, params.atqa, params.sak, false, 300)) {
            FURI_LOG_D(TAG, "POS terminal detected");
            if(emv_card_emulation(&tx_rx)) {
                FURI_LOG_D(TAG, "EMV card emulated");
            }
        } else {
            FURI_LOG_D(TAG, "Can't find reader");
        }
        furi_hal_nfc_sleep();
        osDelay(20);
    }
}

void nfc_worker_read_mifare_ultralight(NfcWorker* nfc_worker) {
    FuriHalNfcTxRxContext tx_rx = {};
    MfUltralightReader reader = {};
    MfUltralightData data = {};
    NfcDeviceData* result = nfc_worker->dev_data;
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;

    while(nfc_worker->state == NfcWorkerStateReadMifareUltralight) {
        if(furi_hal_nfc_detect(nfc_data, 300)) {
            if(nfc_data->type == FuriHalNfcTypeA &&
               mf_ul_check_card_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak)) {
                FURI_LOG_D(TAG, "Found Mifare Ultralight tag. Start reading");
                if(mf_ul_read_card(&tx_rx, &reader, &data)) {
                    result->protocol = NfcDeviceProtocolMifareUl;
                    result->mf_ul_data = data;
                    // Notify caller and exit
                    if(nfc_worker->callback) {
                        nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
                    }
                    break;
                } else {
                    FURI_LOG_D(TAG, "Failed reading Mifare Ultralight");
                }
            } else {
                FURI_LOG_W(TAG, "Tag is not Mifare Ultralight");
            }
        } else {
            FURI_LOG_D(TAG, "Can't find any tags");
        }
        furi_hal_nfc_sleep();
        osDelay(100);
    }
}

void nfc_worker_emulate_mifare_ul(NfcWorker* nfc_worker) {
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    MfUltralightEmulator emulator = {};
    mf_ul_prepare_emulation(&emulator, &nfc_worker->dev_data->mf_ul_data);
    while(nfc_worker->state == NfcWorkerStateEmulateMifareUltralight) {
        furi_hal_nfc_emulate_nfca(
            nfc_data->uid,
            nfc_data->uid_len,
            nfc_data->atqa,
            nfc_data->sak,
            mf_ul_prepare_emulation_response,
            &emulator,
            5000);
        // Check if data was modified
        if(emulator.data_changed) {
            nfc_worker->dev_data->mf_ul_data = emulator.data;
            if(nfc_worker->callback) {
                nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
            }
            emulator.data_changed = false;
        }
    }
}

void nfc_worker_mifare_classic_dict_attack(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker->callback);
    FuriHalNfcTxRxContext tx_rx_ctx = {};
    MfClassicAuthContext auth_ctx = {};
    MfClassicReader reader = {};
    uint64_t curr_key = 0;
    uint16_t curr_sector = 0;
    uint8_t total_sectors = 0;
    NfcWorkerEvent event;
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;

    // Open dictionary
    nfc_worker->dict_stream = file_stream_alloc(nfc_worker->storage);
    if(!nfc_mf_classic_dict_open_file(nfc_worker->dict_stream)) {
        event = NfcWorkerEventNoDictFound;
        nfc_worker->callback(event, nfc_worker->context);
        nfc_mf_classic_dict_close_file(nfc_worker->dict_stream);
        stream_free(nfc_worker->dict_stream);
        return;
    }

    // Detect Mifare Classic card
    while(nfc_worker->state == NfcWorkerStateReadMifareClassic) {
        if(furi_hal_nfc_detect(nfc_data, 300)) {
            if(mf_classic_get_type(
                   nfc_data->uid,
                   nfc_data->uid_len,
                   nfc_data->atqa[0],
                   nfc_data->atqa[1],
                   nfc_data->sak,
                   &reader)) {
                total_sectors = mf_classic_get_total_sectors_num(&reader);
                if(reader.type == MfClassicType1k) {
                    event = NfcWorkerEventDetectedClassic1k;
                } else {
                    event = NfcWorkerEventDetectedClassic4k;
                }
                nfc_worker->callback(event, nfc_worker->context);
                break;
            }
        } else {
            event = NfcWorkerEventNoCardDetected;
            nfc_worker->callback(event, nfc_worker->context);
        }
    }

    if(nfc_worker->state == NfcWorkerStateReadMifareClassic) {
        bool card_removed_notified = false;
        bool card_found_notified = false;
        // Seek for mifare classic keys
        for(curr_sector = 0; curr_sector < total_sectors; curr_sector++) {
            FURI_LOG_I(TAG, "Sector: %d ...", curr_sector);
            event = NfcWorkerEventNewSector;
            nfc_worker->callback(event, nfc_worker->context);
            mf_classic_auth_init_context(&auth_ctx, reader.cuid, curr_sector);
            bool sector_key_found = false;
            while(nfc_mf_classic_dict_get_next_key(nfc_worker->dict_stream, &curr_key)) {
                furi_hal_nfc_sleep();
                if(furi_hal_nfc_activate_nfca(300, &reader.cuid)) {
                    if(!card_found_notified) {
                        if(reader.type == MfClassicType1k) {
                            event = NfcWorkerEventDetectedClassic1k;
                        } else {
                            event = NfcWorkerEventDetectedClassic4k;
                        }
                        nfc_worker->callback(event, nfc_worker->context);
                        card_found_notified = true;
                        card_removed_notified = false;
                    }
                    FURI_LOG_D(
                        TAG,
                        "Try to auth to sector %d with key %04lx%08lx",
                        curr_sector,
                        (uint32_t)(curr_key >> 32),
                        (uint32_t)curr_key);
                    if(mf_classic_auth_attempt(&tx_rx_ctx, &auth_ctx, curr_key)) {
                        sector_key_found = true;
                        if((auth_ctx.key_a != MF_CLASSIC_NO_KEY) &&
                           (auth_ctx.key_b != MF_CLASSIC_NO_KEY))
                            break;
                    }
                } else {
                    // Notify that no tag is availalble
                    FURI_LOG_D(TAG, "Can't find tags");
                    if(!card_removed_notified) {
                        event = NfcWorkerEventNoCardDetected;
                        nfc_worker->callback(event, nfc_worker->context);
                        card_removed_notified = true;
                        card_found_notified = false;
                    }
                }
                if(nfc_worker->state != NfcWorkerStateReadMifareClassic) break;
                osDelay(1);
            }
            if(nfc_worker->state != NfcWorkerStateReadMifareClassic) break;
            if(sector_key_found) {
                // Notify that keys were found
                if(auth_ctx.key_a != MF_CLASSIC_NO_KEY) {
                    FURI_LOG_I(
                        TAG,
                        "Sector %d key A: %04lx%08lx",
                        curr_sector,
                        (uint32_t)(auth_ctx.key_a >> 32),
                        (uint32_t)auth_ctx.key_a);
                    event = NfcWorkerEventFoundKeyA;
                    nfc_worker->callback(event, nfc_worker->context);
                }
                if(auth_ctx.key_b != MF_CLASSIC_NO_KEY) {
                    FURI_LOG_I(
                        TAG,
                        "Sector %d key B: %04lx%08lx",
                        curr_sector,
                        (uint32_t)(auth_ctx.key_b >> 32),
                        (uint32_t)auth_ctx.key_b);
                    event = NfcWorkerEventFoundKeyB;
                    nfc_worker->callback(event, nfc_worker->context);
                }
                // Add sectors to read sequence
                mf_classic_reader_add_sector(&reader, curr_sector, auth_ctx.key_a, auth_ctx.key_b);
            }
            nfc_mf_classic_dict_reset(nfc_worker->dict_stream);
        }
    }

    if(nfc_worker->state == NfcWorkerStateReadMifareClassic) {
        FURI_LOG_I(TAG, "Found keys to %d sectors. Start reading sectors", reader.sectors_to_read);
        uint8_t sectors_read =
            mf_classic_read_card(&tx_rx_ctx, &reader, &nfc_worker->dev_data->mf_classic_data);
        if(sectors_read) {
            event = NfcWorkerEventSuccess;
            nfc_worker->dev_data->protocol = NfcDeviceProtocolMifareClassic;
            FURI_LOG_I(TAG, "Successfully read %d sectors", sectors_read);
        } else {
            event = NfcWorkerEventFail;
            FURI_LOG_W(TAG, "Failed to read any sector");
        }
        nfc_worker->callback(event, nfc_worker->context);
    }

    nfc_mf_classic_dict_close_file(nfc_worker->dict_stream);
    stream_free(nfc_worker->dict_stream);
}

void nfc_worker_read_mifare_desfire(NfcWorker* nfc_worker) {
    ReturnCode err;
    uint8_t tx_buff[64] = {};
    uint16_t tx_len = 0;
    uint8_t rx_buff[512] = {};
    uint16_t rx_len;
    NfcDeviceData* result = nfc_worker->dev_data;
    nfc_device_data_clear(result);
    MifareDesfireData* data = &result->mf_df_data;
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;

    while(nfc_worker->state == NfcWorkerStateReadMifareDesfire) {
        furi_hal_nfc_sleep();
        if(!furi_hal_nfc_detect(nfc_data, 300)) {
            osDelay(100);
            continue;
        }
        memset(data, 0, sizeof(MifareDesfireData));
        if(nfc_data->type != FuriHalNfcTypeA ||
           !mf_df_check_card_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak)) {
            FURI_LOG_D(TAG, "Tag is not DESFire");
            osDelay(100);
            continue;
        }

        FURI_LOG_D(TAG, "Found DESFire tag");

        result->protocol = NfcDeviceProtocolMifareDesfire;

        // Get DESFire version
        tx_len = mf_df_prepare_get_version(tx_buff);
        err = furi_hal_nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
        if(err != ERR_NONE) {
            FURI_LOG_W(TAG, "Bad exchange getting version, err: %d", err);
            continue;
        }
        if(!mf_df_parse_get_version_response(rx_buff, rx_len, &data->version)) {
            FURI_LOG_W(TAG, "Bad DESFire GET_VERSION response");
            continue;
        }

        tx_len = mf_df_prepare_get_free_memory(tx_buff);
        err = furi_hal_nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
        if(err == ERR_NONE) {
            data->free_memory = malloc(sizeof(MifareDesfireFreeMemory));
            memset(data->free_memory, 0, sizeof(MifareDesfireFreeMemory));
            if(!mf_df_parse_get_free_memory_response(rx_buff, rx_len, data->free_memory)) {
                FURI_LOG_D(TAG, "Bad DESFire GET_FREE_MEMORY response (normal for pre-EV1 cards)");
                free(data->free_memory);
                data->free_memory = NULL;
            }
        }

        tx_len = mf_df_prepare_get_key_settings(tx_buff);
        err = furi_hal_nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
        if(err != ERR_NONE) {
            FURI_LOG_D(TAG, "Bad exchange getting key settings, err: %d", err);
        } else {
            data->master_key_settings = malloc(sizeof(MifareDesfireKeySettings));
            memset(data->master_key_settings, 0, sizeof(MifareDesfireKeySettings));
            if(!mf_df_parse_get_key_settings_response(rx_buff, rx_len, data->master_key_settings)) {
                FURI_LOG_W(TAG, "Bad DESFire GET_KEY_SETTINGS response");
                free(data->master_key_settings);
                data->master_key_settings = NULL;
                continue;
            }

            MifareDesfireKeyVersion** key_version_head =
                &data->master_key_settings->key_version_head;
            for(uint8_t key_id = 0; key_id < data->master_key_settings->max_keys; key_id++) {
                tx_len = mf_df_prepare_get_key_version(tx_buff, key_id);
                err =
                    furi_hal_nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
                if(err != ERR_NONE) {
                    FURI_LOG_W(TAG, "Bad exchange getting key version, err: %d", err);
                    continue;
                }
                MifareDesfireKeyVersion* key_version = malloc(sizeof(MifareDesfireKeyVersion));
                memset(key_version, 0, sizeof(MifareDesfireKeyVersion));
                key_version->id = key_id;
                if(!mf_df_parse_get_key_version_response(rx_buff, rx_len, key_version)) {
                    FURI_LOG_W(TAG, "Bad DESFire GET_KEY_VERSION response");
                    free(key_version);
                    continue;
                }
                *key_version_head = key_version;
                key_version_head = &key_version->next;
            }
        }

        tx_len = mf_df_prepare_get_application_ids(tx_buff);
        err = furi_hal_nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
        if(err != ERR_NONE) {
            FURI_LOG_W(TAG, "Bad exchange getting application IDs, err: %d", err);
        } else {
            if(!mf_df_parse_get_application_ids_response(rx_buff, rx_len, &data->app_head)) {
                FURI_LOG_W(TAG, "Bad DESFire GET_APPLICATION_IDS response");
            }
        }

        for(MifareDesfireApplication* app = data->app_head; app; app = app->next) {
            tx_len = mf_df_prepare_select_application(tx_buff, app->id);
            err = furi_hal_nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
            if(!mf_df_parse_select_application_response(rx_buff, rx_len)) {
                FURI_LOG_W(TAG, "Bad exchange selecting application, err: %d", err);
                continue;
            }
            tx_len = mf_df_prepare_get_key_settings(tx_buff);
            err = furi_hal_nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
            if(err != ERR_NONE) {
                FURI_LOG_W(TAG, "Bad exchange getting key settings, err: %d", err);
            } else {
                app->key_settings = malloc(sizeof(MifareDesfireKeySettings));
                memset(app->key_settings, 0, sizeof(MifareDesfireKeySettings));
                if(!mf_df_parse_get_key_settings_response(rx_buff, rx_len, app->key_settings)) {
                    FURI_LOG_W(TAG, "Bad DESFire GET_KEY_SETTINGS response");
                    free(app->key_settings);
                    app->key_settings = NULL;
                    continue;
                }

                MifareDesfireKeyVersion** key_version_head = &app->key_settings->key_version_head;
                for(uint8_t key_id = 0; key_id < app->key_settings->max_keys; key_id++) {
                    tx_len = mf_df_prepare_get_key_version(tx_buff, key_id);
                    err = furi_hal_nfc_exchange_full(
                        tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
                    if(err != ERR_NONE) {
                        FURI_LOG_W(TAG, "Bad exchange getting key version, err: %d", err);
                        continue;
                    }
                    MifareDesfireKeyVersion* key_version = malloc(sizeof(MifareDesfireKeyVersion));
                    memset(key_version, 0, sizeof(MifareDesfireKeyVersion));
                    key_version->id = key_id;
                    if(!mf_df_parse_get_key_version_response(rx_buff, rx_len, key_version)) {
                        FURI_LOG_W(TAG, "Bad DESFire GET_KEY_VERSION response");
                        free(key_version);
                        continue;
                    }
                    *key_version_head = key_version;
                    key_version_head = &key_version->next;
                }
            }

            tx_len = mf_df_prepare_get_file_ids(tx_buff);
            err = furi_hal_nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
            if(err != ERR_NONE) {
                FURI_LOG_W(TAG, "Bad exchange getting file IDs, err: %d", err);
            } else {
                if(!mf_df_parse_get_file_ids_response(rx_buff, rx_len, &app->file_head)) {
                    FURI_LOG_W(TAG, "Bad DESFire GET_FILE_IDS response");
                }
            }

            for(MifareDesfireFile* file = app->file_head; file; file = file->next) {
                tx_len = mf_df_prepare_get_file_settings(tx_buff, file->id);
                err =
                    furi_hal_nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
                if(err != ERR_NONE) {
                    FURI_LOG_W(TAG, "Bad exchange getting file settings, err: %d", err);
                    continue;
                }
                if(!mf_df_parse_get_file_settings_response(rx_buff, rx_len, file)) {
                    FURI_LOG_W(TAG, "Bad DESFire GET_FILE_SETTINGS response");
                    continue;
                }
                switch(file->type) {
                case MifareDesfireFileTypeStandard:
                case MifareDesfireFileTypeBackup:
                    tx_len = mf_df_prepare_read_data(tx_buff, file->id, 0, 0);
                    break;
                case MifareDesfireFileTypeValue:
                    tx_len = mf_df_prepare_get_value(tx_buff, file->id);
                    break;
                case MifareDesfireFileTypeLinearRecord:
                case MifareDesfireFileTypeCyclicRecord:
                    tx_len = mf_df_prepare_read_records(tx_buff, file->id, 0, 0);
                    break;
                }
                err =
                    furi_hal_nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
                if(err != ERR_NONE) {
                    FURI_LOG_W(TAG, "Bad exchange reading file %d, err: %d", file->id, err);
                    continue;
                }
                if(!mf_df_parse_read_data_response(rx_buff, rx_len, file)) {
                    FURI_LOG_W(TAG, "Bad response reading file %d", file->id);
                    continue;
                }
            }
        }

        // Notify caller and exit
        if(nfc_worker->callback) {
            nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
        }
        break;
    }
}
