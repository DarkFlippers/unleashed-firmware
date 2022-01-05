#include "rfal_analogConfigTbl.h"

const uint8_t rfalAnalogConfigCustomSettings[] = {
    /****** Default Analog Configuration for Chip-Specific Reset ******/
    MODE_ENTRY_16_REG(
        (RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_INIT),
        ST25R3916_REG_IO_CONF1,
        (ST25R3916_REG_IO_CONF1_out_cl_mask | ST25R3916_REG_IO_CONF1_lf_clk_off),
        0x07 /* Disable MCU_CLK */
        ,
        ST25R3916_REG_IO_CONF2,
        (ST25R3916_REG_IO_CONF2_miso_pd1 | ST25R3916_REG_IO_CONF2_miso_pd2),
        0x18 /* SPI Pull downs */
        // , ST25R3916_REG_IO_CONF2,  ST25R3916_REG_IO_CONF2_aat_en, ST25R3916_REG_IO_CONF2_aat_en                                                /* Enable AAT */
        ,
        ST25R3916_REG_TX_DRIVER,
        ST25R3916_REG_TX_DRIVER_d_res_mask,
        0x00 /* Set RFO resistance Active Tx */
        ,
        ST25R3916_REG_RES_AM_MOD,
        0xFF,
        0x80 /* Use minimum non-overlap */
        ,
        ST25R3916_REG_FIELD_THRESHOLD_ACTV,
        ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_mask,
        ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_105mV /* Lower activation threshold (higher than deactivation)*/
        ,
        ST25R3916_REG_FIELD_THRESHOLD_ACTV,
        ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_mask,
        ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_105mV /* Lower activation threshold (higher than deactivation)*/
        ,
        ST25R3916_REG_FIELD_THRESHOLD_DEACTV,
        ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_mask,
        ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_75mV /* Lower deactivation threshold */
        ,
        ST25R3916_REG_FIELD_THRESHOLD_DEACTV,
        ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_mask,
        ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_75mV /* Lower deactivation threshold */
        ,
        ST25R3916_REG_AUX_MOD,
        ST25R3916_REG_AUX_MOD_lm_ext,
        ST25R3916_REG_AUX_MOD_lm_ext /* Disable External Load Modulation */
        ,
        ST25R3916_REG_AUX_MOD,
        ST25R3916_REG_AUX_MOD_lm_dri,
        ST25R3916_REG_AUX_MOD_lm_dri /* Use internal Load Modulation */
        ,
        ST25R3916_REG_PASSIVE_TARGET,
        ST25R3916_REG_PASSIVE_TARGET_fdel_mask,
        (5U
         << ST25R3916_REG_PASSIVE_TARGET_fdel_shift) /* Adjust the FDT to be aligned with the bitgrid  */
        ,
        ST25R3916_REG_PT_MOD,
        (ST25R3916_REG_PT_MOD_ptm_res_mask | ST25R3916_REG_PT_MOD_pt_res_mask),
        0x0f /* Reduce RFO resistance in Modulated state */
        ,
        ST25R3916_REG_EMD_SUP_CONF,
        ST25R3916_REG_EMD_SUP_CONF_rx_start_emv,
        ST25R3916_REG_EMD_SUP_CONF_rx_start_emv_on /* Enable start on first 4 bits */
        ,
        ST25R3916_REG_ANT_TUNE_A,
        0xFF,
        0x82 /* Set Antenna Tuning (Poller): ANTL */
        ,
        ST25R3916_REG_ANT_TUNE_B,
        0xFF,
        0x82 /* Set Antenna Tuning (Poller): ANTL */
        ,
        0x84U,
        0x10,
        0x10 /* Avoid chip internal overheat protection */
        )

    /****** Default Analog Configuration for Chip-Specific Poll Common ******/
    ,
    MODE_ENTRY_9_REG(
        (RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_POLL_COMMON),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        ,
        ST25R3916_REG_TX_DRIVER,
        ST25R3916_REG_TX_DRIVER_am_mod_mask,
        ST25R3916_REG_TX_DRIVER_am_mod_12percent /* Set Modulation index */
        ,
        ST25R3916_REG_AUX_MOD,
        (ST25R3916_REG_AUX_MOD_dis_reg_am | ST25R3916_REG_AUX_MOD_res_am),
        0x00 /* Use AM via regulator */
        ,
        ST25R3916_REG_ANT_TUNE_A,
        0xFF,
        0x82 /* Set Antenna Tuning (Poller): ANTL */
        ,
        ST25R3916_REG_ANT_TUNE_B,
        0xFF,
        0x82 /* Set Antenna Tuning (Poller): ANTL */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        )

    /****** Default Analog Configuration for Poll NFC-A Rx Common ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_correlator /* Use Correlator Receiver */
        )

    /****** Default Analog Configuration for Poll NFC-A Tx 106 ******/
    ,
    MODE_ENTRY_5_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_106 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_ook /* Use OOK */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Overshoot Protection */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Overshoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Undershoot Protection */
        )

    /****** Default Analog Configuration for Poll NFC-A Rx 106 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_106 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x08,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x2D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x51,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-A Tx 212 ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_212 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        ,
        ST25R3916_REG_AUX_MOD,
        (ST25R3916_REG_AUX_MOD_dis_reg_am | ST25R3916_REG_AUX_MOD_res_am),
        0x88 /* Use Resistive AM */
        ,
        ST25R3916_REG_RES_AM_MOD,
        ST25R3916_REG_RES_AM_MOD_md_res_mask,
        0x7F /* Set Resistive modulation */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Undershoot Protection */
        )

    /****** Default Analog Configuration for Poll NFC-A Rx 212 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_212 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x02,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x14,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-A Tx 424 ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_424 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        ,
        ST25R3916_REG_AUX_MOD,
        (ST25R3916_REG_AUX_MOD_dis_reg_am | ST25R3916_REG_AUX_MOD_res_am),
        0x88 /* Use Resistive AM */
        ,
        ST25R3916_REG_RES_AM_MOD,
        ST25R3916_REG_RES_AM_MOD_md_res_mask,
        0x7F /* Set Resistive modulation */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Undershoot Protection */
        )

    /****** Default Analog Configuration for Poll NFC-A Rx 424 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_424 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x42,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x54,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-A Tx 848 ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_848 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        ,
        ST25R3916_REG_TX_DRIVER,
        ST25R3916_REG_TX_DRIVER_am_mod_mask,
        ST25R3916_REG_TX_DRIVER_am_mod_40percent /* Set Modulation index */
        ,
        ST25R3916_REG_AUX_MOD,
        (ST25R3916_REG_AUX_MOD_dis_reg_am | ST25R3916_REG_AUX_MOD_res_am),
        0x00 /* Use AM via regulator */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        )

    /****** Default Analog Configuration for Poll NFC-A Rx 848 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_848 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x42,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x44,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-A Anticolision setting ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_ANTICOL),
        ST25R3916_REG_CORR_CONF1,
        ST25R3916_REG_CORR_CONF1_corr_s6,
        0x00 /* Set collision detection level different from data */
        )

