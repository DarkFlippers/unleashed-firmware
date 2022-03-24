#include "furi_hal_nfc.h"
#include <st25r3916.h>
#include <rfal_rf.h>
#include <furi.h>
#include <m-string.h>
#include <lib/nfc_protocols/nfca.h>

#define TAG "FuriHalNfc"

static const uint32_t clocks_in_ms = 64 * 1000;

osEventFlagsId_t event = NULL;
#define EVENT_FLAG_INTERRUPT (1UL << 0)
#define EVENT_FLAG_STATE_CHANGED (1UL << 1)
#define EVENT_FLAG_STOP (1UL << 2)
#define EVENT_FLAG_ALL (EVENT_FLAG_INTERRUPT | EVENT_FLAG_STATE_CHANGED | EVENT_FLAG_STOP)

void furi_hal_nfc_init() {
    ReturnCode ret = rfalNfcInitialize();
    if(ret == ERR_NONE) {
        furi_hal_nfc_start_sleep();
        event = osEventFlagsNew(NULL);
        FURI_LOG_I(TAG, "Init OK");
    } else {
        FURI_LOG_W(TAG, "Initialization failed, RFAL returned: %d", ret);
    }
}

bool furi_hal_nfc_is_busy() {
    return rfalNfcGetState() != RFAL_NFC_STATE_IDLE;
}

void furi_hal_nfc_field_on() {
    furi_hal_nfc_exit_sleep();
    st25r3916TxRxOn();
}

void furi_hal_nfc_field_off() {
    st25r3916TxRxOff();
    furi_hal_nfc_start_sleep();
}

void furi_hal_nfc_start_sleep() {
    rfalLowPowerModeStart();
}

void furi_hal_nfc_exit_sleep() {
    rfalLowPowerModeStop();
}

bool furi_hal_nfc_detect(
    rfalNfcDevice** dev_list,
    uint8_t* dev_cnt,
    uint32_t timeout,
    bool deactivate) {
    furi_assert(dev_list);
    furi_assert(dev_cnt);

    rfalLowPowerModeStop();
    rfalNfcState state = rfalNfcGetState();
    if(state == RFAL_NFC_STATE_NOTINIT) {
        rfalNfcInitialize();
    }
    rfalNfcDiscoverParam params;
    params.compMode = RFAL_COMPLIANCE_MODE_EMV;
    params.techs2Find = RFAL_NFC_POLL_TECH_A | RFAL_NFC_POLL_TECH_B | RFAL_NFC_POLL_TECH_F |
                        RFAL_NFC_POLL_TECH_V | RFAL_NFC_POLL_TECH_AP2P | RFAL_NFC_POLL_TECH_ST25TB;
    params.totalDuration = 1000;
    params.devLimit = 3;
    params.wakeupEnabled = false;
    params.wakeupConfigDefault = true;
    params.nfcfBR = RFAL_BR_212;
    params.ap2pBR = RFAL_BR_424;
    params.maxBR = RFAL_BR_KEEP;
    params.GBLen = RFAL_NFCDEP_GB_MAX_LEN;
    params.notifyCb = NULL;

    uint32_t start = DWT->CYCCNT;
    rfalNfcDiscover(&params);
    while(state != RFAL_NFC_STATE_ACTIVATED) {
        rfalNfcWorker();
        state = rfalNfcGetState();
        FURI_LOG_T(TAG, "Current state %d", state);
        if(state == RFAL_NFC_STATE_POLL_ACTIVATION) {
            start = DWT->CYCCNT;
            continue;
        }
        if(state == RFAL_NFC_STATE_POLL_SELECT) {
            rfalNfcSelect(0);
        }
        if(DWT->CYCCNT - start > timeout * clocks_in_ms) {
            rfalNfcDeactivate(true);
            FURI_LOG_T(TAG, "Timeout");
            return false;
        }
        osThreadYield();
    }
    rfalNfcGetDevicesFound(dev_list, dev_cnt);
    if(deactivate) {
        rfalNfcDeactivate(false);
        rfalLowPowerModeStart();
    }
    return true;
}

