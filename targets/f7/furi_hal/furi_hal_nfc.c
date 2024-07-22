#include "furi_hal_nfc_i.h"
#include "furi_hal_nfc_tech_i.h"

#include <lib/drivers/st25r3916.h>

#include <furi.h>
#include <furi_hal_spi.h>

#define TAG "FuriHalNfc"

const FuriHalNfcTechBase* furi_hal_nfc_tech[FuriHalNfcTechNum] = {
    [FuriHalNfcTechIso14443a] = &furi_hal_nfc_iso14443a,
    [FuriHalNfcTechIso14443b] = &furi_hal_nfc_iso14443b,
    [FuriHalNfcTechIso15693] = &furi_hal_nfc_iso15693,
    [FuriHalNfcTechFelica] = &furi_hal_nfc_felica,
    // Add new technologies here
};

FuriHalNfc furi_hal_nfc;

static FuriHalNfcError furi_hal_nfc_turn_on_osc(FuriHalSpiBusHandle* handle) {
    FuriHalNfcError error = FuriHalNfcErrorNone;
    furi_hal_nfc_event_start();

    if(!st25r3916_check_reg(
           handle,
           ST25R3916_REG_OP_CONTROL,
           ST25R3916_REG_OP_CONTROL_en,
           ST25R3916_REG_OP_CONTROL_en)) {
        st25r3916_mask_irq(handle, ~ST25R3916_IRQ_MASK_OSC);
        st25r3916_set_reg_bits(handle, ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en);
        furi_hal_nfc_event_wait_for_specific_irq(handle, ST25R3916_IRQ_MASK_OSC, 10);
    }
    // Disable IRQs
    st25r3916_mask_irq(handle, ST25R3916_IRQ_MASK_ALL);

    bool osc_on = st25r3916_check_reg(
        handle,
        ST25R3916_REG_AUX_DISPLAY,
        ST25R3916_REG_AUX_DISPLAY_osc_ok,
        ST25R3916_REG_AUX_DISPLAY_osc_ok);
    if(!osc_on) {
        error = FuriHalNfcErrorOscillator;
    }

    return error;
}

FuriHalNfcError furi_hal_nfc_is_hal_ready(void) {
    FuriHalNfcError error = FuriHalNfcErrorNone;

    do {
        error = furi_hal_nfc_acquire();
        if(error != FuriHalNfcErrorNone) break;

        FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;
        uint8_t chip_id = 0;
        st25r3916_read_reg(handle, ST25R3916_REG_IC_IDENTITY, &chip_id);
        if((chip_id & ST25R3916_REG_IC_IDENTITY_ic_type_mask) !=
           ST25R3916_REG_IC_IDENTITY_ic_type_st25r3916) {
            FURI_LOG_E(TAG, "Wrong chip id");
            error = FuriHalNfcErrorCommunication;
        }

        furi_hal_nfc_release();
    } while(false);

    return error;
}

