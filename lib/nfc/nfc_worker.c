#include "nfc_worker_i.h"
#include <furi_hal.h>

#include <platform.h>
#include "parsers/nfc_supported_card.h"

#define TAG "NfcWorker"

/***************************** NFC Worker API *******************************/

NfcWorker* nfc_worker_alloc() {
    NfcWorker* nfc_worker = malloc(sizeof(NfcWorker));

    // Worker thread attributes
    nfc_worker->thread = furi_thread_alloc_ex("NfcWorker", 8192, nfc_worker_task, nfc_worker);

    nfc_worker->callback = NULL;
    nfc_worker->context = NULL;
    nfc_worker->storage = furi_record_open(RECORD_STORAGE);

    // Initialize rfal
    while(furi_hal_nfc_is_busy()) {
        furi_delay_ms(10);
    }
    nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);

    nfc_worker->reader_analyzer = reader_analyzer_alloc(nfc_worker->storage);

    return nfc_worker;
}

void nfc_worker_free(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);

    furi_thread_free(nfc_worker->thread);

    furi_record_close(RECORD_STORAGE);

    reader_analyzer_free(nfc_worker->reader_analyzer);

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
        furi_delay_ms(10);
    }
    furi_hal_nfc_deinit();
    furi_hal_nfc_init();

    nfc_worker->callback = callback;
    nfc_worker->context = context;
    nfc_worker->dev_data = dev_data;
    nfc_worker_change_state(nfc_worker, state);
    furi_thread_start(nfc_worker->thread);
}

void nfc_worker_stop(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    furi_assert(nfc_worker->thread);
    if(furi_thread_get_state(nfc_worker->thread) != FuriThreadStateStopped) {
        furi_hal_nfc_stop();
        nfc_worker_change_state(nfc_worker, NfcWorkerStateStop);
        furi_thread_join(nfc_worker->thread);
    }
}

void nfc_worker_change_state(NfcWorker* nfc_worker, NfcWorkerState state) {
    nfc_worker->state = state;
}

/***************************** NFC Worker Thread *******************************/

