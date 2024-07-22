#pragma once

#include <furi_hal_spi.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ST25R3916 direct commands */
#define ST25R3916_CMD_SET_DEFAULT \
    0xC1U /** Puts the chip in default state (same as after power-up */
#define ST25R3916_CMD_STOP                 0xC2U /*!< Stops all activities and clears FIFO */
#define ST25R3916_CMD_TRANSMIT_WITH_CRC    0xC4U /** Transmit with CRC */
#define ST25R3916_CMD_TRANSMIT_WITHOUT_CRC 0xC5U /** Transmit without CRC */
#define ST25R3916_CMD_TRANSMIT_REQA        0xC6U /** Transmit REQA */
#define ST25R3916_CMD_TRANSMIT_WUPA        0xC7U /** Transmit WUPA */
#define ST25R3916_CMD_INITIAL_RF_COLLISION \
    0xC8U /** NFC transmit with Initial RF Collision Avoidance */
#define ST25R3916_CMD_RESPONSE_RF_COLLISION_N \
    0xC9U /** NFC transmit with Response RF Collision Avoidance */
#define ST25R3916_CMD_GOTO_SENSE          0xCDU /** Passive target logic to Sense/Idle state */
#define ST25R3916_CMD_GOTO_SLEEP          0xCEU /** Passive target logic to Sleep/Halt state */
#define ST25R3916_CMD_MASK_RECEIVE_DATA   0xD0U /** Mask receive data */
#define ST25R3916_CMD_UNMASK_RECEIVE_DATA 0xD1U /** Unmask receive data */
#define ST25R3916_CMD_AM_MOD_STATE_CHANGE 0xD2U /** AM Modulation state change */
#define ST25R3916_CMD_MEASURE_AMPLITUDE   0xD3U /** Measure singal amplitude on RFI inputs */
#define ST25R3916_CMD_RESET_RXGAIN        0xD5U /** Reset RX Gain */
#define ST25R3916_CMD_ADJUST_REGULATORS   0xD6U /** Adjust regulators */
#define ST25R3916_CMD_CALIBRATE_DRIVER_TIMING \
    0xD8U /** Starts the sequence to adjust the driver timing */
#define ST25R3916_CMD_MEASURE_PHASE            0xD9U /** Measure phase between RFO and RFI signal */
#define ST25R3916_CMD_CLEAR_RSSI               0xDAU /** Clear RSSI bits and restart the measurement */
#define ST25R3916_CMD_CLEAR_FIFO               0xDBU /** Clears FIFO, Collision and IRQ status */
#define ST25R3916_CMD_TRANSPARENT_MODE         0xDCU /** Transparent mode */
#define ST25R3916_CMD_CALIBRATE_C_SENSOR       0xDDU /** Calibrate the capacitive sensor */
#define ST25R3916_CMD_MEASURE_CAPACITANCE      0xDEU /** Measure capacitance */
#define ST25R3916_CMD_MEASURE_VDD              0xDFU /** Measure power supply voltage */
#define ST25R3916_CMD_START_GP_TIMER           0xE0U /** Start the general purpose timer */
#define ST25R3916_CMD_START_WUP_TIMER          0xE1U /** Start the wake-up timer */
#define ST25R3916_CMD_START_MASK_RECEIVE_TIMER 0xE2U /** Start the mask-receive timer */
#define ST25R3916_CMD_START_NO_RESPONSE_TIMER  0xE3U /** Start the no-response timer */
#define ST25R3916_CMD_START_PPON2_TIMER        0xE4U /** Start PPon2 timer */
#define ST25R3916_CMD_STOP_NRT                 0xE8U /** Stop No Response Timer */
#define ST25R3916_CMD_SPACE_B_ACCESS           0xFBU /** Enable R/W access to the test registers */
#define ST25R3916_CMD_TEST_ACCESS              0xFCU /** Enable R/W access to the test registers */

#define ST25R3916_SPACE_B         0x40U /** ST25R3916 Space-B indicator */
#define ST25R3916_SPACE_B_REG_LEN 16U /** Number of register in the space B */

#define ST25R3916_FIFO_STATUS_LEN 2 /** Number of FIFO Status Register */

#define ST25R3916_PTM_A_LEN   15U /** Passive target memory A config length */
#define ST25R3916_PTM_B_LEN   0U /** Passive target memory B config length */
#define ST25R3916_PTM_F_LEN   21U /** Passive target memory F config length */
#define ST25R3916_PTM_TSN_LEN 12U /** Passive target memory TSN data length */

/** Full Passive target memory length */
#define ST25R3916_PTM_LEN \
    (ST25R3916_PTM_A_LEN + ST25R3916_PTM_B_LEN + ST25R3916_PTM_F_LEN + ST25R3916_PTM_TSN_LEN)

/** IO configuration registers */
#define ST25R3916_REG_IO_CONF1 0x00U /** RW IO Configuration Register 1 */
#define ST25R3916_REG_IO_CONF2 0x01U /** RW IO Configuration Register 2 */

/** Operation control and mode definition  */
#define ST25R3916_REG_OP_CONTROL 0x02U /** RW Operation Control Register */
#define ST25R3916_REG_MODE       0x03U /** RW Mode Definition Register */
#define ST25R3916_REG_BIT_RATE   0x04U /** RW Bit Rate Definition Register */

/** Protocol Configuration registers */
#define ST25R3916_REG_ISO14443A_NFC 0x05U /** RW ISO14443A and NFC 106 kBit/s Settings Register */
#define ST25R3916_REG_EMD_SUP_CONF \
    (ST25R3916_SPACE_B | 0x05U) /*!< RW EMD Suppression Configuration Register */
#define ST25R3916_REG_ISO14443B_1 0x06U /** RW ISO14443B Settings Register 1 */
#define ST25R3916_REG_SUBC_START_TIME \
    (ST25R3916_SPACE_B | 0x06U) /*!< RW Subcarrier Start Time Register */
#define ST25R3916_REG_ISO14443B_2    0x07U /** RW ISO14443B Settings Register 2 */
#define ST25R3916_REG_PASSIVE_TARGET 0x08U /** RW Passive Target Definition Register */
#define ST25R3916_REG_STREAM_MODE    0x09U /** RW Stream Mode Definition Register */
#define ST25R3916_REG_AUX            0x0AU /** RW Auxiliary Definition Register */

/** Receiver Configuration registers */
#define ST25R3916_REG_RX_CONF1 0x0BU /** RW Receiver Configuration Register 1 */
#define ST25R3916_REG_RX_CONF2 0x0CU /** RW Receiver Configuration Register 2 */
#define ST25R3916_REG_RX_CONF3 0x0DU /** RW Receiver Configuration Register 3 */
#define ST25R3916_REG_RX_CONF4 0x0EU /** RW Receiver Configuration Register 4 */
#define ST25R3916_REG_P2P_RX_CONF \
    (ST25R3916_SPACE_B | 0x0BU) /** RW P2P Receiver Configuration Register 1 */
#define ST25R3916_REG_CORR_CONF1 \
    (ST25R3916_SPACE_B | 0x0CU) /** RW Correlator configuration register 1 */
#define ST25R3916_REG_CORR_CONF2 \
    (ST25R3916_SPACE_B | 0x0DU) /** RW Correlator configuration register 2 */

/** Timer definition registers */
#define ST25R3916_REG_MASK_RX_TIMER      0x0FU /** RW Mask Receive Timer Register */
#define ST25R3916_REG_NO_RESPONSE_TIMER1 0x10U /** RW No-response Timer Register 1 */
#define ST25R3916_REG_NO_RESPONSE_TIMER2 0x11U /** RW No-response Timer Register 2 */
#define ST25R3916_REG_TIMER_EMV_CONTROL  0x12U /** RW Timer and EMV Control */
#define ST25R3916_REG_GPT1               0x13U /** RW General Purpose Timer Register 1 */
#define ST25R3916_REG_GPT2               0x14U /** RW General Purpose Timer Register 2 */
#define ST25R3916_REG_PPON2              0x15U /** RW PPON2 Field waiting Timer Register */
#define ST25R3916_REG_SQUELCH_TIMER      (ST25R3916_SPACE_B | 0x0FU) /** RW Squelch timeout Register */
#define ST25R3916_REG_FIELD_ON_GT        (ST25R3916_SPACE_B | 0x15U) /** RW NFC Field on guard time */

/** Interrupt and associated reporting registers */
#define ST25R3916_REG_IRQ_MASK_MAIN         0x16U /** RW Mask Main Interrupt Register */
#define ST25R3916_REG_IRQ_MASK_TIMER_NFC    0x17U /** RW Mask Timer and NFC Interrupt Register */
#define ST25R3916_REG_IRQ_MASK_ERROR_WUP    0x18U /** RW Mask Error and Wake-up Interrupt Register */
#define ST25R3916_REG_IRQ_MASK_TARGET       0x19U /** RW Mask 3916 Target Interrupt Register */
#define ST25R3916_REG_IRQ_MAIN              0x1AU /** R  Main Interrupt Register */
#define ST25R3916_REG_IRQ_TIMER_NFC         0x1BU /** R  Timer and NFC Interrupt Register */
#define ST25R3916_REG_IRQ_ERROR_WUP         0x1CU /** R  Error and Wake-up Interrupt Register */
#define ST25R3916_REG_IRQ_TARGET            0x1DU /*!< R  ST25R3916 Target Interrupt Register */
#define ST25R3916_REG_FIFO_STATUS1          0x1EU /** R  FIFO Status Register 1 */
#define ST25R3916_REG_FIFO_STATUS2          0x1FU /** R  FIFO Status Register 2 */
#define ST25R3916_REG_COLLISION_STATUS      0x20U /** R  Collision Display Register */
#define ST25R3916_REG_PASSIVE_TARGET_STATUS 0x21U /** R  Passive target state status */