#ifdef RFAL_USE_COHE
    /****** Default Analog Configuration for Poll NFC-B Rx Common ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_coherent /* Use Coherent Receiver */
        )
#else
    /****** Default Analog Configuration for Poll NFC-B Rx Common ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_correlator /* Use Correlator Receiver */
        )
#endif /*RFAL_USE_COHE*/

    /****** Default Analog Configuration for Poll NFC-B Rx 106 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB | RFAL_ANALOG_CONFIG_BITRATE_106 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x04,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x1B,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-B Rx 212 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB | RFAL_ANALOG_CONFIG_BITRATE_212 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x02,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x14,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-B Rx 424 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB | RFAL_ANALOG_CONFIG_BITRATE_424 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x42,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x54,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-B Rx 848 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB | RFAL_ANALOG_CONFIG_BITRATE_848 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x42,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x44,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)
#ifdef RFAL_USE_COHE

    /****** Default Analog Configuration for Poll NFC-F Rx Common ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCF |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_coherent /* Use Pulse Receiver */
        ,
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x13,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x54,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)
#else
    /****** Default Analog Configuration for Poll NFC-F Rx Common ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCF |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_correlator /* Use Correlator Receiver */
        ,
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x13,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x54,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)
#endif /*RFAL_USE_COHE*/

        ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV | RFAL_ANALOG_CONFIG_BITRATE_1OF4 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_ook /* Use OOK */
        )

