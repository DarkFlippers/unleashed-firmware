
/******************************************************************************
  * \attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        www.st.com/myliberty
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/

/*
 *      PROJECT:   ST25R3916 firmware
 *      Revision: 
 *      LANGUAGE:  ISO C99
 */

/*! \file
 *
 *  \author Gustavo Patricio
 *
 *  \brief ST25R3916 high level interface
 *
 *
 * \addtogroup RFAL
 * @{
 *
 * \addtogroup RFAL-HAL
 * \brief RFAL Hardware Abstraction Layer
 * @{
 *
 * \addtogroup ST25R3916
 * \brief RFAL ST25R3916 Driver
 * @{
 * 
 * \addtogroup ST25R3916_Driver
 * \brief RFAL ST25R3916 Driver
 * @{
 * 
 */

#ifndef ST25R3916_H
#define ST25R3916_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"
#include "st_errno.h"
#include "st25r3916_com.h"

/*
******************************************************************************
* GLOBAL DATATYPES
******************************************************************************
*/

/*! Struct to represent all regs on ST25R3916                                                             */
typedef struct {
    uint8_t RsA[(
        ST25R3916_REG_IC_IDENTITY + 1U)]; /*!< Registers contained on ST25R3916 space A (Rs-A)     */
    uint8_t
        RsB[ST25R3916_SPACE_B_REG_LEN]; /*!< Registers contained on ST25R3916 space B (Rs-B)     */
} t_st25r3916Regs;

/*! Parameters how the stream mode should work                                                            */
struct st25r3916StreamConfig {
    uint8_t useBPSK; /*!< 0: subcarrier, 1:BPSK                                */
    uint8_t din; /*!< Divider for the in subcarrier frequency: fc/2^din    */
    uint8_t dout; /*!< Divider for the in subcarrier frequency fc/2^dout    */
    uint8_t report_period_length; /*!< Length of the reporting period 2^report_period_length*/
};

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

/* ST25R3916 direct commands */
#define ST25R3916_CMD_SET_DEFAULT \
    0xC1U /*!< Puts the chip in default state (same as after power-up) */
#define ST25R3916_CMD_STOP 0xC2U /*!< Stops all activities and clears FIFO                    */
#define ST25R3916_CMD_TRANSMIT_WITH_CRC \
    0xC4U /*!< Transmit with CRC                                       */
#define ST25R3916_CMD_TRANSMIT_WITHOUT_CRC \
    0xC5U /*!< Transmit without CRC                                    */
#define ST25R3916_CMD_TRANSMIT_REQA \
    0xC6U /*!< Transmit REQA                                           */
#define ST25R3916_CMD_TRANSMIT_WUPA \
    0xC7U /*!< Transmit WUPA                                           */
#define ST25R3916_CMD_INITIAL_RF_COLLISION \
    0xC8U /*!< NFC transmit with Initial RF Collision Avoidance        */
#define ST25R3916_CMD_RESPONSE_RF_COLLISION_N \
    0xC9U /*!< NFC transmit with Response RF Collision Avoidance       */
#define ST25R3916_CMD_GOTO_SENSE \
    0xCDU /*!< Passive target logic to Sense/Idle state                */
#define ST25R3916_CMD_GOTO_SLEEP \
    0xCEU /*!< Passive target logic to Sleep/Halt state                */
#define ST25R3916_CMD_MASK_RECEIVE_DATA \
    0xD0U /*!< Mask receive data                                       */
#define ST25R3916_CMD_UNMASK_RECEIVE_DATA \
    0xD1U /*!< Unmask receive data                                     */
#define ST25R3916_CMD_AM_MOD_STATE_CHANGE \
    0xD2U /*!< AM Modulation state change                              */
#define ST25R3916_CMD_MEASURE_AMPLITUDE \
    0xD3U /*!< Measure signal amplitude on RFI inputs                  */
#define ST25R3916_CMD_RESET_RXGAIN \
    0xD5U /*!< Reset RX Gain                                           */
#define ST25R3916_CMD_ADJUST_REGULATORS \
    0xD6U /*!< Adjust regulators                                       */