FuriHalNfcError furi_hal_nfc_init(void) {
    furi_check(furi_hal_nfc.mutex == NULL);

    furi_hal_nfc.mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    FuriHalNfcError error = FuriHalNfcErrorNone;

    furi_hal_nfc_event_init();
    furi_hal_nfc_event_start();

    do {
        error = furi_hal_nfc_acquire();
        if(error != FuriHalNfcErrorNone) {
            furi_hal_nfc_low_power_mode_start();
        }

        FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;
        // Set default state
        st25r3916_direct_cmd(handle, ST25R3916_CMD_SET_DEFAULT);
        // Increase IO driver strength of MISO and IRQ
        st25r3916_write_reg(handle, ST25R3916_REG_IO_CONF2, ST25R3916_REG_IO_CONF2_io_drv_lvl);
        // Check chip ID
        uint8_t chip_id = 0;
        st25r3916_read_reg(handle, ST25R3916_REG_IC_IDENTITY, &chip_id);
        if((chip_id & ST25R3916_REG_IC_IDENTITY_ic_type_mask) !=
           ST25R3916_REG_IC_IDENTITY_ic_type_st25r3916) {
            FURI_LOG_E(TAG, "Wrong chip id");
            error = FuriHalNfcErrorCommunication;
            furi_hal_nfc_low_power_mode_start();
            furi_hal_nfc_release();
            break;
        }
        // Clear interrupts
        st25r3916_get_irq(handle);
        // Mask all interrupts
        st25r3916_mask_irq(handle, ST25R3916_IRQ_MASK_ALL);
        // Enable interrupts
        furi_hal_nfc_init_gpio_isr();
        // Disable internal overheat protection
        st25r3916_change_test_reg_bits(handle, 0x04, 0x10, 0x10);

        error = furi_hal_nfc_turn_on_osc(handle);
        if(error != FuriHalNfcErrorNone) {
            furi_hal_nfc_low_power_mode_start();
            furi_hal_nfc_release();
            break;
        }

        // Measure voltage
        // Set measure power supply voltage source
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_REGULATOR_CONTROL,
            ST25R3916_REG_REGULATOR_CONTROL_mpsv_mask,
            ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd);
        // Enable timer and interrupt register
        st25r3916_mask_irq(handle, ~ST25R3916_IRQ_MASK_DCT);
        st25r3916_direct_cmd(handle, ST25R3916_CMD_MEASURE_VDD);
        furi_hal_nfc_event_wait_for_specific_irq(handle, ST25R3916_IRQ_MASK_DCT, 100);
        st25r3916_mask_irq(handle, ST25R3916_IRQ_MASK_ALL);
        uint8_t ad_res = 0;
        st25r3916_read_reg(handle, ST25R3916_REG_AD_RESULT, &ad_res);
        uint16_t mV = ((uint16_t)ad_res) * 23U;
        mV += (((((uint16_t)ad_res) * 4U) + 5U) / 10U);

        if(mV < 3600) {
            st25r3916_change_reg_bits(
                handle,
                ST25R3916_REG_IO_CONF2,
                ST25R3916_REG_IO_CONF2_sup3V,
                ST25R3916_REG_IO_CONF2_sup3V_3V);
        } else {
            st25r3916_change_reg_bits(
                handle,
                ST25R3916_REG_IO_CONF2,
                ST25R3916_REG_IO_CONF2_sup3V,
                ST25R3916_REG_IO_CONF2_sup3V_5V);
        }

        // Disable MCU CLK
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_IO_CONF1,
            ST25R3916_REG_IO_CONF1_out_cl_mask | ST25R3916_REG_IO_CONF1_lf_clk_off,
            0x07);
        // Disable MISO pull-down
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_IO_CONF2,
            ST25R3916_REG_IO_CONF2_miso_pd1 | ST25R3916_REG_IO_CONF2_miso_pd2,
            0x00);
        // Set tx driver resistance to 1 Om
        st25r3916_change_reg_bits(
            handle, ST25R3916_REG_TX_DRIVER, ST25R3916_REG_TX_DRIVER_d_res_mask, 0x00);
        // Use minimum non-overlap
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_RES_AM_MOD,
            ST25R3916_REG_RES_AM_MOD_fa3_f,
            ST25R3916_REG_RES_AM_MOD_fa3_f);

        // Set activation threashold
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_FIELD_THRESHOLD_ACTV,
            ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_mask,
            ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_105mV);
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_FIELD_THRESHOLD_ACTV,
            ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_mask,
            ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_105mV);
        // Set deactivation threashold
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_FIELD_THRESHOLD_DEACTV,
            ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_mask,
            ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_75mV);
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_FIELD_THRESHOLD_DEACTV,
            ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_mask,
            ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_75mV);
        // Enable external load modulation
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_AUX_MOD,
            ST25R3916_REG_AUX_MOD_lm_ext,
            ST25R3916_REG_AUX_MOD_lm_ext);
        // Enable internal load modulation
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_AUX_MOD,
            ST25R3916_REG_AUX_MOD_lm_dri,
            ST25R3916_REG_AUX_MOD_lm_dri);
        // Adjust FDT
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_PASSIVE_TARGET,
            ST25R3916_REG_PASSIVE_TARGET_fdel_mask,
            (5U << ST25R3916_REG_PASSIVE_TARGET_fdel_shift));
        // Reduce RFO resistance in Modulated state
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_PT_MOD,
            ST25R3916_REG_PT_MOD_ptm_res_mask | ST25R3916_REG_PT_MOD_pt_res_mask,
            0x0f);
        // Enable RX start on first 4 bits
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_EMD_SUP_CONF,
            ST25R3916_REG_EMD_SUP_CONF_rx_start_emv,
            ST25R3916_REG_EMD_SUP_CONF_rx_start_emv_on);
        // Set antena tunning
        st25r3916_change_reg_bits(handle, ST25R3916_REG_ANT_TUNE_A, 0xff, 0x82);
        st25r3916_change_reg_bits(handle, ST25R3916_REG_ANT_TUNE_B, 0xff, 0x82);
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_OP_CONTROL,
            ST25R3916_REG_OP_CONTROL_en_fd_mask,
            ST25R3916_REG_OP_CONTROL_en_fd_auto_efd);

        // Perform calibration
        if(st25r3916_check_reg(
               handle,
               ST25R3916_REG_REGULATOR_CONTROL,
               ST25R3916_REG_REGULATOR_CONTROL_reg_s,
               0x00)) {
            FURI_LOG_I(TAG, "Adjusting regulators");
            // Reset logic
            st25r3916_set_reg_bits(
                handle, ST25R3916_REG_REGULATOR_CONTROL, ST25R3916_REG_REGULATOR_CONTROL_reg_s);
            st25r3916_clear_reg_bits(
                handle, ST25R3916_REG_REGULATOR_CONTROL, ST25R3916_REG_REGULATOR_CONTROL_reg_s);
            st25r3916_direct_cmd(handle, ST25R3916_CMD_ADJUST_REGULATORS);
            furi_delay_ms(6);
        }

        furi_hal_nfc_low_power_mode_start();
        furi_hal_nfc_release();
    } while(false);

    return error;
}