#ifdef RFAL_USE_COHE
    /****** Default Analog Configuration for Poll NFC-V Rx Common ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_coherent /* Use Pulse Receiver */
        ,
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x13,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x2D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x13,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x01)
#else
    /****** Default Analog Configuration for Poll NFC-V Rx Common ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_correlator /* Use Correlator Receiver */
        ,
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x13,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x2D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x13,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x01)
#endif /*RFAL_USE_COHE*/

    /****** Default Analog Configuration for Poll AP2P Tx 106 ******/
    ,
    MODE_ENTRY_5_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_AP2P | RFAL_ANALOG_CONFIG_BITRATE_106 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_ook /* Use OOK modulation */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Undershoot Protection */
        )

    /****** Default Analog Configuration for Poll AP2P Tx 212 ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_AP2P | RFAL_ANALOG_CONFIG_BITRATE_212 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        )

    /****** Default Analog Configuration for Poll AP2P Tx 424 ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_AP2P | RFAL_ANALOG_CONFIG_BITRATE_424 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        )

    /****** Default Analog Configuration for Chip-Specific Listen On ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_LISTEN_ON),
        ST25R3916_REG_ANT_TUNE_A,
        0xFF,
        0x00 /* Set Antenna Tuning (Listener): ANTL */
        ,
        ST25R3916_REG_ANT_TUNE_B,
        0xFF,
        0xff /* Set Antenna Tuning (Listener): ANTL */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        )

    /****** Default Analog Configuration for Listen AP2P Tx Common ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_ANT_TUNE_A,
        0xFF,
        0x82 /* Set Antenna Tuning (Poller): ANTL */
        ,
        ST25R3916_REG_ANT_TUNE_B,
        0xFF,
        0x82 /* Set Antenna Tuning (Poller): ANTL */
        ,
        ST25R3916_REG_TX_DRIVER,
        ST25R3916_REG_TX_DRIVER_am_mod_mask,
        ST25R3916_REG_TX_DRIVER_am_mod_12percent /* Set Modulation index */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        )

    /****** Default Analog Configuration for Listen AP2P Rx Common ******/
    ,
    MODE_ENTRY_3_REG(
        (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        ST25R3916_REG_RX_CONF1_lp_mask,
        ST25R3916_REG_RX_CONF1_lp_1200khz /* Set Rx filter configuration */
        ,
        ST25R3916_REG_RX_CONF1,
        ST25R3916_REG_RX_CONF1_hz_mask,
        ST25R3916_REG_RX_CONF1_hz_12_200khz /* Set Rx filter configuration */
        ,
        ST25R3916_REG_RX_CONF2,
        ST25R3916_REG_RX_CONF2_amd_sel,
        ST25R3916_REG_RX_CONF2_amd_sel_mixer /* AM demodulator: mixer */
        )

    /****** Default Analog Configuration for Listen AP2P Tx 106 ******/
    ,
    MODE_ENTRY_5_REG(
        (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P |
         RFAL_ANALOG_CONFIG_BITRATE_106 | RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_ook /* Use OOK modulation */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Undershoot Protection */
        )

    /****** Default Analog Configuration for Listen AP2P Tx 212 ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P |
         RFAL_ANALOG_CONFIG_BITRATE_212 | RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        )

    /****** Default Analog Configuration for Listen AP2P Tx 424 ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P |
         RFAL_ANALOG_CONFIG_BITRATE_424 | RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        )

};

const uint16_t rfalAnalogConfigCustomSettingsLength = sizeof(rfalAnalogConfigCustomSettings);