#define ST25R3916_CMD_CALIBRATE_DRIVER_TIMING \
    0xD8U /*!< Starts the sequence to adjust the driver timing         */
#define ST25R3916_CMD_MEASURE_PHASE \
    0xD9U /*!< Measure phase between RFO and RFI signal                */
#define ST25R3916_CMD_CLEAR_RSSI \
    0xDAU /*!< Clear RSSI bits and restart the measurement             */
#define ST25R3916_CMD_CLEAR_FIFO \
    0xDBU /*!< Clears FIFO, Collision and IRQ status                   */
#define ST25R3916_CMD_TRANSPARENT_MODE \
    0xDCU /*!< Transparent mode                                        */
#define ST25R3916_CMD_CALIBRATE_C_SENSOR \
    0xDDU /*!< Calibrate the capacitive sensor                         */
#define ST25R3916_CMD_MEASURE_CAPACITANCE \
    0xDEU /*!< Measure capacitance                                     */
#define ST25R3916_CMD_MEASURE_VDD \
    0xDFU /*!< Measure power supply voltage                            */
#define ST25R3916_CMD_START_GP_TIMER \
    0xE0U /*!< Start the general purpose timer                         */
#define ST25R3916_CMD_START_WUP_TIMER \
    0xE1U /*!< Start the wake-up timer                                 */
#define ST25R3916_CMD_START_MASK_RECEIVE_TIMER \
    0xE2U /*!< Start the mask-receive timer                            */
#define ST25R3916_CMD_START_NO_RESPONSE_TIMER \
    0xE3U /*!< Start the no-response timer                             */
#define ST25R3916_CMD_START_PPON2_TIMER \
    0xE4U /*!< Start PPon2 timer                                       */
#define ST25R3916_CMD_STOP_NRT \
    0xE8U /*!< Stop No Response Timer                                  */
#define ST25R3916_CMD_SPACE_B_ACCESS \
    0xFBU /*!< Enable R/W access to the test registers                 */
#define ST25R3916_CMD_TEST_ACCESS \
    0xFCU /*!< Enable R/W access to the test registers                 */

#define ST25R3916_THRESHOLD_DO_NOT_SET \
    0xFFU /*!< Indicates not to change this Threshold                  */

#define ST25R3916_BR_DO_NOT_SET \
    0xFFU /*!< Indicates not to change this Bit Rate                   */
#define ST25R3916_BR_106 0x00U /*!< ST25R3916 Bit Rate  106 kbit/s (fc/128)                 */
#define ST25R3916_BR_212 0x01U /*!< ST25R3916 Bit Rate  212 kbit/s (fc/64)                  */
#define ST25R3916_BR_424 0x02U /*!< ST25R3916 Bit Rate  424 kbit/s (fc/32)                  */
#define ST25R3916_BR_848 0x03U /*!< ST25R3916 Bit Rate  848 kbit/s (fc/16)                  */
#define ST25R3916_BR_1695 0x04U /*!< ST25R3916 Bit Rate 1696 kbit/s (fc/8)                   */
#define ST25R3916_BR_3390 0x05U /*!< ST25R3916 Bit Rate 3390 kbit/s (fc/4)                   */
#define ST25R3916_BR_6780 0x07U /*!< ST25R3916 Bit Rate 6780 kbit/s (fc/2)                   */

#define ST25R3916_FIFO_DEPTH 512U /*!< Depth of FIFO                                           */
#define ST25R3916_TOUT_OSC_STABLE \
    10U /*!< Max timeout for Oscillator to get stable      DS: 700us */

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

/*! Enables the Transmitter (Field On) and Receiver                                          */
#define st25r3916TxRxOn()         \
    st25r3916SetRegisterBits(     \
        ST25R3916_REG_OP_CONTROL, \
        (ST25R3916_REG_OP_CONTROL_rx_en | ST25R3916_REG_OP_CONTROL_tx_en))

/*! Disables the Transmitter (Field Off) and Receiver                                         */
#define st25r3916TxRxOff()        \
    st25r3916ClrRegisterBits(     \
        ST25R3916_REG_OP_CONTROL, \
        (ST25R3916_REG_OP_CONTROL_rx_en | ST25R3916_REG_OP_CONTROL_tx_en))