static bool furi_hal_nfc_is_mine(void) {
    return furi_mutex_get_owner(furi_hal_nfc.mutex) == furi_thread_get_current_id();
}

FuriHalNfcError furi_hal_nfc_acquire(void) {
    furi_check(furi_hal_nfc.mutex);

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_nfc);

    FuriHalNfcError error = FuriHalNfcErrorNone;
    if(furi_mutex_acquire(furi_hal_nfc.mutex, 100) != FuriStatusOk) {
        furi_hal_spi_release(&furi_hal_spi_bus_handle_nfc);
        error = FuriHalNfcErrorBusy;
    }

    return error;
}

FuriHalNfcError furi_hal_nfc_release(void) {
    furi_check(furi_hal_nfc.mutex);
    furi_check(furi_hal_nfc_is_mine());
    furi_check(furi_mutex_release(furi_hal_nfc.mutex) == FuriStatusOk);

    furi_hal_spi_release(&furi_hal_spi_bus_handle_nfc);

    return FuriHalNfcErrorNone;
}

FuriHalNfcError furi_hal_nfc_low_power_mode_start(void) {
    FuriHalNfcError error = FuriHalNfcErrorNone;
    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    st25r3916_direct_cmd(handle, ST25R3916_CMD_STOP);
    st25r3916_clear_reg_bits(
        handle,
        ST25R3916_REG_OP_CONTROL,
        (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_rx_en |
         ST25R3916_REG_OP_CONTROL_wu | ST25R3916_REG_OP_CONTROL_tx_en |
         ST25R3916_REG_OP_CONTROL_en_fd_mask));
    furi_hal_nfc_deinit_gpio_isr();
    furi_hal_nfc_timers_deinit();
    furi_hal_nfc_event_stop();

    return error;
}

FuriHalNfcError furi_hal_nfc_low_power_mode_stop(void) {
    FuriHalNfcError error = FuriHalNfcErrorNone;
    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    do {
        furi_hal_nfc_init_gpio_isr();
        furi_hal_nfc_timers_init();
        error = furi_hal_nfc_turn_on_osc(handle);
        if(error != FuriHalNfcErrorNone) break;
        st25r3916_change_reg_bits(
            handle,
            ST25R3916_REG_OP_CONTROL,
            ST25R3916_REG_OP_CONTROL_en_fd_mask,
            ST25R3916_REG_OP_CONTROL_en_fd_auto_efd);

    } while(false);

    return error;
}