/** Definition of number of transmitted bytes */
#define ST25R3916_REG_NUM_TX_BYTES1 0x22U /** RW Number of Transmitted Bytes Register 1 */
#define ST25R3916_REG_NUM_TX_BYTES2 0x23U /** RW Number of Transmitted Bytes Register 2 */

/** NFCIP Bit Rate Display Register */
#define ST25R3916_REG_NFCIP1_BIT_RATE 0x24U /** R  NFCIP Bit Rate Detection Display Register */

/** A/D Converter Output Register */
#define ST25R3916_REG_AD_RESULT 0x25U /** R  A/D Converter Output Register */

/** Antenna tuning registers */
#define ST25R3916_REG_ANT_TUNE_A 0x26U /** RW Antenna Tuning Control (AAT-A) Register 1 */
#define ST25R3916_REG_ANT_TUNE_B 0x27U /** RW Antenna Tuning Control (AAT-B) Register 2 */

/** Antenna Driver and Modulation registers */
#define ST25R3916_REG_TX_DRIVER 0x28U /** RW TX driver register */
#define ST25R3916_REG_PT_MOD    0x29U /** RW PT modulation Register */
#define ST25R3916_REG_AUX_MOD   (ST25R3916_SPACE_B | 0x28U) /** RW Aux Modulation setting Register */
#define ST25R3916_REG_TX_DRIVER_TIMING \
    (ST25R3916_SPACE_B | 0x29U) /** RW TX driver timing Register */
#define ST25R3916_REG_RES_AM_MOD \
    (ST25R3916_SPACE_B | 0x2AU) /** RW Resistive AM modulation register */
#define ST25R3916_REG_TX_DRIVER_STATUS \
    (ST25R3916_SPACE_B | 0x2BU) /** R  TX driver timing readout Register */

/** External Field Detector Threshold Registers */
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV \
    0x2AU /** RW External Field Detector Activation Threshold Reg */
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV \
    0x2BU /** RW External Field Detector Deactivation Threshold Reg */

/** Regulator registers */
#define ST25R3916_REG_REGULATOR_CONTROL 0x2CU /** RW Regulated Voltage Control Register */
#define ST25R3916_REG_REGULATOR_RESULT \
    (ST25R3916_SPACE_B | 0x2CU) /** R Regulator Display Register */

/** Receiver State Display Register */
#define ST25R3916_REG_RSSI_RESULT        0x2DU /** R RSSI Display Register */
#define ST25R3916_REG_GAIN_RED_STATE     0x2EU /** R Gain Reduction State Register */
#define ST25R3916_REG_CAP_SENSOR_CONTROL 0x2FU /** RW Capacitive Sensor Control Register */
#define ST25R3916_REG_CAP_SENSOR_RESULT  0x30U /** R  Capacitive Sensor Display Register */
#define ST25R3916_REG_AUX_DISPLAY        0x31U /** R Auxiliary Display Register */

/** Over/Undershoot Protection Configuration Registers */
#define ST25R3916_REG_OVERSHOOT_CONF1 \
    (ST25R3916_SPACE_B | 0x30U) /** RW  Overshoot Protection Configuration Register 1 */
#define ST25R3916_REG_OVERSHOOT_CONF2 \
    (ST25R3916_SPACE_B | 0x31U) /** RW  Overshoot Protection Configuration Register 2 */
#define ST25R3916_REG_UNDERSHOOT_CONF1 \
    (ST25R3916_SPACE_B | 0x32U) /** RW  Undershoot Protection Configuration Register 1 */
#define ST25R3916_REG_UNDERSHOOT_CONF2 \
    (ST25R3916_SPACE_B | 0x33U) /** RW  Undershoot Protection Configuration Register 2 */

/** Detection of card presence */
#define ST25R3916_REG_WUP_TIMER_CONTROL 0x32U /** RW Wake-up Timer Control Register */
#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF \
    0x33U /** RW Amplitude Measurement Configuration Register */
#define ST25R3916_REG_AMPLITUDE_MEASURE_REF \
    0x34U /** RW Amplitude Measurement Reference Register */
#define ST25R3916_REG_AMPLITUDE_MEASURE_AA_RESULT \
    0x35U /** R  Amplitude Measurement Auto Averaging Display Reg */
#define ST25R3916_REG_AMPLITUDE_MEASURE_RESULT \
    0x36U /** R  Amplitude Measurement Display Register */
#define ST25R3916_REG_PHASE_MEASURE_CONF 0x37U /** RW Phase Measurement Configuration Register */
#define ST25R3916_REG_PHASE_MEASURE_REF  0x38U /** RW Phase Measurement Reference Register */
#define ST25R3916_REG_PHASE_MEASURE_AA_RESULT \
    0x39U /** R  Phase Measurement Auto Averaging Display */
#define ST25R3916_REG_PHASE_MEASURE_RESULT 0x3AU /** R  Phase Measurement Display Register */
#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF \
    0x3BU /** RW Capacitance Measurement Configuration Register */
#define ST25R3916_REG_CAPACITANCE_MEASURE_REF \
    0x3CU /** RW Capacitance Measurement Reference Register */
#define ST25R3916_REG_CAPACITANCE_MEASURE_AA_RESULT \
    0x3DU /** R  Capacitance Measurement Auto Averaging Display Reg */
#define ST25R3916_REG_CAPACITANCE_MEASURE_RESULT \
    0x3EU /** R  Capacitance Measurement Display Register */

/** IC identity  */
#define ST25R3916_REG_IC_IDENTITY 0x3FU /** R  Chip Id: 0 for old silicon, v2 silicon: 0x09 */

/** Register bit definitions */

#define ST25R3916_REG_IO_CONF1_single          (1U << 7)
#define ST25R3916_REG_IO_CONF1_rfo2            (1U << 6)
#define ST25R3916_REG_IO_CONF1_i2c_thd1        (1U << 5)
#define ST25R3916_REG_IO_CONF1_i2c_thd0        (1U << 4)
#define ST25R3916_REG_IO_CONF1_i2c_thd_mask    (3U << 4)
#define ST25R3916_REG_IO_CONF1_i2c_thd_shift   (4U)
#define ST25R3916_REG_IO_CONF1_rfu             (1U << 3)
#define ST25R3916_REG_IO_CONF1_out_cl1         (1U << 2)
#define ST25R3916_REG_IO_CONF1_out_cl0         (1U << 1)
#define ST25R3916_REG_IO_CONF1_out_cl_disabled (3U << 1)
#define ST25R3916_REG_IO_CONF1_out_cl_13_56MHZ (2U << 1)
#define ST25R3916_REG_IO_CONF1_out_cl_4_78MHZ  (1U << 1)
#define ST25R3916_REG_IO_CONF1_out_cl_3_39MHZ  (0U << 1)
#define ST25R3916_REG_IO_CONF1_out_cl_mask     (3U << 1)
#define ST25R3916_REG_IO_CONF1_out_cl_shift    (1U)
#define ST25R3916_REG_IO_CONF1_lf_clk_off      (1U << 0)
#define ST25R3916_REG_IO_CONF1_lf_clk_off_on   (1U << 0)
#define ST25R3916_REG_IO_CONF1_lf_clk_off_off  (0U << 0)

#define ST25R3916_REG_IO_CONF2_sup3V      (1U << 7)
#define ST25R3916_REG_IO_CONF2_sup3V_3V   (1U << 7)
#define ST25R3916_REG_IO_CONF2_sup3V_5V   (0U << 7)
#define ST25R3916_REG_IO_CONF2_vspd_off   (1U << 6)
#define ST25R3916_REG_IO_CONF2_aat_en     (1U << 5)
#define ST25R3916_REG_IO_CONF2_miso_pd2   (1U << 4)
#define ST25R3916_REG_IO_CONF2_miso_pd1   (1U << 3)
#define ST25R3916_REG_IO_CONF2_io_drv_lvl (1U << 2)
#define ST25R3916_REG_IO_CONF2_slow_up    (1U << 0)

#define ST25R3916_REG_OP_CONTROL_en                   (1U << 7)
#define ST25R3916_REG_OP_CONTROL_rx_en                (1U << 6)
#define ST25R3916_REG_OP_CONTROL_rx_chn               (1U << 5)
#define ST25R3916_REG_OP_CONTROL_rx_man               (1U << 4)
#define ST25R3916_REG_OP_CONTROL_tx_en                (1U << 3)
#define ST25R3916_REG_OP_CONTROL_wu                   (1U << 2)
#define ST25R3916_REG_OP_CONTROL_en_fd_c1             (1U << 1)
#define ST25R3916_REG_OP_CONTROL_en_fd_c0             (1U << 0)
#define ST25R3916_REG_OP_CONTROL_en_fd_efd_off        (0U << 0)
#define ST25R3916_REG_OP_CONTROL_en_fd_manual_efd_ca  (1U << 0)
#define ST25R3916_REG_OP_CONTROL_en_fd_manual_efd_pdt (2U << 0)
#define ST25R3916_REG_OP_CONTROL_en_fd_auto_efd       (3U << 0)
#define ST25R3916_REG_OP_CONTROL_en_fd_shift          (0U)
#define ST25R3916_REG_OP_CONTROL_en_fd_mask           (3U << 0)