/*! Disables the Transmitter (Field Off)                                         */
#define st25r3916TxOff() \
    st25r3916ClrRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_tx_en)

/*! Checks if General Purpose Timer is still running by reading gpt_on flag                  */
#define st25r3916IsGPTRunning()               \
    st25r3916CheckReg(                        \
        ST25R3916_REG_NFCIP1_BIT_RATE,        \
        ST25R3916_REG_NFCIP1_BIT_RATE_gpt_on, \
        ST25R3916_REG_NFCIP1_BIT_RATE_gpt_on)

/*! Checks if External Filed is detected by reading ST25R3916 External Field Detector output    */
#define st25r3916IsExtFieldOn()          \
    st25r3916CheckReg(                   \
        ST25R3916_REG_AUX_DISPLAY,       \
        ST25R3916_REG_AUX_DISPLAY_efd_o, \
        ST25R3916_REG_AUX_DISPLAY_efd_o)

/*! Checks if Transmitter is enabled (Field On) */
#define st25r3916IsTxEnabled() \
    st25r3916CheckReg(         \
        ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_tx_en, ST25R3916_REG_OP_CONTROL_tx_en)

/*! Checks if NRT is in EMV mode */
#define st25r3916IsNRTinEMV()                    \
    st25r3916CheckReg(                           \
        ST25R3916_REG_TIMER_EMV_CONTROL,         \
        ST25R3916_REG_TIMER_EMV_CONTROL_nrt_emv, \
        ST25R3916_REG_TIMER_EMV_CONTROL_nrt_emv_on)

/*! Checks if last FIFO byte is complete */
#define st25r3916IsLastFIFOComplete() \
    st25r3916CheckReg(ST25R3916_REG_FIFO_STATUS2, ST25R3916_REG_FIFO_STATUS2_fifo_lb_mask, 0)

/*! Checks if the Oscillator is enabled  */
#define st25r3916IsOscOn() \
    st25r3916CheckReg(     \
        ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en, ST25R3916_REG_OP_CONTROL_en)

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*! 
 *****************************************************************************
 *  \brief  Initialise ST25R3916 driver
 *
 *  This function initialises the ST25R3916 driver.
 *
 *  \return ERR_NONE         : Operation successful
 *  \return ERR_HW_MISMATCH  : Expected HW do not match or communication error
 *  \return ERR_IO           : Error during communication selftest. Check communication interface
 *  \return ERR_TIMEOUT      : Timeout during IRQ selftest. Check IRQ handling
 *  \return ERR_SYSTEM       : Failure during oscillator activation or timer error 
 *
 *****************************************************************************
 */
ReturnCode st25r3916Initialize(void);

/*! 
 *****************************************************************************
 *  \brief  Deinitialize ST25R3916 driver
 *
 *  Calling this function deinitializes the ST25R3916 driver.
 *
 *****************************************************************************
 */
void st25r3916Deinitialize(void);

/*! 
 *****************************************************************************
 *  \brief  Turn on Oscillator and Regulator
 *  
 *  This function turn on oscillator and regulator and waits for the 
 *  oscillator to become stable
 * 
 *  \return ERR_SYSTEM : Failure dusring Oscillator activation
 *  \return ERR_NONE   : No error, Oscillator is active and stable, Regulator is on
 *
 *****************************************************************************
 */
ReturnCode st25r3916OscOn(void);

/*! 
 *****************************************************************************
 *  \brief  Sets the bitrate
 *
 *  This function sets the bitrates for rx and tx
 *
 *  \param txrate : speed is 2^txrate * 106 kb/s
 *                  0xff : don't set txrate (ST25R3916_BR_DO_NOT_SET)
 *  \param rxrate : speed is 2^rxrate * 106 kb/s
 *                  0xff : don't set rxrate (ST25R3916_BR_DO_NOT_SET)
 *
 *  \return ERR_PARAM: At least one bit rate was invalid
 *  \return ERR_NONE : No error, both bit rates were set
 *
 *****************************************************************************
 */
