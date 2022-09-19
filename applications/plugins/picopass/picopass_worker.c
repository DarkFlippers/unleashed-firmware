#include "picopass_worker_i.h"

#define TAG "PicopassWorker"

const uint8_t picopass_iclass_key[] = {0xaf, 0xa7, 0x85, 0xa7, 0xda, 0xb3, 0x33, 0x78};
const uint8_t picopass_factory_key[] = {0x76, 0x65, 0x54, 0x43, 0x32, 0x21, 0x10, 0x00};

static void picopass_worker_enable_field() {
    furi_hal_nfc_ll_txrx_on();
    furi_hal_nfc_exit_sleep();
    furi_hal_nfc_ll_poll();
}

static ReturnCode picopass_worker_disable_field(ReturnCode rc) {
    furi_hal_nfc_ll_txrx_off();
    furi_hal_nfc_start_sleep();
    return rc;
}

/***************************** Picopass Worker API *******************************/

PicopassWorker* picopass_worker_alloc() {
    PicopassWorker* picopass_worker = malloc(sizeof(PicopassWorker));

    // Worker thread attributes
    picopass_worker->thread = furi_thread_alloc();
    furi_thread_set_name(picopass_worker->thread, "PicopassWorker");
    furi_thread_set_stack_size(picopass_worker->thread, 8192);
    furi_thread_set_callback(picopass_worker->thread, picopass_worker_task);
    furi_thread_set_context(picopass_worker->thread, picopass_worker);

    picopass_worker->callback = NULL;
    picopass_worker->context = NULL;
    picopass_worker->storage = furi_record_open(RECORD_STORAGE);

    picopass_worker_change_state(picopass_worker, PicopassWorkerStateReady);

    return picopass_worker;
}

void picopass_worker_free(PicopassWorker* picopass_worker) {
    furi_assert(picopass_worker);

    furi_thread_free(picopass_worker->thread);

    furi_record_close(RECORD_STORAGE);

    free(picopass_worker);
}

PicopassWorkerState picopass_worker_get_state(PicopassWorker* picopass_worker) {
    return picopass_worker->state;
}

void picopass_worker_start(
    PicopassWorker* picopass_worker,
    PicopassWorkerState state,
    PicopassDeviceData* dev_data,
    PicopassWorkerCallback callback,
    void* context) {
    furi_assert(picopass_worker);
    furi_assert(dev_data);

    picopass_worker->callback = callback;
    picopass_worker->context = context;
    picopass_worker->dev_data = dev_data;
    picopass_worker_change_state(picopass_worker, state);
    furi_thread_start(picopass_worker->thread);
}

void picopass_worker_stop(PicopassWorker* picopass_worker) {
    furi_assert(picopass_worker);
    if(picopass_worker->state == PicopassWorkerStateBroken ||
       picopass_worker->state == PicopassWorkerStateReady) {
        return;
    }
    picopass_worker_disable_field(ERR_NONE);

    picopass_worker_change_state(picopass_worker, PicopassWorkerStateStop);
    furi_thread_join(picopass_worker->thread);
}

void picopass_worker_change_state(PicopassWorker* picopass_worker, PicopassWorkerState state) {
    picopass_worker->state = state;
}

/***************************** Picopass Worker Thread *******************************/

ReturnCode picopass_detect_card(int timeout) {
    UNUSED(timeout);

    ReturnCode err;

    err = rfalPicoPassPollerInitialize();
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerInitialize error %d", err);
        return err;
    }

    err = rfalFieldOnAndStartGT();
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalFieldOnAndStartGT error %d", err);
        return err;
    }

    err = rfalPicoPassPollerCheckPresence();
    if(err != ERR_RF_COLLISION) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerCheckPresence error %d", err);
        return err;
    }

    return ERR_NONE;
}