bool furi_hal_nfc_activate_nfca(uint32_t timeout, uint32_t* cuid) {
    rfalNfcDevice* dev_list;
    uint8_t dev_cnt = 0;
    rfalLowPowerModeStop();
    rfalNfcState state = rfalNfcGetState();
    if(state == RFAL_NFC_STATE_NOTINIT) {
        rfalNfcInitialize();
    }
    rfalNfcDiscoverParam params = {
        .compMode = RFAL_COMPLIANCE_MODE_NFC,
        .techs2Find = RFAL_NFC_POLL_TECH_A,
        .totalDuration = 1000,
        .devLimit = 3,
        .wakeupEnabled = false,
        .wakeupConfigDefault = true,
        .nfcfBR = RFAL_BR_212,
        .ap2pBR = RFAL_BR_424,
        .maxBR = RFAL_BR_KEEP,
        .GBLen = RFAL_NFCDEP_GB_MAX_LEN,
        .notifyCb = NULL,
    };
    uint32_t start = DWT->CYCCNT;
    rfalNfcDiscover(&params);
    while(state != RFAL_NFC_STATE_ACTIVATED) {
        rfalNfcWorker();
        state = rfalNfcGetState();
        FURI_LOG_T(TAG, "Current state %d", state);
        if(state == RFAL_NFC_STATE_POLL_ACTIVATION) {
            start = DWT->CYCCNT;
            continue;
        }
        if(state == RFAL_NFC_STATE_POLL_SELECT) {
            rfalNfcSelect(0);
        }
        if(DWT->CYCCNT - start > timeout * clocks_in_ms) {
            rfalNfcDeactivate(true);
            FURI_LOG_T(TAG, "Timeout");
            return false;
        }
        osThreadYield();
    }
    rfalNfcGetDevicesFound(&dev_list, &dev_cnt);
    // Take first device and set cuid
    if(cuid) {
        uint8_t* cuid_start = dev_list[0].nfcid;
        if(dev_list[0].nfcidLen == 7) {
            cuid_start = &dev_list[0].nfcid[3];
        }
        *cuid = (cuid_start[0] << 24) | (cuid_start[1] << 16) | (cuid_start[2] << 8) |
                (cuid_start[3]);
        FURI_LOG_T(TAG, "Activated tag with cuid: %lX", *cuid);
    }
    return true;
}

bool furi_hal_nfc_listen(
    uint8_t* uid,
    uint8_t uid_len,
    uint8_t* atqa,
    uint8_t sak,
    bool activate_after_sak,
    uint32_t timeout) {
    rfalNfcState state = rfalNfcGetState();
    if(state == RFAL_NFC_STATE_NOTINIT) {
        rfalNfcInitialize();
    } else if(state >= RFAL_NFC_STATE_ACTIVATED) {
        rfalNfcDeactivate(false);
    }
    rfalLowPowerModeStop();
    rfalNfcDiscoverParam params = {
        .compMode = RFAL_COMPLIANCE_MODE_NFC,
        .techs2Find = RFAL_NFC_LISTEN_TECH_A,
        .totalDuration = 1000,
        .devLimit = 1,
        .wakeupEnabled = false,
        .wakeupConfigDefault = true,
        .nfcfBR = RFAL_BR_212,
        .ap2pBR = RFAL_BR_424,
        .maxBR = RFAL_BR_KEEP,
        .GBLen = RFAL_NFCDEP_GB_MAX_LEN,
        .notifyCb = NULL,
        .activate_after_sak = activate_after_sak,
    };
    params.lmConfigPA.nfcidLen = uid_len;
    memcpy(params.lmConfigPA.nfcid, uid, uid_len);
    params.lmConfigPA.SENS_RES[0] = atqa[0];
    params.lmConfigPA.SENS_RES[1] = atqa[1];
    params.lmConfigPA.SEL_RES = sak;
    rfalNfcDiscover(&params);

    uint32_t start = DWT->CYCCNT;
    while(state != RFAL_NFC_STATE_ACTIVATED) {
        rfalNfcWorker();
        state = rfalNfcGetState();
        if(DWT->CYCCNT - start > timeout * clocks_in_ms) {
            rfalNfcDeactivate(true);
            return false;
        }
        osThreadYield();
    }
    return true;
}

void rfal_interrupt_callback_handler() {
    osEventFlagsSet(event, EVENT_FLAG_INTERRUPT);
}

void rfal_state_changed_callback(void* context) {
    osEventFlagsSet(event, EVENT_FLAG_STATE_CHANGED);
}

void furi_hal_nfc_stop() {
    if(event) {
        osEventFlagsSet(event, EVENT_FLAG_STOP);
    }
}