static FuriHalNfcError furi_hal_nfc_poller_init_common(FuriHalSpiBusHandle* handle) {
    // Disable wake up
    st25r3916_clear_reg_bits(handle, ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);
    // Enable correlator
    st25r3916_change_reg_bits(
        handle,
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_correlator);

    st25r3916_change_reg_bits(handle, ST25R3916_REG_ANT_TUNE_A, 0xff, 0x82);
    st25r3916_change_reg_bits(handle, ST25R3916_REG_ANT_TUNE_B, 0xFF, 0x82);

    st25r3916_write_reg(handle, ST25R3916_REG_OVERSHOOT_CONF1, 0x00);
    st25r3916_write_reg(handle, ST25R3916_REG_OVERSHOOT_CONF2, 0x00);
    st25r3916_write_reg(handle, ST25R3916_REG_UNDERSHOOT_CONF1, 0x00);
    st25r3916_write_reg(handle, ST25R3916_REG_UNDERSHOOT_CONF2, 0x00);

    return FuriHalNfcErrorNone;
}

static FuriHalNfcError furi_hal_nfc_listener_init_common(FuriHalSpiBusHandle* handle) {
    UNUSED(handle);
    // No common listener configuration
    return FuriHalNfcErrorNone;
}

FuriHalNfcError furi_hal_nfc_set_mode(FuriHalNfcMode mode, FuriHalNfcTech tech) {
    furi_check(mode < FuriHalNfcModeNum);
    furi_check(tech < FuriHalNfcTechNum);

    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    FuriHalNfcError error = FuriHalNfcErrorNone;

    if(mode == FuriHalNfcModePoller) {
        do {
            error = furi_hal_nfc_poller_init_common(handle);
            if(error != FuriHalNfcErrorNone) break;
            error = furi_hal_nfc_tech[tech]->poller.init(handle);
        } while(false);

    } else if(mode == FuriHalNfcModeListener) {
        do {
            error = furi_hal_nfc_listener_init_common(handle);
            if(error != FuriHalNfcErrorNone) break;
            error = furi_hal_nfc_tech[tech]->listener.init(handle);
        } while(false);
    }

    furi_hal_nfc.mode = mode;
    furi_hal_nfc.tech = tech;
    return error;
}

FuriHalNfcError furi_hal_nfc_reset_mode(void) {
    FuriHalNfcError error = FuriHalNfcErrorNone;
    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    st25r3916_direct_cmd(handle, ST25R3916_CMD_STOP);

    const FuriHalNfcMode mode = furi_hal_nfc.mode;
    const FuriHalNfcTech tech = furi_hal_nfc.tech;
    if(mode == FuriHalNfcModePoller) {
        error = furi_hal_nfc_tech[tech]->poller.deinit(handle);
    } else if(mode == FuriHalNfcModeListener) {
        error = furi_hal_nfc_tech[tech]->listener.deinit(handle);
    }
    // Set default value in mode register
    st25r3916_write_reg(handle, ST25R3916_REG_MODE, ST25R3916_REG_MODE_om0);
    st25r3916_write_reg(handle, ST25R3916_REG_STREAM_MODE, 0);
    st25r3916_clear_reg_bits(handle, ST25R3916_REG_AUX, ST25R3916_REG_AUX_no_crc_rx);
    st25r3916_clear_reg_bits(
        handle,
        ST25R3916_REG_BIT_RATE,
        ST25R3916_REG_BIT_RATE_txrate_mask | ST25R3916_REG_BIT_RATE_rxrate_mask);

    // Write default values
    st25r3916_write_reg(handle, ST25R3916_REG_RX_CONF1, 0);
    st25r3916_write_reg(
        handle,
        ST25R3916_REG_RX_CONF2,
        ST25R3916_REG_RX_CONF2_sqm_dyn | ST25R3916_REG_RX_CONF2_agc_en |
            ST25R3916_REG_RX_CONF2_agc_m);

    st25r3916_write_reg(
        handle,
        ST25R3916_REG_CORR_CONF1,
        ST25R3916_REG_CORR_CONF1_corr_s7 | ST25R3916_REG_CORR_CONF1_corr_s4 |
            ST25R3916_REG_CORR_CONF1_corr_s1 | ST25R3916_REG_CORR_CONF1_corr_s0);
    st25r3916_write_reg(handle, ST25R3916_REG_CORR_CONF2, 0);

    return error;
}