#define ST25R3916_REG_MODE_targ                 (1U << 7)
#define ST25R3916_REG_MODE_targ_targ            (1U << 7)
#define ST25R3916_REG_MODE_targ_init            (0U << 7)
#define ST25R3916_REG_MODE_om3                  (1U << 6)
#define ST25R3916_REG_MODE_om2                  (1U << 5)
#define ST25R3916_REG_MODE_om1                  (1U << 4)
#define ST25R3916_REG_MODE_om0                  (1U << 3)
#define ST25R3916_REG_MODE_om_bpsk_stream       (0xfU << 3)
#define ST25R3916_REG_MODE_om_subcarrier_stream (0xeU << 3)
#define ST25R3916_REG_MODE_om_topaz             (0x4U << 3)
#define ST25R3916_REG_MODE_om_felica            (0x3U << 3)
#define ST25R3916_REG_MODE_om_iso14443b         (0x2U << 3)
#define ST25R3916_REG_MODE_om_iso14443a         (0x1U << 3)
#define ST25R3916_REG_MODE_om_targ_nfca         (0x1U << 3)
#define ST25R3916_REG_MODE_om_targ_nfcb         (0x2U << 3)
#define ST25R3916_REG_MODE_om_targ_nfcf         (0x4U << 3)
#define ST25R3916_REG_MODE_om_targ_nfcip        (0x7U << 3)
#define ST25R3916_REG_MODE_om_nfc               (0x0U << 3)
#define ST25R3916_REG_MODE_om_mask              (0xfU << 3)
#define ST25R3916_REG_MODE_om_shift             (3U)
#define ST25R3916_REG_MODE_tr_am                (1U << 2)
#define ST25R3916_REG_MODE_tr_am_ook            (0U << 2)
#define ST25R3916_REG_MODE_tr_am_am             (1U << 2)
#define ST25R3916_REG_MODE_nfc_ar1              (1U << 1)
#define ST25R3916_REG_MODE_nfc_ar0              (1U << 0)
#define ST25R3916_REG_MODE_nfc_ar_off           (0U << 0)
#define ST25R3916_REG_MODE_nfc_ar_auto_rx       (1U << 0)
#define ST25R3916_REG_MODE_nfc_ar_eof           (2U << 0)
#define ST25R3916_REG_MODE_nfc_ar_rfu           (3U << 0)
#define ST25R3916_REG_MODE_nfc_ar_mask          (3U << 0)
#define ST25R3916_REG_MODE_nfc_ar_shift         (0U)

#define ST25R3916_REG_BIT_RATE_txrate_106   (0x0U << 4)
#define ST25R3916_REG_BIT_RATE_txrate_212   (0x1U << 4)
#define ST25R3916_REG_BIT_RATE_txrate_424   (0x2U << 4)
#define ST25R3916_REG_BIT_RATE_txrate_848   (0x3U << 4)
#define ST25R3916_REG_BIT_RATE_txrate_mask  (0x3U << 4)
#define ST25R3916_REG_BIT_RATE_txrate_shift (4U)
#define ST25R3916_REG_BIT_RATE_rxrate_106   (0x0U << 0)
#define ST25R3916_REG_BIT_RATE_rxrate_212   (0x1U << 0)
#define ST25R3916_REG_BIT_RATE_rxrate_424   (0x2U << 0)
#define ST25R3916_REG_BIT_RATE_rxrate_848   (0x3U << 0)
#define ST25R3916_REG_BIT_RATE_rxrate_mask  (0x3U << 0)
#define ST25R3916_REG_BIT_RATE_rxrate_shift (0U)

#define ST25R3916_REG_ISO14443A_NFC_no_tx_par     (1U << 7)
#define ST25R3916_REG_ISO14443A_NFC_no_tx_par_off (0U << 7)
#define ST25R3916_REG_ISO14443A_NFC_no_rx_par     (1U << 6)
#define ST25R3916_REG_ISO14443A_NFC_no_rx_par_off (0U << 6)
#define ST25R3916_REG_ISO14443A_NFC_nfc_f0        (1U << 5)
#define ST25R3916_REG_ISO14443A_NFC_nfc_f0_off    (0U << 5)
#define ST25R3916_REG_ISO14443A_NFC_p_len3        (1U << 4)
#define ST25R3916_REG_ISO14443A_NFC_p_len2        (1U << 3)
#define ST25R3916_REG_ISO14443A_NFC_p_len1        (1U << 2)
#define ST25R3916_REG_ISO14443A_NFC_p_len0        (1U << 1)
#define ST25R3916_REG_ISO14443A_NFC_p_len_mask    (0xfU << 1)
#define ST25R3916_REG_ISO14443A_NFC_p_len_shift   (1U)
#define ST25R3916_REG_ISO14443A_NFC_antcl         (1U << 0)

#define ST25R3916_REG_EMD_SUP_CONF_emd_emv          (1U << 7)
#define ST25R3916_REG_EMD_SUP_CONF_emd_emv_on       (1U << 7)
#define ST25R3916_REG_EMD_SUP_CONF_emd_emv_off      (0U << 7)
#define ST25R3916_REG_EMD_SUP_CONF_rx_start_emv     (1U << 6)
#define ST25R3916_REG_EMD_SUP_CONF_rx_start_emv_on  (1U << 6)
#define ST25R3916_REG_EMD_SUP_CONF_rx_start_emv_off (0U << 6)
#define ST25R3916_REG_EMD_SUP_CONF_rfu1             (1U << 5)
#define ST25R3916_REG_EMD_SUP_CONF_rfu0             (1U << 4)
#define ST25R3916_REG_EMD_SUP_CONF_emd_thld3        (1U << 3)
#define ST25R3916_REG_EMD_SUP_CONF_emd_thld2        (1U << 2)
#define ST25R3916_REG_EMD_SUP_CONF_emd_thld1        (1U << 1)
#define ST25R3916_REG_EMD_SUP_CONF_emd_thld0        (1U << 0)
#define ST25R3916_REG_EMD_SUP_CONF_emd_thld_mask    (0xfU << 0)
#define ST25R3916_REG_EMD_SUP_CONF_emd_thld_shift   (0U)

#define ST25R3916_REG_SUBC_START_TIME_rfu2      (1U << 7)
#define ST25R3916_REG_SUBC_START_TIME_rfu1      (1U << 6)
#define ST25R3916_REG_SUBC_START_TIME_rfu0      (1U << 5)
#define ST25R3916_REG_SUBC_START_TIME_sst4      (1U << 4)
#define ST25R3916_REG_SUBC_START_TIME_sst3      (1U << 3)
#define ST25R3916_REG_SUBC_START_TIME_sst2      (1U << 2)
#define ST25R3916_REG_SUBC_START_TIME_sst1      (1U << 1)
#define ST25R3916_REG_SUBC_START_TIME_sst0      (1U << 0)
#define ST25R3916_REG_SUBC_START_TIME_sst_mask  (0x1fU << 0)
#define ST25R3916_REG_SUBC_START_TIME_sst_shift (0U)

#define ST25R3916_REG_ISO14443B_1_egt2        (1U << 7)
#define ST25R3916_REG_ISO14443B_1_egt1        (1U << 6)
#define ST25R3916_REG_ISO14443B_1_egt0        (1U << 5)
#define ST25R3916_REG_ISO14443B_1_egt_shift   (5U)
#define ST25R3916_REG_ISO14443B_1_egt_mask    (7U << 5)
#define ST25R3916_REG_ISO14443B_1_sof_1       (1U << 3)
#define ST25R3916_REG_ISO14443B_1_sof_1_3etu  (1U << 3)
#define ST25R3916_REG_ISO14443B_1_sof_1_2etu  (0U << 3)
#define ST25R3916_REG_ISO14443B_1_sof_0       (1U << 4)
#define ST25R3916_REG_ISO14443B_1_sof_0_11etu (1U << 4)
#define ST25R3916_REG_ISO14443B_1_sof_0_10etu (0U << 4)
#define ST25R3916_REG_ISO14443B_1_sof_mask    (3U << 3)
#define ST25R3916_REG_ISO14443B_1_eof         (1U << 2)
#define ST25R3916_REG_ISO14443B_1_eof_11etu   (1U << 2)
#define ST25R3916_REG_ISO14443B_1_eof_10etu   (0U << 2)
#define ST25R3916_REG_ISO14443B_1_half        (1U << 1)
#define ST25R3916_REG_ISO14443B_1_rx_st_om    (1U << 0)

#define ST25R3916_REG_ISO14443B_2_tr1_1        (1U << 7)
#define ST25R3916_REG_ISO14443B_2_tr1_0        (1U << 6)
#define ST25R3916_REG_ISO14443B_2_tr1_64fs32fs (1U << 6)
#define ST25R3916_REG_ISO14443B_2_tr1_80fs80fs (0U << 6)
#define ST25R3916_REG_ISO14443B_2_tr1_mask     (3U << 6)
#define ST25R3916_REG_ISO14443B_2_tr1_shift    (6U)
#define ST25R3916_REG_ISO14443B_2_no_sof       (1U << 5)
#define ST25R3916_REG_ISO14443B_2_no_eof       (1U << 4)
#define ST25R3916_REG_ISO14443B_rfu1           (1U << 3)
#define ST25R3916_REG_ISO14443B_rfu0           (1U << 2)
#define ST25R3916_REG_ISO14443B_2_f_p1         (1U << 1)
#define ST25R3916_REG_ISO14443B_2_f_p0         (1U << 0)
#define ST25R3916_REG_ISO14443B_2_f_p_96       (3U << 0)
#define ST25R3916_REG_ISO14443B_2_f_p_80       (2U << 0)
#define ST25R3916_REG_ISO14443B_2_f_p_64       (1U << 0)
#define ST25R3916_REG_ISO14443B_2_f_p_48       (0U << 0)
#define ST25R3916_REG_ISO14443B_2_f_p_mask     (3U << 0)
#define ST25R3916_REG_ISO14443B_2_f_p_shift    (0U)