ReturnCode st25r3916SetBitrate(uint8_t txrate, uint8_t rxrate);

/*! 
 *****************************************************************************
 *  \brief  Adjusts supply regulators according to the current supply voltage
 *
 *  This function the power level is measured in maximum load conditions and
 *  the regulated voltage reference is set to 250mV below this level.
 *  Execution of this function lasts around 5ms. 
 *
 *  The regulated voltages will be set to the result of Adjust Regulators
 *  
 *  \param [out] result_mV : Result of calibration in milliVolts
 *
 *  \return ERR_IO : Error during communication with ST25R3916
 *  \return ERR_NONE : No error
 *
 *****************************************************************************
 */
ReturnCode st25r3916AdjustRegulators(uint16_t* result_mV);

/*! 
 *****************************************************************************
 *  \brief  Measure Amplitude
 *
 *  This function measured the amplitude on the RFI inputs and stores the
 *  result in parameter \a result.
 *
 *  \param[out] result:  result of RF measurement.
 *
 *  \return ERR_PARAM : Invalid parameter
 *  \return ERR_NONE  : No error
 *  
 *****************************************************************************
 */
ReturnCode st25r3916MeasureAmplitude(uint8_t* result);

/*! 
 *****************************************************************************
 *  \brief  Measure Power Supply
 *
 *  This function executes Measure Power Supply and returns the raw value
 *
 *  \param[in] mpsv : one of ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd
 *                           ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd_rf
 *                           ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd_a
 *                           ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd_d
 *                           ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd_am
 *
 *  \return the measured voltage in raw format.
 *
 *****************************************************************************
 */
uint8_t st25r3916MeasurePowerSupply(uint8_t mpsv);

/*! 
 *****************************************************************************
 *  \brief  Measure Voltage
 *
 *  This function measures the voltage on one of VDD and VDD_* and returns 
 *  the result in mV
 *
 *  \param[in] mpsv : one of ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd
 *                           ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd_rf
 *                           ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd_a
 *                           ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd_d
 *                    or     ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd_am
 *
 *  \return the measured voltage in mV
 *
 *****************************************************************************
 */
uint16_t st25r3916MeasureVoltage(uint8_t mpsv);

/*! 
 *****************************************************************************
 *  \brief  Measure Phase
 *
 *  This function performs a Phase measurement.
 *  The result is stored in the \a result parameter.
 *
 *  \param[out] result: 8 bit long result of the measurement.
 *
 *  \return ERR_PARAM : Invalid parameter
 *  \return ERR_NONE  : No error
 *  
 *****************************************************************************
 */
ReturnCode st25r3916MeasurePhase(uint8_t* result);

/*! 
 *****************************************************************************
 *  \brief  Measure Capacitance
 *
 *  This function performs the capacitance measurement and stores the
 *  result in parameter \a result.
 *
 *  \param[out] result: 8 bit long result of RF measurement.
 *
 *  \return ERR_PARAM : Invalid parameter
 *  \return ERR_NONE  : No error
 *  
 *****************************************************************************
 */
ReturnCode st25r3916MeasureCapacitance(uint8_t* result);

/*! 
 *****************************************************************************
 *  \brief  Calibrates Capacitive Sensor
 *
 *  This function performs automatic calibration of the capacitive sensor 
 *  and stores the result in parameter \a result.
 *
 * \warning To avoid interference with Xtal oscillator and reader magnetic 
 *          field, it is strongly recommended to perform calibration
 *          in Power-down mode only.
 *          This method does not modify the Oscillator nor transmitter state, 
 *          these should be configured before by user.
 *
 *  \param[out] result: 5 bit long result of the calibration.
 *                      Binary weighted, step 0.1 pF, max 3.1 pF
 *
 *  \return ERR_PARAM : Invalid parameter
 *  \return ERR_IO    : The calibration was not successful 
 *  \return ERR_NONE  : No error
 *  
 *****************************************************************************
 */
ReturnCode st25r3916CalibrateCapacitiveSensor(uint8_t* result);

