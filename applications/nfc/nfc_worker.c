#include "nfc_worker_i.h"
#include <furi_hal.h>

#include <lib/nfc_protocols/nfc_util.h>
#include <lib/nfc_protocols/emv_decoder.h>
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

    furi_hal_power_insomnia_enter();
    furi_hal_nfc_exit_sleep();

    if(nfc_worker->state == NfcWorkerStateDetect) {
        nfc_worker_detect(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulate) {
        nfc_worker_emulate(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadEMVApp) {
        nfc_worker_read_emv_app(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadEMV) {
        nfc_worker_read_emv(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulateApdu) {
        nfc_worker_emulate_apdu(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadMifareUl) {
        nfc_worker_read_mifare_ul(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulateMifareUl) {
        nfc_worker_emulate_mifare_ul(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadMifareClassic) {
        nfc_worker_mifare_classic_dict_attack(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadMifareDesfire) {
        nfc_worker_read_mifare_desfire(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateField) {
        nfc_worker_field(nfc_worker);
    }
    furi_hal_nfc_deactivate();
    nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);
    furi_hal_power_insomnia_exit();

    return 0;
}

void nfc_worker_detect(NfcWorker* nfc_worker) {
    rfalNfcDevice* dev_list;
    rfalNfcDevice* dev;
    uint8_t dev_cnt;
    nfc_device_data_clear(nfc_worker->dev_data);
    NfcDeviceCommonData* result = &nfc_worker->dev_data->nfc_data;

    while(nfc_worker->state == NfcWorkerStateDetect) {
        if(furi_hal_nfc_detect(&dev_list, &dev_cnt, 1000, true)) {
            // Process first found device
            dev = &dev_list[0];
            result->uid_len = dev->nfcidLen;
            memcpy(result->uid, dev->nfcid, dev->nfcidLen);
            if(dev->type == RFAL_NFC_LISTEN_TYPE_NFCA) {
                result->device = NfcDeviceNfca;
                result->atqa[0] = dev->dev.nfca.sensRes.anticollisionInfo;
                result->atqa[1] = dev->dev.nfca.sensRes.platformInfo;
                result->sak = dev->dev.nfca.selRes.sak;
                if(mf_ul_check_card_type(
                       dev->dev.nfca.sensRes.anticollisionInfo,
                       dev->dev.nfca.sensRes.platformInfo,
                       dev->dev.nfca.selRes.sak)) {
                    result->protocol = NfcDeviceProtocolMifareUl;
                } else if(mf_classic_check_card_type(
                              dev->dev.nfca.sensRes.anticollisionInfo,
                              dev->dev.nfca.sensRes.platformInfo,
                              dev->dev.nfca.selRes.sak)) {
                    result->protocol = NfcDeviceProtocolMifareClassic;
                } else if(mf_df_check_card_type(
                              dev->dev.nfca.sensRes.anticollisionInfo,
                              dev->dev.nfca.sensRes.platformInfo,
                              dev->dev.nfca.selRes.sak)) {
                    result->protocol = NfcDeviceProtocolMifareDesfire;
                } else if(dev->rfInterface == RFAL_NFC_INTERFACE_ISODEP) {
                    result->protocol = NfcDeviceProtocolEMV;
                } else {
                    result->protocol = NfcDeviceProtocolUnknown;
                }
            } else if(dev->type == RFAL_NFC_LISTEN_TYPE_NFCB) {
                result->device = NfcDeviceNfcb;
            } else if(dev->type == RFAL_NFC_LISTEN_TYPE_NFCF) {
                result->device = NfcDeviceNfcf;
            } else if(dev->type == RFAL_NFC_LISTEN_TYPE_NFCV) {
                result->device = NfcDeviceNfcv;
            }
            // Notify caller and exit
            if(nfc_worker->callback) {
                nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
            }
            break;
        }
        osDelay(100);
    }
}

bool nfc_worker_emulate_uid_callback(
    uint8_t* buff_rx,
    uint16_t buff_rx_len,
    uint8_t* buff_tx,
    uint16_t* buff_tx_len,
    uint32_t* data_type,
    void* context) {
    furi_assert(context);
    NfcWorker* nfc_worker = context;
    NfcReaderRequestData* reader_data = &nfc_worker->dev_data->reader_data;
    reader_data->size = buff_rx_len / 8;
    if(reader_data->size > 0) {
        memcpy(reader_data->data, buff_rx, reader_data->size);
        if(nfc_worker->callback) {
            nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
        }
    }
    return true;
}

void nfc_worker_emulate(NfcWorker* nfc_worker) {
    NfcDeviceCommonData* data = &nfc_worker->dev_data->nfc_data;
    while(nfc_worker->state == NfcWorkerStateEmulate) {
        furi_hal_nfc_emulate_nfca(
            data->uid,
            data->uid_len,
            data->atqa,
            data->sak,
            nfc_worker_emulate_uid_callback,
            nfc_worker,
            1000);
    }
}

void nfc_worker_read_emv_app(NfcWorker* nfc_worker) {
    ReturnCode err;
    rfalNfcDevice* dev_list;
    EmvApplication emv_app = {};
    uint8_t dev_cnt = 0;
    uint8_t tx_buff[255] = {};
    uint16_t tx_len = 0;
    uint8_t* rx_buff;
    uint16_t* rx_len;
    NfcDeviceData* result = nfc_worker->dev_data;
    nfc_device_data_clear(result);

    while(nfc_worker->state == NfcWorkerStateReadEMVApp) {
        memset(&emv_app, 0, sizeof(emv_app));
        if(furi_hal_nfc_detect(&dev_list, &dev_cnt, 1000, false)) {
            // Card was found. Check that it supports EMV
            if(dev_list[0].rfInterface == RFAL_NFC_INTERFACE_ISODEP) {
                result->nfc_data.uid_len = dev_list[0].dev.nfca.nfcId1Len;
                result->nfc_data.atqa[0] = dev_list[0].dev.nfca.sensRes.anticollisionInfo;
                result->nfc_data.atqa[1] = dev_list[0].dev.nfca.sensRes.platformInfo;
                result->nfc_data.sak = dev_list[0].dev.nfca.selRes.sak;
                memcpy(
                    result->nfc_data.uid, dev_list[0].dev.nfca.nfcId1, result->nfc_data.uid_len);
                result->nfc_data.protocol = NfcDeviceProtocolEMV;

                FURI_LOG_D(TAG, "Send select PPSE command");
                tx_len = emv_prepare_select_ppse(tx_buff);
                err = furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_D(TAG, "Error during selection PPSE request: %d", err);
                    furi_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_D(TAG, "Select PPSE response received. Start parsing response");
                if(emv_decode_ppse_response(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_D(TAG, "Select PPSE responce parced");
                    // Notify caller and exit
                    result->emv_data.aid_len = emv_app.aid_len;
                    memcpy(result->emv_data.aid, emv_app.aid, emv_app.aid_len);
                    if(nfc_worker->callback) {
                        nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
                    }
                    break;
                } else {
                    FURI_LOG_D(TAG, "Can't find pay application");
                    furi_hal_nfc_deactivate();
                    continue;
                }
            } else {
                // Can't find EMV card
                FURI_LOG_W(TAG, "Card doesn't support EMV");
                furi_hal_nfc_deactivate();
            }
        } else {
            // Can't find EMV card
            FURI_LOG_D(TAG, "Can't find any cards");
            furi_hal_nfc_deactivate();
        }
        osDelay(20);
    }
}

void nfc_worker_read_emv(NfcWorker* nfc_worker) {
    ReturnCode err;
    rfalNfcDevice* dev_list;
    EmvApplication emv_app = {};
    uint8_t dev_cnt = 0;
    uint8_t tx_buff[255] = {};
    uint16_t tx_len = 0;
    uint8_t* rx_buff;
    uint16_t* rx_len;
    NfcDeviceData* result = nfc_worker->dev_data;
    nfc_device_data_clear(result);

    while(nfc_worker->state == NfcWorkerStateReadEMV) {
        memset(&emv_app, 0, sizeof(emv_app));
        if(furi_hal_nfc_detect(&dev_list, &dev_cnt, 1000, false)) {
            // Card was found. Check that it supports EMV
            if(dev_list[0].rfInterface == RFAL_NFC_INTERFACE_ISODEP) {
                result->nfc_data.uid_len = dev_list[0].dev.nfca.nfcId1Len;
                result->nfc_data.atqa[0] = dev_list[0].dev.nfca.sensRes.anticollisionInfo;
                result->nfc_data.atqa[1] = dev_list[0].dev.nfca.sensRes.platformInfo;
                result->nfc_data.sak = dev_list[0].dev.nfca.selRes.sak;
                memcpy(
                    result->nfc_data.uid, dev_list[0].dev.nfca.nfcId1, result->nfc_data.uid_len);
                result->nfc_data.protocol = NfcDeviceProtocolEMV;

                FURI_LOG_D(TAG, "Send select PPSE command");
                tx_len = emv_prepare_select_ppse(tx_buff);
                err = furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_D(TAG, "Error during selection PPSE request: %d", err);
                    furi_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_D(TAG, "Select PPSE response received. Start parsing response");
                if(emv_decode_ppse_response(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_D(TAG, "Select PPSE responce parced");
                    result->emv_data.aid_len = emv_app.aid_len;
                    memcpy(result->emv_data.aid, emv_app.aid, emv_app.aid_len);
                } else {
                    FURI_LOG_D(TAG, "Can't find pay application");
                    furi_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_D(TAG, "Starting application ...");
                tx_len = emv_prepare_select_app(tx_buff, &emv_app);
                err = furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_D(TAG, "Error during application selection request: %d", err);
                    furi_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_D(TAG, "Select application response received. Start parsing response");
                if(emv_decode_select_app_response(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_D(TAG, "Card name: %s", emv_app.name);
                    memcpy(result->emv_data.name, emv_app.name, sizeof(emv_app.name));
                } else if(emv_app.pdol.size > 0) {
                    FURI_LOG_D(TAG, "Can't find card name, but PDOL is present.");
                } else {
                    FURI_LOG_D(TAG, "Can't find card name or PDOL");
                    furi_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_D(TAG, "Starting Get Processing Options command ...");
                tx_len = emv_prepare_get_proc_opt(tx_buff, &emv_app);
                err = furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_D(TAG, "Error during Get Processing Options command: %d", err);
                    furi_hal_nfc_deactivate();
                    continue;
                }
                if(emv_decode_get_proc_opt(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_D(TAG, "Card number parsed");
                    result->emv_data.number_len = emv_app.card_number_len;
                    memcpy(result->emv_data.number, emv_app.card_number, emv_app.card_number_len);
                    // Notify caller and exit
                    if(nfc_worker->callback) {
                        nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
                    }
                    break;
                } else {
                    // Mastercard doesn't give PAN / card number as GPO response
                    // Iterate over all files found in application
                    bool pan_found = false;
                    for(uint8_t i = 0; (i < emv_app.afl.size) && !pan_found; i += 4) {
                        uint8_t sfi = emv_app.afl.data[i] >> 3;
                        uint8_t record_start = emv_app.afl.data[i + 1];
                        uint8_t record_end = emv_app.afl.data[i + 2];

                        // Iterate over all records in file
                        for(uint8_t record = record_start; record <= record_end; ++record) {
                            tx_len = emv_prepare_read_sfi_record(tx_buff, sfi, record);
                            err = furi_hal_nfc_data_exchange(
                                tx_buff, tx_len, &rx_buff, &rx_len, false);
                            if(err != ERR_NONE) {
                                FURI_LOG_D(
                                    TAG,
                                    "Error reading application sfi %d, record %d",
                                    sfi,
                                    record);
                            }
                            if(emv_decode_read_sfi_record(rx_buff, *rx_len, &emv_app)) {
                                pan_found = true;
                                break;
                            }
                        }
                    }
                    if(pan_found) {
                        FURI_LOG_D(TAG, "Card PAN found");
                        result->emv_data.number_len = emv_app.card_number_len;
                        memcpy(
                            result->emv_data.number,
                            emv_app.card_number,
                            result->emv_data.number_len);
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
                    } else {
                        FURI_LOG_D(TAG, "Can't read card number");
                    }
                    furi_hal_nfc_deactivate();
                }
            } else {
                // Can't find EMV card
                FURI_LOG_W(TAG, "Card doesn't support EMV");
                furi_hal_nfc_deactivate();
            }
        } else {
            // Can't find EMV card
            FURI_LOG_D(TAG, "Can't find any cards");
            furi_hal_nfc_deactivate();
        }
        osDelay(20);
    }
}

void nfc_worker_emulate_apdu(NfcWorker* nfc_worker) {
    ReturnCode err;
    uint8_t tx_buff[255] = {};
    uint16_t tx_len = 0;
    uint8_t* rx_buff;
    uint16_t* rx_len;
    NfcDeviceCommonData params = {
        .uid = {0xCF, 0x72, 0xd4, 0x40},
        .uid_len = 4,
        .atqa = {0x00, 0x04},
        .sak = 0x20,
        .device = NfcDeviceNfca,
        .protocol = NfcDeviceProtocolEMV,
    };
    // Test RX data
    const uint8_t debug_rx[] = {
        0xba, 0x0b, 0xba, 0xba, 0x20, 0x00, 0x02, 0x28, 0xde, 0xad, 0xbe, 0xef, 0x00, 0xca, 0xca,
        0xca, 0xfe, 0xfa, 0xce, 0x14, 0x88, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
        0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0xba,
        0x0b, 0xba, 0xba, 0x20, 0x00, 0x02, 0x28, 0xde, 0xad, 0xbe, 0xef, 0x00, 0xca, 0xca, 0xca,
        0xfe, 0xfa, 0xce, 0x14, 0x88, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0xba, 0x0b,
        0xba, 0xba, 0x20, 0x00, 0x02, 0x28, 0xde, 0xad, 0xbe, 0xef, 0x00, 0xca, 0xca, 0xca, 0xfe,
        0xfa, 0xce, 0x14, 0x88, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
        0xbb, 0xcc, 0xdd, 0xee, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0xba, 0x0b, 0xba,
        0xba, 0x20, 0x00, 0x02, 0x28, 0xde, 0xad, 0xbe, 0xef, 0x00, 0xca, 0xca, 0xca, 0xfe, 0xfa,
        0xce, 0x14, 0x88, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
        0xcc, 0xdd, 0xee, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0xba, 0x0b, 0xba, 0xba,
        0x20, 0x00, 0x02, 0x28, 0xde, 0xad, 0xbe, 0xef, 0x00, 0xca, 0xca, 0xca, 0xfe, 0xfa, 0xce,
        0x14, 0x88, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
        0xdd, 0xee, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0xba, 0x0b, 0xba, 0xba, 0x20,
        0x00, 0x02, 0x28, 0xde, 0xad, 0xbe, 0xef, 0x00, 0xca, 0xca, 0xca, 0xfe, 0xfa, 0xce, 0x14,
        0x88, 0x00};
    // Test TX data
    const uint8_t debug_tx[] = {
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xff, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32,
        0x10, 0x14, 0x88, 0x02, 0x28, 0x00, 0x00, 0xca, 0xca, 0x00, 0xc0, 0xc0, 0x00, 0xde, 0xad,
        0xbe, 0xef, 0xce, 0xee, 0xec, 0xca, 0xfe, 0xba, 0xba, 0xb0, 0xb0, 0xac, 0xdc, 0x11, 0x12,
        0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xff, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
        0x14, 0x88, 0x02, 0x28, 0x00, 0x00, 0xca, 0xca, 0x00, 0xc0, 0xc0, 0x00, 0xde, 0xad, 0xbe,
        0xef, 0xce, 0xee, 0xec, 0xca, 0xfe, 0xba, 0xba, 0xb0, 0xb0, 0xac, 0xdc, 0x11, 0x12, 0x34,
        0x56, 0x78, 0x9a, 0xbc, 0xde, 0xff, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0x14,
        0x88, 0x02, 0x28, 0x00, 0x00, 0xca, 0xca, 0x00, 0xc0, 0xc0, 0x00, 0xde, 0xad, 0xbe, 0xef,
        0xce, 0xee, 0xec, 0xca, 0xfe, 0xba, 0xba, 0xb0, 0xb0, 0xac, 0xdc, 0x11, 0x12, 0x34, 0x56,
        0x78, 0x9a, 0xbc, 0xde, 0xff, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0x14, 0x88,
        0x02, 0x28, 0x00, 0x00, 0xca, 0xca, 0x00, 0xc0, 0xc0, 0x00, 0xde, 0xad, 0xbe, 0xef, 0xce,
        0xee, 0xec, 0xca, 0xfe, 0xba, 0xba, 0xb0, 0xb0, 0xac, 0xdc, 0x11, 0x12, 0x34, 0x56, 0x78,
        0x9a, 0xbc, 0xde, 0xff, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0x14, 0x88, 0x02,
        0x28, 0x00, 0x00, 0xca, 0xca, 0x00, 0xc0, 0xc0, 0x00, 0xde, 0xad, 0xbe, 0xef, 0xce, 0xee,
        0xec, 0xca, 0xfe, 0xba, 0xba, 0xb0, 0xb0, 0xac, 0xdc, 0x11, 0x12, 0x34, 0x56, 0x78, 0x9a,
        0xbc, 0xde, 0xff, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0x14, 0x88, 0x02, 0x28,
        0x00, 0x00};

    while(nfc_worker->state == NfcWorkerStateEmulateApdu) {
        if(furi_hal_nfc_listen(params.uid, params.uid_len, params.atqa, params.sak, false, 300)) {
            FURI_LOG_D(TAG, "POS terminal detected");
            // Read data from POS terminal
            err = furi_hal_nfc_data_exchange(NULL, 0, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_D(TAG, "Received Select PPSE");
            } else {
                FURI_LOG_D(TAG, "Error in 1st data exchange: select PPSE");
                furi_hal_nfc_deactivate();
                continue;
            }
            FURI_LOG_D(TAG, "Transive SELECT PPSE ANS");
            tx_len = emv_select_ppse_ans(tx_buff);
            err = furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_D(TAG, "Received Select APP");
            } else {
                FURI_LOG_D(TAG, "Error in 2nd data exchange: select APP");
                furi_hal_nfc_deactivate();
                continue;
            }

            FURI_LOG_D(TAG, "Transive SELECT APP ANS");
            tx_len = emv_select_app_ans(tx_buff);
            err = furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_D(TAG, "Received PDOL");
            } else {
                FURI_LOG_D(TAG, "Error in 3rd data exchange: receive PDOL");
                furi_hal_nfc_deactivate();
                continue;
            }

            FURI_LOG_D(TAG, "Transive PDOL ANS");
            tx_len = emv_get_proc_opt_ans(tx_buff);
            err = furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_D(TAG, "Transive PDOL ANS");
            } else {
                FURI_LOG_D(TAG, "Error in 4rd data exchange: Transive PDOL ANS");
                furi_hal_nfc_deactivate();
                continue;
            }

            if(*rx_len != sizeof(debug_rx) || memcmp(rx_buff, debug_rx, sizeof(debug_rx))) {
                FURI_LOG_D(TAG, "Failed long message test");
            } else {
                FURI_LOG_D(TAG, "Correct debug message received");
                tx_len = sizeof(debug_tx);
                err = furi_hal_nfc_data_exchange(
                    (uint8_t*)debug_tx, tx_len, &rx_buff, &rx_len, false);
                if(err == ERR_NONE) {
                    FURI_LOG_D(TAG, "Transive Debug message");
                }
            }
            furi_hal_nfc_deactivate();
        } else {
            FURI_LOG_D(TAG, "Can't find reader");
        }
        osDelay(20);
    }
}

void nfc_worker_read_mifare_ul(NfcWorker* nfc_worker) {
    ReturnCode err;
    rfalNfcDevice* dev_list;
    uint8_t dev_cnt = 0;
    uint8_t tx_buff[255] = {};
    uint16_t tx_len = 0;
    uint8_t* rx_buff;
    uint16_t* rx_len;
    MifareUlDevice mf_ul_read;
    NfcDeviceData* result = nfc_worker->dev_data;
    nfc_device_data_clear(result);

    while(nfc_worker->state == NfcWorkerStateReadMifareUl) {
        furi_hal_nfc_deactivate();
        memset(&mf_ul_read, 0, sizeof(mf_ul_read));
        if(furi_hal_nfc_detect(&dev_list, &dev_cnt, 300, false)) {
            if(dev_list[0].type == RFAL_NFC_LISTEN_TYPE_NFCA &&
               mf_ul_check_card_type(
                   dev_list[0].dev.nfca.sensRes.anticollisionInfo,
                   dev_list[0].dev.nfca.sensRes.platformInfo,
                   dev_list[0].dev.nfca.selRes.sak)) {
                // Get Mifare Ultralight version
                FURI_LOG_D(TAG, "Found Mifare Ultralight tag. Reading tag version");
                tx_len = mf_ul_prepare_get_version(tx_buff);
                err = furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err == ERR_NONE) {
                    mf_ul_parse_get_version_response(rx_buff, &mf_ul_read);
                    FURI_LOG_D(
                        TAG,
                        "Mifare Ultralight Type: %d, Pages: %d",
                        mf_ul_read.data.type,
                        mf_ul_read.pages_to_read);
                    FURI_LOG_D(TAG, "Reading signature ...");
                    tx_len = mf_ul_prepare_read_signature(tx_buff);
                    if(furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false)) {
                        FURI_LOG_D(TAG, "Failed reading signature");
                        memset(mf_ul_read.data.signature, 0, sizeof(mf_ul_read.data.signature));
                    } else {
                        mf_ul_parse_read_signature_response(rx_buff, &mf_ul_read);
                    }
                } else if(err == ERR_TIMEOUT) {
                    FURI_LOG_D(
                        TAG,
                        "Card doesn't respond to GET VERSION command. Setting default read parameters");
                    err = ERR_NONE;
                    mf_ul_set_default_version(&mf_ul_read);
                    // Reinit device
                    furi_hal_nfc_deactivate();
                    if(!furi_hal_nfc_detect(&dev_list, &dev_cnt, 300, false)) {
                        FURI_LOG_D(TAG, "Lost connection. Restarting search");
                        continue;
                    }
                } else {
                    FURI_LOG_D(
                        TAG, "Error getting Mifare Ultralight version. Error code: %d", err);
                    continue;
                }

                if(mf_ul_read.support_fast_read) {
                    FURI_LOG_D(TAG, "Reading pages ...");
                    tx_len = mf_ul_prepare_fast_read(tx_buff, 0x00, mf_ul_read.pages_to_read - 1);
                    if(furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false)) {
                        FURI_LOG_D(TAG, "Failed reading pages");
                        continue;
                    } else {
                        mf_ul_parse_fast_read_response(
                            rx_buff, 0x00, mf_ul_read.pages_to_read - 1, &mf_ul_read);
                    }

                    FURI_LOG_D(TAG, "Reading 3 counters ...");
                    for(uint8_t i = 0; i < 3; i++) {
                        tx_len = mf_ul_prepare_read_cnt(tx_buff, i);
                        if(furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false)) {
                            FURI_LOG_W(TAG, "Failed reading Counter %d", i);
                            mf_ul_read.data.counter[i] = 0;
                        } else {
                            mf_ul_parse_read_cnt_response(rx_buff, i, &mf_ul_read);
                        }
                    }

                    FURI_LOG_D(TAG, "Checking tearing flags ...");
                    for(uint8_t i = 0; i < 3; i++) {
                        tx_len = mf_ul_prepare_check_tearing(tx_buff, i);
                        if(furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false)) {
                            FURI_LOG_D(TAG, "Error checking tearing flag %d", i);
                            mf_ul_read.data.tearing[i] = MF_UL_TEARING_FLAG_DEFAULT;
                        } else {
                            mf_ul_parse_check_tearing_response(rx_buff, i, &mf_ul_read);
                        }
                    }
                } else {
                    // READ card with READ command (4 pages at a time)
                    for(uint8_t page = 0; page < mf_ul_read.pages_to_read; page += 4) {
                        FURI_LOG_D(TAG, "Reading pages %d - %d ...", page, page + 3);
                        tx_len = mf_ul_prepare_read(tx_buff, page);
                        if(furi_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false)) {
                            FURI_LOG_D(TAG, "Read pages %d - %d failed", page, page + 3);
                            continue;
                        } else {
                            mf_ul_parse_read_response(rx_buff, page, &mf_ul_read);
                        }
                    }
                }

                // Fill result data
                result->nfc_data.uid_len = dev_list[0].dev.nfca.nfcId1Len;
                result->nfc_data.atqa[0] = dev_list[0].dev.nfca.sensRes.anticollisionInfo;
                result->nfc_data.atqa[1] = dev_list[0].dev.nfca.sensRes.platformInfo;
                result->nfc_data.sak = dev_list[0].dev.nfca.selRes.sak;
                result->nfc_data.protocol = NfcDeviceProtocolMifareUl;
                memcpy(
                    result->nfc_data.uid, dev_list[0].dev.nfca.nfcId1, result->nfc_data.uid_len);
                result->mf_ul_data = mf_ul_read.data;

                // Notify caller and exit
                if(nfc_worker->callback) {
                    nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
                }
                break;
            } else {
                FURI_LOG_W(TAG, "Tag does not support Mifare Ultralight");
            }
        } else {
            FURI_LOG_D(TAG, "Can't find any tags");
        }
        osDelay(100);
    }
}

void nfc_worker_emulate_mifare_ul(NfcWorker* nfc_worker) {
    NfcDeviceCommonData* nfc_common = &nfc_worker->dev_data->nfc_data;
    MifareUlDevice mf_ul_emulate;
    mf_ul_prepare_emulation(&mf_ul_emulate, &nfc_worker->dev_data->mf_ul_data);
    while(nfc_worker->state == NfcWorkerStateEmulateMifareUl) {
        furi_hal_nfc_emulate_nfca(
            nfc_common->uid,
            nfc_common->uid_len,
            nfc_common->atqa,
            nfc_common->sak,
            mf_ul_prepare_emulation_response,
            &mf_ul_emulate,
            5000);
        // Check if data was modified
        if(mf_ul_emulate.data_changed) {
            nfc_worker->dev_data->mf_ul_data = mf_ul_emulate.data;
            if(nfc_worker->callback) {
                nfc_worker->callback(NfcWorkerEventSuccess, nfc_worker->context);
            }
            mf_ul_emulate.data_changed = false;
        }
    }
}

void nfc_worker_mifare_classic_dict_attack(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker->callback);
    rfalNfcDevice* dev_list;
    rfalNfcDevice* dev;
    NfcDeviceCommonData* nfc_common;
    uint8_t dev_cnt = 0;
    FuriHalNfcTxRxContext tx_rx_ctx = {};
    MfClassicAuthContext auth_ctx = {};
    MfClassicReader reader = {};
    uint64_t curr_key = 0;
    uint16_t curr_sector = 0;
    uint8_t total_sectors = 0;
    NfcWorkerEvent event;

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
        if(furi_hal_nfc_detect(&dev_list, &dev_cnt, 300, false)) {
            dev = &dev_list[0];
            if(mf_classic_get_type(
                   dev->nfcid,
                   dev->nfcidLen,
                   dev->dev.nfca.sensRes.anticollisionInfo,
                   dev->dev.nfca.sensRes.platformInfo,
                   dev->dev.nfca.selRes.sak,
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
                furi_hal_nfc_deactivate();
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
            dev = &dev_list[0];
            nfc_common = &nfc_worker->dev_data->nfc_data;
            nfc_common->uid_len = dev->dev.nfca.nfcId1Len;
            nfc_common->atqa[0] = dev->dev.nfca.sensRes.anticollisionInfo;
            nfc_common->atqa[1] = dev->dev.nfca.sensRes.platformInfo;
            nfc_common->sak = dev->dev.nfca.selRes.sak;
            nfc_common->protocol = NfcDeviceProtocolMifareClassic;
            memcpy(nfc_common->uid, dev->dev.nfca.nfcId1, nfc_common->uid_len);
            event = NfcWorkerEventSuccess;
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

ReturnCode nfc_exchange_full(
    uint8_t* tx_buff,
    uint16_t tx_len,
    uint8_t* rx_buff,
    uint16_t rx_cap,
    uint16_t* rx_len) {
    ReturnCode err;
    uint8_t* part_buff;
    uint16_t* part_len;

    err = furi_hal_nfc_data_exchange(tx_buff, tx_len, &part_buff, &part_len, false);
    if(*part_len > rx_cap) {
        return ERR_OVERRUN;
    }
    memcpy(rx_buff, part_buff, *part_len);
    *rx_len = *part_len;
    while(err == ERR_NONE && rx_buff[0] == 0xAF) {
        err = furi_hal_nfc_data_exchange(rx_buff, 1, &part_buff, &part_len, false);
        if(*part_len > rx_cap - *rx_len) {
            return ERR_OVERRUN;
        }
        if(*part_len == 0) {
            return ERR_PROTO;
        }
        memcpy(rx_buff + *rx_len, part_buff + 1, *part_len - 1);
        *rx_buff = *part_buff;
        *rx_len += *part_len - 1;
    }

    return err;
}

void nfc_worker_read_mifare_desfire(NfcWorker* nfc_worker) {
    ReturnCode err;
    rfalNfcDevice* dev_list;
    uint8_t dev_cnt = 0;
    uint8_t tx_buff[64] = {};
    uint16_t tx_len = 0;
    uint8_t rx_buff[512] = {};
    uint16_t rx_len;
    NfcDeviceData* result = nfc_worker->dev_data;
    nfc_device_data_clear(result);
    MifareDesfireData* data = &result->mf_df_data;

    while(nfc_worker->state == NfcWorkerStateReadMifareDesfire) {
        furi_hal_nfc_deactivate();
        if(!furi_hal_nfc_detect(&dev_list, &dev_cnt, 300, false)) {
            osDelay(100);
            continue;
        }
        memset(data, 0, sizeof(MifareDesfireData));
        if(dev_list[0].type != RFAL_NFC_LISTEN_TYPE_NFCA ||
           !mf_df_check_card_type(
               dev_list[0].dev.nfca.sensRes.anticollisionInfo,
               dev_list[0].dev.nfca.sensRes.platformInfo,
               dev_list[0].dev.nfca.selRes.sak)) {
            FURI_LOG_D(TAG, "Tag is not DESFire");
            osDelay(100);
            continue;
        }

        FURI_LOG_D(TAG, "Found DESFire tag");

        // Fill non-DESFire result data
        result->nfc_data.uid_len = dev_list[0].dev.nfca.nfcId1Len;
        result->nfc_data.atqa[0] = dev_list[0].dev.nfca.sensRes.anticollisionInfo;
        result->nfc_data.atqa[1] = dev_list[0].dev.nfca.sensRes.platformInfo;
        result->nfc_data.sak = dev_list[0].dev.nfca.selRes.sak;
        result->nfc_data.device = NfcDeviceNfca;
        result->nfc_data.protocol = NfcDeviceProtocolMifareDesfire;
        memcpy(result->nfc_data.uid, dev_list[0].dev.nfca.nfcId1, result->nfc_data.uid_len);

        // Get DESFire version
        tx_len = mf_df_prepare_get_version(tx_buff);
        err = nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
        if(err != ERR_NONE) {
            FURI_LOG_W(TAG, "Bad exchange getting version, err: %d", err);
            continue;
        }
        if(!mf_df_parse_get_version_response(rx_buff, rx_len, &data->version)) {
            FURI_LOG_W(TAG, "Bad DESFire GET_VERSION response");
            continue;
        }

        tx_len = mf_df_prepare_get_free_memory(tx_buff);
        err = nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
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
        err = nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
        if(err != ERR_NONE) {
            FURI_LOG_D(TAG, "Bad exchange getting key settings, err: %d", err);
        } else {
            data->master_key_settings = malloc(sizeof(MifareDesfireKeySettings));
            memset(data->master_key_settings, 0, sizeof(MifareDesfireKeySettings));
            if(!mf_df_parse_get_key_settings_response(rx_buff, rx_len, data->master_key_settings)) {
                FURI_LOG_W(TAG, "Bad DESFire GET_KEY_SETTINGS response");
                free(data->master_key_settings);
                data->master_key_settings = NULL;
            }

            MifareDesfireKeyVersion** key_version_head =
                &data->master_key_settings->key_version_head;
            for(uint8_t key_id = 0; key_id < data->master_key_settings->max_keys; key_id++) {
                tx_len = mf_df_prepare_get_key_version(tx_buff, key_id);
                err = nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
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
        err = nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
        if(err != ERR_NONE) {
            FURI_LOG_W(TAG, "Bad exchange getting application IDs, err: %d", err);
        } else {
            if(!mf_df_parse_get_application_ids_response(rx_buff, rx_len, &data->app_head)) {
                FURI_LOG_W(TAG, "Bad DESFire GET_APPLICATION_IDS response");
            }
        }

        for(MifareDesfireApplication* app = data->app_head; app; app = app->next) {
            tx_len = mf_df_prepare_select_application(tx_buff, app->id);
            err = nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
            if(!mf_df_parse_select_application_response(rx_buff, rx_len)) {
                FURI_LOG_W(TAG, "Bad exchange selecting application, err: %d", err);
                continue;
            }
            tx_len = mf_df_prepare_get_key_settings(tx_buff);
            err = nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
            if(err != ERR_NONE) {
                FURI_LOG_W(TAG, "Bad exchange getting key settings, err: %d", err);
            } else {
                app->key_settings = malloc(sizeof(MifareDesfireKeySettings));
                memset(app->key_settings, 0, sizeof(MifareDesfireKeySettings));
                if(!mf_df_parse_get_key_settings_response(rx_buff, rx_len, app->key_settings)) {
                    FURI_LOG_W(TAG, "Bad DESFire GET_KEY_SETTINGS response");
                    free(app->key_settings);
                    app->key_settings = NULL;
                }

                MifareDesfireKeyVersion** key_version_head = &app->key_settings->key_version_head;
                for(uint8_t key_id = 0; key_id < app->key_settings->max_keys; key_id++) {
                    tx_len = mf_df_prepare_get_key_version(tx_buff, key_id);
                    err = nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
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
            err = nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
            if(err != ERR_NONE) {
                FURI_LOG_W(TAG, "Bad exchange getting file IDs, err: %d", err);
            } else {
                if(!mf_df_parse_get_file_ids_response(rx_buff, rx_len, &app->file_head)) {
                    FURI_LOG_W(TAG, "Bad DESFire GET_FILE_IDS response");
                }
            }

            for(MifareDesfireFile* file = app->file_head; file; file = file->next) {
                tx_len = mf_df_prepare_get_file_settings(tx_buff, file->id);
                err = nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
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
                err = nfc_exchange_full(tx_buff, tx_len, rx_buff, sizeof(rx_buff), &rx_len);
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

void nfc_worker_field(NfcWorker* nfc_worker) {
    furi_hal_nfc_field_on();
    while(nfc_worker->state == NfcWorkerStateField) {
        osDelay(50);
    }
    furi_hal_nfc_field_off();
}
