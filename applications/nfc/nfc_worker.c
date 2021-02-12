#include "nfc_worker_i.h"
#include <api-hal.h>

NfcWorker* nfc_worker_alloc(osMessageQueueId_t message_queue) {
    NfcWorker* nfc_worker = furi_alloc(sizeof(NfcWorker));
    nfc_worker->message_queue = message_queue;
    // Worker thread attributes
    nfc_worker->thread_attr.name = "nfc_worker";
    nfc_worker->thread_attr.stack_size = 2048;
    // Initialize rfal
    rfalAnalogConfigInitialize();
    nfc_worker->error = rfalNfcInitialize();
    if(nfc_worker->error == ERR_NONE) {
        rfalLowPowerModeStart();
        nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);
    } else {
        nfc_worker_change_state(nfc_worker, NfcWorkerStateBroken);
    }

    return nfc_worker;
}

NfcWorkerState nfc_worker_get_state(NfcWorker* nfc_worker) {
    return nfc_worker->state;
}

ReturnCode nfc_worker_get_error(NfcWorker* nfc_worker) {
    return nfc_worker->error;
}

void nfc_worker_free(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
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

    rfalLowPowerModeStop();
    if(nfc_worker->state == NfcWorkerStatePoll) {
        nfc_worker_poll(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulate) {
        nfc_worker_emulate(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateField) {
        nfc_worker_field(nfc_worker);
    }
    rfalLowPowerModeStart();

    nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);

    api_hal_power_insomnia_exit();
    osThreadExit();
}

void nfc_worker_poll(NfcWorker* nfc_worker) {
    while(nfc_worker->state == NfcWorkerStatePoll) {
        bool is_found = false;
        is_found |= nfc_worker_nfca_poll(nfc_worker);
        is_found |= nfc_worker_nfcb_poll(nfc_worker);
        is_found |= nfc_worker_nfcf_poll(nfc_worker);
        is_found |= nfc_worker_nfcv_poll(nfc_worker);
        rfalFieldOff();
        if(!is_found) {
            NfcMessage message;
            message.type = NfcMessageTypeDeviceNotFound;
            furi_check(
                osMessageQueuePut(nfc_worker->message_queue, &message, 0, osWaitForever) == osOK);
        }
        platformDelay(333);
    }
}

bool nfc_worker_nfca_poll(NfcWorker* nfc_worker) {
    ReturnCode ret;
    rfalNfcaSensRes sense_res;

    rfalNfcaPollerInitialize();
    rfalFieldOnAndStartGT();
    ret = rfalNfcaPollerTechnologyDetection(RFAL_COMPLIANCE_MODE_NFC, &sense_res);
    if(ret != ERR_NONE) {
        return false;
    }

    uint8_t dev_cnt;
    rfalNfcaListenDevice device;
    ret = rfalNfcaPollerFullCollisionResolution(RFAL_COMPLIANCE_MODE_NFC, 1, &device, &dev_cnt);
    if(ret != ERR_NONE) {
        return false;
    }

    if(dev_cnt) {
        rfalNfcaPollerSleep();
        NfcMessage message;
        message.type = NfcMessageTypeDeviceFound;
        message.device.type = NfcDeviceTypeNfca;
        message.device.nfca = device;
        furi_check(
            osMessageQueuePut(nfc_worker->message_queue, &message, 0, osWaitForever) == osOK);
        return true;
    }
    return false;
}

bool nfc_worker_nfcb_poll(NfcWorker* nfc_worker) {
    ReturnCode ret;

    rfalNfcbPollerInitialize();
    rfalFieldOnAndStartGT();

    rfalNfcbSensbRes sensb_res;
    uint8_t sensb_res_len;
    ret = rfalNfcbPollerTechnologyDetection(RFAL_COMPLIANCE_MODE_NFC, &sensb_res, &sensb_res_len);
    if(ret != ERR_NONE) {
        return false;
    }

    uint8_t dev_cnt;
    rfalNfcbListenDevice device;
    ret = rfalNfcbPollerCollisionResolution(RFAL_COMPLIANCE_MODE_NFC, 1, &device, &dev_cnt);
    if(ret != ERR_NONE) {
        return false;
    }

    if(dev_cnt) {
        rfalNfcbPollerSleep(device.sensbRes.nfcid0);
        NfcMessage message;
        message.type = NfcMessageTypeDeviceFound;
        message.device.type = NfcDeviceTypeNfcb;
        message.device.nfcb = device;
        furi_check(
            osMessageQueuePut(nfc_worker->message_queue, &message, 0, osWaitForever) == osOK);
        return true;
    }
    return false;
}

bool nfc_worker_nfcf_poll(NfcWorker* nfc_worker) {
    ReturnCode ret;

    rfalNfcfPollerInitialize(RFAL_BR_212);
    rfalFieldOnAndStartGT();

    ret = rfalNfcfPollerCheckPresence();
    if(ret != ERR_NONE) {
        return false;
    }

    uint8_t dev_cnt;
    rfalNfcfListenDevice device;
    ret = rfalNfcfPollerCollisionResolution(RFAL_COMPLIANCE_MODE_NFC, 1, &device, &dev_cnt);
    if(ret != ERR_NONE) {
        return false;
    }

    if(dev_cnt) {
        NfcMessage message;
        message.type = NfcMessageTypeDeviceFound;
        message.device.type = NfcDeviceTypeNfcf;
        message.device.nfcf = device;
        furi_check(
            osMessageQueuePut(nfc_worker->message_queue, &message, 0, osWaitForever) == osOK);
        return true;
    }
    return false;
}

bool nfc_worker_nfcv_poll(NfcWorker* nfc_worker) {
    ReturnCode ret;
    rfalNfcvInventoryRes invRes;

    rfalNfcvPollerInitialize();
    rfalFieldOnAndStartGT();

    ret = rfalNfcvPollerCheckPresence(&invRes);
    if(ret != ERR_NONE) {
        return false;
    }

    uint8_t dev_cnt;
    rfalNfcvListenDevice device;
    ret = rfalNfcvPollerCollisionResolution(RFAL_COMPLIANCE_MODE_NFC, 1, &device, &dev_cnt);
    if(ret != ERR_NONE) {
        return false;
    }

    if(dev_cnt) {
        rfalNfcvPollerSleep(RFAL_NFCV_REQ_FLAG_DEFAULT, device.InvRes.UID);
        NfcMessage message;
        message.type = NfcMessageTypeDeviceFound;
        message.device.type = NfcDeviceTypeNfcv;
        message.device.nfcv = device;
        furi_check(
            osMessageQueuePut(nfc_worker->message_queue, &message, 0, osWaitForever) == osOK);
        return true;
    }
    return false;
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
    ReturnCode err;
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
}

void nfc_worker_field(NfcWorker* nfc_worker) {
    st25r3916TxRxOn();
    while(nfc_worker->state == NfcWorkerStateField) {
        osDelay(50);
    }
    st25r3916TxRxOff();
}
