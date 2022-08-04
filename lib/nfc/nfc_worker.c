#include "nfc_worker_i.h"
#include <furi_hal.h>

#include <platform.h>
#include "parsers/nfc_supported_card.h"

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
    nfc_worker->storage = furi_record_open(RECORD_STORAGE);

    // Initialize rfal
    while(furi_hal_nfc_is_busy()) {
        furi_delay_ms(10);
    }
    nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        nfc_worker->debug_pcap_worker = nfc_debug_pcap_alloc(nfc_worker->storage);
    }

    return nfc_worker;
}

void nfc_worker_free(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);

    furi_thread_free(nfc_worker->thread);

    furi_record_close(RECORD_STORAGE);

    if(nfc_worker->debug_pcap_worker) nfc_debug_pcap_free(nfc_worker->debug_pcap_worker);

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

    if(nfc_worker->state == NfcWorkerStateRead) {
        nfc_worker_read(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateUidEmulate) {
        nfc_worker_emulate_uid(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulateApdu) {
        nfc_worker_emulate_apdu(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateMfUltralightEmulate) {
        nfc_worker_emulate_mf_ultralight(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateMfClassicEmulate) {
        nfc_worker_emulate_mf_classic(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateMfClassicDictAttack) {
        nfc_worker_mf_classic_dict_attack(nfc_worker);
    }
    furi_hal_nfc_sleep();
    nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);

    return 0;
}

static bool nfc_worker_read_mf_ultralight(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    bool read_success = false;
    MfUltralightReader reader = {};
    MfUltralightData data = {};

    nfc_debug_pcap_prepare_tx_rx(nfc_worker->debug_pcap_worker, tx_rx, false);
    do {
        // Read card
        if(!furi_hal_nfc_detect(&nfc_worker->dev_data->nfc_data, 200)) break;
        if(!mf_ul_read_card(tx_rx, &reader, &data)) break;
        // Copy data
        nfc_worker->dev_data->mf_ul_data = data;
        read_success = true;
    } while(false);

    return read_success;
}

static bool nfc_worker_read_mf_classic(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(nfc_worker->callback);
    bool read_success = false;

    nfc_debug_pcap_prepare_tx_rx(nfc_worker->debug_pcap_worker, tx_rx, false);
    do {
        // Try to read supported card
        FURI_LOG_I(TAG, "Try read supported card ...");
        for(size_t i = 0; i < NfcSupportedCardTypeEnd; i++) {
            if(nfc_supported_card[i].protocol == NfcDeviceProtocolMifareClassic) {
                if(nfc_supported_card[i].verify(nfc_worker, tx_rx)) {
                    if(nfc_supported_card[i].read(nfc_worker, tx_rx)) {
                        read_success = true;
                        nfc_supported_card[i].parse(nfc_worker);
                    }
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
            read_success = (sectors_read == sectors_total);
        }
    } while(false);

    return read_success;
}

static bool nfc_worker_read_mf_desfire(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    bool read_success = false;
    MifareDesfireData* data = &nfc_worker->dev_data->mf_df_data;

    nfc_debug_pcap_prepare_tx_rx(nfc_worker->debug_pcap_worker, tx_rx, false);
    do {
        if(!furi_hal_nfc_detect(&nfc_worker->dev_data->nfc_data, 300)) break;
        if(!mf_df_read_card(tx_rx, data)) break;
        read_success = true;
    } while(false);

    return read_success;
}

static bool nfc_worker_read_bank_card(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    bool read_success = false;
    EmvApplication emv_app = {};
    EmvData* result = &nfc_worker->dev_data->emv_data;

    nfc_debug_pcap_prepare_tx_rx(nfc_worker->debug_pcap_worker, tx_rx, false);
    do {
        // Read card
        if(!furi_hal_nfc_detect(&nfc_worker->dev_data->nfc_data, 300)) break;
        if(!emv_read_bank_card(tx_rx, &emv_app)) break;
        // Copy data
        // TODO Set EmvData to reader or like in mifare ultralight!
        result->number_len = emv_app.card_number_len;
        memcpy(result->number, emv_app.card_number, result->number_len);
        result->aid_len = emv_app.aid_len;
        memcpy(result->aid, emv_app.aid, result->aid_len);
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
        read_success = true;
    } while(false);

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

void nfc_worker_emulate_uid(NfcWorker* nfc_worker) {
    FuriHalNfcTxRxContext tx_rx = {};
    nfc_debug_pcap_prepare_tx_rx(nfc_worker->debug_pcap_worker, &tx_rx, true);
    FuriHalNfcDevData* data = &nfc_worker->dev_data->nfc_data;
    NfcReaderRequestData* reader_data = &nfc_worker->dev_data->reader_data;

    // TODO add support for RATS
    // Now remove bit 6 in SAK to support ISO-14443A-3 emulation
    // Need to save ATS to support ISO-14443A-4 emulation
    uint8_t sak = data->sak;
    FURI_BIT_CLEAR(sak, 5);

    while(nfc_worker->state == NfcWorkerStateUidEmulate) {
        if(furi_hal_nfc_listen(data->uid, data->uid_len, data->atqa, sak, true, 100)) {
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
    nfc_debug_pcap_prepare_tx_rx(nfc_worker->debug_pcap_worker, &tx_rx, true);
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
        furi_delay_ms(20);
    }
}

void nfc_worker_emulate_mf_ultralight(NfcWorker* nfc_worker) {
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    MfUltralightEmulator emulator = {};
    mf_ul_prepare_emulation(&emulator, &nfc_worker->dev_data->mf_ul_data);
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

void nfc_worker_mf_classic_dict_attack(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    furi_assert(nfc_worker->callback);

    MfClassicData* data = &nfc_worker->dev_data->mf_classic_data;
    NfcMfClassicDictAttackData* dict_attack_data =
        &nfc_worker->dev_data->mf_classic_dict_attack_data;
    uint32_t total_sectors = mf_classic_get_total_sectors_num(data->type);
    uint64_t key = 0;
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
        TAG,
        "Start Dictionary attack, Key Count %d",
        mf_classic_dict_get_total_keys(dict));
    for(size_t i = 0; i < total_sectors; i++) {
        FURI_LOG_I(TAG, "Sector %d", i);
        nfc_worker->callback(NfcWorkerEventNewSector, nfc_worker->context);
        uint8_t block_num = mf_classic_get_sector_trailer_block_num_by_sector(i);
        if(mf_classic_is_sector_read(data, i)) continue;
        bool is_key_a_found = mf_classic_is_key_found(data, i, MfClassicKeyA);
        bool is_key_b_found = mf_classic_is_key_found(data, i, MfClassicKeyB);
        uint16_t key_index = 0;
        while(mf_classic_dict_get_next_key(dict, &key)) {
            if(++key_index % NFC_DICT_KEY_BATCH_SIZE == 0) {
                nfc_worker->callback(NfcWorkerEventNewDictKeyBatch, nfc_worker->context);
            }
            furi_hal_nfc_sleep();
            if(furi_hal_nfc_activate_nfca(200, NULL)) {
                furi_hal_nfc_sleep();
                if(!card_found_notified) {
                    nfc_worker->callback(NfcWorkerEventCardDetected, nfc_worker->context);
                    card_found_notified = true;
                    card_removed_notified = false;
                }
                FURI_LOG_D(
                    TAG,
                    "Try to auth to sector %d with key %04lx%08lx",
                    i,
                    (uint32_t)(key >> 32),
                    (uint32_t)key);
                if(!is_key_a_found) {
                    is_key_a_found = mf_classic_is_key_found(data, i, MfClassicKeyA);
                    if(mf_classic_authenticate(&tx_rx, block_num, key, MfClassicKeyA)) {
                        mf_classic_set_key_found(data, i, MfClassicKeyA, key);
                        nfc_worker->callback(NfcWorkerEventFoundKeyA, nfc_worker->context);
                    }
                    furi_hal_nfc_sleep();
                }
                if(!is_key_b_found) {
                    is_key_b_found = mf_classic_is_key_found(data, i, MfClassicKeyB);
                    if(mf_classic_authenticate(&tx_rx, block_num, key, MfClassicKeyB)) {
                        mf_classic_set_key_found(data, i, MfClassicKeyB, key);
                        nfc_worker->callback(NfcWorkerEventFoundKeyB, nfc_worker->context);
                    }
                }
                if(is_key_a_found && is_key_b_found) break;
                if(nfc_worker->state != NfcWorkerStateMfClassicDictAttack)
                    break;
            } else {
                if(!card_removed_notified) {
                    nfc_worker->callback(NfcWorkerEventNoCardDetected, nfc_worker->context);
                    card_removed_notified = true;
                    card_found_notified = false;
                }
                if(nfc_worker->state != NfcWorkerStateMfClassicDictAttack)
                    break;
            }
        }
        if(nfc_worker->state != NfcWorkerStateMfClassicDictAttack)
            break;
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
    nfc_debug_pcap_prepare_tx_rx(nfc_worker->debug_pcap_worker, &tx_rx, true);
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
    while(nfc_worker->state == NfcWorkerStateMfClassicEmulate) {
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