FuriHalNfcError furi_hal_nfc_field_detect_start(void) {
    FuriHalNfcError error = FuriHalNfcErrorNone;
    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    st25r3916_write_reg(
        handle,
        ST25R3916_REG_OP_CONTROL,
        ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_en_fd_mask);
    st25r3916_write_reg(
        handle, ST25R3916_REG_MODE, ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om0);

    return error;
}

FuriHalNfcError furi_hal_nfc_field_detect_stop(void) {
    FuriHalNfcError error = FuriHalNfcErrorNone;
    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    st25r3916_clear_reg_bits(
        handle,
        ST25R3916_REG_OP_CONTROL,
        (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_en_fd_mask));

    return error;
}

bool furi_hal_nfc_field_is_present(void) {
    bool is_present = false;
    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    if(st25r3916_check_reg(
           handle,
           ST25R3916_REG_AUX_DISPLAY,
           ST25R3916_REG_AUX_DISPLAY_efd_o,
           ST25R3916_REG_AUX_DISPLAY_efd_o)) {
        is_present = true;
    }

    return is_present;
}

FuriHalNfcError furi_hal_nfc_poller_field_on(void) {
    FuriHalNfcError error = FuriHalNfcErrorNone;
    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    if(!st25r3916_check_reg(
           handle,
           ST25R3916_REG_OP_CONTROL,
           ST25R3916_REG_OP_CONTROL_tx_en,
           ST25R3916_REG_OP_CONTROL_tx_en)) {
        // Set min guard time
        st25r3916_write_reg(handle, ST25R3916_REG_FIELD_ON_GT, 0);
        // Enable tx rx
        st25r3916_set_reg_bits(
            handle,
            ST25R3916_REG_OP_CONTROL,
            (ST25R3916_REG_OP_CONTROL_rx_en | ST25R3916_REG_OP_CONTROL_tx_en));
    }

    return error;
}

FuriHalNfcError furi_hal_nfc_poller_tx_common(
    FuriHalSpiBusHandle* handle,
    const uint8_t* tx_data,
    size_t tx_bits) {
    furi_check(tx_data);

    FuriHalNfcError err = FuriHalNfcErrorNone;

    // Prepare tx
    st25r3916_direct_cmd(handle, ST25R3916_CMD_CLEAR_FIFO);
    st25r3916_clear_reg_bits(
        handle, ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_emv);
    st25r3916_change_reg_bits(
        handle,
        ST25R3916_REG_ISO14443A_NFC,
        (ST25R3916_REG_ISO14443A_NFC_no_tx_par | ST25R3916_REG_ISO14443A_NFC_no_rx_par),
        (ST25R3916_REG_ISO14443A_NFC_no_tx_par_off | ST25R3916_REG_ISO14443A_NFC_no_rx_par_off));
    uint32_t interrupts =
        (ST25R3916_IRQ_MASK_FWL | ST25R3916_IRQ_MASK_TXE | ST25R3916_IRQ_MASK_RXS |
         ST25R3916_IRQ_MASK_RXE | ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_CRC |
         ST25R3916_IRQ_MASK_ERR1 | ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_NRE);
    // Clear interrupts
    st25r3916_get_irq(handle);
    // Enable interrupts
    st25r3916_mask_irq(handle, ~interrupts);

    st25r3916_write_fifo(handle, tx_data, tx_bits);
    st25r3916_direct_cmd(handle, ST25R3916_CMD_TRANSMIT_WITHOUT_CRC);

    return err;
}

FuriHalNfcError furi_hal_nfc_common_fifo_tx(
    FuriHalSpiBusHandle* handle,
    const uint8_t* tx_data,
    size_t tx_bits) {
    FuriHalNfcError err = FuriHalNfcErrorNone;

    st25r3916_direct_cmd(handle, ST25R3916_CMD_CLEAR_FIFO);
    st25r3916_write_fifo(handle, tx_data, tx_bits);
    st25r3916_direct_cmd(handle, ST25R3916_CMD_TRANSMIT_WITHOUT_CRC);

    return err;
}

