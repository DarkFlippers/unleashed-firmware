#include "nfc_worker_i.h"
#include <api-hal.h>
#include "nfc_protocols/emv_decoder.h"

#define NFC_WORKER_TAG "nfc worker"

NfcWorker* nfc_worker_alloc(osMessageQueueId_t message_queue) {
    NfcWorker* nfc_worker = furi_alloc(sizeof(NfcWorker));
    nfc_worker->message_queue = message_queue;
    // Worker thread attributes
    nfc_worker->thread_attr.name = "nfc_worker";
    nfc_worker->thread_attr.stack_size = 8192;
    // Initialize rfal
    nfc_worker->error = api_hal_nfc_init();
    if(nfc_worker->error == ERR_NONE) {
        api_hal_nfc_start_sleep();
        nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);
    } else {
        nfc_worker_change_state(nfc_worker, NfcWorkerStateBroken);
    }

    return nfc_worker;
}

void nfc_worker_free(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    free(nfc_worker);
}

NfcWorkerState nfc_worker_get_state(NfcWorker* nfc_worker) {
    return nfc_worker->state;
}

ReturnCode nfc_worker_get_error(NfcWorker* nfc_worker) {
    return nfc_worker->error;
}

void nfc_worker_start(NfcWorker* nfc_worker, NfcWorkerState state) {
    furi_assert(nfc_worker);
    furi_assert(nfc_worker->state == NfcWorkerStateReady);
    nfc_worker_change_state(nfc_worker, state);
    nfc_worker->thread = osThreadNew(nfc_worker_task, nfc_worker, &nfc_worker->thread_attr);
}

void nfc_worker_stop(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    if(nfc_worker->state == NfcWorkerStateBroken) {
        return;
    }

    nfc_worker_change_state(nfc_worker, NfcWorkerStateStop);
}

void nfc_worker_change_state(NfcWorker* nfc_worker, NfcWorkerState state) {
    nfc_worker->state = state;
}