#define ST25R3916_REG_PASSIVE_TARGET_fdel_3       (1U << 7)
#define ST25R3916_REG_PASSIVE_TARGET_fdel_2       (1U << 6)
#define ST25R3916_REG_PASSIVE_TARGET_fdel_1       (1U << 5)
#define ST25R3916_REG_PASSIVE_TARGET_fdel_0       (1U << 4)
#define ST25R3916_REG_PASSIVE_TARGET_fdel_mask    (0xfU << 4)
#define ST25R3916_REG_PASSIVE_TARGET_fdel_shift   (4U)
#define ST25R3916_REG_PASSIVE_TARGET_d_ac_ap2p    (1U << 3)
#define ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r (1U << 2)
#define ST25R3916_REG_PASSIVE_TARGET_rfu          (1U << 1)
#define ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a   (1U << 0)

#define ST25R3916_REG_STREAM_MODE_rfu          (1U << 7)
#define ST25R3916_REG_STREAM_MODE_scf1         (1U << 6)
#define ST25R3916_REG_STREAM_MODE_scf0         (1U << 5)
#define ST25R3916_REG_STREAM_MODE_scf_sc212    (0U << 5)
#define ST25R3916_REG_STREAM_MODE_scf_sc424    (1U << 5)
#define ST25R3916_REG_STREAM_MODE_scf_sc848    (2U << 5)
#define ST25R3916_REG_STREAM_MODE_scf_sc1695   (3U << 5)
#define ST25R3916_REG_STREAM_MODE_scf_bpsk848  (0U << 5)
#define ST25R3916_REG_STREAM_MODE_scf_bpsk1695 (1U << 5)
#define ST25R3916_REG_STREAM_MODE_scf_bpsk3390 (2U << 5)
#define ST25R3916_REG_STREAM_MODE_scf_bpsk106  (3U << 5)
#define ST25R3916_REG_STREAM_MODE_scf_mask     (3U << 5)
#define ST25R3916_REG_STREAM_MODE_scf_shift    (5U)
#define ST25R3916_REG_STREAM_MODE_scp1         (1U << 4)
#define ST25R3916_REG_STREAM_MODE_scp0         (1U << 3)
#define ST25R3916_REG_STREAM_MODE_scp_1pulse   (0U << 3)
#define ST25R3916_REG_STREAM_MODE_scp_2pulses  (1U << 3)
#define ST25R3916_REG_STREAM_MODE_scp_4pulses  (2U << 3)
#define ST25R3916_REG_STREAM_MODE_scp_8pulses  (3U << 3)
#define ST25R3916_REG_STREAM_MODE_scp_mask     (3U << 3)
#define ST25R3916_REG_STREAM_MODE_scp_shift    (3U)
#define ST25R3916_REG_STREAM_MODE_stx2         (1U << 2)
#define ST25R3916_REG_STREAM_MODE_stx1         (1U << 1)
#define ST25R3916_REG_STREAM_MODE_stx0         (1U << 0)
#define ST25R3916_REG_STREAM_MODE_stx_106      (0U << 0)
#define ST25R3916_REG_STREAM_MODE_stx_212      (1U << 0)
#define ST25R3916_REG_STREAM_MODE_stx_424      (2U << 0)
#define ST25R3916_REG_STREAM_MODE_stx_848      (3U << 0)
#define ST25R3916_REG_STREAM_MODE_stx_mask     (7U << 0)
#define ST25R3916_REG_STREAM_MODE_stx_shift    (0U)

#define ST25R3916_REG_AUX_no_crc_rx           (1U << 7)
#define ST25R3916_REG_AUX_rfu                 (1U << 6)
#define ST25R3916_REG_AUX_nfc_id1             (1U << 5)
#define ST25R3916_REG_AUX_nfc_id0             (1U << 4)
#define ST25R3916_REG_AUX_nfc_id_7bytes       (1U << 4)
#define ST25R3916_REG_AUX_nfc_id_4bytes       (0U << 4)
#define ST25R3916_REG_AUX_nfc_id_mask         (3U << 4)
#define ST25R3916_REG_AUX_nfc_id_shift        (4U)
#define ST25R3916_REG_AUX_mfaz_cl90           (1U << 3)
#define ST25R3916_REG_AUX_dis_corr            (1U << 2)
#define ST25R3916_REG_AUX_dis_corr_coherent   (1U << 2)
#define ST25R3916_REG_AUX_dis_corr_correlator (0U << 2)
#define ST25R3916_REG_AUX_nfc_n1              (1U << 1)
#define ST25R3916_REG_AUX_nfc_n0              (1U << 0)
#define ST25R3916_REG_AUX_nfc_n_mask          (3U << 0)
#define ST25R3916_REG_AUX_nfc_n_shift         (0U)

#define ST25R3916_REG_RX_CONF1_ch_sel           (1U << 7)
#define ST25R3916_REG_RX_CONF1_ch_sel_PM        (1U << 7)
#define ST25R3916_REG_RX_CONF1_ch_sel_AM        (0U << 7)
#define ST25R3916_REG_RX_CONF1_lp2              (1U << 6)
#define ST25R3916_REG_RX_CONF1_lp1              (1U << 5)
#define ST25R3916_REG_RX_CONF1_lp0              (1U << 4)
#define ST25R3916_REG_RX_CONF1_lp_1200khz       (0U << 4)
#define ST25R3916_REG_RX_CONF1_lp_600khz        (1U << 4)
#define ST25R3916_REG_RX_CONF1_lp_300khz        (2U << 4)
#define ST25R3916_REG_RX_CONF1_lp_2000khz       (4U << 4)
#define ST25R3916_REG_RX_CONF1_lp_7000khz       (5U << 4)
#define ST25R3916_REG_RX_CONF1_lp_mask          (7U << 4)
#define ST25R3916_REG_RX_CONF1_lp_shift         (4U)
#define ST25R3916_REG_RX_CONF1_z600k            (1U << 3)
#define ST25R3916_REG_RX_CONF1_h200             (1U << 2)
#define ST25R3916_REG_RX_CONF1_h80              (1U << 1)
#define ST25R3916_REG_RX_CONF1_z12k             (1U << 0)
#define ST25R3916_REG_RX_CONF1_hz_60_400khz     (0U << 0)
#define ST25R3916_REG_RX_CONF1_hz_60_200khz     (4U << 0)
#define ST25R3916_REG_RX_CONF1_hz_40_80khz      (2U << 0)
#define ST25R3916_REG_RX_CONF1_hz_12_200khz     (1U << 0)
#define ST25R3916_REG_RX_CONF1_hz_12_80khz      (3U << 0)
#define ST25R3916_REG_RX_CONF1_hz_12_200khz_alt (5U << 0)
#define ST25R3916_REG_RX_CONF1_hz_600_400khz    (8U << 0)
#define ST25R3916_REG_RX_CONF1_hz_600_200khz    (12U << 0)
#define ST25R3916_REG_RX_CONF1_hz_mask          (0xfU << 0)
#define ST25R3916_REG_RX_CONF1_hz_shift         (0U)

#define ST25R3916_REG_RX_CONF2_demod_mode    (1U << 7)
#define ST25R3916_REG_RX_CONF2_amd_sel       (1U << 6)
#define ST25R3916_REG_RX_CONF2_amd_sel_mixer (1U << 6)
#define ST25R3916_REG_RX_CONF2_amd_sel_peak  (0U << 6)
#define ST25R3916_REG_RX_CONF2_sqm_dyn       (1U << 5)
#define ST25R3916_REG_RX_CONF2_pulz_61       (1U << 4)
#define ST25R3916_REG_RX_CONF2_agc_en        (1U << 3)
#define ST25R3916_REG_RX_CONF2_agc_m         (1U << 2)
#define ST25R3916_REG_RX_CONF2_agc_alg       (1U << 1)
#define ST25R3916_REG_RX_CONF2_agc6_3        (1U << 0)

#define ST25R3916_REG_RX_CONF3_rg1_am2      (1U << 7)
#define ST25R3916_REG_RX_CONF3_rg1_am1      (1U << 6)
#define ST25R3916_REG_RX_CONF3_rg1_am0      (1U << 5)
#define ST25R3916_REG_RX_CONF3_rg1_am_mask  (0x7U << 5)
#define ST25R3916_REG_RX_CONF3_rg1_am_shift (5U)
#define ST25R3916_REG_RX_CONF3_rg1_pm2      (1U << 4)
#define ST25R3916_REG_RX_CONF3_rg1_pm1      (1U << 3)
#define ST25R3916_REG_RX_CONF3_rg1_pm0      (1U << 2)
#define ST25R3916_REG_RX_CONF3_rg1_pm_mask  (0x7U << 2)
#define ST25R3916_REG_RX_CONF3_rg1_pm_shift (2U)
#define ST25R3916_REG_RX_CONF3_lf_en        (1U << 1)
#define ST25R3916_REG_RX_CONF3_lf_op        (1U << 0)

