#include "api-hal-nfc.h"
#include <st25r3916.h>

static const uint32_t clocks_in_ms = 64 * 1000;

void api_hal_nfc_init() {
    ReturnCode ret = rfalNfcInitialize();
    if(ret == ERR_NONE) {
        api_hal_nfc_start_sleep();
        FURI_LOG_I("FuriHalNfc", "Init OK");
    } else {
        FURI_LOG_W("FuriHalNfc", "Initialization failed, RFAL returned: %d", ret);
    }
}

bool api_hal_nfc_is_busy() {
    return rfalNfcGetState() != RFAL_NFC_STATE_IDLE;
}

void api_hal_nfc_field_on() {
    api_hal_nfc_exit_sleep();
    st25r3916TxRxOn();
}

void api_hal_nfc_field_off() {
    st25r3916TxRxOff();
    api_hal_nfc_start_sleep();
}

void api_hal_nfc_start_sleep() {
    rfalLowPowerModeStart();
}

void api_hal_nfc_exit_sleep() {
    rfalLowPowerModeStop();
}

bool api_hal_nfc_detect(rfalNfcDevice **dev_list, uint8_t* dev_cnt, uint32_t timeout, bool deactivate) {
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
        FURI_LOG_D("HAL NFC", "Current state %d", state);
        if(state == RFAL_NFC_STATE_POLL_ACTIVATION) {
            start = DWT->CYCCNT;
            continue;
        }
        if(state == RFAL_NFC_STATE_POLL_SELECT) {
            rfalNfcSelect(0);
        }
        if(DWT->CYCCNT - start > timeout * clocks_in_ms) {
            rfalNfcDeactivate(true);
            FURI_LOG_D("HAL NFC", "Timeout");
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

bool api_hal_nfc_listen(uint8_t* uid, uint8_t uid_len, uint8_t* atqa, uint8_t sak, uint32_t timeout) {
    rfalNfcState state = rfalNfcGetState();
    if(state == RFAL_NFC_STATE_NOTINIT) {
        rfalNfcInitialize();
    } else if(state >= RFAL_NFC_STATE_ACTIVATED) {
        rfalNfcDeactivate(false);
    }

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
        FURI_LOG_D("HAL NFC", "Current state %d", state);
        if(DWT->CYCCNT - start > timeout * clocks_in_ms) {
            rfalNfcDeactivate(true);
            return false;
        }
        osThreadYield();
    }
    return true;
}

ReturnCode api_hal_nfc_data_exchange(uint8_t* tx_buff, uint16_t tx_len, uint8_t** rx_buff, uint16_t** rx_len, bool deactivate) {
    furi_assert(rx_buff);
    furi_assert(rx_len);

    ReturnCode ret;
    rfalNfcState state = RFAL_NFC_STATE_ACTIVATED;
    ret = rfalNfcDataExchangeStart(tx_buff, tx_len, rx_buff, rx_len, 0);
    if(ret != ERR_NONE) {
        return ret;
    }
    uint32_t start = DWT->CYCCNT;
    while(state != RFAL_NFC_STATE_DATAEXCHANGE_DONE) {
        rfalNfcWorker();
        state = rfalNfcGetState();
        ret = rfalNfcDataExchangeGetStatus();
        FURI_LOG_D("HAL NFC", "Nfc st: %d Data st: %d", state, ret);
        if(ret > ERR_SLEEP_REQ) {
            return ret;
        }
        if(ret == ERR_BUSY) {
            if(DWT->CYCCNT - start > 1000 * clocks_in_ms) {
                return ERR_TIMEOUT;
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
    return ERR_NONE;
}

void api_hal_nfc_deactivate() {
    rfalNfcDeactivate(false);
    rfalLowPowerModeStart();
}