int32_t nfc_worker_task(void* context) {
    NfcWorker* nfc_worker = context;

    furi_hal_nfc_exit_sleep();

    if(nfc_worker->state == NfcWorkerStateRead) {
        if(nfc_worker->dev_data->read_mode == NfcReadModeAuto) {
            nfc_worker_read(nfc_worker);
        } else {
            nfc_worker_read_type(nfc_worker);
        }
    } else if(nfc_worker->state == NfcWorkerStateUidEmulate) {
        nfc_worker_emulate_uid(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulateApdu) {
        nfc_worker_emulate_apdu(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateMfUltralightEmulate) {
        nfc_worker_emulate_mf_ultralight(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateMfClassicEmulate) {
        nfc_worker_emulate_mf_classic(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateMfClassicWrite) {
        nfc_worker_write_mf_classic(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateMfClassicUpdate) {
        nfc_worker_update_mf_classic(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadMfUltralightReadAuth) {
        nfc_worker_mf_ultralight_read_auth(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateMfClassicDictAttack) {
        nfc_worker_mf_classic_dict_attack(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateAnalyzeReader) {
        nfc_worker_analyze_reader(nfc_worker);
    }
    furi_hal_nfc_sleep();
    nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);

    return 0;
}

static bool nfc_worker_read_mf_ultralight(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    bool read_success = false;
    MfUltralightReader reader = {};
    MfUltralightData data = {};

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        reader_analyzer_prepare_tx_rx(nfc_worker->reader_analyzer, tx_rx, false);
        reader_analyzer_start(nfc_worker->reader_analyzer, ReaderAnalyzerModeDebugLog);
    }

    do {
        // Try to read supported card
        FURI_LOG_I(TAG, "Trying to read a supported card ...");
        for(size_t i = 0; i < NfcSupportedCardTypeEnd; i++) {
            if(nfc_supported_card[i].protocol == NfcDeviceProtocolMifareUl) {
                if(nfc_supported_card[i].verify(nfc_worker, tx_rx)) {
                    if(nfc_supported_card[i].read(nfc_worker, tx_rx)) {
                        read_success = true;
                        nfc_supported_card[i].parse(nfc_worker->dev_data);
                        break;
                    }
                } else {
                    furi_hal_nfc_sleep();
                }
            }
        }
        if(read_success) break;
        furi_hal_nfc_sleep();

        // Otherwise, try to read as usual
        if(!furi_hal_nfc_detect(&nfc_worker->dev_data->nfc_data, 200)) break;
        if(!mf_ul_read_card(tx_rx, &reader, &data)) break;
        // Copy data
        nfc_worker->dev_data->mf_ul_data = data;
        read_success = true;
    } while(false);

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        reader_analyzer_stop(nfc_worker->reader_analyzer);
    }

    return read_success;
}

static bool nfc_worker_read_mf_classic(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(nfc_worker->callback);
    bool read_success = false;

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        reader_analyzer_prepare_tx_rx(nfc_worker->reader_analyzer, tx_rx, false);
        reader_analyzer_start(nfc_worker->reader_analyzer, ReaderAnalyzerModeDebugLog);
    }

    do {
        // Try to read supported card
        FURI_LOG_I(TAG, "Trying to read a supported card ...");
        for(size_t i = 0; i < NfcSupportedCardTypeEnd; i++) {
            if(nfc_supported_card[i].protocol == NfcDeviceProtocolMifareClassic) {
                if(nfc_supported_card[i].verify(nfc_worker, tx_rx)) {
                    if(nfc_supported_card[i].read(nfc_worker, tx_rx)) {
                        read_success = true;
                        nfc_supported_card[i].parse(nfc_worker->dev_data);
                        break;
                    }
                } else {
                    furi_hal_nfc_sleep();
                }
            }
        }
        if(read_success) break;
        // Try to read card with key cache
        FURI_LOG_I(TAG, "Search for key cache ...");
        if(nfc_worker->callback(NfcWorkerEventReadMfClassicLoadKeyCache, nfc_worker->context)) {
            FURI_LOG_I(TAG, "Load keys cache success. Start reading");
            uint8_t sectors_read =
                mf_classic_update_card(tx_rx, &nfc_worker->dev_data->mf_classic_data);
            uint8_t sectors_total =
                mf_classic_get_total_sectors_num(nfc_worker->dev_data->mf_classic_data.type);
            FURI_LOG_I(TAG, "Read %d sectors out of %d total", sectors_read, sectors_total);
            read_success = mf_classic_is_card_read(&nfc_worker->dev_data->mf_classic_data);
        }
    } while(false);

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        reader_analyzer_stop(nfc_worker->reader_analyzer);
    }
    return read_success;
}

static bool nfc_worker_read_mf_desfire(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    bool read_success = false;
    MifareDesfireData* data = &nfc_worker->dev_data->mf_df_data;

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        reader_analyzer_prepare_tx_rx(nfc_worker->reader_analyzer, tx_rx, false);
        reader_analyzer_start(nfc_worker->reader_analyzer, ReaderAnalyzerModeDebugLog);
    }

    do {
        if(!furi_hal_nfc_detect(&nfc_worker->dev_data->nfc_data, 300)) break;
        if(!mf_df_read_card(tx_rx, data)) break;
        read_success = true;
    } while(false);

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        reader_analyzer_stop(nfc_worker->reader_analyzer);
    }

    return read_success;
}

static bool nfc_worker_read_bank_card(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    bool read_success = false;
    EmvApplication emv_app = {};
    EmvData* result = &nfc_worker->dev_data->emv_data;

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        reader_analyzer_prepare_tx_rx(nfc_worker->reader_analyzer, tx_rx, false);
        reader_analyzer_start(nfc_worker->reader_analyzer, ReaderAnalyzerModeDebugLog);
    }

    // Bank cards require strong field to start application. If we find AID, try at least several
    // times to start EMV application
    uint8_t start_application_attempts = 0;
    while(start_application_attempts < 3) {
        if(nfc_worker->state != NfcWorkerStateRead) break;
        start_application_attempts++;
        if(!furi_hal_nfc_detect(&nfc_worker->dev_data->nfc_data, 300)) break;
        if(emv_read_bank_card(tx_rx, &emv_app)) {
            FURI_LOG_D(TAG, "Bank card number read from %d attempt", start_application_attempts);
            break;
        } else if(emv_app.aid_len && !emv_app.app_started) {
            FURI_LOG_D(
                TAG,
                "AID found but failed to start EMV app from %d attempt",
                start_application_attempts);
            furi_hal_nfc_sleep();
            continue;
        } else {
            FURI_LOG_D(TAG, "Failed to find AID");
            break;
        }
    }
    // Copy data
    if(emv_app.aid_len) {
        result->aid_len = emv_app.aid_len;
        memcpy(result->aid, emv_app.aid, result->aid_len);
        read_success = true;
    }
    if(emv_app.card_number_len) {
        result->number_len = emv_app.card_number_len;
        memcpy(result->number, emv_app.card_number, result->number_len);
    }
    if(emv_app.name_found) {
        memcpy(result->name, emv_app.name, sizeof(emv_app.name));
    }
    if(emv_app.exp_month) {
        result->exp_mon = emv_app.exp_month;
        result->exp_year = emv_app.exp_year;
    }
    if(emv_app.country_code) {
        result->country_code = emv_app.country_code;
    }
    if(emv_app.currency_code) {
        result->currency_code = emv_app.currency_code;
    }

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        reader_analyzer_stop(nfc_worker->reader_analyzer);
    }

    return read_success;
}

static bool nfc_worker_read_nfca(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;

    bool card_read = false;
    furi_hal_nfc_sleep();
    if(mf_ul_check_card_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak)) {
        FURI_LOG_I(TAG, "Mifare Ultralight / NTAG detected");
        nfc_worker->dev_data->protocol = NfcDeviceProtocolMifareUl;
        card_read = nfc_worker_read_mf_ultralight(nfc_worker, tx_rx);
    } else if(mf_classic_check_card_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak)) {
        FURI_LOG_I(TAG, "Mifare Classic detected");
        nfc_worker->dev_data->protocol = NfcDeviceProtocolMifareClassic;
        nfc_worker->dev_data->mf_classic_data.type =
            mf_classic_get_classic_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak);
        card_read = nfc_worker_read_mf_classic(nfc_worker, tx_rx);
    } else if(mf_df_check_card_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak)) {
        FURI_LOG_I(TAG, "Mifare DESFire detected");
        nfc_worker->dev_data->protocol = NfcDeviceProtocolMifareDesfire;
        if(!nfc_worker_read_mf_desfire(nfc_worker, tx_rx)) {
            FURI_LOG_I(TAG, "Unknown card. Save UID");
            nfc_worker->dev_data->protocol = NfcDeviceProtocolUnknown;
        }
        card_read = true;
    } else if(nfc_data->interface == FuriHalNfcInterfaceIsoDep) {
        FURI_LOG_I(TAG, "ISO14443-4 card detected");
        nfc_worker->dev_data->protocol = NfcDeviceProtocolEMV;
        if(!nfc_worker_read_bank_card(nfc_worker, tx_rx)) {
            FURI_LOG_I(TAG, "Unknown card. Save UID");
            nfc_worker->dev_data->protocol = NfcDeviceProtocolUnknown;
        }
        card_read = true;
    } else {
        nfc_worker->dev_data->protocol = NfcDeviceProtocolUnknown;
        card_read = true;
    }

    return card_read;
}