bool furi_hal_nfc_emulate_nfca(
    uint8_t* uid,
    uint8_t uid_len,
    uint8_t* atqa,
    uint8_t sak,
    FuriHalNfcEmulateCallback callback,
    void* context,
    uint32_t timeout) {
    rfalSetUpperLayerCallback(rfal_interrupt_callback_handler);
    rfal_set_state_changed_callback(rfal_state_changed_callback);

    rfalLmConfPA config;
    config.nfcidLen = uid_len;
    memcpy(config.nfcid, uid, uid_len);
    memcpy(config.SENS_RES, atqa, RFAL_LM_SENS_RES_LEN);
    config.SEL_RES = sak;
    uint8_t buff_rx[256];
    uint16_t buff_rx_size = 256;
    uint16_t buff_rx_len = 0;
    uint8_t buff_tx[256];
    uint16_t buff_tx_len = 0;
    uint32_t data_type = FURI_HAL_NFC_TXRX_DEFAULT;

    rfalLowPowerModeStop();
    if(rfalListenStart(
           RFAL_LM_MASK_NFCA,
           &config,
           NULL,
           NULL,
           buff_rx,
           rfalConvBytesToBits(buff_rx_size),
           &buff_rx_len)) {
        rfalListenStop();
        FURI_LOG_E(TAG, "Failed to start listen mode");
        return false;
    }
    while(true) {
        buff_rx_len = 0;
        buff_tx_len = 0;
        uint32_t flag = osEventFlagsWait(event, EVENT_FLAG_ALL, osFlagsWaitAny, timeout);
        if(flag == osErrorTimeout || flag == EVENT_FLAG_STOP) {
            break;
        }
        bool data_received = false;
        buff_rx_len = 0;
        rfalWorker();
        rfalLmState state = rfalListenGetState(&data_received, NULL);
        if(data_received) {
            rfalTransceiveBlockingRx();
            if(nfca_emulation_handler(buff_rx, buff_rx_len, buff_tx, &buff_tx_len)) {
                if(rfalListenSleepStart(
                       RFAL_LM_STATE_SLEEP_A,
                       buff_rx,
                       rfalConvBytesToBits(buff_rx_size),
                       &buff_rx_len)) {
                    FURI_LOG_E(TAG, "Failed to enter sleep mode");
                    break;
                } else {
                    continue;
                }
            }
            if(buff_tx_len) {
                ReturnCode ret = rfalTransceiveBitsBlockingTx(
                    buff_tx,
                    buff_tx_len,
                    buff_rx,
                    sizeof(buff_rx),
                    &buff_rx_len,
                    data_type,
                    RFAL_FWT_NONE);
                if(ret) {
                    FURI_LOG_E(TAG, "Tranceive failed with status %d", ret);
                    break;
                }
                continue;
            }
            if((state == RFAL_LM_STATE_ACTIVE_A || state == RFAL_LM_STATE_ACTIVE_Ax)) {
                if(callback) {
                    callback(buff_rx, buff_rx_len, buff_tx, &buff_tx_len, &data_type, context);
                }
                if(!rfalIsExtFieldOn()) {
                    break;
                }
                if(buff_tx_len) {
                    ReturnCode ret = rfalTransceiveBitsBlockingTx(
                        buff_tx,
                        buff_tx_len,
                        buff_rx,
                        sizeof(buff_rx),
                        &buff_rx_len,
                        data_type,
                        RFAL_FWT_NONE);
                    if(ret) {
                        FURI_LOG_E(TAG, "Tranceive failed with status %d", ret);
                        continue;
                    }
                } else {
                    break;
                }
            }
        }
    }
    rfalListenStop();
    return true;
}

bool furi_hal_nfc_get_first_frame(uint8_t** rx_buff, uint16_t** rx_len) {
    ReturnCode ret =
        rfalNfcDataExchangeStart(NULL, 0, rx_buff, rx_len, 0, RFAL_TXRX_FLAGS_DEFAULT);
    return ret == ERR_NONE;
}

ReturnCode furi_hal_nfc_data_exchange(
    uint8_t* tx_buff,
    uint16_t tx_len,
    uint8_t** rx_buff,
    uint16_t** rx_len,
    bool deactivate) {
    furi_assert(rx_buff);
    furi_assert(rx_len);

    ReturnCode ret;
    rfalNfcState state = RFAL_NFC_STATE_ACTIVATED;
    ret = rfalNfcDataExchangeStart(tx_buff, tx_len, rx_buff, rx_len, 0, RFAL_TXRX_FLAGS_DEFAULT);
    if(ret != ERR_NONE) {
        return ret;
    }
    uint32_t start = DWT->CYCCNT;
    while(state != RFAL_NFC_STATE_DATAEXCHANGE_DONE) {
        rfalNfcWorker();
        state = rfalNfcGetState();
        ret = rfalNfcDataExchangeGetStatus();
        if(ret == ERR_BUSY) {
            if(DWT->CYCCNT - start > 1000 * clocks_in_ms) {
                ret = ERR_TIMEOUT;
                break;
            }
            continue;
        } else {
            start = DWT->CYCCNT;
        }
        taskYIELD();
    }
    if(deactivate) {
        rfalNfcDeactivate(false);
        rfalLowPowerModeStart();
    }
    return ret;
}

