#include "furi_hal_nfc_i.h"
#include "furi_hal_nfc_tech_i.h"

static FuriHalNfcError furi_hal_nfc_iso14443b_common_init(FuriHalSpiBusHandle* handle) {
    // Common NFC-B settings, 106kbps

    // 1st stage zero = 60kHz, 3rd stage zero = 200 kHz
    st25r3916_write_reg(handle, ST25R3916_REG_RX_CONF1, ST25R3916_REG_RX_CONF1_h200);

    // Enable AGC
    // AGC Ratio 6
    // AGC algorithm with RESET (recommended for ISO14443-B)
    // AGC operation during complete receive period
    // Squelch ratio 6/3 (recommended for ISO14443-B)
    // Squelch automatic activation on TX end
    st25r3916_write_reg(
        handle,
        ST25R3916_REG_RX_CONF2,
        ST25R3916_REG_RX_CONF2_agc6_3 | ST25R3916_REG_RX_CONF2_agc_alg |
            ST25R3916_REG_RX_CONF2_agc_m | ST25R3916_REG_RX_CONF2_agc_en |
            ST25R3916_REG_RX_CONF2_pulz_61 | ST25R3916_REG_RX_CONF2_sqm_dyn);

    // HF operation, full gain on AM and PM channels
    st25r3916_write_reg(handle, ST25R3916_REG_RX_CONF3, 0x00);
    // No gain reduction on AM and PM channels
    st25r3916_write_reg(handle, ST25R3916_REG_RX_CONF4, 0x00);

    // Subcarrier end detector enabled
    // Subcarrier end detection level = 66%
    // BPSK start 33 pilot pulses
    // AM & PM summation before digitizing on
    st25r3916_write_reg(
        handle,
        ST25R3916_REG_CORR_CONF1,
        ST25R3916_REG_CORR_CONF1_corr_s0 | ST25R3916_REG_CORR_CONF1_corr_s1 |
            ST25R3916_REG_CORR_CONF1_corr_s3 | ST25R3916_REG_CORR_CONF1_corr_s4);
    // Sleep mode disable, 424kHz mode off
    st25r3916_write_reg(handle, ST25R3916_REG_CORR_CONF2, 0x00);

    return FuriHalNfcErrorNone;
}

static FuriHalNfcError furi_hal_nfc_iso14443b_poller_init(FuriHalSpiBusHandle* handle) {
    // Enable ISO14443B mode, AM modulation
    st25r3916_change_reg_bits(
        handle,
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_om_mask | ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_om_iso14443b | ST25R3916_REG_MODE_tr_am_am);

    // 10% ASK modulation
    st25r3916_change_reg_bits(
        handle,
        ST25R3916_REG_TX_DRIVER,
        ST25R3916_REG_TX_DRIVER_am_mod_mask,
        ST25R3916_REG_TX_DRIVER_am_mod_10percent);

    // Use regulator AM, resistive AM disabled
    st25r3916_clear_reg_bits(
        handle,
        ST25R3916_REG_AUX_MOD,
        ST25R3916_REG_AUX_MOD_dis_reg_am | ST25R3916_REG_AUX_MOD_res_am);

    // EGT = 0 etu
    // SOF = 10 etu LOW + 2 etu HIGH
    // EOF = 10 etu
    st25r3916_change_reg_bits(
        handle,
        ST25R3916_REG_ISO14443B_1,
        ST25R3916_REG_ISO14443B_1_egt_mask | ST25R3916_REG_ISO14443B_1_sof_mask |
            ST25R3916_REG_ISO14443B_1_eof,
        (0U << ST25R3916_REG_ISO14443B_1_egt_shift) | ST25R3916_REG_ISO14443B_1_sof_0_10etu |
            ST25R3916_REG_ISO14443B_1_sof_1_2etu | ST25R3916_REG_ISO14443B_1_eof_10etu);

    // TR1 = 80 / fs
    // B' mode off (no_sof & no_eof = 0)
    st25r3916_change_reg_bits(
        handle,
        ST25R3916_REG_ISO14443B_2,
        ST25R3916_REG_ISO14443B_2_tr1_mask | ST25R3916_REG_ISO14443B_2_no_sof |
            ST25R3916_REG_ISO14443B_2_no_eof,
        ST25R3916_REG_ISO14443B_2_tr1_80fs80fs);

    return furi_hal_nfc_iso14443b_common_init(handle);
}

static FuriHalNfcError furi_hal_nfc_iso14443b_poller_deinit(FuriHalSpiBusHandle* handle) {
    UNUSED(handle);
    return FuriHalNfcErrorNone;
}

const FuriHalNfcTechBase furi_hal_nfc_iso14443b = {
    .poller =
        {
            .compensation =
                {
                    .fdt = FURI_HAL_NFC_POLLER_FDT_COMP_FC,
                    .fwt = FURI_HAL_NFC_POLLER_FWT_COMP_FC,
                },
            .init = furi_hal_nfc_iso14443b_poller_init,
            .deinit = furi_hal_nfc_iso14443b_poller_deinit,
            .wait_event = furi_hal_nfc_wait_event_common,
            .tx = furi_hal_nfc_poller_tx_common,
            .rx = furi_hal_nfc_common_fifo_rx,
        },

    .listener = {},
};