#define ST25R3916_REG_RX_CONF4_rg2_am3      (1U << 7)
#define ST25R3916_REG_RX_CONF4_rg2_am2      (1U << 6)
#define ST25R3916_REG_RX_CONF4_rg2_am1      (1U << 5)
#define ST25R3916_REG_RX_CONF4_rg2_am0      (1U << 4)
#define ST25R3916_REG_RX_CONF4_rg2_am_mask  (0xfU << 4)
#define ST25R3916_REG_RX_CONF4_rg2_am_shift (4U)
#define ST25R3916_REG_RX_CONF4_rg2_pm3      (1U << 3)
#define ST25R3916_REG_RX_CONF4_rg2_pm2      (1U << 2)
#define ST25R3916_REG_RX_CONF4_rg2_pm1      (1U << 1)
#define ST25R3916_REG_RX_CONF4_rg2_pm0      (1U << 0)
#define ST25R3916_REG_RX_CONF4_rg2_pm_mask  (0xfU << 0)
#define ST25R3916_REG_RX_CONF4_rg2_pm_shift (0U)

#define ST25R3916_REG_P2P_RX_CONF_ook_fd   (1U << 7)
#define ST25R3916_REG_P2P_RX_CONF_ook_rc1  (1U << 6)
#define ST25R3916_REG_P2P_RX_CONF_ook_rc0  (1U << 5)
#define ST25R3916_REG_P2P_RX_CONF_ook_thd1 (1U << 4)
#define ST25R3916_REG_P2P_RX_CONF_ook_thd0 (1U << 3)
#define ST25R3916_REG_P2P_RX_CONF_ask_rc1  (1U << 2)
#define ST25R3916_REG_P2P_RX_CONF_ask_rc0  (1U << 1)
#define ST25R3916_REG_P2P_RX_CONF_ask_thd  (1U << 0)

#define ST25R3916_REG_CORR_CONF1_corr_s7 (1U << 7)
#define ST25R3916_REG_CORR_CONF1_corr_s6 (1U << 6)
#define ST25R3916_REG_CORR_CONF1_corr_s5 (1U << 5)
#define ST25R3916_REG_CORR_CONF1_corr_s4 (1U << 4)
#define ST25R3916_REG_CORR_CONF1_corr_s3 (1U << 3)
#define ST25R3916_REG_CORR_CONF1_corr_s2 (1U << 2)
#define ST25R3916_REG_CORR_CONF1_corr_s1 (1U << 1)
#define ST25R3916_REG_CORR_CONF1_corr_s0 (1U << 0)

#define ST25R3916_REG_CORR_CONF2_rfu5    (1U << 7)
#define ST25R3916_REG_CORR_CONF2_rfu4    (1U << 6)
#define ST25R3916_REG_CORR_CONF2_rfu3    (1U << 5)
#define ST25R3916_REG_CORR_CONF2_rfu2    (1U << 4)
#define ST25R3916_REG_CORR_CONF2_rfu1    (1U << 3)
#define ST25R3916_REG_CORR_CONF2_rfu0    (1U << 2)
#define ST25R3916_REG_CORR_CONF2_corr_s9 (1U << 1)
#define ST25R3916_REG_CORR_CONF2_corr_s8 (1U << 0)

#define ST25R3916_REG_TIMER_EMV_CONTROL_gptc2            (1U << 7)
#define ST25R3916_REG_TIMER_EMV_CONTROL_gptc1            (1U << 6)
#define ST25R3916_REG_TIMER_EMV_CONTROL_gptc0            (1U << 5)
#define ST25R3916_REG_TIMER_EMV_CONTROL_gptc_no_trigger  (0U << 5)
#define ST25R3916_REG_TIMER_EMV_CONTROL_gptc_erx         (1U << 5)
#define ST25R3916_REG_TIMER_EMV_CONTROL_gptc_srx         (2U << 5)
#define ST25R3916_REG_TIMER_EMV_CONTROL_gptc_etx_nfc     (3U << 5)
#define ST25R3916_REG_TIMER_EMV_CONTROL_gptc_mask        (7U << 5)
#define ST25R3916_REG_TIMER_EMV_CONTROL_gptc_shift       (5U)
#define ST25R3916_REG_TIMER_EMV_CONTROL_rfu              (1U << 4)
#define ST25R3916_REG_TIMER_EMV_CONTROL_mrt_step         (1U << 3)
#define ST25R3916_REG_TIMER_EMV_CONTROL_mrt_step_512     (1U << 3)
#define ST25R3916_REG_TIMER_EMV_CONTROL_mrt_step_64      (0U << 3)
#define ST25R3916_REG_TIMER_EMV_CONTROL_nrt_nfc          (1U << 2)
#define ST25R3916_REG_TIMER_EMV_CONTROL_nrt_nfc_on       (1U << 2)
#define ST25R3916_REG_TIMER_EMV_CONTROL_nrt_nfc_off      (0U << 2)
#define ST25R3916_REG_TIMER_EMV_CONTROL_nrt_emv          (1U << 1)
#define ST25R3916_REG_TIMER_EMV_CONTROL_nrt_emv_on       (1U << 1)
#define ST25R3916_REG_TIMER_EMV_CONTROL_nrt_emv_off      (0U << 1)
#define ST25R3916_REG_TIMER_EMV_CONTROL_nrt_step         (1U << 0)
#define ST25R3916_REG_TIMER_EMV_CONTROL_nrt_step_64fc    (0U << 0)
#define ST25R3916_REG_TIMER_EMV_CONTROL_nrt_step_4096_fc (1U << 0)

#define ST25R3916_REG_FIFO_STATUS2_fifo_b9       (1U << 7)
#define ST25R3916_REG_FIFO_STATUS2_fifo_b8       (1U << 6)
#define ST25R3916_REG_FIFO_STATUS2_fifo_b_mask   (3U << 6)
#define ST25R3916_REG_FIFO_STATUS2_fifo_b_shift  (6U)
#define ST25R3916_REG_FIFO_STATUS2_fifo_unf      (1U << 5)
#define ST25R3916_REG_FIFO_STATUS2_fifo_ovr      (1U << 4)
#define ST25R3916_REG_FIFO_STATUS2_fifo_lb2      (1U << 3)
#define ST25R3916_REG_FIFO_STATUS2_fifo_lb1      (1U << 2)
#define ST25R3916_REG_FIFO_STATUS2_fifo_lb0      (1U << 1)
#define ST25R3916_REG_FIFO_STATUS2_fifo_lb_mask  (7U << 1)
#define ST25R3916_REG_FIFO_STATUS2_fifo_lb_shift (1U)
#define ST25R3916_REG_FIFO_STATUS2_np_lb         (1U << 0)

#define ST25R3916_REG_COLLISION_STATUS_c_byte3      (1U << 7)
#define ST25R3916_REG_COLLISION_STATUS_c_byte2      (1U << 6)
#define ST25R3916_REG_COLLISION_STATUS_c_byte1      (1U << 5)
#define ST25R3916_REG_COLLISION_STATUS_c_byte0      (1U << 4)
#define ST25R3916_REG_COLLISION_STATUS_c_byte_mask  (0xfU << 4)
#define ST25R3916_REG_COLLISION_STATUS_c_byte_shift (4U)
#define ST25R3916_REG_COLLISION_STATUS_c_bit2       (1U << 3)
#define ST25R3916_REG_COLLISION_STATUS_c_bit1       (1U << 2)
#define ST25R3916_REG_COLLISION_STATUS_c_bit0       (1U << 1)
#define ST25R3916_REG_COLLISION_STATUS_c_pb         (1U << 0)
#define ST25R3916_REG_COLLISION_STATUS_c_bit_mask   (3U << 1)
#define ST25R3916_REG_COLLISION_STATUS_c_bit_shift  (1U)

#define ST25R3916_REG_PASSIVE_TARGET_STATUS_rfu               (1U << 7)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_rfu1              (1U << 6)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_rfu2              (1U << 5)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_rfu3              (1U << 4)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_state3        (1U << 3)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_state2        (1U << 2)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_state1        (1U << 1)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_state0        (1U << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_power_off  (0x0U << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_idle       (0x1U << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_ready_l1   (0x2U << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_ready_l2   (0x3U << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_rfu4       (0x4U << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_active     (0x5U << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_rfu6       (0x6U << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_rfu7       (0x7U << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_rfu8       (0x8U << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_halt       (0x9U << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_ready_l1_x (0xaU << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_ready_l2_x (0xbU << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_rfu12      (0xcU << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_active_x   (0xdU << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_state_mask    (0xfU << 0)
#define ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_state_shift   (0U)

#define ST25R3916_REG_NUM_TX_BYTES2_ntx4       (1U << 7)
#define ST25R3916_REG_NUM_TX_BYTES2_ntx3       (1U << 6)
#define ST25R3916_REG_NUM_TX_BYTES2_ntx2       (1U << 5)
#define ST25R3916_REG_NUM_TX_BYTES2_ntx1       (1U << 4)
#define ST25R3916_REG_NUM_TX_BYTES2_ntx0       (1U << 3)
#define ST25R3916_REG_NUM_TX_BYTES2_ntx_mask   (0x1fU << 3)
#define ST25R3916_REG_NUM_TX_BYTES2_ntx_shift  (3U)
#define ST25R3916_REG_NUM_TX_BYTES2_nbtx2      (1U << 2)
#define ST25R3916_REG_NUM_TX_BYTES2_nbtx1      (1U << 1)
#define ST25R3916_REG_NUM_TX_BYTES2_nbtx0      (1U << 0)
#define ST25R3916_REG_NUM_TX_BYTES2_nbtx_mask  (7U << 0)
#define ST25R3916_REG_NUM_TX_BYTES2_nbtx_shift (0U)