ReturnCode picopass_read_preauth(PicopassBlock* AA1) {
    rfalPicoPassIdentifyRes idRes;
    rfalPicoPassSelectRes selRes;

    ReturnCode err;

    err = rfalPicoPassPollerIdentify(&idRes);
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerIdentify error %d", err);
        return err;
    }

    err = rfalPicoPassPollerSelect(idRes.CSN, &selRes);
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerSelect error %d", err);
        return err;
    }

    memcpy(AA1[PICOPASS_CSN_BLOCK_INDEX].data, selRes.CSN, sizeof(selRes.CSN));
    FURI_LOG_D(
        TAG,
        "csn %02x%02x%02x%02x%02x%02x%02x%02x",
        AA1[PICOPASS_CSN_BLOCK_INDEX].data[0],
        AA1[PICOPASS_CSN_BLOCK_INDEX].data[1],
        AA1[PICOPASS_CSN_BLOCK_INDEX].data[2],
        AA1[PICOPASS_CSN_BLOCK_INDEX].data[3],
        AA1[PICOPASS_CSN_BLOCK_INDEX].data[4],
        AA1[PICOPASS_CSN_BLOCK_INDEX].data[5],
        AA1[PICOPASS_CSN_BLOCK_INDEX].data[6],
        AA1[PICOPASS_CSN_BLOCK_INDEX].data[7]);

    rfalPicoPassReadBlockRes cfg = {0};
    err = rfalPicoPassPollerReadBlock(PICOPASS_CONFIG_BLOCK_INDEX, &cfg);
    memcpy(AA1[PICOPASS_CONFIG_BLOCK_INDEX].data, cfg.data, sizeof(cfg.data));
    FURI_LOG_D(
        TAG,
        "config %02x%02x%02x%02x%02x%02x%02x%02x",
        AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[0],
        AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[1],
        AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[2],
        AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[3],
        AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[4],
        AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[5],
        AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[6],
        AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[7]);

    rfalPicoPassReadBlockRes aia;
    err = rfalPicoPassPollerReadBlock(PICOPASS_AIA_BLOCK_INDEX, &aia);
    memcpy(AA1[PICOPASS_AIA_BLOCK_INDEX].data, aia.data, sizeof(aia.data));
    FURI_LOG_D(
        TAG,
        "aia %02x%02x%02x%02x%02x%02x%02x%02x",
        AA1[PICOPASS_AIA_BLOCK_INDEX].data[0],
        AA1[PICOPASS_AIA_BLOCK_INDEX].data[1],
        AA1[PICOPASS_AIA_BLOCK_INDEX].data[2],
        AA1[PICOPASS_AIA_BLOCK_INDEX].data[3],
        AA1[PICOPASS_AIA_BLOCK_INDEX].data[4],
        AA1[PICOPASS_AIA_BLOCK_INDEX].data[5],
        AA1[PICOPASS_AIA_BLOCK_INDEX].data[6],
        AA1[PICOPASS_AIA_BLOCK_INDEX].data[7]);

    return ERR_NONE;
}

ReturnCode picopass_read_card(PicopassBlock* AA1) {
    rfalPicoPassReadCheckRes rcRes;
    rfalPicoPassCheckRes chkRes;

    ReturnCode err;

    uint8_t div_key[8] = {0};
    uint8_t mac[4] = {0};
    uint8_t ccnr[12] = {0};

    err = rfalPicoPassPollerReadCheck(&rcRes);
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerReadCheck error %d", err);
        return err;
    }
    memcpy(ccnr, rcRes.CCNR, sizeof(rcRes.CCNR)); // last 4 bytes left 0

    loclass_diversifyKey(AA1[PICOPASS_CSN_BLOCK_INDEX].data, picopass_iclass_key, div_key);
    loclass_opt_doReaderMAC(ccnr, div_key, mac);

    err = rfalPicoPassPollerCheck(mac, &chkRes);
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerCheck error %d", err);
        return err;
    }

    size_t app_limit = AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[0] < PICOPASS_MAX_APP_LIMIT ?
                           AA1[PICOPASS_CONFIG_BLOCK_INDEX].data[0] :
                           PICOPASS_MAX_APP_LIMIT;

    for(size_t i = 2; i < app_limit; i++) {
        rfalPicoPassReadBlockRes block;
        err = rfalPicoPassPollerReadBlock(i, &block);
        if(err != ERR_NONE) {
            FURI_LOG_E(TAG, "rfalPicoPassPollerReadBlock error %d", err);
            return err;
        }

        FURI_LOG_D(
            TAG,
            "rfalPicoPassPollerReadBlock %d %02x%02x%02x%02x%02x%02x%02x%02x",
            i,
            block.data[0],
            block.data[1],
            block.data[2],
            block.data[3],
            block.data[4],
            block.data[5],
            block.data[6],
            block.data[7]);

        memcpy(AA1[i].data, block.data, sizeof(block.data));
    }

    return ERR_NONE;
}

ReturnCode picopass_write_card(PicopassBlock* AA1) {
    rfalPicoPassIdentifyRes idRes;
    rfalPicoPassSelectRes selRes;
    rfalPicoPassReadCheckRes rcRes;
    rfalPicoPassCheckRes chkRes;

    ReturnCode err;

    uint8_t div_key[8] = {0};
    uint8_t mac[4] = {0};
    uint8_t ccnr[12] = {0};

    err = rfalPicoPassPollerIdentify(&idRes);
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerIdentify error %d", err);
        return err;
    }

    err = rfalPicoPassPollerSelect(idRes.CSN, &selRes);
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerSelect error %d", err);
        return err;
    }

    err = rfalPicoPassPollerReadCheck(&rcRes);
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerReadCheck error %d", err);
        return err;
    }
    memcpy(ccnr, rcRes.CCNR, sizeof(rcRes.CCNR)); // last 4 bytes left 0

    loclass_diversifyKey(selRes.CSN, picopass_iclass_key, div_key);
    loclass_opt_doReaderMAC(ccnr, div_key, mac);

    err = rfalPicoPassPollerCheck(mac, &chkRes);
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerCheck error %d", err);
        return err;
    }

    for(size_t i = 6; i < 10; i++) {
        FURI_LOG_D(TAG, "rfalPicoPassPollerWriteBlock %d", i);
        uint8_t data[9] = {0};
        data[0] = i;
        memcpy(data + 1, AA1[i].data, RFAL_PICOPASS_MAX_BLOCK_LEN);
        loclass_doMAC_N(data, sizeof(data), div_key, mac);
        FURI_LOG_D(
            TAG,
            "loclass_doMAC_N %d %02x%02x%02x%02x%02x%02x%02x%02x %02x%02x%02x%02x",
            i,
            data[1],
            data[2],
            data[3],
            data[4],
            data[5],
            data[6],
            data[7],
            data[8],
            mac[0],
            mac[1],
            mac[2],
            mac[3]);

        err = rfalPicoPassPollerWriteBlock(i, AA1[i].data, mac);
        if(err != ERR_NONE) {
            FURI_LOG_E(TAG, "rfalPicoPassPollerWriteBlock error %d", err);
            return err;
        }
    }

    return ERR_NONE;
}