void nfc_worker_read(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    furi_assert(nfc_worker->callback);

    nfc_device_data_clear(nfc_worker->dev_data);
    NfcDeviceData* dev_data = nfc_worker->dev_data;
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    FuriHalNfcTxRxContext tx_rx = {};
    NfcWorkerEvent event = 0;
    bool card_not_detected_notified = false;

    while(nfc_worker->state == NfcWorkerStateRead) {
        if(furi_hal_nfc_detect(nfc_data, 300)) {
            // Process first found device
            nfc_worker->callback(NfcWorkerEventCardDetected, nfc_worker->context);
            card_not_detected_notified = false;
            if(nfc_data->type == FuriHalNfcTypeA) {
                if(nfc_worker_read_nfca(nfc_worker, &tx_rx)) {
                    if(dev_data->protocol == NfcDeviceProtocolMifareUl) {
                        event = NfcWorkerEventReadMfUltralight;
                        break;
                    } else if(dev_data->protocol == NfcDeviceProtocolMifareClassic) {
                        event = NfcWorkerEventReadMfClassicDone;
                        break;
                    } else if(dev_data->protocol == NfcDeviceProtocolMifareDesfire) {
                        event = NfcWorkerEventReadMfDesfire;
                        break;
                    } else if(dev_data->protocol == NfcDeviceProtocolEMV) {
                        event = NfcWorkerEventReadBankCard;
                        break;
                    } else if(dev_data->protocol == NfcDeviceProtocolUnknown) {
                        event = NfcWorkerEventReadUidNfcA;
                        break;
                    }
                } else {
                    if(dev_data->protocol == NfcDeviceProtocolMifareClassic) {
                        event = NfcWorkerEventReadMfClassicDictAttackRequired;
                        break;
                    }
                }
            } else if(nfc_data->type == FuriHalNfcTypeB) {
                event = NfcWorkerEventReadUidNfcB;
                break;
            } else if(nfc_data->type == FuriHalNfcTypeF) {
                event = NfcWorkerEventReadUidNfcF;
                break;
            } else if(nfc_data->type == FuriHalNfcTypeV) {
                event = NfcWorkerEventReadUidNfcV;
                break;
            }
        } else {
            if(!card_not_detected_notified) {
                nfc_worker->callback(NfcWorkerEventNoCardDetected, nfc_worker->context);
                card_not_detected_notified = true;
            }
        }
        furi_hal_nfc_sleep();
        furi_delay_ms(100);
    }
    // Notify caller and exit
    if(event > NfcWorkerEventReserved) {
        nfc_worker->callback(event, nfc_worker->context);
    }
}

void nfc_worker_read_type(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    furi_assert(nfc_worker->callback);

    NfcReadMode read_mode = nfc_worker->dev_data->read_mode;
    nfc_device_data_clear(nfc_worker->dev_data);
    NfcDeviceData* dev_data = nfc_worker->dev_data;
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    FuriHalNfcTxRxContext tx_rx = {};
    NfcWorkerEvent event = 0;
    bool card_not_detected_notified = false;

    while(nfc_worker->state == NfcWorkerStateRead) {
        if(furi_hal_nfc_detect(nfc_data, 300)) {
            FURI_LOG_D(TAG, "Card detected");
            furi_hal_nfc_sleep();
            // Process first found device
            nfc_worker->callback(NfcWorkerEventCardDetected, nfc_worker->context);
            card_not_detected_notified = false;
            if(nfc_data->type == FuriHalNfcTypeA) {
                if(read_mode == NfcReadModeMfClassic) {
                    nfc_worker->dev_data->protocol = NfcDeviceProtocolMifareClassic;
                    nfc_worker->dev_data->mf_classic_data.type = mf_classic_get_classic_type(
                        nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak);
                    if(nfc_worker_read_mf_classic(nfc_worker, &tx_rx)) {
                        FURI_LOG_D(TAG, "Card read");
                        dev_data->protocol = NfcDeviceProtocolMifareClassic;
                        event = NfcWorkerEventReadMfClassicDone;
                        break;
                    } else {
                        FURI_LOG_D(TAG, "Card read failed");
                        dev_data->protocol = NfcDeviceProtocolMifareClassic;
                        event = NfcWorkerEventReadMfClassicDictAttackRequired;
                        break;
                    }
                } else if(read_mode == NfcReadModeMfUltralight) {
                    FURI_LOG_I(TAG, "Mifare Ultralight / NTAG");
                    nfc_worker->dev_data->protocol = NfcDeviceProtocolMifareUl;
                    if(nfc_worker_read_mf_ultralight(nfc_worker, &tx_rx)) {
                        event = NfcWorkerEventReadMfUltralight;
                        break;
                    }
                } else if(read_mode == NfcReadModeMfDesfire) {
                    nfc_worker->dev_data->protocol = NfcDeviceProtocolMifareDesfire;
                    if(nfc_worker_read_mf_desfire(nfc_worker, &tx_rx)) {
                        event = NfcWorkerEventReadMfDesfire;
                        break;
                    }
                } else if(read_mode == NfcReadModeEMV) {
                    nfc_worker->dev_data->protocol = NfcDeviceProtocolEMV;
                    if(nfc_worker_read_bank_card(nfc_worker, &tx_rx)) {
                        event = NfcWorkerEventReadBankCard;
                        break;
                    }
                } else if(read_mode == NfcReadModeNFCA) {
                    nfc_worker->dev_data->protocol = NfcDeviceProtocolUnknown;
                    event = NfcWorkerEventReadUidNfcA;
                    break;
                }
            }
        } else {
            if(!card_not_detected_notified) {
                nfc_worker->callback(NfcWorkerEventNoCardDetected, nfc_worker->context);
                card_not_detected_notified = true;
            }
        }
        furi_hal_nfc_sleep();
        furi_delay_ms(100);
    }
    // Notify caller and exit
    if(event > NfcWorkerEventReserved) {
        nfc_worker->callback(event, nfc_worker->context);
    }
}