/*! 
 *****************************************************************************
 *  \brief  Get NRT time
 *
 *  This returns the last value set on the NRT
 *   
 *  \warning it does not read chip register, just the sw var that contains the 
 *  last value set before
 *
 *  \return the value of the NRT in 64/fc 
 */
uint32_t st25r3916GetNoResponseTime(void);

/*! 
 *****************************************************************************
 *  \brief  Set NRT time
 *
 *  This function sets the No Response Time with the given value
 *
 *  \param [in] nrt_64fcs : no response time in steps of 64/fc (4.72us)
 *
 *  \return ERR_PARAM : Invalid parameter (time is too large)
 *  \return ERR_NONE  : No error
 *
 *****************************************************************************  
 */
ReturnCode st25r3916SetNoResponseTime(uint32_t nrt_64fcs);

/*! 
 *****************************************************************************
 *  \brief  Set and Start NRT
 *
 *  This function sets the No Response Time with the given value and 
 *  immediately starts it
 *  Used when needs to add more time before timeout without performing Tx
 *
 *  \param [in] nrt_64fcs : no response time in steps of 64/fc (4.72us)
 *
 *  \return ERR_PARAM : Invalid parameter
 *  \return ERR_NONE  : No error
 *
 *****************************************************************************  
 */
ReturnCode st25r3916SetStartNoResponseTimer(uint32_t nrt_64fcs);

/*! 
 *****************************************************************************
 *  \brief  Set GPT time
 *
 *  This function sets the General Purpose Timer time registers
 *
 *  \param [in] gpt_8fcs : general purpose timer timeout in steps of 8/fc (590ns)
 *
 *****************************************************************************
 */
void st25r3916SetGPTime(uint16_t gpt_8fcs);

/*! 
 *****************************************************************************
 *  \brief  Set and Start GPT
 *
 *  This function sets the General Purpose Timer with the given timeout and 
 *  immediately starts it ONLY if the trigger source is not set to none.
 *
 *  \param [in] gpt_8fcs : general purpose timer timeout in  steps of8/fc (590ns)
 *  \param [in] trigger_source : no trigger, start of Rx, end of Rx, end of Tx in NFC mode
 *   
 *  \return ERR_PARAM : Invalid parameter
 *  \return ERR_NONE  : No error 
 *  
 *****************************************************************************
 */
ReturnCode st25r3916SetStartGPTimer(uint16_t gpt_8fcs, uint8_t trigger_source);

/*! 
 *****************************************************************************
 *  \brief  Sets the number Tx Bits
 *  
 *  Sets ST25R3916 internal registers with correct number of complete bytes and
 *  bits to be sent
 *  
 *  \param [in] nBits : number of bits to be set/transmitted
 *    
 *****************************************************************************
 */
void st25r3916SetNumTxBits(uint16_t nBits);

/*! 
 *****************************************************************************
 *  \brief  Get amount of bytes in FIFO
 *  
 *  Gets the number of bytes currently in the FIFO
 *  
 *  \return the number of bytes currently in the FIFO
 *    
 *****************************************************************************
 */
uint16_t st25r3916GetNumFIFOBytes(void);

/*! 
 *****************************************************************************
 *  \brief  Get amount of bits of the last FIFO byte if incomplete
 *  
 *  Gets the number of bits of the last FIFO byte if incomplete
 *  
 *  \return the number of bits of the last FIFO byte if incomplete, 0 if 
 *          the last byte is complete
 *    
 *****************************************************************************
 */
uint8_t st25r3916GetNumFIFOLastBits(void);

