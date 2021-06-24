#include "nfc_worker_i.h"
#include <api-hal.h>
#include "nfc_protocols/emv_decoder.h"
#include "nfc_protocols/mifare_ultralight.h"

#define NFC_WORKER_TAG "nfc worker"

/***************************** NFC Worker API *******************************/

NfcWorker* nfc_worker_alloc(osMessageQueueId_t message_queue) {
    NfcWorker* nfc_worker = furi_alloc(sizeof(NfcWorker));
    nfc_worker->message_queue = message_queue;
    // Worker thread attributes
    nfc_worker->thread_attr.name = "nfc_worker";
    nfc_worker->thread_attr.stack_size = 8192;
    nfc_worker->callback = NULL;
    nfc_worker->context = NULL;
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

void nfc_worker_start(
    NfcWorker* nfc_worker,
    NfcWorkerState state,
    NfcWorkerCallback callback,
    void* context) {
    furi_assert(nfc_worker);
    furi_assert(nfc_worker->state == NfcWorkerStateReady);

    nfc_worker->callback = callback;
    nfc_worker->context = context;
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

/***************************** NFC Worker Thread *******************************/

void nfc_worker_task(void* context) {
    NfcWorker* nfc_worker = context;

    api_hal_power_insomnia_enter();
    api_hal_nfc_exit_sleep();

    if(nfc_worker->state == NfcWorkerStateDetect) {
        nfc_worker_detect(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulate) {
        nfc_worker_emulate(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadEMV) {
        nfc_worker_read_emv(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulateEMV) {
        nfc_worker_field(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadMfUltralight) {
        nfc_worker_read_mf_ultralight(nfc_worker);
        nfc_worker_emulate_emv(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateField) {
    }
    api_hal_nfc_deactivate();
    nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);
    api_hal_power_insomnia_exit();
    osThreadExit();
}

void nfc_worker_detect(NfcWorker* nfc_worker) {
    rfalNfcDevice* dev_list;
    rfalNfcDevice* dev;
    uint8_t dev_cnt;
    NfcMessage message;

    while(nfc_worker->state == NfcWorkerStateDetect) {
        message.found = false;
        if(api_hal_nfc_detect(&dev_list, &dev_cnt, 1000, true)) {
            // Process first found device
            dev = &dev_list[0];
            message.found = true;
            message.nfc_detect_data.uid_len = dev->nfcidLen;
            memcpy(message.nfc_detect_data.uid, dev->nfcid, dev->nfcidLen);

            if(dev->type == RFAL_NFC_LISTEN_TYPE_NFCA) {
                message.nfc_detect_data.device = NfcDeviceNfca;
                message.nfc_detect_data.atqa[0] = dev->dev.nfca.sensRes.anticollisionInfo;
                message.nfc_detect_data.atqa[1] = dev->dev.nfca.sensRes.platformInfo;
                message.nfc_detect_data.sak = dev->dev.nfca.selRes.sak;
                // TODO check protocols

            } else if(dev->type == RFAL_NFC_LISTEN_TYPE_NFCB) {
                message.nfc_detect_data.device = NfcDeviceNfcb;
            } else if(dev->type == RFAL_NFC_LISTEN_TYPE_NFCF) {
                message.nfc_detect_data.device = NfcDeviceNfcf;
            } else if(dev->type == RFAL_NFC_LISTEN_TYPE_NFCV) {
                message.nfc_detect_data.device = NfcDeviceNfcv;
            }
        }
        if(nfc_worker->callback) {
            nfc_worker->callback(nfc_worker->context);
        }
        furi_check(
            osMessageQueuePut(nfc_worker->message_queue, &message, 0, osWaitForever) == osOK);
        osDelay(100);
    }
}

void nfc_worker_emulate(NfcWorker* nfc_worker) {
    while(nfc_worker->state == NfcWorkerStateEmulate) {
        if(api_hal_nfc_listen(ApiHalNfcEmulateParamsMifare, 100)) {
            FURI_LOG_I(NFC_WORKER_TAG, "Reader detected");
            api_hal_nfc_deactivate();
        }
        osDelay(10);
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

    NfcMessage message = {.found = false};
    while(nfc_worker->state == NfcWorkerStateReadEMV) {
        if(nfc_worker->callback) {
            nfc_worker->callback(nfc_worker->context);
        }
        furi_check(
            osMessageQueuePut(nfc_worker->message_queue, &message, 0, osWaitForever) == osOK);
        memset(&emv_app, 0, sizeof(emv_app));
        if(api_hal_nfc_detect(&dev_list, &dev_cnt, 1000, false)) {
            // Card was found. Check that it supports EMV
            if(dev_list[0].rfInterface == RFAL_NFC_INTERFACE_ISODEP) {
                FURI_LOG_I(NFC_WORKER_TAG, "Send select PPSE command");
                tx_len = emv_prepare_select_ppse(tx_buff);
                err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_E(NFC_WORKER_TAG, "Error during selection PPSE request: %d", err);
                    message.found = false;
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(
                    NFC_WORKER_TAG, "Select PPSE response received. Start parsing response");
                if(emv_decode_ppse_response(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_I(NFC_WORKER_TAG, "Select PPSE responce parced");
                } else {
                    FURI_LOG_E(NFC_WORKER_TAG, "Can't find pay application");
                    message.found = false;
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(NFC_WORKER_TAG, "Starting application ...");
                tx_len = emv_prepare_select_app(tx_buff, &emv_app);
                err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_E(
                        NFC_WORKER_TAG, "Error during application selection request: %d", err);
                    message.found = false;
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(
                    NFC_WORKER_TAG,
                    "Select application response received. Start parsing response");
                if(emv_decode_select_app_response(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_I(NFC_WORKER_TAG, "Card name: %s", emv_app.name);
                    memcpy(message.nfc_emv_data.name, emv_app.name, sizeof(emv_app.name));
                } else {
                    FURI_LOG_E(NFC_WORKER_TAG, "Can't read card name");
                    message.found = false;
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(NFC_WORKER_TAG, "Starting Get Processing Options command ...");
                tx_len = emv_prepare_get_proc_opt(tx_buff, &emv_app);
                err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_E(
                        NFC_WORKER_TAG, "Error during Get Processing Options command: %d", err);
                    message.found = false;
                    api_hal_nfc_deactivate();
                    continue;
                }
                if(emv_decode_get_proc_opt(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_I(NFC_WORKER_TAG, "Card number parsed");
                    message.found = true;
                    memcpy(
                        message.nfc_emv_data.number,
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
                                tx_buff, tx_len, &rx_buff, &rx_len, false);
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
                        message.found = true;
                        memcpy(
                            message.nfc_emv_data.number,
                            emv_app.card_number,
                            sizeof(emv_app.card_number));
                    } else {
                        FURI_LOG_E(NFC_WORKER_TAG, "Can't read card number");
                        message.found = false;
                    }
                    api_hal_nfc_deactivate();
                }
            } else {
                // Can't find EMV card
                FURI_LOG_W(NFC_WORKER_TAG, "Card doesn't support EMV");
                message.found = false;
                api_hal_nfc_deactivate();
            }
        } else {
            // Can't find EMV card
            FURI_LOG_W(NFC_WORKER_TAG, "Can't find any cards");
            message.found = false;
            api_hal_nfc_deactivate();
        }
        osDelay(20);
    }
}

void nfc_worker_emulate_emv(NfcWorker* nfc_worker) {
    ReturnCode err;
    uint8_t tx_buff[255] = {};
    uint16_t tx_len = 0;
    uint8_t* rx_buff;
    uint16_t* rx_len;

    while(nfc_worker->state == NfcWorkerStateEmulateEMV) {
        if(api_hal_nfc_listen(ApiHalNfcEmulateParamsEMV, 1000)) {
            FURI_LOG_I(NFC_WORKER_TAG, "POS terminal detected");
            // Read data from POS terminal
            err = api_hal_nfc_data_exchange(NULL, 0, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_I(NFC_WORKER_TAG, "Received Select PPSE");
            } else {
                FURI_LOG_E(NFC_WORKER_TAG, "Error in 1st data exchange: select PPSE");
                api_hal_nfc_deactivate();
                continue;
            }
            FURI_LOG_I(NFC_WORKER_TAG, "Transive SELECT PPSE ANS");
            tx_len = emv_select_ppse_ans(tx_buff);
            err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_I(NFC_WORKER_TAG, "Received Select APP");
            } else {
                FURI_LOG_E(NFC_WORKER_TAG, "Error in 2nd data exchange: select APP");
                api_hal_nfc_deactivate();
                continue;
            }

            FURI_LOG_I(NFC_WORKER_TAG, "Transive SELECT APP ANS");
            tx_len = emv_select_app_ans(tx_buff);
            err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_I(NFC_WORKER_TAG, "Received PDOL");
            } else {
                FURI_LOG_E(NFC_WORKER_TAG, "Error in 3rd data exchange: receive PDOL");
                api_hal_nfc_deactivate();
                continue;
            }

            FURI_LOG_I(NFC_WORKER_TAG, "Transive PDOL ANS");
            tx_len = emv_get_proc_opt_ans(tx_buff);
            err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_I(NFC_WORKER_TAG, "Received PDOL");
            }
            api_hal_nfc_deactivate();
        } else {
            FURI_LOG_W(NFC_WORKER_TAG, "Can't find reader");
        }
        osDelay(20);
    }
}

void nfc_worker_read_mf_ultralight(NfcWorker* nfc_worker) {
    ReturnCode err;
    rfalNfcDevice* dev_list;
    uint8_t dev_cnt = 0;
    uint8_t tx_buff[255] = {};
    uint16_t tx_len = 0;
    uint8_t* rx_buff;
    uint16_t* rx_len;
    MfUltralightRead mf_ul_read;

    // Update screen before start searching
    NfcMessage message = {.found = false};
    while(nfc_worker->state == NfcWorkerStateReadMfUltralight) {
        if(nfc_worker->callback) {
            nfc_worker->callback(nfc_worker->context);
        }
        furi_check(
            osMessageQueuePut(nfc_worker->message_queue, &message, 0, osWaitForever) == osOK);
        api_hal_nfc_deactivate();
        memset(&mf_ul_read, 0, sizeof(mf_ul_read));
        if(api_hal_nfc_detect(&dev_list, &dev_cnt, 1000, false)) {
            if(dev_list[0].type == RFAL_NFC_LISTEN_TYPE_NFCA &&
               mf_ul_check_card_type(
                   dev_list[0].dev.nfca.sensRes.anticollisionInfo,
                   dev_list[0].dev.nfca.sensRes.platformInfo,
                   dev_list[0].dev.nfca.selRes.sak)) {
                // Get Mifare Ultralight version
                FURI_LOG_I(
                    NFC_WORKER_TAG, "Found Mifare Ultralight tag. Trying to get tag version");
                tx_len = mf_ul_prepare_get_version(tx_buff);
                err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err == ERR_NONE) {
                    mf_ul_parse_get_version_response(rx_buff, &mf_ul_read);
                    FURI_LOG_I(
                        NFC_WORKER_TAG,
                        "Mifare Ultralight Type: %d, Pages: %d",
                        mf_ul_read.type,
                        mf_ul_read.pages_to_read);
                } else if(err == ERR_TIMEOUT) {
                    FURI_LOG_W(
                        NFC_WORKER_TAG,
                        "Card doesn't respond to GET VERSION command. Reinit card and set default read parameters");
                    err = ERR_NONE;
                    mf_ul_set_default_version(&mf_ul_read);
                    // Reinit device
                    api_hal_nfc_deactivate();
                    if(!api_hal_nfc_detect(&dev_list, &dev_cnt, 1000, false)) {
                        FURI_LOG_E(NFC_WORKER_TAG, "Lost connection. Restarting search");
                        message.found = false;
                        continue;
                    }
                } else {
                    FURI_LOG_E(
                        NFC_WORKER_TAG,
                        "Error getting Mifare Ultralight version. Error code: %d",
                        err);
                    message.found = false;
                    continue;
                }

                // Dump Mifare Ultralight card
                FURI_LOG_I(NFC_WORKER_TAG, "Trying to read pages");
                if(mf_ul_read.support_fast_read) {
                    // Read card with FAST_READ command
                    tx_len = mf_ul_prepare_fast_read(tx_buff, 0x00, mf_ul_read.pages_to_read - 1);
                    err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                    if(err == ERR_NONE) {
                        FURI_LOG_I(
                            NFC_WORKER_TAG,
                            "Fast read pages %d - %d succeed",
                            0,
                            mf_ul_read.pages_to_read - 1);
                        memcpy(mf_ul_read.dump, rx_buff, mf_ul_read.pages_to_read * 4);
                        mf_ul_read.pages_readed = mf_ul_read.pages_to_read;
                    } else {
                        FURI_LOG_E(NFC_WORKER_TAG, "Fast read failed");
                        message.found = false;
                        continue;
                    }
                } else {
                    // READ card with READ command (4 pages at a time)
                    for(uint8_t page = 0; page < mf_ul_read.pages_to_read; page += 4) {
                        tx_len = mf_ul_prepare_read(tx_buff, page);
                        err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                        if(err == ERR_NONE) {
                            FURI_LOG_I(
                                NFC_WORKER_TAG, "Read pages %d - %d succeed", page, page + 3);
                            memcpy(&mf_ul_read.dump[page * 4], rx_buff, 4 * 4);
                            mf_ul_read.pages_readed += 4;
                        } else {
                            FURI_LOG_W(
                                NFC_WORKER_TAG, "Read pages %d - %d failed", page, page + 3);
                        }
                    }
                }

                // Fill message for nfc application
                message.found = true;
                message.nfc_mifare_ul_data.nfc_data.uid_len = dev_list[0].dev.nfca.nfcId1Len;
                message.nfc_mifare_ul_data.nfc_data.atqa[0] =
                    dev_list[0].dev.nfca.sensRes.anticollisionInfo;
                message.nfc_mifare_ul_data.nfc_data.atqa[1] =
                    dev_list[0].dev.nfca.sensRes.platformInfo;
                message.nfc_mifare_ul_data.nfc_data.sak = dev_list[0].dev.nfca.selRes.sak;
                memcpy(
                    message.nfc_mifare_ul_data.nfc_data.uid,
                    dev_list[0].dev.nfca.nfcId1,
                    message.nfc_mifare_ul_data.nfc_data.uid_len);
                memcpy(message.nfc_mifare_ul_data.man_block, mf_ul_read.dump, 4 * 3);
                memcpy(message.nfc_mifare_ul_data.otp, &mf_ul_read.dump[4 * 3], 4);
                for(uint8_t i = 0; i < mf_ul_read.pages_readed * 4; i += 4) {
                    printf("Page %2d: ", i / 4);
                    for(uint8_t j = 0; j < 4; j++) {
                        printf("%02X ", mf_ul_read.dump[i + j]);
                    }
                    printf("\r\n");
                }
            } else {
                message.found = false;
                FURI_LOG_W(NFC_WORKER_TAG, "Tag does not support Mifare Ultralight");
            }
        } else {
            message.found = false;
            FURI_LOG_W(NFC_WORKER_TAG, "Can't find any tags");
        }
        osDelay(100);
    }
}

void nfc_worker_field(NfcWorker* nfc_worker) {
    api_hal_nfc_field_on();
    while(nfc_worker->state == NfcWorkerStateField) {
        osDelay(50);
    }
    api_hal_nfc_field_off();
}