#define ST25R3916_REG_NFCIP1_BIT_RATE_nfc_rfu1       (1U << 7)
#define ST25R3916_REG_NFCIP1_BIT_RATE_nfc_rfu0       (1U << 6)
#define ST25R3916_REG_NFCIP1_BIT_RATE_nfc_rate1      (1U << 5)
#define ST25R3916_REG_NFCIP1_BIT_RATE_nfc_rate0      (1U << 4)
#define ST25R3916_REG_NFCIP1_BIT_RATE_nfc_rate_mask  (0x3U << 4)
#define ST25R3916_REG_NFCIP1_BIT_RATE_nfc_rate_shift (4U)
#define ST25R3916_REG_NFCIP1_BIT_RATE_ppt2_on        (1U << 3)
#define ST25R3916_REG_NFCIP1_BIT_RATE_gpt_on         (1U << 2)
#define ST25R3916_REG_NFCIP1_BIT_RATE_nrt_on         (1U << 1)
#define ST25R3916_REG_NFCIP1_BIT_RATE_mrt_on         (1U << 0)

#define ST25R3916_REG_TX_DRIVER_am_mod3          (1U << 7)
#define ST25R3916_REG_TX_DRIVER_am_mod2          (1U << 6)
#define ST25R3916_REG_TX_DRIVER_am_mod1          (1U << 5)
#define ST25R3916_REG_TX_DRIVER_am_mod0          (1U << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_5percent  (0x0U << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_6percent  (0x1U << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_7percent  (0x2U << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_8percent  (0x3U << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_9percent  (0x4U << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_10percent (0x5U << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_11percent (0x6U << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_12percent (0x7U << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_13percent (0x8U << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_14percent (0x9U << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_15percent (0xaU << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_17percent (0xbU << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_19percent (0xcU << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_22percent (0xdU << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_26percent (0xeU << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_40percent (0xfU << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_mask      (0xfU << 4)
#define ST25R3916_REG_TX_DRIVER_am_mod_shift     (4U)
#define ST25R3916_REG_TX_DRIVER_d_res3           (1U << 3)
#define ST25R3916_REG_TX_DRIVER_d_res2           (1U << 2)
#define ST25R3916_REG_TX_DRIVER_d_res1           (1U << 1)
#define ST25R3916_REG_TX_DRIVER_d_res0           (1U << 0)
#define ST25R3916_REG_TX_DRIVER_d_res_mask       (0xfU << 0)
#define ST25R3916_REG_TX_DRIVER_d_res_shift      (0U)

#define ST25R3916_REG_PT_MOD_ptm_res3      (1U << 7)
#define ST25R3916_REG_PT_MOD_ptm_res2      (1U << 6)
#define ST25R3916_REG_PT_MOD_ptm_res1      (1U << 5)
#define ST25R3916_REG_PT_MOD_ptm_res0      (1U << 4)
#define ST25R3916_REG_PT_MOD_ptm_res_mask  (0xfU << 4)
#define ST25R3916_REG_PT_MOD_ptm_res_shift (4U)
#define ST25R3916_REG_PT_MOD_pt_res3       (1U << 3)
#define ST25R3916_REG_PT_MOD_pt_res2       (1U << 2)
#define ST25R3916_REG_PT_MOD_pt_res1       (1U << 1)
#define ST25R3916_REG_PT_MOD_pt_res0       (1U << 0)
#define ST25R3916_REG_PT_MOD_pt_res_mask   (0xfU << 0)
#define ST25R3916_REG_PT_MOD_pt_res_shift  (0U)

#define ST25R3916_REG_AUX_MOD_dis_reg_am (1U << 7)
#define ST25R3916_REG_AUX_MOD_lm_ext_pol (1U << 6)
#define ST25R3916_REG_AUX_MOD_lm_ext     (1U << 5)
#define ST25R3916_REG_AUX_MOD_lm_dri     (1U << 4)
#define ST25R3916_REG_AUX_MOD_res_am     (1U << 3)
#define ST25R3916_REG_AUX_MOD_rfu2       (1U << 2)
#define ST25R3916_REG_AUX_MOD_rfu1       (1U << 1)
#define ST25R3916_REG_AUX_MOD_rfu0       (1U << 0)

#define ST25R3916_REG_TX_DRIVER_TIMING_d_rat_t3      (1U << 7)
#define ST25R3916_REG_TX_DRIVER_TIMING_d_rat_t2      (1U << 6)
#define ST25R3916_REG_TX_DRIVER_TIMING_d_rat_t1      (1U << 5)
#define ST25R3916_REG_TX_DRIVER_TIMING_d_rat_t0      (1U << 4)
#define ST25R3916_REG_TX_DRIVER_TIMING_d_rat_mask    (0xfU << 4)
#define ST25R3916_REG_TX_DRIVER_TIMING_d_rat_shift   (4U)
#define ST25R3916_REG_TX_DRIVER_TIMING_rfu           (1U << 3)
#define ST25R3916_REG_TX_DRIVER_TIMING_d_tim_m2      (1U << 2)
#define ST25R3916_REG_TX_DRIVER_TIMING_d_tim_m1      (1U << 1)
#define ST25R3916_REG_TX_DRIVER_TIMING_d_tim_m0      (1U << 0)
#define ST25R3916_REG_TX_DRIVER_TIMING_d_tim_m_mask  (0x7U << 0)
#define ST25R3916_REG_TX_DRIVER_TIMING_d_tim_m_shift (0U)

#define ST25R3916_REG_RES_AM_MOD_fa3_f        (1U << 7)
#define ST25R3916_REG_RES_AM_MOD_md_res6      (1U << 6)
#define ST25R3916_REG_RES_AM_MOD_md_res5      (1U << 5)
#define ST25R3916_REG_RES_AM_MOD_md_res4      (1U << 4)
#define ST25R3916_REG_RES_AM_MOD_md_res3      (1U << 3)
#define ST25R3916_REG_RES_AM_MOD_md_res2      (1U << 2)
#define ST25R3916_REG_RES_AM_MOD_md_res1      (1U << 1)
#define ST25R3916_REG_RES_AM_MOD_md_res0      (1U << 0)
#define ST25R3916_REG_RES_AM_MOD_md_res_mask  (0x7FU << 0)
#define ST25R3916_REG_RES_AM_MOD_md_res_shift (0U)

#define ST25R3916_REG_TX_DRIVER_STATUS_d_rat_r3    (1U << 7)
#define ST25R3916_REG_TX_DRIVER_STATUS_d_rat_r2    (1U << 6)
#define ST25R3916_REG_TX_DRIVER_STATUS_d_rat_r1    (1U << 5)
#define ST25R3916_REG_TX_DRIVER_STATUS_d_rat_r0    (1U << 4)
#define ST25R3916_REG_TX_DRIVER_STATUS_d_rat_mask  (0xfU << 4)
#define ST25R3916_REG_TX_DRIVER_STATUS_d_rat_shift (4U)
#define ST25R3916_REG_TX_DRIVER_STATUS_rfu         (1U << 3)
#define ST25R3916_REG_TX_DRIVER_STATUS_d_tim_r2    (1U << 2)
#define ST25R3916_REG_TX_DRIVER_STATUS_d_tim_r1    (1U << 1)
#define ST25R3916_REG_TX_DRIVER_STATUS_d_tim_r0    (1U << 0)
#define ST25R3916_REG_TX_DRIVER_STATUS_d_tim_mask  (0x7U << 0)
#define ST25R3916_REG_TX_DRIVER_STATUS_d_tim_shift (0U)

#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_l2a   (1U << 6)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_l1a   (1U << 5)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_l0a   (1U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_75mV  (0x0U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_105mV (0x1U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_150mV (0x2U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_205mV (0x3U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_290mV (0x4U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_400mV (0x5U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_560mV (0x6U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_800mV (0x7U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_mask  (7U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_shift (4U)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_t3a   (1U << 3)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_t2a   (1U << 2)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_t1a   (1U << 1)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_t0a   (1U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_75mV  (0x0U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_105mV (0x1U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_150mV (0x2U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_205mV (0x3U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_290mV (0x4U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_400mV (0x5U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_560mV (0x6U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_800mV (0x7U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_25mV  (0x8U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_33mV  (0x9U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_47mV  (0xAU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_64mV  (0xBU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_90mV  (0xCU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_125mV (0xDU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_175mV (0xEU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_250mV (0xFU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_mask  (0xfU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_shift (0U)

#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_l2d   (1U << 6)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_l1d   (1U << 5)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_l0d   (1U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_75mV  (0x0U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_105mV (0x1U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_150mV (0x2U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_205mV (0x3U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_290mV (0x4U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_400mV (0x5U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_560mV (0x6U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_800mV (0x7U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_mask  (7U << 4)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_shift (4U)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_t3d   (1U << 3)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_t2d   (1U << 2)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_t1d   (1U << 1)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_t0d   (1U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_75mV  (0x0U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_105mV (0x1U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_150mV (0x2U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_205mV (0x3U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_290mV (0x4U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_400mV (0x5U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_560mV (0x6U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_800mV (0x7U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_25mV  (0x8U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_33mV  (0x9U << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_47mV  (0xAU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_64mV  (0xBU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_90mV  (0xCU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_125mV (0xDU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_175mV (0xEU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_250mV (0xFU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_mask  (0xfU << 0)
#define ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_shift (0U)