static uint16_t furi_hal_nfc_data_and_parity_to_bitstream(
    uint8_t* data,
    uint16_t len,
    uint8_t* parity,
    uint8_t* out) {
    furi_assert(data);
    furi_assert(out);

    uint8_t next_par_bit = 0;
    uint16_t curr_bit_pos = 0;
    for(uint16_t i = 0; i < len; i++) {
        next_par_bit = FURI_BIT(parity[i / 8], 7 - (i % 8));
        if(curr_bit_pos % 8 == 0) {
            out[curr_bit_pos / 8] = data[i];
            curr_bit_pos += 8;
            out[curr_bit_pos / 8] = next_par_bit;
            curr_bit_pos++;
        } else {
            out[curr_bit_pos / 8] |= data[i] << curr_bit_pos % 8;
            out[curr_bit_pos / 8 + 1] = data[i] >> (8 - curr_bit_pos % 8);
            out[curr_bit_pos / 8 + 1] |= next_par_bit << curr_bit_pos % 8;
            curr_bit_pos += 9;
        }
    }
    return curr_bit_pos;
}

uint16_t furi_hal_nfc_bitstream_to_data_and_parity(
    uint8_t* in_buff,
    uint16_t in_buff_bits,
    uint8_t* out_data,
    uint8_t* out_parity) {
    if(in_buff_bits % 9 != 0) {
        return 0;
    }

    uint8_t curr_byte = 0;
    uint16_t bit_processed = 0;
    memset(out_parity, 0, in_buff_bits / 9);
    while(bit_processed < in_buff_bits) {
        out_data[curr_byte] = in_buff[bit_processed / 8] >> bit_processed % 8;
        out_data[curr_byte] |= in_buff[bit_processed / 8 + 1] << (8 - bit_processed % 8);
        out_parity[curr_byte / 8] |= FURI_BIT(in_buff[bit_processed / 8 + 1], bit_processed % 8)
                                     << (7 - curr_byte % 8);
        bit_processed += 9;
        curr_byte++;
    }
    return curr_byte;
}

bool furi_hal_nfc_tx_rx(FuriHalNfcTxRxContext* tx_rx_ctx) {
    furi_assert(tx_rx_ctx);

    ReturnCode ret;
    rfalNfcState state = RFAL_NFC_STATE_ACTIVATED;
    uint8_t temp_tx_buff[FURI_HAL_NFC_DATA_BUFF_SIZE] = {};
    uint16_t temp_tx_bits = 0;
    uint8_t* temp_rx_buff = NULL;
    uint16_t* temp_rx_bits = NULL;

    // Prepare data for FIFO if necessary
    if(tx_rx_ctx->tx_rx_type == FURI_HAL_NFC_TXRX_RAW) {
        temp_tx_bits = furi_hal_nfc_data_and_parity_to_bitstream(
            tx_rx_ctx->tx_data, tx_rx_ctx->tx_bits / 8, tx_rx_ctx->tx_parity, temp_tx_buff);
        ret = rfalNfcDataExchangeCustomStart(
            temp_tx_buff,
            temp_tx_bits,
            &temp_rx_buff,
            &temp_rx_bits,
            RFAL_FWT_NONE,
            tx_rx_ctx->tx_rx_type);
    } else {
        ret = rfalNfcDataExchangeCustomStart(
            tx_rx_ctx->tx_data,
            tx_rx_ctx->tx_bits,
            &temp_rx_buff,
            &temp_rx_bits,
            RFAL_FWT_NONE,
            tx_rx_ctx->tx_rx_type);
    }
    if(ret != ERR_NONE) {
        return false;
    }
    uint32_t start = DWT->CYCCNT;
    while(state != RFAL_NFC_STATE_DATAEXCHANGE_DONE) {
        rfalNfcWorker();
        state = rfalNfcGetState();
        ret = rfalNfcDataExchangeGetStatus();
        if(ret == ERR_BUSY) {
            if(DWT->CYCCNT - start > 4 * clocks_in_ms) {
                return false;
            }
            continue;
        } else {
            start = DWT->CYCCNT;
        }
        taskYIELD();
    }

    if(tx_rx_ctx->tx_rx_type == FURI_HAL_NFC_TXRX_RAW) {
        tx_rx_ctx->rx_bits =
            8 * furi_hal_nfc_bitstream_to_data_and_parity(
                    temp_rx_buff, *temp_rx_bits, tx_rx_ctx->rx_data, tx_rx_ctx->rx_parity);
    } else {
        memcpy(tx_rx_ctx->rx_data, temp_rx_buff, *temp_rx_bits / 8);
    }

    return true;
}

void furi_hal_nfc_deactivate() {
    rfalNfcDeactivate(false);
    rfalLowPowerModeStart();
}