void nfc_worker_emulate_uid(NfcWorker* nfc_worker) {
    FuriHalNfcTxRxContext tx_rx = {};
    FuriHalNfcDevData* data = &nfc_worker->dev_data->nfc_data;
    NfcReaderRequestData* reader_data = &nfc_worker->dev_data->reader_data;

    // TODO add support for RATS
    // Need to save ATS to support ISO-14443A-4 emulation

    while(nfc_worker->state == NfcWorkerStateUidEmulate) {
        if(furi_hal_nfc_listen(data->uid, data->uid_len, data->atqa, data->sak, false, 100)) {
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

void nfc_worker_emulate_apdu(NfcWorker* nfc_worker) {
    FuriHalNfcTxRxContext tx_rx = {};
    FuriHalNfcDevData params = {
        .uid = {0xCF, 0x72, 0xd4, 0x40},
        .uid_len = 4,
        .atqa = {0x00, 0x04},
        .sak = 0x20,
        .type = FuriHalNfcTypeA,
    };

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        reader_analyzer_prepare_tx_rx(nfc_worker->reader_analyzer, &tx_rx, true);
        reader_analyzer_start(nfc_worker->reader_analyzer, ReaderAnalyzerModeDebugLog);
    }

    while(nfc_worker->state == NfcWorkerStateEmulateApdu) { //-V1044
        if(furi_hal_nfc_listen(params.uid, params.uid_len, params.atqa, params.sak, false, 300)) {
            FURI_LOG_D(TAG, "POS terminal detected");
            if(emv_card_emulation(&tx_rx)) {
                FURI_LOG_D(TAG, "EMV card emulated");
            }
        } else {
            FURI_LOG_D(TAG, "Can't find reader");
        }
        furi_hal_nfc_sleep();
        furi_delay_ms(20);
    }

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        reader_analyzer_stop(nfc_worker->reader_analyzer);
    }
}

void nfc_worker_mf_ultralight_auth_received_callback(MfUltralightAuth auth, void* context) {
    furi_assert(context);

    NfcWorker* nfc_worker = context;
    nfc_worker->dev_data->mf_ul_auth = auth;
    if(nfc_worker->callback) {
        nfc_worker->callback(NfcWorkerEventMfUltralightPwdAuth, nfc_worker->context);
    }
}

void nfc_worker_emulate_mf_ultralight(NfcWorker* nfc_worker) {
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    MfUltralightEmulator emulator = {};
    mf_ul_prepare_emulation(&emulator, &nfc_worker->dev_data->mf_ul_data);

    // TODO rework with reader analyzer
    emulator.auth_received_callback = nfc_worker_mf_ultralight_auth_received_callback;
    emulator.context = nfc_worker;

    while(nfc_worker->state == NfcWorkerStateMfUltralightEmulate) {
        mf_ul_reset_emulation(&emulator, true);
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

static bool nfc_worker_mf_get_b_key_from_sector_trailer(
    FuriHalNfcTxRxContext* tx_rx,
    uint16_t sector,
    uint64_t key,
    uint64_t* found_key) {
    // Some access conditions allow reading B key via A key

    uint8_t block = mf_classic_get_sector_trailer_block_num_by_sector(sector);

    Crypto1 crypto = {};
    MfClassicBlock block_tmp = {};
    MfClassicAuthContext auth_context = {.sector = sector, .key_a = MF_CLASSIC_NO_KEY, .key_b = 0};

    furi_hal_nfc_sleep();

    if(mf_classic_auth_attempt(tx_rx, &crypto, &auth_context, key)) {
        if(mf_classic_read_block(tx_rx, &crypto, block, &block_tmp)) {
            *found_key = nfc_util_bytes2num(&block_tmp.value[10], sizeof(uint8_t) * 6);

            return *found_key;
        }
    }

    return false;
}

static void nfc_worker_mf_classic_key_attack(
    NfcWorker* nfc_worker,
    uint64_t key,
    FuriHalNfcTxRxContext* tx_rx,
    uint16_t start_sector) {
    furi_assert(nfc_worker);
    furi_assert(nfc_worker->callback);

    bool card_found_notified = true;
    bool card_removed_notified = false;

    MfClassicData* data = &nfc_worker->dev_data->mf_classic_data;
    NfcMfClassicDictAttackData* dict_attack_data =
        &nfc_worker->dev_data->mf_classic_dict_attack_data;
    uint32_t total_sectors = mf_classic_get_total_sectors_num(data->type);

    furi_assert(start_sector < total_sectors);

    nfc_worker->callback(NfcWorkerEventKeyAttackStart, nfc_worker->context);

    // Check every sector's A and B keys with the given key
    for(size_t i = start_sector; i < total_sectors; i++) {
        nfc_worker->callback(NfcWorkerEventKeyAttackNextSector, nfc_worker->context);
        dict_attack_data->current_sector = i;
        furi_hal_nfc_sleep();
        if(furi_hal_nfc_activate_nfca(200, NULL)) {
            furi_hal_nfc_sleep();
            if(!card_found_notified) {
                nfc_worker->callback(NfcWorkerEventCardDetected, nfc_worker->context);
                card_found_notified = true;
                card_removed_notified = false;
            }
            uint8_t block_num = mf_classic_get_sector_trailer_block_num_by_sector(i);
            if(mf_classic_is_sector_read(data, i)) continue;
            if(!mf_classic_is_key_found(data, i, MfClassicKeyA)) {
                FURI_LOG_D(
                    TAG,
                    "Trying A key for sector %d, key: %04lx%08lx",
                    i,
                    (uint32_t)(key >> 32),
                    (uint32_t)key);
                if(mf_classic_authenticate(tx_rx, block_num, key, MfClassicKeyA)) {
                    mf_classic_set_key_found(data, i, MfClassicKeyA, key);
                    FURI_LOG_D(TAG, "Key found");
                    nfc_worker->callback(NfcWorkerEventFoundKeyA, nfc_worker->context);

                    uint64_t found_key;
                    if(nfc_worker_mf_get_b_key_from_sector_trailer(tx_rx, i, key, &found_key)) {
                        FURI_LOG_D(TAG, "Found B key via reading sector %d", i);
                        mf_classic_set_key_found(data, i, MfClassicKeyB, found_key);

                        if(nfc_worker->state == NfcWorkerStateMfClassicDictAttack) {
                            nfc_worker->callback(NfcWorkerEventFoundKeyB, nfc_worker->context);
                        }
                    }
                }
            }
            if(!mf_classic_is_key_found(data, i, MfClassicKeyB)) {
                FURI_LOG_D(
                    TAG,
                    "Trying B key for sector %d, key: %04lx%08lx",
                    i,
                    (uint32_t)(key >> 32),
                    (uint32_t)key);
                if(mf_classic_authenticate(tx_rx, block_num, key, MfClassicKeyB)) {
                    mf_classic_set_key_found(data, i, MfClassicKeyB, key);
                    FURI_LOG_D(TAG, "Key found");
                    nfc_worker->callback(NfcWorkerEventFoundKeyB, nfc_worker->context);
                }
            }

            if(mf_classic_is_sector_read(data, i)) continue;
            mf_classic_read_sector(tx_rx, data, i);
        } else {
            if(!card_removed_notified) {
                nfc_worker->callback(NfcWorkerEventNoCardDetected, nfc_worker->context);
                card_removed_notified = true;
                card_found_notified = false;
            }
        }
        if(nfc_worker->state != NfcWorkerStateMfClassicDictAttack) break;
    }
    nfc_worker->callback(NfcWorkerEventKeyAttackStop, nfc_worker->context);
}

void nfc_worker_mf_classic_dict_attack(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    furi_assert(nfc_worker->callback);

    MfClassicData* data = &nfc_worker->dev_data->mf_classic_data;
    NfcMfClassicDictAttackData* dict_attack_data =
        &nfc_worker->dev_data->mf_classic_dict_attack_data;
    uint32_t total_sectors = mf_classic_get_total_sectors_num(data->type);
    uint64_t key = 0;
    uint64_t prev_key = 0;
    FuriHalNfcTxRxContext tx_rx = {};
    bool card_found_notified = true;
    bool card_removed_notified = false;

    // Load dictionary
    MfClassicDict* dict = dict_attack_data->dict;
    if(!dict) {
        FURI_LOG_E(TAG, "Dictionary not found");
        nfc_worker->callback(NfcWorkerEventNoDictFound, nfc_worker->context);
        return;
    }

    FURI_LOG_D(
        TAG, "Start Dictionary attack, Key Count %lu", mf_classic_dict_get_total_keys(dict));
    for(size_t i = 0; i < total_sectors; i++) {
        FURI_LOG_I(TAG, "Sector %d", i);
        nfc_worker->callback(NfcWorkerEventNewSector, nfc_worker->context);
        uint8_t block_num = mf_classic_get_sector_trailer_block_num_by_sector(i);
        if(mf_classic_is_sector_read(data, i)) continue;
        bool is_key_a_found = mf_classic_is_key_found(data, i, MfClassicKeyA);
        bool is_key_b_found = mf_classic_is_key_found(data, i, MfClassicKeyB);
        uint16_t key_index = 0;
        while(mf_classic_dict_get_next_key(dict, &key)) {
            FURI_LOG_T(TAG, "Key %d", key_index);
            if(++key_index % NFC_DICT_KEY_BATCH_SIZE == 0) {
                nfc_worker->callback(NfcWorkerEventNewDictKeyBatch, nfc_worker->context);
            }
            furi_hal_nfc_sleep();
            uint32_t cuid;
            if(furi_hal_nfc_activate_nfca(200, &cuid)) {
                bool deactivated = false;
                if(!card_found_notified) {
                    nfc_worker->callback(NfcWorkerEventCardDetected, nfc_worker->context);
                    card_found_notified = true;
                    card_removed_notified = false;
                    nfc_worker_mf_classic_key_attack(nfc_worker, prev_key, &tx_rx, i);
                    deactivated = true;
                }
                FURI_LOG_D(
                    TAG,
                    "Try to auth to sector %d with key %04lx%08lx",
                    i,
                    (uint32_t)(key >> 32),
                    (uint32_t)key);
                if(!is_key_a_found) {
                    is_key_a_found = mf_classic_is_key_found(data, i, MfClassicKeyA);
                    if(mf_classic_authenticate_skip_activate(
                           &tx_rx, block_num, key, MfClassicKeyA, !deactivated, cuid)) {
                        mf_classic_set_key_found(data, i, MfClassicKeyA, key);
                        FURI_LOG_D(TAG, "Key A found");
                        nfc_worker->callback(NfcWorkerEventFoundKeyA, nfc_worker->context);

                        uint64_t found_key;
                        if(nfc_worker_mf_get_b_key_from_sector_trailer(
                               &tx_rx, i, key, &found_key)) {
                            FURI_LOG_D(TAG, "Found B key via reading sector %d", i);
                            mf_classic_set_key_found(data, i, MfClassicKeyB, found_key);

                            if(nfc_worker->state == NfcWorkerStateMfClassicDictAttack) {
                                nfc_worker->callback(NfcWorkerEventFoundKeyB, nfc_worker->context);
                            }

                            nfc_worker_mf_classic_key_attack(nfc_worker, found_key, &tx_rx, i + 1);
                            break;
                        }
                        nfc_worker_mf_classic_key_attack(nfc_worker, key, &tx_rx, i + 1);
                    }
                    furi_hal_nfc_sleep();
                    deactivated = true;
                } else {
                    mf_classic_set_key_not_found(data, i, MfClassicKeyA);
                    is_key_a_found = false;
                    FURI_LOG_D(TAG, "Key %dA not found in attack", i);
                }
                if(!is_key_b_found) {
                    is_key_b_found = mf_classic_is_key_found(data, i, MfClassicKeyB);
                    if(mf_classic_authenticate_skip_activate(
                           &tx_rx, block_num, key, MfClassicKeyB, !deactivated, cuid)) {
                        FURI_LOG_D(TAG, "Key B found");
                        mf_classic_set_key_found(data, i, MfClassicKeyB, key);
                        nfc_worker->callback(NfcWorkerEventFoundKeyB, nfc_worker->context);
                        nfc_worker_mf_classic_key_attack(nfc_worker, key, &tx_rx, i + 1);
                    }
                    deactivated = true;
                } else {
                    mf_classic_set_key_not_found(data, i, MfClassicKeyB);
                    is_key_b_found = false;
                    FURI_LOG_D(TAG, "Key %dB not found in attack", i);
                }
                if(is_key_a_found && is_key_b_found) break;
                if(nfc_worker->state != NfcWorkerStateMfClassicDictAttack) break;
            } else {
                if(!card_removed_notified) {
                    nfc_worker->callback(NfcWorkerEventNoCardDetected, nfc_worker->context);
                    card_removed_notified = true;
                    card_found_notified = false;
                }
                if(nfc_worker->state != NfcWorkerStateMfClassicDictAttack) break;
            }
            memcpy(&prev_key, &key, sizeof(key));
        }
        if(nfc_worker->state != NfcWorkerStateMfClassicDictAttack) break;
        mf_classic_read_sector(&tx_rx, data, i);
        mf_classic_dict_rewind(dict);
    }
    if(nfc_worker->state == NfcWorkerStateMfClassicDictAttack) {
        nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
    } else {
        nfc_worker->callback(NfcWorkerEventAborted, nfc_worker->context);
    }
}

void nfc_worker_emulate_mf_classic(NfcWorker* nfc_worker) {
    FuriHalNfcTxRxContext tx_rx = {};
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    MfClassicEmulator emulator = {
        .cuid = nfc_util_bytes2num(&nfc_data->uid[nfc_data->uid_len - 4], 4),
        .data = nfc_worker->dev_data->mf_classic_data,
        .data_changed = false,
    };
    NfcaSignal* nfca_signal = nfca_signal_alloc();
    tx_rx.nfca_signal = nfca_signal;

    rfal_platform_spi_acquire();

    furi_hal_nfc_listen_start(nfc_data);
    while(nfc_worker->state == NfcWorkerStateMfClassicEmulate) { //-V1044
        if(furi_hal_nfc_listen_rx(&tx_rx, 300)) {
            mf_classic_emulator(&emulator, &tx_rx);
        }
    }
    if(emulator.data_changed) {
        nfc_worker->dev_data->mf_classic_data = emulator.data;
        if(nfc_worker->callback) {
            nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
        }
        emulator.data_changed = false;
    }

    nfca_signal_free(nfca_signal);

    rfal_platform_spi_release();
}

void nfc_worker_write_mf_classic(NfcWorker* nfc_worker) {
    FuriHalNfcTxRxContext tx_rx = {};
    bool card_found_notified = false;
    FuriHalNfcDevData nfc_data = {};
    MfClassicData* src_data = &nfc_worker->dev_data->mf_classic_data;
    MfClassicData dest_data = *src_data;

    while(nfc_worker->state == NfcWorkerStateMfClassicWrite) {
        if(furi_hal_nfc_detect(&nfc_data, 200)) {
            if(!card_found_notified) {
                nfc_worker->callback(NfcWorkerEventCardDetected, nfc_worker->context);
                card_found_notified = true;
            }
            furi_hal_nfc_sleep();

            FURI_LOG_I(TAG, "Check low level nfc data");
            if(memcmp(&nfc_data, &nfc_worker->dev_data->nfc_data, sizeof(FuriHalNfcDevData)) !=
               0) {
                FURI_LOG_E(TAG, "Wrong card");
                nfc_worker->callback(NfcWorkerEventWrongCard, nfc_worker->context);
                break;
            }

            FURI_LOG_I(TAG, "Check mf classic type");
            MfClassicType type =
                mf_classic_get_classic_type(nfc_data.atqa[0], nfc_data.atqa[1], nfc_data.sak);
            if(type != nfc_worker->dev_data->mf_classic_data.type) {
                FURI_LOG_E(TAG, "Wrong mf classic type");
                nfc_worker->callback(NfcWorkerEventWrongCard, nfc_worker->context);
                break;
            }

            // Set blocks not read
            mf_classic_set_sector_data_not_read(&dest_data);
            FURI_LOG_I(TAG, "Updating card sectors");
            uint8_t total_sectors = mf_classic_get_total_sectors_num(type);
            bool write_success = true;
            for(uint8_t i = 0; i < total_sectors; i++) {
                FURI_LOG_I(TAG, "Reading sector %d", i);
                mf_classic_read_sector(&tx_rx, &dest_data, i);
                bool old_data_read = mf_classic_is_sector_data_read(src_data, i);
                bool new_data_read = mf_classic_is_sector_data_read(&dest_data, i);
                if(old_data_read != new_data_read) {
                    FURI_LOG_E(TAG, "Failed to update sector %d", i);
                    write_success = false;
                    break;
                }
                if(nfc_worker->state != NfcWorkerStateMfClassicWrite) break;
                if(!mf_classic_write_sector(&tx_rx, &dest_data, src_data, i)) {
                    FURI_LOG_E(TAG, "Failed to write %d sector", i);
                    write_success = false;
                    break;
                }
            }
            if(nfc_worker->state != NfcWorkerStateMfClassicWrite) break;
            if(write_success) {
                nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
                break;
            } else {
                nfc_worker->callback(NfcWorkerEventFail, nfc_worker->context);
                break;
            }

        } else {
            if(card_found_notified) {
                nfc_worker->callback(NfcWorkerEventNoCardDetected, nfc_worker->context);
                card_found_notified = false;
            }
        }
        furi_delay_ms(300);
    }
}

void nfc_worker_update_mf_classic(NfcWorker* nfc_worker) {
    FuriHalNfcTxRxContext tx_rx = {};
    bool card_found_notified = false;
    FuriHalNfcDevData nfc_data = {};
    MfClassicData* old_data = &nfc_worker->dev_data->mf_classic_data;
    MfClassicData new_data = *old_data;

    while(nfc_worker->state == NfcWorkerStateMfClassicUpdate) {
        if(furi_hal_nfc_detect(&nfc_data, 200)) {
            if(!card_found_notified) {
                nfc_worker->callback(NfcWorkerEventCardDetected, nfc_worker->context);
                card_found_notified = true;
            }
            furi_hal_nfc_sleep();

            FURI_LOG_I(TAG, "Check low level nfc data");
            if(memcmp(&nfc_data, &nfc_worker->dev_data->nfc_data, sizeof(FuriHalNfcDevData)) !=
               0) {
                FURI_LOG_E(TAG, "Low level nfc data mismatch");
                nfc_worker->callback(NfcWorkerEventWrongCard, nfc_worker->context);
                break;
            }

            FURI_LOG_I(TAG, "Check MF classic type");
            MfClassicType type =
                mf_classic_get_classic_type(nfc_data.atqa[0], nfc_data.atqa[1], nfc_data.sak);
            if(type != nfc_worker->dev_data->mf_classic_data.type) {
                FURI_LOG_E(TAG, "MF classic type mismatch");
                nfc_worker->callback(NfcWorkerEventWrongCard, nfc_worker->context);
                break;
            }

            // Set blocks not read
            mf_classic_set_sector_data_not_read(&new_data);
            FURI_LOG_I(TAG, "Updating card sectors");
            uint8_t total_sectors = mf_classic_get_total_sectors_num(type);
            bool update_success = true;
            for(uint8_t i = 0; i < total_sectors; i++) {
                FURI_LOG_I(TAG, "Reading sector %d", i);
                mf_classic_read_sector(&tx_rx, &new_data, i);
                bool old_data_read = mf_classic_is_sector_data_read(old_data, i);
                bool new_data_read = mf_classic_is_sector_data_read(&new_data, i);
                if(old_data_read != new_data_read) {
                    FURI_LOG_E(TAG, "Failed to update sector %d", i);
                    update_success = false;
                    break;
                }
                if(nfc_worker->state != NfcWorkerStateMfClassicUpdate) break;
            }
            if(nfc_worker->state != NfcWorkerStateMfClassicUpdate) break;

            // Check updated data
            if(update_success) {
                *old_data = new_data;
                nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
                break;
            }
        } else {
            if(card_found_notified) {
                nfc_worker->callback(NfcWorkerEventNoCardDetected, nfc_worker->context);
                card_found_notified = false;
            }
        }
        furi_delay_ms(300);
    }
}

void nfc_worker_mf_ultralight_read_auth(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    furi_assert(nfc_worker->callback);

    MfUltralightData* data = &nfc_worker->dev_data->mf_ul_data;
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    FuriHalNfcTxRxContext tx_rx = {};
    MfUltralightReader reader = {};
    mf_ul_reset(data);

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        reader_analyzer_prepare_tx_rx(nfc_worker->reader_analyzer, &tx_rx, true);
        reader_analyzer_start(nfc_worker->reader_analyzer, ReaderAnalyzerModeDebugLog);
    }

    uint32_t key = 0;
    uint16_t pack = 0;
    while(nfc_worker->state == NfcWorkerStateReadMfUltralightReadAuth) {
        furi_hal_nfc_sleep();
        if(furi_hal_nfc_detect(nfc_data, 300) && nfc_data->type == FuriHalNfcTypeA) {
            if(mf_ul_check_card_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak)) {
                nfc_worker->callback(NfcWorkerEventCardDetected, nfc_worker->context);
                if(data->auth_method == MfUltralightAuthMethodManual ||
                   data->auth_method == MfUltralightAuthMethodAuto) {
                    nfc_worker->callback(NfcWorkerEventMfUltralightPassKey, nfc_worker->context);
                    key = nfc_util_bytes2num(data->auth_key, 4);
                } else if(data->auth_method == MfUltralightAuthMethodAmeebo) {
                    key = mf_ul_pwdgen_amiibo(nfc_data);
                } else if(data->auth_method == MfUltralightAuthMethodXiaomi) {
                    key = mf_ul_pwdgen_xiaomi(nfc_data);
                } else {
                    FURI_LOG_E(TAG, "Incorrect auth method");
                    break;
                }

                data->auth_success = mf_ultralight_authenticate(&tx_rx, key, &pack);

                if(!data->auth_success) {
                    // Reset card
                    furi_hal_nfc_sleep();
                    if(!furi_hal_nfc_activate_nfca(300, NULL)) {
                        nfc_worker->callback(NfcWorkerEventFail, nfc_worker->context);
                        break;
                    }
                }

                mf_ul_read_card(&tx_rx, &reader, data);
                if(data->auth_success) {
                    MfUltralightConfigPages* config_pages = mf_ultralight_get_config_pages(data);
                    if(config_pages != NULL) {
                        config_pages->auth_data.pwd.value = REVERSE_BYTES_U32(key);
                        config_pages->auth_data.pack.value = pack;
                    }
                    nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
                    break;
                } else {
                    nfc_worker->callback(NfcWorkerEventFail, nfc_worker->context);
                    break;
                }
            } else {
                nfc_worker->callback(NfcWorkerEventWrongCardDetected, nfc_worker->context);
                furi_delay_ms(10);
            }
        } else {
            nfc_worker->callback(NfcWorkerEventNoCardDetected, nfc_worker->context);
            furi_delay_ms(10);
        }
    }

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        reader_analyzer_stop(nfc_worker->reader_analyzer);
    }
}