int32_t picopass_worker_task(void* context) {
    PicopassWorker* picopass_worker = context;

    picopass_worker_enable_field();
    if(picopass_worker->state == PicopassWorkerStateDetect) {
        picopass_worker_detect(picopass_worker);
    } else if(picopass_worker->state == PicopassWorkerStateWrite) {
        picopass_worker_write(picopass_worker);
    }
    picopass_worker_disable_field(ERR_NONE);

    picopass_worker_change_state(picopass_worker, PicopassWorkerStateReady);

    return 0;
}

void picopass_worker_detect(PicopassWorker* picopass_worker) {
    picopass_device_data_clear(picopass_worker->dev_data);
    PicopassDeviceData* dev_data = picopass_worker->dev_data;

    PicopassBlock* AA1 = dev_data->AA1;
    PicopassPacs* pacs = &dev_data->pacs;
    ReturnCode err;

    // reset device data
    for(size_t i = 0; i < PICOPASS_MAX_APP_LIMIT; i++) {
        memset(AA1[i].data, 0, sizeof(AA1[i].data));
    }
    memset(pacs, 0, sizeof(PicopassPacs));

    PicopassWorkerEvent nextState = PicopassWorkerEventSuccess;

    while(picopass_worker->state == PicopassWorkerStateDetect) {
        if(picopass_detect_card(1000) == ERR_NONE) {
            // Process first found device
            err = picopass_read_preauth(AA1);
            if(err != ERR_NONE) {
                FURI_LOG_E(TAG, "picopass_read_preauth error %d", err);
                nextState = PicopassWorkerEventFail;
            }

            // Thank you proxmark!
            pacs->legacy = (memcmp(AA1[5].data, "\xff\xff\xff\xff\xff\xff\xff\xff", 8) == 0);
            pacs->se_enabled = (memcmp(AA1[5].data, "\xff\xff\xff\x00\x06\xff\xff\xff", 8) == 0);
            if(pacs->se_enabled) {
                FURI_LOG_D(TAG, "SE enabled");
            }

            err = picopass_read_card(AA1);
            if(err != ERR_NONE) {
                FURI_LOG_E(TAG, "picopass_read_card error %d", err);
                nextState = PicopassWorkerEventFail;
            }

            if(nextState == PicopassWorkerEventSuccess) {
                err = picopass_device_parse_credential(AA1, pacs);
            }
            if(err != ERR_NONE) {
                FURI_LOG_E(TAG, "picopass_device_parse_credential error %d", err);
                nextState = PicopassWorkerEventFail;
            }

            if(nextState == PicopassWorkerEventSuccess) {
                err = picopass_device_parse_wiegand(pacs->credential, &pacs->record);
            }
            if(err != ERR_NONE) {
                FURI_LOG_E(TAG, "picopass_device_parse_wiegand error %d", err);
                nextState = PicopassWorkerEventFail;
            }

            // Notify caller and exit
            if(picopass_worker->callback) {
                picopass_worker->callback(nextState, picopass_worker->context);
            }
            break;
        }
        furi_delay_ms(100);
    }
}

void picopass_worker_write(PicopassWorker* picopass_worker) {
    PicopassDeviceData* dev_data = picopass_worker->dev_data;
    PicopassBlock* AA1 = dev_data->AA1;
    ReturnCode err;
    PicopassWorkerEvent nextState = PicopassWorkerEventSuccess;

    while(picopass_worker->state == PicopassWorkerStateWrite) {
        if(picopass_detect_card(1000) == ERR_NONE) {
            err = picopass_write_card(AA1);
            if(err != ERR_NONE) {
                FURI_LOG_E(TAG, "picopass_write_card error %d", err);
                nextState = PicopassWorkerEventFail;
            }

            // Notify caller and exit
            if(picopass_worker->callback) {
                picopass_worker->callback(nextState, picopass_worker->context);
            }
            break;
        }
        furi_delay_ms(100);
    }
}