#define ST25R3916_REG_REGULATOR_CONTROL_reg_s       (1U << 7)
#define ST25R3916_REG_REGULATOR_CONTROL_rege_3      (1U << 6)
#define ST25R3916_REG_REGULATOR_CONTROL_rege_2      (1U << 5)
#define ST25R3916_REG_REGULATOR_CONTROL_rege_1      (1U << 4)
#define ST25R3916_REG_REGULATOR_CONTROL_rege_0      (1U << 3)
#define ST25R3916_REG_REGULATOR_CONTROL_rege_mask   (0xfU << 3)
#define ST25R3916_REG_REGULATOR_CONTROL_rege_shift  (3U)
#define ST25R3916_REG_REGULATOR_CONTROL_mpsv2       (2U << 2)
#define ST25R3916_REG_REGULATOR_CONTROL_mpsv1       (1U << 1)
#define ST25R3916_REG_REGULATOR_CONTROL_mpsv0       (1U << 0)
#define ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd    (0U)
#define ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd_a  (1U)
#define ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd_d  (2U)
#define ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd_rf (3U)
#define ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd_am (4U)
#define ST25R3916_REG_REGULATOR_CONTROL_rfu         (5U)
#define ST25R3916_REG_REGULATOR_CONTROL_rfu1        (6U)
#define ST25R3916_REG_REGULATOR_CONTROL_rfu2        (7U)
#define ST25R3916_REG_REGULATOR_CONTROL_mpsv_mask   (7U)
#define ST25R3916_REG_REGULATOR_CONTROL_mpsv_shift  (0U)

#define ST25R3916_REG_REGULATOR_RESULT_reg_3     (1U << 7)
#define ST25R3916_REG_REGULATOR_RESULT_reg_2     (1U << 6)
#define ST25R3916_REG_REGULATOR_RESULT_reg_1     (1U << 5)
#define ST25R3916_REG_REGULATOR_RESULT_reg_0     (1U << 4)
#define ST25R3916_REG_REGULATOR_RESULT_reg_mask  (0xfU << 4)
#define ST25R3916_REG_REGULATOR_RESULT_reg_shift (4U)
#define ST25R3916_REG_REGULATOR_RESULT_i_lim     (1U << 0)

#define ST25R3916_REG_RSSI_RESULT_rssi_am_3     (1U << 7)
#define ST25R3916_REG_RSSI_RESULT_rssi_am_2     (1U << 6)
#define ST25R3916_REG_RSSI_RESULT_rssi_am_1     (1U << 5)
#define ST25R3916_REG_RSSI_RESULT_rssi_am_0     (1U << 4)
#define ST25R3916_REG_RSSI_RESULT_rssi_am_mask  (0xfU << 4)
#define ST25R3916_REG_RSSI_RESULT_rssi_am_shift (4U)
#define ST25R3916_REG_RSSI_RESULT_rssi_pm3      (1U << 3)
#define ST25R3916_REG_RSSI_RESULT_rssi_pm2      (1U << 2)
#define ST25R3916_REG_RSSI_RESULT_rssi_pm1      (1U << 1)
#define ST25R3916_REG_RSSI_RESULT_rssi_pm0      (1U << 0)
#define ST25R3916_REG_RSSI_RESULT_rssi_pm_mask  (0xfU << 0)
#define ST25R3916_REG_RSSI_RESULT_rssi_pm_shift (0U)

#define ST25R3916_REG_GAIN_RED_STATE_gs_am_3     (1U << 7)
#define ST25R3916_REG_GAIN_RED_STATE_gs_am_2     (1U << 6)
#define ST25R3916_REG_GAIN_RED_STATE_gs_am_1     (1U << 5)
#define ST25R3916_REG_GAIN_RED_STATE_gs_am_0     (1U << 4)
#define ST25R3916_REG_GAIN_RED_STATE_gs_am_mask  (0xfU << 4)
#define ST25R3916_REG_GAIN_RED_STATE_gs_am_shift (4U)
#define ST25R3916_REG_GAIN_RED_STATE_gs_pm_3     (1U << 3)
#define ST25R3916_REG_GAIN_RED_STATE_gs_pm_2     (1U << 2)
#define ST25R3916_REG_GAIN_RED_STATE_gs_pm_1     (1U << 1)
#define ST25R3916_REG_GAIN_RED_STATE_gs_pm_0     (1U << 0)
#define ST25R3916_REG_GAIN_RED_STATE_gs_pm_mask  (0xfU << 0)
#define ST25R3916_REG_GAIN_RED_STATE_gs_pm_shift (0U)

#define ST25R3916_REG_CAP_SENSOR_CONTROL_cs_mcal4      (1U << 7)
#define ST25R3916_REG_CAP_SENSOR_CONTROL_cs_mcal3      (1U << 6)
#define ST25R3916_REG_CAP_SENSOR_CONTROL_cs_mcal2      (1U << 5)
#define ST25R3916_REG_CAP_SENSOR_CONTROL_cs_mcal1      (1U << 4)
#define ST25R3916_REG_CAP_SENSOR_CONTROL_cs_mcal0      (1U << 3)
#define ST25R3916_REG_CAP_SENSOR_CONTROL_cs_mcal_mask  (0x1fU << 3)
#define ST25R3916_REG_CAP_SENSOR_CONTROL_cs_mcal_shift (3U)
#define ST25R3916_REG_CAP_SENSOR_CONTROL_cs_g2         (1U << 2)
#define ST25R3916_REG_CAP_SENSOR_CONTROL_cs_g1         (1U << 1)
#define ST25R3916_REG_CAP_SENSOR_CONTROL_cs_g0         (1U << 0)
#define ST25R3916_REG_CAP_SENSOR_CONTROL_cs_g_mask     (7U << 0)
#define ST25R3916_REG_CAP_SENSOR_CONTROL_cs_g_shift    (0U)

#define ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal4      (1U << 7)
#define ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal3      (1U << 6)
#define ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal2      (1U << 5)
#define ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal1      (1U << 4)
#define ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal0      (1U << 3)
#define ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_mask  (0x1fU << 3)
#define ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_shift (3U)
#define ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_end   (1U << 2)
#define ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_err   (1U << 1)

#define ST25R3916_REG_AUX_DISPLAY_a_cha   (1U << 7)
#define ST25R3916_REG_AUX_DISPLAY_efd_o   (1U << 6)
#define ST25R3916_REG_AUX_DISPLAY_tx_on   (1U << 5)
#define ST25R3916_REG_AUX_DISPLAY_osc_ok  (1U << 4)
#define ST25R3916_REG_AUX_DISPLAY_rx_on   (1U << 3)
#define ST25R3916_REG_AUX_DISPLAY_rx_act  (1U << 2)
#define ST25R3916_REG_AUX_DISPLAY_en_peer (1U << 1)
#define ST25R3916_REG_AUX_DISPLAY_en_ac   (1U << 0)

#define ST25R3916_REG_OVERSHOOT_CONF1_ov_tx_mode1  (1U << 7)
#define ST25R3916_REG_OVERSHOOT_CONF1_ov_tx_mode0  (1U << 6)
#define ST25R3916_REG_OVERSHOOT_CONF1_ov_pattern13 (1U << 5)
#define ST25R3916_REG_OVERSHOOT_CONF1_ov_pattern12 (1U << 4)
#define ST25R3916_REG_OVERSHOOT_CONF1_ov_pattern11 (1U << 3)
#define ST25R3916_REG_OVERSHOOT_CONF1_ov_pattern10 (1U << 2)
#define ST25R3916_REG_OVERSHOOT_CONF1_ov_pattern9  (1U << 1)
#define ST25R3916_REG_OVERSHOOT_CONF1_ov_pattern8  (1U << 0)

#define ST25R3916_REG_OVERSHOOT_CONF2_ov_pattern7 (1U << 7)
#define ST25R3916_REG_OVERSHOOT_CONF2_ov_pattern6 (1U << 6)
#define ST25R3916_REG_OVERSHOOT_CONF2_ov_pattern5 (1U << 5)
#define ST25R3916_REG_OVERSHOOT_CONF2_ov_pattern4 (1U << 4)
#define ST25R3916_REG_OVERSHOOT_CONF2_ov_pattern3 (1U << 3)
#define ST25R3916_REG_OVERSHOOT_CONF2_ov_pattern2 (1U << 2)
#define ST25R3916_REG_OVERSHOOT_CONF2_ov_pattern1 (1U << 1)
#define ST25R3916_REG_OVERSHOOT_CONF2_ov_pattern0 (1U << 0)

#define ST25R3916_REG_UNDERSHOOT_CONF1_un_tx_mode1  (1U << 7)
#define ST25R3916_REG_UNDERSHOOT_CONF1_un_tx_mode0  (1U << 6)
#define ST25R3916_REG_UNDERSHOOT_CONF1_un_pattern13 (1U << 5)
#define ST25R3916_REG_UNDERSHOOT_CONF1_un_pattern12 (1U << 4)
#define ST25R3916_REG_UNDERSHOOT_CONF1_un_pattern11 (1U << 3)
#define ST25R3916_REG_UNDERSHOOT_CONF1_un_pattern10 (1U << 2)
#define ST25R3916_REG_UNDERSHOOT_CONF1_un_pattern9  (1U << 1)
#define ST25R3916_REG_UNDERSHOOT_CONF1_un_pattern8  (1U << 0)