static void nfc_worker_reader_analyzer_callback(ReaderAnalyzerEvent event, void* context) {
    furi_assert(context);
    NfcWorker* nfc_worker = context;

    if((nfc_worker->state == NfcWorkerStateAnalyzeReader) &&
       (event == ReaderAnalyzerEventMfkeyCollected)) {
        if(nfc_worker->callback) {
            nfc_worker->callback(NfcWorkerEventDetectReaderMfkeyCollected, nfc_worker->context);
        }
    }
}

void nfc_worker_analyze_reader(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    furi_assert(nfc_worker->callback);

    FuriHalNfcTxRxContext tx_rx = {};

    ReaderAnalyzer* reader_analyzer = nfc_worker->reader_analyzer;
    FuriHalNfcDevData* nfc_data = NULL;
    if(nfc_worker->dev_data->protocol == NfcDeviceProtocolMifareClassic) {
        nfc_data = &nfc_worker->dev_data->nfc_data;
        reader_analyzer_set_nfc_data(reader_analyzer, nfc_data);
    } else {
        nfc_data = reader_analyzer_get_nfc_data(reader_analyzer);
    }
    MfClassicEmulator emulator = {
        .cuid = nfc_util_bytes2num(&nfc_data->uid[nfc_data->uid_len - 4], 4),
        .data = nfc_worker->dev_data->mf_classic_data,
        .data_changed = false,
    };
    NfcaSignal* nfca_signal = nfca_signal_alloc();
    tx_rx.nfca_signal = nfca_signal;
    reader_analyzer_prepare_tx_rx(reader_analyzer, &tx_rx, true);
    reader_analyzer_start(nfc_worker->reader_analyzer, ReaderAnalyzerModeMfkey);
    reader_analyzer_set_callback(reader_analyzer, nfc_worker_reader_analyzer_callback, nfc_worker);

    rfal_platform_spi_acquire();

    FURI_LOG_D(TAG, "Start reader analyzer");

    uint8_t reader_no_data_received_cnt = 0;
    bool reader_no_data_notified = true;

    while(nfc_worker->state == NfcWorkerStateAnalyzeReader) {
        furi_hal_nfc_stop_cmd();
        furi_delay_ms(5);
        furi_hal_nfc_listen_start(nfc_data);
        if(furi_hal_nfc_listen_rx(&tx_rx, 300)) {
            if(reader_no_data_notified) {
                nfc_worker->callback(NfcWorkerEventDetectReaderDetected, nfc_worker->context);
            }
            reader_no_data_received_cnt = 0;
            reader_no_data_notified = false;
            NfcProtocol protocol =
                reader_analyzer_guess_protocol(reader_analyzer, tx_rx.rx_data, tx_rx.rx_bits / 8);
            if(protocol == NfcDeviceProtocolMifareClassic) {
                mf_classic_emulator(&emulator, &tx_rx);
            }
        } else {
            reader_no_data_received_cnt++;
            if(!reader_no_data_notified && (reader_no_data_received_cnt > 5)) {
                nfc_worker->callback(NfcWorkerEventDetectReaderLost, nfc_worker->context);
                reader_no_data_received_cnt = 0;
                reader_no_data_notified = true;
            }
            FURI_LOG_D(TAG, "No data from reader");
            continue;
        }
    }

    rfal_platform_spi_release();

    reader_analyzer_stop(nfc_worker->reader_analyzer);

    nfca_signal_free(nfca_signal);
}
