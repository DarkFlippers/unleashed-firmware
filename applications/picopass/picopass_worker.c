#include "picopass_worker_i.h"
#include <furi_hal.h>

#include <stdlib.h>
#include <st25r3916.h>
#include <rfal_analogConfig.h>
#include <rfal_rf.h>
#include <rfal_nfc.h>

#include <mbedtls/des.h>
#include <loclass/optimized_ikeys.h>
#include <loclass/optimized_cipher.h>

#include <platform.h>

#define TAG "PicopassWorker"

const uint8_t picopass_iclass_key[] = {0xaf, 0xa7, 0x85, 0xa7, 0xda, 0xb3, 0x33, 0x78};
const uint8_t picopass_iclass_decryptionkey[] =
    {0xb4, 0x21, 0x2c, 0xca, 0xb7, 0xed, 0x21, 0x0f, 0x7b, 0x93, 0xd4, 0x59, 0x39, 0xc7, 0xdd, 0x36};

static void picopass_worker_enable_field() {
    st25r3916TxRxOn();
    rfalLowPowerModeStop();
    rfalWorker();
}

static ReturnCode picopass_worker_disable_field(ReturnCode rc) {
    st25r3916TxRxOff();
    rfalLowPowerModeStart();
    return rc;
}

static ReturnCode picopass_worker_decrypt(uint8_t* enc_data, uint8_t* dec_data) {
    uint8_t key[32] = {0};
    memcpy(key, picopass_iclass_decryptionkey, sizeof(picopass_iclass_decryptionkey));
    mbedtls_des3_context ctx;
    mbedtls_des3_init(&ctx);
    mbedtls_des3_set2key_dec(&ctx, key);
    mbedtls_des3_crypt_ecb(&ctx, enc_data, dec_data);
    mbedtls_des3_free(&ctx);
    return ERR_NONE;
}

static ReturnCode picopass_worker_parse_wiegand(uint8_t* data, PicopassWiegandRecord* record) {
    uint32_t* halves = (uint32_t*)data;
    if(halves[0] == 0) {
        uint8_t leading0s = __builtin_clz(REVERSE_BYTES_U32(halves[1]));
        record->bitLength = 31 - leading0s;
    } else {
        uint8_t leading0s = __builtin_clz(REVERSE_BYTES_U32(halves[0]));
        record->bitLength = 63 - leading0s;
    }
    FURI_LOG_D(TAG, "bitLength: %d", record->bitLength);

    if(record->bitLength == 26) {
        uint8_t* v4 = data + 4;
        v4[0] = 0;

        uint32_t bot = v4[3] | (v4[2] << 8) | (v4[1] << 16) | (v4[0] << 24);

        record->CardNumber = (bot >> 1) & 0xFFFF;
        record->FacilityCode = (bot >> 17) & 0xFF;
        record->valid = true;
    } else {
        record->CardNumber = 0;
        record->FacilityCode = 0;
        record->valid = false;
    }
    return ERR_NONE;
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
    picopass_worker->storage = furi_record_open("storage");

    picopass_worker_change_state(picopass_worker, PicopassWorkerStateReady);

    return picopass_worker;
}

void picopass_worker_free(PicopassWorker* picopass_worker) {
    furi_assert(picopass_worker);

    furi_thread_free(picopass_worker->thread);

    furi_record_close("storage");

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

ReturnCode picopass_read_card(ApplicationArea* AA1) {
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

    for(size_t i = 0; i < 4; i++) {
        FURI_LOG_D(TAG, "rfalPicoPassPollerReadBlock block %d", i + 6);
        rfalPicoPassReadBlockRes block;
        err = rfalPicoPassPollerReadBlock(i + 6, &block);
        if(err != ERR_NONE) {
            FURI_LOG_E(TAG, "rfalPicoPassPollerReadBlock error %d", err);
            return err;
        }

        FURI_LOG_D(
            TAG,
            "rfalPicoPassPollerReadBlock %d %02x%02x%02x%02x%02x%02x%02x%02x",
            i + 6,
            block.data[0],
            block.data[1],
            block.data[2],
            block.data[3],
            block.data[4],
            block.data[5],
            block.data[6],
            block.data[7]);

        memcpy(&(AA1->block[i]), &block, sizeof(block));
    }

    return ERR_NONE;
}

int32_t picopass_worker_task(void* context) {
    PicopassWorker* picopass_worker = context;

    picopass_worker_enable_field();
    if(picopass_worker->state == PicopassWorkerStateDetect) {
        picopass_worker_detect(picopass_worker);
    }
    picopass_worker_disable_field(ERR_NONE);

    picopass_worker_change_state(picopass_worker, PicopassWorkerStateReady);

    return 0;
}

void picopass_worker_detect(PicopassWorker* picopass_worker) {
    picopass_device_data_clear(picopass_worker->dev_data);
    PicopassDeviceData* dev_data = picopass_worker->dev_data;

    ApplicationArea* AA1 = &dev_data->AA1;
    PicopassPacs* pacs = &dev_data->pacs;
    ReturnCode err;

    while(picopass_worker->state == PicopassWorkerStateDetect) {
        if(picopass_detect_card(1000) == ERR_NONE) {
            // Process first found device
            err = picopass_read_card(AA1);
            if(err != ERR_NONE) {
                FURI_LOG_E(TAG, "picopass_read_card error %d", err);
            }

            pacs->biometrics = AA1->block[0].data[4];
            pacs->encryption = AA1->block[0].data[7];

            if(pacs->encryption == 0x17) {
                FURI_LOG_D(TAG, "3DES Encrypted");
                err = picopass_worker_decrypt(AA1->block[1].data, pacs->credential);
                if(err != ERR_NONE) {
                    FURI_LOG_E(TAG, "decrypt error %d", err);
                    break;
                }

                err = picopass_worker_decrypt(AA1->block[2].data, pacs->pin0);
                if(err != ERR_NONE) {
                    FURI_LOG_E(TAG, "decrypt error %d", err);
                    break;
                }

                err = picopass_worker_decrypt(AA1->block[3].data, pacs->pin1);
                if(err != ERR_NONE) {
                    FURI_LOG_E(TAG, "decrypt error %d", err);
                    break;
                }
            } else if(pacs->encryption == 0x14) {
                FURI_LOG_D(TAG, "No Encryption");
                memcpy(pacs->credential, AA1->block[1].data, RFAL_PICOPASS_MAX_BLOCK_LEN);
                memcpy(pacs->pin0, AA1->block[2].data, RFAL_PICOPASS_MAX_BLOCK_LEN);
                memcpy(pacs->pin1, AA1->block[3].data, RFAL_PICOPASS_MAX_BLOCK_LEN);
            } else if(pacs->encryption == 0x15) {
                FURI_LOG_D(TAG, "DES Encrypted");
            } else {
                FURI_LOG_D(TAG, "Unknown encryption");
                break;
            }

            picopass_worker_parse_wiegand(pacs->credential, &pacs->record);

            // Notify caller and exit
            if(picopass_worker->callback) {
                picopass_worker->callback(PicopassWorkerEventSuccess, picopass_worker->context);
            }
            break;
        }
        furi_delay_ms(100);
    }
}