FuriHalNfcError furi_hal_nfc_poller_tx(const uint8_t* tx_data, size_t tx_bits) {
    furi_check(furi_hal_nfc.mode == FuriHalNfcModePoller);
    furi_check(furi_hal_nfc.tech < FuriHalNfcTechNum);
    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    return furi_hal_nfc_tech[furi_hal_nfc.tech]->poller.tx(handle, tx_data, tx_bits);
}

FuriHalNfcError furi_hal_nfc_poller_rx(uint8_t* rx_data, size_t rx_data_size, size_t* rx_bits) {
    furi_check(furi_hal_nfc.mode == FuriHalNfcModePoller);
    furi_check(furi_hal_nfc.tech < FuriHalNfcTechNum);
    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    return furi_hal_nfc_tech[furi_hal_nfc.tech]->poller.rx(handle, rx_data, rx_data_size, rx_bits);
}

FuriHalNfcEvent furi_hal_nfc_poller_wait_event(uint32_t timeout_ms) {
    furi_check(furi_hal_nfc.mode == FuriHalNfcModePoller);
    furi_check(furi_hal_nfc.tech < FuriHalNfcTechNum);

    return furi_hal_nfc_tech[furi_hal_nfc.tech]->poller.wait_event(timeout_ms);
}

FuriHalNfcEvent furi_hal_nfc_listener_wait_event(uint32_t timeout_ms) {
    furi_check(furi_hal_nfc.mode == FuriHalNfcModeListener);
    furi_check(furi_hal_nfc.tech < FuriHalNfcTechNum);

    return furi_hal_nfc_tech[furi_hal_nfc.tech]->listener.wait_event(timeout_ms);
}

FuriHalNfcError furi_hal_nfc_listener_tx(const uint8_t* tx_data, size_t tx_bits) {
    furi_check(tx_data);

    furi_check(furi_hal_nfc.mode == FuriHalNfcModeListener);
    furi_check(furi_hal_nfc.tech < FuriHalNfcTechNum);

    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;
    return furi_hal_nfc_tech[furi_hal_nfc.tech]->listener.tx(handle, tx_data, tx_bits);
}

FuriHalNfcError furi_hal_nfc_common_fifo_rx(
    FuriHalSpiBusHandle* handle,
    uint8_t* rx_data,
    size_t rx_data_size,
    size_t* rx_bits) {
    FuriHalNfcError error = FuriHalNfcErrorNone;

    if(!st25r3916_read_fifo(handle, rx_data, rx_data_size, rx_bits)) {
        error = FuriHalNfcErrorBufferOverflow;
    }

    return error;
}

FuriHalNfcError furi_hal_nfc_listener_rx(uint8_t* rx_data, size_t rx_data_size, size_t* rx_bits) {
    furi_check(rx_data);
    furi_check(rx_bits);

    furi_check(furi_hal_nfc.mode == FuriHalNfcModeListener);
    furi_check(furi_hal_nfc.tech < FuriHalNfcTechNum);

    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;
    return furi_hal_nfc_tech[furi_hal_nfc.tech]->listener.rx(
        handle, rx_data, rx_data_size, rx_bits);
}

FuriHalNfcError furi_hal_nfc_trx_reset(void) {
    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    st25r3916_direct_cmd(handle, ST25R3916_CMD_STOP);

    return FuriHalNfcErrorNone;
}

FuriHalNfcError furi_hal_nfc_listener_sleep(void) {
    furi_check(furi_hal_nfc.mode == FuriHalNfcModeListener);
    furi_check(furi_hal_nfc.tech < FuriHalNfcTechNum);

    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    return furi_hal_nfc_tech[furi_hal_nfc.tech]->listener.sleep(handle);
}

FuriHalNfcError furi_hal_nfc_listener_idle(void) {
    furi_check(furi_hal_nfc.mode == FuriHalNfcModeListener);
    furi_check(furi_hal_nfc.tech < FuriHalNfcTechNum);

    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    return furi_hal_nfc_tech[furi_hal_nfc.tech]->listener.idle(handle);
}

FuriHalNfcError furi_hal_nfc_listener_enable_rx(void) {
    FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;

    st25r3916_direct_cmd(handle, ST25R3916_CMD_UNMASK_RECEIVE_DATA);

    return FuriHalNfcErrorNone;
}