#define ST25R3916_REG_UNDERSHOOT_CONF2_un_pattern7 (1U << 7)
#define ST25R3916_REG_UNDERSHOOT_CONF2_un_pattern6 (1U << 6)
#define ST25R3916_REG_UNDERSHOOT_CONF2_un_pattern5 (1U << 5)
#define ST25R3916_REG_UNDERSHOOT_CONF2_un_pattern4 (1U << 4)
#define ST25R3916_REG_UNDERSHOOT_CONF2_un_pattern3 (1U << 3)
#define ST25R3916_REG_UNDERSHOOT_CONF2_un_pattern2 (1U << 2)
#define ST25R3916_REG_UNDERSHOOT_CONF2_un_pattern1 (1U << 1)
#define ST25R3916_REG_UNDERSHOOT_CONF2_un_pattern0 (1U << 0)

#define ST25R3916_REG_WUP_TIMER_CONTROL_wur       (1U << 7)
#define ST25R3916_REG_WUP_TIMER_CONTROL_wut2      (1U << 6)
#define ST25R3916_REG_WUP_TIMER_CONTROL_wut1      (1U << 5)
#define ST25R3916_REG_WUP_TIMER_CONTROL_wut0      (1U << 4)
#define ST25R3916_REG_WUP_TIMER_CONTROL_wut_mask  (7U << 4)
#define ST25R3916_REG_WUP_TIMER_CONTROL_wut_shift (4U)
#define ST25R3916_REG_WUP_TIMER_CONTROL_wto       (1U << 3)
#define ST25R3916_REG_WUP_TIMER_CONTROL_wam       (1U << 2)
#define ST25R3916_REG_WUP_TIMER_CONTROL_wph       (1U << 1)
#define ST25R3916_REG_WUP_TIMER_CONTROL_wcap      (1U << 0)

#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_d3        (1U << 7)
#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_d2        (1U << 6)
#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_d1        (1U << 5)
#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_d0        (1U << 4)
#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_d_mask    (0xfU << 4)
#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_d_shift   (4U)
#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_aam       (1U << 3)
#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_aew1      (1U << 2)
#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_aew0      (1U << 1)
#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_aew_mask  (0x3U << 1)
#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_aew_shift (1U)
#define ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_ae        (1U << 0)

#define ST25R3916_REG_PHASE_MEASURE_CONF_pm_d3        (1U << 7)
#define ST25R3916_REG_PHASE_MEASURE_CONF_pm_d2        (1U << 6)
#define ST25R3916_REG_PHASE_MEASURE_CONF_pm_d1        (1U << 5)
#define ST25R3916_REG_PHASE_MEASURE_CONF_pm_d0        (1U << 4)
#define ST25R3916_REG_PHASE_MEASURE_CONF_pm_d_mask    (0xfU << 4)
#define ST25R3916_REG_PHASE_MEASURE_CONF_pm_d_shift   (4U)
#define ST25R3916_REG_PHASE_MEASURE_CONF_pm_aam       (1U << 3)
#define ST25R3916_REG_PHASE_MEASURE_CONF_pm_aew1      (1U << 2)
#define ST25R3916_REG_PHASE_MEASURE_CONF_pm_aew0      (1U << 1)
#define ST25R3916_REG_PHASE_MEASURE_CONF_pm_aew_mask  (0x3U << 1)
#define ST25R3916_REG_PHASE_MEASURE_CONF_pm_aew_shift (1U)
#define ST25R3916_REG_PHASE_MEASURE_CONF_pm_ae        (1U << 0)

#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_d3        (1U << 7)
#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_d2        (1U << 6)
#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_d1        (1U << 5)
#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_d0        (1U << 4)
#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_d_mask    (0xfU << 4)
#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_d_shift   (4U)
#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_aam       (1U << 3)
#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_aew1      (1U << 2)
#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_aew0      (1U << 1)
#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_aew_mask  (0x3U << 1)
#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_aew_shift (1U)
#define ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_ae        (1U << 0)

#define ST25R3916_REG_IC_IDENTITY_ic_type4          (1U << 7)
#define ST25R3916_REG_IC_IDENTITY_ic_type3          (1U << 6)
#define ST25R3916_REG_IC_IDENTITY_ic_type2          (1U << 5)
#define ST25R3916_REG_IC_IDENTITY_ic_type1          (1U << 4)
#define ST25R3916_REG_IC_IDENTITY_ic_type0          (1U << 3)
#define ST25R3916_REG_IC_IDENTITY_ic_type_st25r3916 (5U << 3)
#define ST25R3916_REG_IC_IDENTITY_ic_type_mask      (0x1fU << 3)
#define ST25R3916_REG_IC_IDENTITY_ic_type_shift     (3U)
#define ST25R3916_REG_IC_IDENTITY_ic_rev2           (1U << 2)
#define ST25R3916_REG_IC_IDENTITY_ic_rev1           (1U << 1)
#define ST25R3916_REG_IC_IDENTITY_ic_rev0           (1U << 0)
#define ST25R3916_REG_IC_IDENTITY_ic_rev_v0         (0U << 0)
#define ST25R3916_REG_IC_IDENTITY_ic_rev_mask       (7U << 0)
#define ST25R3916_REG_IC_IDENTITY_ic_rev_shift      (0U)

/** Read register
 *
 * @param   handle      - pointer t FuriHalSpiBusHandle instance
 * @param   reg         - register address
 * @param   val         - pointer to the variable to store the read value
 */
void st25r3916_read_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t* val);

/** Read multiple registers
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   reg_start   - start register address
 * @param   values      - pointer to the buffer to store the read values
 * @param   length      - number of registers to read
 */
void st25r3916_read_burst_regs(
    FuriHalSpiBusHandle* handle,
    uint8_t reg_start,
    uint8_t* values,
    uint8_t length);

/** Write register
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   reg         - register address
 * @param   val         - value to write
 */
void st25r3916_write_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t val);

/** Write multiple registers
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   reg_start   - start register address
 * @param   values      - pointer to buffer to write
 * @param   length      - number of registers to write
 */
void st25r3916_write_burst_regs(
    FuriHalSpiBusHandle* handle,
    uint8_t reg_start,
    const uint8_t* values,
    uint8_t length);

/** Write fifo register
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   buff        - buffer to write to FIFO
 * @param   length      - number of bytes to write
 */
void st25r3916_reg_write_fifo(FuriHalSpiBusHandle* handle, const uint8_t* buff, size_t length);

/** Read fifo register
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   buff        - buffer to store the read values
 * @param   length      - number of bytes to read
 */
void st25r3916_reg_read_fifo(FuriHalSpiBusHandle* handle, uint8_t* buff, size_t length);

/** Write PTA memory register
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   values      - pointer to buffer to write
 * @param   length      - number of bytes to write
 */
void st25r3916_write_pta_mem(FuriHalSpiBusHandle* handle, const uint8_t* values, size_t length);

/** Read PTA memory register
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   values      - buffer to store the read values
 * @param   length      - number of bytes to read
 */
void st25r3916_read_pta_mem(FuriHalSpiBusHandle* handle, uint8_t* values, size_t length);

/** Write PTF memory register
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   values      - pointer to buffer to write
 * @param   length      - number of bytes to write
 */
void st25r3916_write_ptf_mem(FuriHalSpiBusHandle* handle, const uint8_t* values, size_t length);

/** Read PTTSN memory register
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   values      - pointer to buffer to write
 * @param   length      - number of bytes to write
 */
void st25r3916_write_pttsn_mem(FuriHalSpiBusHandle* handle, uint8_t* values, size_t length);

/** Send Direct command
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   cmd         - direct command
 */
void st25r3916_direct_cmd(FuriHalSpiBusHandle* handle, uint8_t cmd);

/** Read test register
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   reg         - register address
 * @param   val         - pointer to the variable to store the read value
 */
void st25r3916_read_test_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t* val);

/** Write test register
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   reg         - register address
 * @param   val         - value to write
 */
void st25r3916_write_test_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t val);

/** Clear register bits
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   reg         - register address
 * @param   clr_mask    - bit mask to clear
 */
void st25r3916_clear_reg_bits(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t clr_mask);

/** Set register bits
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   reg         - register address
 * @param   set_mask    - bit mask to set
 */
void st25r3916_set_reg_bits(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t set_mask);

/** Change register bits
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   reg         - register address
 * @param   mask        - bit mask to change
 * @param   value       - new register value to write
 */
void st25r3916_change_reg_bits(
    FuriHalSpiBusHandle* handle,
    uint8_t reg,
    uint8_t mask,
    uint8_t value);

/** Modify register
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   reg         - register address
 * @param   clr_mask    - bit mask to clear
 * @param   set_mask    - bit mask to set
 */
void st25r3916_modify_reg(
    FuriHalSpiBusHandle* handle,
    uint8_t reg,
    uint8_t clr_mask,
    uint8_t set_mask);

/** Change test register bits
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   reg         - register address
 * @param   mask        - bit mask to change
 * @param   value       - new register value to write
 */
void st25r3916_change_test_reg_bits(
    FuriHalSpiBusHandle* handle,
    uint8_t reg,
    uint8_t mask,
    uint8_t value);

/** Check register
 *
 * @param   handle      - pointer to FuriHalSpiBusHandle instance
 * @param   reg         - register address
 * @param   mask        - bit mask to check
 * @param   val         - expected register value
 *
 * @return  true if register value matches the expected value, false otherwise
 */
bool st25r3916_check_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t mask, uint8_t val);

#ifdef __cplusplus
}
#endif