void nfc_worker_task(void* context) {
    NfcWorker* nfc_worker = context;

    api_hal_power_insomnia_enter();

    if(nfc_worker->state == NfcWorkerStatePoll) {
        nfc_worker_poll(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadEMV) {
        nfc_worker_read_emv(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulate) {
        nfc_worker_emulate(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateField) {
        nfc_worker_field(nfc_worker);
    }
    nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);
    api_hal_power_insomnia_exit();
    osThreadExit();
}

void nfc_worker_read_emv(NfcWorker* nfc_worker) {
    ReturnCode err;
    rfalNfcDevice* dev_list;
    rfalNfcDevice* dev_active;
    EmvApplication emv_app = {};
    uint8_t dev_cnt = 0;
    uint8_t tx_buff[255] = {};
    uint16_t tx_len = 0;
    uint8_t* rx_buff;
    uint16_t* rx_len;

    // Update screen before start searching
    NfcMessage message = {.type = NfcMessageTypeEMVNotFound};
    while(nfc_worker->state == NfcWorkerStateReadEMV) {
        furi_check(
            osMessageQueuePut(nfc_worker->message_queue, &message, 0, osWaitForever) == osOK);
        memset(&emv_app, 0, sizeof(emv_app));
        if(api_hal_nfc_detect(&dev_list, &dev_cnt, 100, false)) {
            // Card was found. Check that it supports EMV
            if(dev_list[0].rfInterface == RFAL_NFC_INTERFACE_ISODEP) {
                dev_active = &dev_list[0];
                FURI_LOG_I(NFC_WORKER_TAG, "Send select PPSE command");
                tx_len = emv_prepare_select_ppse(tx_buff);
                err = api_hal_nfc_data_exchange(
                    dev_active, tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_E(NFC_WORKER_TAG, "Error during selection PPSE request: %d", err);
                    message.type = NfcMessageTypeEMVNotFound;
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(
                    NFC_WORKER_TAG, "Select PPSE response received. Start parsing response");
                if(emv_decode_ppse_response(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_I(NFC_WORKER_TAG, "Select PPSE responce parced");
                } else {
                    FURI_LOG_E(NFC_WORKER_TAG, "Can't find pay application");
                    message.type = NfcMessageTypeEMVNotFound;
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(NFC_WORKER_TAG, "Starting application ...");
                tx_len = emv_prepare_select_app(tx_buff, &emv_app);
                err = api_hal_nfc_data_exchange(
                    dev_active, tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_E(
                        NFC_WORKER_TAG, "Error during application selection request: %d", err);
                    message.type = NfcMessageTypeEMVNotFound;
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(
                    NFC_WORKER_TAG,
                    "Select application response received. Start parsing response");
                if(emv_decode_select_app_response(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_I(NFC_WORKER_TAG, "Card name: %s", emv_app.name);
                    memcpy(message.device.emv_card.name, emv_app.name, sizeof(emv_app.name));
                } else {
                    FURI_LOG_E(NFC_WORKER_TAG, "Can't read card name");
                    message.type = NfcMessageTypeEMVNotFound;
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(NFC_WORKER_TAG, "Starting Get Processing Options command ...");
                tx_len = emv_prepare_get_proc_opt(tx_buff, &emv_app);
                err = api_hal_nfc_data_exchange(
                    dev_active, tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_E(
                        NFC_WORKER_TAG, "Error during Get Processing Options command: %d", err);
                    message.type = NfcMessageTypeEMVNotFound;
                    api_hal_nfc_deactivate();
                    continue;
                }
                if(emv_decode_get_proc_opt(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_I(NFC_WORKER_TAG, "Card number parsed");
                    message.type = NfcMessageTypeEMVFound;
                    memcpy(
                        message.device.emv_card.number,
                        emv_app.card_number,
                        sizeof(emv_app.card_number));
                    api_hal_nfc_deactivate();
                    continue;
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
                            err = api_hal_nfc_data_exchange(
                                dev_active, tx_buff, tx_len, &rx_buff, &rx_len, false);
                            if(err != ERR_NONE) {
                                FURI_LOG_E(
                                    NFC_WORKER_TAG,
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
                        FURI_LOG_I(NFC_WORKER_TAG, "Card PAN found");
                        message.type = NfcMessageTypeEMVFound;
                        memcpy(
                            message.device.emv_card.number,
                            emv_app.card_number,
                            sizeof(emv_app.card_number));
                    } else {
                        FURI_LOG_E(NFC_WORKER_TAG, "Can't read card number");
                        message.type = NfcMessageTypeEMVNotFound;
                    }
                    api_hal_nfc_deactivate();
                }
            } else {
                // Can't find EMV card
                FURI_LOG_W(NFC_WORKER_TAG, "Card doesn't support EMV");
                message.type = NfcMessageTypeEMVNotFound;
                api_hal_nfc_deactivate();
            }
        } else {
            // Can't find EMV card
            FURI_LOG_W(NFC_WORKER_TAG, "Can't find any cards");
            message.type = NfcMessageTypeEMVNotFound;
            api_hal_nfc_deactivate();
        }
        osDelay(20);
    }
    api_hal_nfc_deactivate();
}

void nfc_worker_poll(NfcWorker* nfc_worker) {
    rfalNfcDevice* dev_list;
    uint8_t dev_cnt;
    // Update screen before start searching
    NfcMessage message = {.type = NfcMessageTypeDeviceNotFound};
    furi_check(osMessageQueuePut(nfc_worker->message_queue, &message, 0, osWaitForever) == osOK);

    while(nfc_worker->state == NfcWorkerStatePoll) {
        if(api_hal_nfc_detect(&dev_list, &dev_cnt, 100, true)) {
            // Send message with first device found
            message.type = NfcMessageTypeDeviceFound;
            if(dev_list[0].type == RFAL_NFC_LISTEN_TYPE_NFCA) {
                message.device.type = NfcDeviceTypeNfca;
                message.device.nfca = dev_list[0].dev.nfca;
            } else if(dev_list[0].type == RFAL_NFC_LISTEN_TYPE_NFCB) {
                message.device.type = NfcDeviceTypeNfcb;
                message.device.nfcb = dev_list[0].dev.nfcb;
            } else if(dev_list[0].type == RFAL_NFC_LISTEN_TYPE_NFCF) {
                message.device.type = NfcDeviceTypeNfcf;
                message.device.nfcf = dev_list[0].dev.nfcf;
            } else if(dev_list[0].type == RFAL_NFC_LISTEN_TYPE_NFCV) {
                message.device.type = NfcDeviceTypeNfcv;
                message.device.nfcv = dev_list[0].dev.nfcv;
            } else {
                // TODO show information about all found devices
                message.type = NfcMessageTypeDeviceNotFound;
            }
            furi_check(
                osMessageQueuePut(nfc_worker->message_queue, &message, 0, osWaitForever) == osOK);
        } else {
            message.type = NfcMessageTypeDeviceNotFound;
            furi_check(
                osMessageQueuePut(nfc_worker->message_queue, &message, 0, osWaitForever) == osOK);
        }
        osDelay(20);
    }
}

void nfc_worker_state_callback(rfalNfcState st) {
    (void)st;
}

ReturnCode nfc_worker_trx(
    uint8_t* txBuf,
    uint16_t txBufSize,
    uint8_t** rxData,
    uint16_t** rcvLen,
    uint32_t fwt) {
    ReturnCode err;

    err = rfalNfcDataExchangeStart(txBuf, txBufSize, rxData, rcvLen, fwt);
    if(err == ERR_NONE) {
        do {
            rfalNfcWorker();
            err = rfalNfcDataExchangeGetStatus();
        } while(err == ERR_BUSY);
    }
    return err;
}

void nfc_worker_exchange(NfcWorker* nfc_worker, rfalNfcDevice* nfc_device) {
    ReturnCode err = ERR_NONE;
    uint8_t* rxData;
    uint16_t* rcvLen;
    uint8_t txBuf[100];
    uint16_t txLen;

    do {
        rfalNfcWorker();
        switch(rfalNfcGetState()) {
        case RFAL_NFC_STATE_ACTIVATED:
            err = nfc_worker_trx(NULL, 0, &rxData, &rcvLen, 0);
            break;
        case RFAL_NFC_STATE_DATAEXCHANGE:
        case RFAL_NFC_STATE_DATAEXCHANGE_DONE:
            // Not supported
            txBuf[0] = ((char)0x68);
            txBuf[1] = ((char)0x00);
            txLen = 2;
            err = nfc_worker_trx(txBuf, txLen, &rxData, &rcvLen, RFAL_FWT_NONE);
            break;
        case RFAL_NFC_STATE_START_DISCOVERY:
            return;
        case RFAL_NFC_STATE_LISTEN_SLEEP:
        default:
            break;
        }
    } while((err == ERR_NONE) || (err == ERR_SLEEP_REQ));
}

void nfc_worker_emulate(NfcWorker* nfc_worker) {
    rfalNfcDiscoverParam params;
    params.compMode = RFAL_COMPLIANCE_MODE_NFC;
    params.techs2Find = RFAL_NFC_LISTEN_TECH_A;
    params.totalDuration = 1000U;
    params.devLimit = 1;
    params.wakeupEnabled = false;
    params.wakeupConfigDefault = true;
    params.nfcfBR = RFAL_BR_212;
    params.ap2pBR = RFAL_BR_424;
    params.maxBR = RFAL_BR_KEEP;
    params.GBLen = RFAL_NFCDEP_GB_MAX_LEN;
    params.notifyCb = nfc_worker_state_callback;

    params.lmConfigPA.nfcidLen = RFAL_LM_NFCID_LEN_07;
    params.lmConfigPA.nfcid[0] = 0x00;
    params.lmConfigPA.nfcid[1] = 0x01;
    params.lmConfigPA.nfcid[2] = 0x02;
    params.lmConfigPA.nfcid[3] = 0x03;
    params.lmConfigPA.nfcid[4] = 0x04;
    params.lmConfigPA.nfcid[5] = 0x05;
    params.lmConfigPA.nfcid[6] = 0x06;
    params.lmConfigPA.SENS_RES[0] = 0x44;
    params.lmConfigPA.SENS_RES[1] = 0x00;
    params.lmConfigPA.SEL_RES = 0x00;
    api_hal_nfc_exit_sleep();

    ReturnCode ret;
    ret = rfalNfcDiscover(&params);
    if(ret != ERR_NONE) {
        asm("bkpt 1");
        return;
    }

    rfalNfcDevice* nfc_device;
    while(nfc_worker->state == NfcWorkerStateEmulate) {
        rfalNfcWorker();
        if(rfalNfcIsDevActivated(rfalNfcGetState())) {
            rfalNfcGetActiveDevice(&nfc_device);
            nfc_worker_exchange(nfc_worker, nfc_device);
        }
        osDelay(10);
    }

    rfalNfcDeactivate(false);
    api_hal_nfc_start_sleep();
}

void nfc_worker_field(NfcWorker* nfc_worker) {
    api_hal_nfc_field_on();
    while(nfc_worker->state == NfcWorkerStateField) {
        osDelay(50);
    }
    api_hal_nfc_field_off();
}