/*! 
 *****************************************************************************
 *  \brief  Perform Collision Avoidance
 *
 *  Performs Collision Avoidance with the given threshold and with the  
 *  n number of TRFW 
 *  
 *  \param[in] FieldONCmd  : Field ON command to be executed ST25R3916_CMD_INITIAL_RF_COLLISION
 *                           or ST25R3916_CMD_RESPONSE_RF_COLLISION_N  
 *  \param[in] pdThreshold : Peer Detection Threshold  (ST25R3916_REG_FIELD_THRESHOLD_trg_xx)
 *                           0xff : don't set Threshold (ST25R3916_THRESHOLD_DO_NOT_SET)
 *  \param[in] caThreshold : Collision Avoidance Threshold (ST25R3916_REG_FIELD_THRESHOLD_rfe_xx)
 *                           0xff : don't set Threshold (ST25R3916_THRESHOLD_DO_NOT_SET)
 *  \param[in] nTRFW       : Number of TRFW
 *
 *  \return ERR_PARAM        : Invalid parameter 
 *  \return ERR_RF_COLLISION : Collision detected
 *  \return ERR_NONE         : No collision detected
 *  
 *****************************************************************************
 */
ReturnCode st25r3916PerformCollisionAvoidance(
    uint8_t FieldONCmd,
    uint8_t pdThreshold,
    uint8_t caThreshold,
    uint8_t nTRFW);

/*! 
 *****************************************************************************
 *  \brief  Check Identity
 *
 *  Checks if the chip ID is as expected.
 *  
 *  5 bit IC type code for ST25R3916: 00101
 *  The 3 lsb contain the IC revision code
 *   
 *  \param[out] rev : the IC revision code
 *    
 *  \return  true when IC type is as expected
 *  \return  false otherwise
 */
bool st25r3916CheckChipID(uint8_t* rev);

/*! 
 *****************************************************************************
 *  \brief  Retrieves all  internal registers from ST25R3916
 *  
 *  \param[out] regDump : pointer to the struct/buffer where the reg dump
 *                        will be written
 *  
 *  \return ERR_PARAM : Invalid parameter
 *  \return ERR_NONE  : No error
 *****************************************************************************
 */
ReturnCode st25r3916GetRegsDump(t_st25r3916Regs* regDump);

/*! 
 *****************************************************************************
 *  \brief  Check if command is valid
 *
 *  Checks if the given command is a valid ST25R3916 command
 *
 *  \param[in] cmd: Command to check
 *  
 *  \return  true if is a valid command
 *  \return  false otherwise
 *
 *****************************************************************************
 */
bool st25r3916IsCmdValid(uint8_t cmd);

/*! 
 *****************************************************************************
 *  \brief  Configure the stream mode of ST25R3916
 *
 *  This function initializes the stream with the given parameters
 *
 *  \param[in] config : all settings for bitrates, type, etc.
 *
 *  \return ERR_PARAM : Invalid parameter
 *  \return ERR_NONE  : No error, stream mode driver initialized
 *
 *****************************************************************************
 */
ReturnCode st25r3916StreamConfigure(const struct st25r3916StreamConfig* config);

/*! 
 *****************************************************************************
 *  \brief  Executes a direct command and returns the result
 *
 *  This function executes the direct command given by \a cmd waits for
 *  \a sleeptime for I_dct and returns the result read from register \a resreg.
 *  The value of cmd is not checked.
 *
 *  \param[in]  cmd   : direct command to execute
 *  \param[in]  resReg: address of the register containing the result
 *  \param[in]  tout  : time in milliseconds to wait before reading the result
 *  \param[out] result: result
 *
 *  \return ERR_NONE  : No error
 *  
 *****************************************************************************
 */
ReturnCode
    st25r3916ExecuteCommandAndGetResult(uint8_t cmd, uint8_t resReg, uint8_t tout, uint8_t* result);

/*! 
 *****************************************************************************
 *  \brief  Gets the RSSI values
 *
 *  This function gets the RSSI value of the previous reception taking into 
 *  account the gain reductions that were used. 
 *  RSSI value for both AM and PM channel can be retrieved.
 *
 *  \param[out] amRssi: the RSSI on the AM channel expressed in mV 
 *  \param[out] pmRssi: the RSSI on the PM channel expressed in mV 
 *  
 *  \return ERR_PARAM : Invalid parameter
 *  \return ERR_NONE  : No error
 *  
 *****************************************************************************
 */
ReturnCode st25r3916GetRSSI(uint16_t* amRssi, uint16_t* pmRssi);
#endif /* ST25R3916_H */

/**
  * @}
  *
  * @}
  *
  * @}
  * 
  * @}
  */
