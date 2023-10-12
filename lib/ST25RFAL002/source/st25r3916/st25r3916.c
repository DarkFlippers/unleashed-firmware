
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
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/

#include "st25r3916.h"
#include "st25r3916_com.h"
#include "st25r3916_led.h"
#include "st25r3916_irq.h"
#include "utils.h"

/*
******************************************************************************
* LOCAL DEFINES
******************************************************************************
*/

#define ST25R3916_SUPPLY_THRESHOLD \
    3600U /*!< Power supply measure threshold between 3.3V or 5V                   */
#define ST25R3916_NRT_MAX \
    0xFFFFU /*!< Max Register value of NRT                                           */

#define ST25R3916_TOUT_MEASURE_VDD \
    100U /*!< Max duration time of Measure Power Supply command  Datasheet: 25us  */
#define ST25R3916_TOUT_MEASURE_AMPLITUDE \
    10U /*!< Max duration time of Measure Amplitude command     Datasheet: 25us  */
#define ST25R3916_TOUT_MEASURE_PHASE \
    10U /*!< Max duration time of Measure Phase command         Datasheet: 25us  */
#define ST25R3916_TOUT_MEASURE_CAPACITANCE \
    10U /*!< Max duration time of Measure Capacitance command   Datasheet: 25us  */
#define ST25R3916_TOUT_CALIBRATE_CAP_SENSOR \
    4U /*!< Max duration Calibrate Capacitive Sensor command   Datasheet: 3ms   */
#define ST25R3916_TOUT_ADJUST_REGULATORS \
    6U /*!< Max duration time of Adjust Regulators command     Datasheet: 5ms   */
#define ST25R3916_TOUT_CA \
    10U /*!< Max duration time of Collision Avoidance command                    */

#define ST25R3916_TEST_REG_PATTERN \
    0x33U /*!< Register Read Write test pattern used during selftest               */
#define ST25R3916_TEST_WU_TOUT \
    12U /*!< Timeout used on WU timer during self test                           */
#define ST25R3916_TEST_TMR_TOUT \
    20U /*!< Timeout used during self test                                       */
#define ST25R3916_TEST_TMR_TOUT_DELTA \
    2U /*!< Timeout used during self test                                       */
#define ST25R3916_TEST_TMR_TOUT_8FC \
    (ST25R3916_TEST_TMR_TOUT * 16950U) /*!< Timeout in 8/fc                          */

/*
******************************************************************************
* LOCAL CONSTANTS
******************************************************************************
*/

/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/

static uint32_t gST25R3916NRT_64fcs;

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*
 ******************************************************************************
 * LOCAL FUNCTION
 ******************************************************************************
 */

ReturnCode st25r3916ExecuteCommandAndGetResult(
    uint8_t cmd,
    uint8_t resReg,
    uint8_t tout,
    uint8_t* result) {
    /* Clear and enable Direct Command interrupt */
    st25r3916GetInterrupt(ST25R3916_IRQ_MASK_DCT);
    st25r3916EnableInterrupts(ST25R3916_IRQ_MASK_DCT);

    st25r3916ExecuteCommand(cmd);

    st25r3916WaitForInterruptsTimed(ST25R3916_IRQ_MASK_DCT, tout);
    st25r3916DisableInterrupts(ST25R3916_IRQ_MASK_DCT);

    /* After execution read out the result if the pointer is not NULL */
    if(result != NULL) {
        st25r3916ReadRegister(resReg, result);
    }

    return ERR_NONE;
}

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

ReturnCode st25r3916Initialize(void) {
    uint16_t vdd_mV;
    ReturnCode ret;

    /* Set default state on the ST25R3916 */
    st25r3916ExecuteCommand(ST25R3916_CMD_SET_DEFAULT);

#ifndef RFAL_USE_I2C
    /* Increase MISO driving level as SPI can go up to 10MHz */
    st25r3916WriteRegister(ST25R3916_REG_IO_CONF2, ST25R3916_REG_IO_CONF2_io_drv_lvl);
#endif /* RFAL_USE_I2C */

    if(!st25r3916CheckChipID(NULL)) {
        platformErrorHandle();
        return ERR_HW_MISMATCH;
    }

    st25r3916InitInterrupts();
    st25r3916ledInit();

    gST25R3916NRT_64fcs = 0;

#ifndef RFAL_USE_I2C
    /* Enable pull downs on MISO line */
    st25r3916SetRegisterBits(
        ST25R3916_REG_IO_CONF2,
        (ST25R3916_REG_IO_CONF2_miso_pd1 | ST25R3916_REG_IO_CONF2_miso_pd2));
#endif /* RFAL_USE_I2C */

    /* Disable internal overheat protection */
    st25r3916ChangeTestRegisterBits(0x04, 0x10, 0x10);

#ifdef ST25R_SELFTEST
    /******************************************************************************
     * Check communication interface: 
     *  - write a pattern in a register
     *  - reads back the register value
     *  - return ERR_IO in case the read value is different
     */
    st25r3916WriteRegister(ST25R3916_REG_BIT_RATE, ST25R3916_TEST_REG_PATTERN);
    if(!st25r3916CheckReg(
           ST25R3916_REG_BIT_RATE,
           (ST25R3916_REG_BIT_RATE_rxrate_mask | ST25R3916_REG_BIT_RATE_txrate_mask),
           ST25R3916_TEST_REG_PATTERN)) {
        platformErrorHandle();
        return ERR_IO;
    }

    /* Restore default value */
    st25r3916WriteRegister(ST25R3916_REG_BIT_RATE, 0x00);

    /*
     * Check IRQ Handling:
     *  - use the Wake-up timer to trigger an IRQ
     *  - wait the Wake-up timer interrupt
     *  - return ERR_TIMEOUT when the Wake-up timer interrupt is not received
     */
    st25r3916WriteRegister(
        ST25R3916_REG_WUP_TIMER_CONTROL,
        ST25R3916_REG_WUP_TIMER_CONTROL_wur | ST25R3916_REG_WUP_TIMER_CONTROL_wto);
    st25r3916EnableInterrupts(ST25R3916_IRQ_MASK_WT);
    st25r3916ExecuteCommand(ST25R3916_CMD_START_WUP_TIMER);
    if(st25r3916WaitForInterruptsTimed(ST25R3916_IRQ_MASK_WT, ST25R3916_TEST_WU_TOUT) == 0U) {
        platformErrorHandle();
        return ERR_TIMEOUT;
    }
    st25r3916DisableInterrupts(ST25R3916_IRQ_MASK_WT);
    st25r3916WriteRegister(ST25R3916_REG_WUP_TIMER_CONTROL, 0U);
    /*******************************************************************************/
#endif /* ST25R_SELFTEST */

    /* Enable Oscillator and wait until it gets stable */
    ret = st25r3916OscOn();
    if(ret != ERR_NONE) {
        platformErrorHandle();
        return ret;
    }

    /* Measure VDD and set sup3V bit according to Power supplied  */
    vdd_mV = st25r3916MeasureVoltage(ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd);
    st25r3916ChangeRegisterBits(
        ST25R3916_REG_IO_CONF2,
        ST25R3916_REG_IO_CONF2_sup3V,
        ((vdd_mV < ST25R3916_SUPPLY_THRESHOLD) ? ST25R3916_REG_IO_CONF2_sup3V_3V :
                                                 ST25R3916_REG_IO_CONF2_sup3V_5V));

    /* Make sure Transmitter and Receiver are disabled */
    st25r3916TxRxOff();

#ifdef ST25R_SELFTEST_TIMER
    /******************************************************************************
     * Check SW timer operation :
     *  - use the General Purpose timer to measure an amount of time
     *  - test whether an interrupt is seen when less time was given
     *  - test whether an interrupt is seen when sufficient time was given
     */

    st25r3916EnableInterrupts(ST25R3916_IRQ_MASK_GPE);
    st25r3916SetStartGPTimer(
        (uint16_t)ST25R3916_TEST_TMR_TOUT_8FC, ST25R3916_REG_TIMER_EMV_CONTROL_gptc_no_trigger);
    if(st25r3916WaitForInterruptsTimed(
           ST25R3916_IRQ_MASK_GPE, (ST25R3916_TEST_TMR_TOUT - ST25R3916_TEST_TMR_TOUT_DELTA)) !=
       0U) {
        platformErrorHandle();
        return ERR_SYSTEM;
    }

    /* Stop all activities to stop the GP timer */
    st25r3916ExecuteCommand(ST25R3916_CMD_STOP);
    st25r3916ClearAndEnableInterrupts(ST25R3916_IRQ_MASK_GPE);
    st25r3916SetStartGPTimer(
        (uint16_t)ST25R3916_TEST_TMR_TOUT_8FC, ST25R3916_REG_TIMER_EMV_CONTROL_gptc_no_trigger);
    if(st25r3916WaitForInterruptsTimed(
           ST25R3916_IRQ_MASK_GPE, (ST25R3916_TEST_TMR_TOUT + ST25R3916_TEST_TMR_TOUT_DELTA)) ==
       0U) {
        platformErrorHandle();
        return ERR_SYSTEM;
    }

    /* Stop all activities to stop the GP timer */
    st25r3916ExecuteCommand(ST25R3916_CMD_STOP);
    /*******************************************************************************/
#endif /* ST25R_SELFTEST_TIMER */

    /* After reset all interrupts are enabled, so disable them at first */
    st25r3916DisableInterrupts(ST25R3916_IRQ_MASK_ALL);

    /* And clear them, just to be sure */
    st25r3916ClearInterrupts();

    return ERR_NONE;
}

/*******************************************************************************/
void st25r3916Deinitialize(void) {
    st25r3916DisableInterrupts(ST25R3916_IRQ_MASK_ALL);

    /* Disable Tx and Rx, Keep OSC On */
    st25r3916TxRxOff();

    return;
}

/*******************************************************************************/
ReturnCode st25r3916OscOn(void) {
    /* Check if oscillator is already turned on and stable                                                */
    /* Use ST25R3916_REG_OP_CONTROL_en instead of ST25R3916_REG_AUX_DISPLAY_osc_ok to be on the safe side */
    if(!st25r3916CheckReg(
           ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en, ST25R3916_REG_OP_CONTROL_en)) {
        /* Clear any eventual previous oscillator IRQ */
        st25r3916GetInterrupt(ST25R3916_IRQ_MASK_OSC);

        /* Enable oscillator frequency stable interrupt */
        st25r3916EnableInterrupts(ST25R3916_IRQ_MASK_OSC);

        /* Enable oscillator and regulator output */
        st25r3916SetRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en);

        /* Wait for the oscillator interrupt */
        st25r3916WaitForInterruptsTimed(ST25R3916_IRQ_MASK_OSC, ST25R3916_TOUT_OSC_STABLE);
        st25r3916DisableInterrupts(ST25R3916_IRQ_MASK_OSC);
    }

    if(!st25r3916CheckReg(
           ST25R3916_REG_AUX_DISPLAY,
           ST25R3916_REG_AUX_DISPLAY_osc_ok,
           ST25R3916_REG_AUX_DISPLAY_osc_ok)) {
        return ERR_SYSTEM;
    }

    return ERR_NONE;
}

/*******************************************************************************/
uint8_t st25r3916MeasurePowerSupply(uint8_t mpsv) {
    uint8_t result;

    /* Set the source of direct command: Measure Power Supply Voltage */
    st25r3916ChangeRegisterBits(
        ST25R3916_REG_REGULATOR_CONTROL, ST25R3916_REG_REGULATOR_CONTROL_mpsv_mask, mpsv);

    /* Execute command: Measure Power Supply Voltage */
    st25r3916ExecuteCommandAndGetResult(
        ST25R3916_CMD_MEASURE_VDD, ST25R3916_REG_AD_RESULT, ST25R3916_TOUT_MEASURE_VDD, &result);

    return result;
}

/*******************************************************************************/
uint16_t st25r3916MeasureVoltage(uint8_t mpsv) {
    uint8_t result;
    uint16_t mV;

    result = st25r3916MeasurePowerSupply(mpsv);

    /* Convert cmd output into mV (each step represents 23.4 mV )*/
    mV = ((uint16_t)result) * 23U;
    mV += (((((uint16_t)result) * 4U) + 5U) / 10U);

    return mV;
}

/*******************************************************************************/
ReturnCode st25r3916AdjustRegulators(uint16_t* result_mV) {
    uint8_t result;

    /* Reset logic and set regulated voltages to be defined by result of Adjust Regulators command */
    st25r3916SetRegisterBits(
        ST25R3916_REG_REGULATOR_CONTROL, ST25R3916_REG_REGULATOR_CONTROL_reg_s);
    st25r3916ClrRegisterBits(
        ST25R3916_REG_REGULATOR_CONTROL, ST25R3916_REG_REGULATOR_CONTROL_reg_s);

    /* Execute Adjust regulators cmd and retrieve result */
    st25r3916ExecuteCommandAndGetResult(
        ST25R3916_CMD_ADJUST_REGULATORS,
        ST25R3916_REG_REGULATOR_RESULT,
        ST25R3916_TOUT_ADJUST_REGULATORS,
        &result);

    /* Calculate result in mV */
    result >>= ST25R3916_REG_REGULATOR_RESULT_reg_shift;

    if(result_mV != NULL) {
        if(st25r3916CheckReg(
               ST25R3916_REG_IO_CONF2,
               ST25R3916_REG_IO_CONF2_sup3V,
               ST25R3916_REG_IO_CONF2_sup3V)) {
            result = MIN(
                result,
                (uint8_t)(result - 5U)); /* In 3.3V mode [0,4] are not used                       */
            *result_mV = 2400U; /* Minimum regulated voltage 2.4V in case of 3.3V supply */
        } else {
            *result_mV = 3600U; /* Minimum regulated voltage 3.6V in case of 5V supply   */
        }

        *result_mV +=
            (uint16_t)result * 100U; /* 100mV steps in both 3.3V and 5V supply                */
    }
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916MeasureAmplitude(uint8_t* result) {
    return st25r3916ExecuteCommandAndGetResult(
        ST25R3916_CMD_MEASURE_AMPLITUDE,
        ST25R3916_REG_AD_RESULT,
        ST25R3916_TOUT_MEASURE_AMPLITUDE,
        result);
}

/*******************************************************************************/
ReturnCode st25r3916MeasurePhase(uint8_t* result) {
    return st25r3916ExecuteCommandAndGetResult(
        ST25R3916_CMD_MEASURE_PHASE, ST25R3916_REG_AD_RESULT, ST25R3916_TOUT_MEASURE_PHASE, result);
}

/*******************************************************************************/
ReturnCode st25r3916MeasureCapacitance(uint8_t* result) {
    return st25r3916ExecuteCommandAndGetResult(
        ST25R3916_CMD_MEASURE_CAPACITANCE,
        ST25R3916_REG_AD_RESULT,
        ST25R3916_TOUT_MEASURE_CAPACITANCE,
        result);
}

/*******************************************************************************/
ReturnCode st25r3916CalibrateCapacitiveSensor(uint8_t* result) {
    ReturnCode ret;
    uint8_t res;

    /* Clear Manual calibration values to enable automatic calibration mode */
    st25r3916ClrRegisterBits(
        ST25R3916_REG_CAP_SENSOR_CONTROL, ST25R3916_REG_CAP_SENSOR_CONTROL_cs_mcal_mask);

    /* Execute automatic calibration */
    ret = st25r3916ExecuteCommandAndGetResult(
        ST25R3916_CMD_CALIBRATE_C_SENSOR,
        ST25R3916_REG_CAP_SENSOR_RESULT,
        ST25R3916_TOUT_CALIBRATE_CAP_SENSOR,
        &res);

    /* Check whether the calibration was successull */
    if(((res & ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_end) !=
        ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_end) ||
       ((res & ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_err) ==
        ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_err) ||
       (ret != ERR_NONE)) {
        return ERR_IO;
    }

    if(result != NULL) {
        (*result) = (uint8_t)(res >> ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_shift);
    }

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916SetBitrate(uint8_t txrate, uint8_t rxrate) {
    uint8_t reg;

    st25r3916ReadRegister(ST25R3916_REG_BIT_RATE, &reg);
    if(rxrate != ST25R3916_BR_DO_NOT_SET) {
        if(rxrate > ST25R3916_BR_848) {
            return ERR_PARAM;
        }

        reg = (uint8_t)(reg & ~ST25R3916_REG_BIT_RATE_rxrate_mask); /* MISRA 10.3 */
        reg |= rxrate << ST25R3916_REG_BIT_RATE_rxrate_shift;
    }
    if(txrate != ST25R3916_BR_DO_NOT_SET) {
        if(txrate > ST25R3916_BR_6780) {
            return ERR_PARAM;
        }

        reg = (uint8_t)(reg & ~ST25R3916_REG_BIT_RATE_txrate_mask); /* MISRA 10.3 */
        reg |= txrate << ST25R3916_REG_BIT_RATE_txrate_shift;
    }
    return st25r3916WriteRegister(ST25R3916_REG_BIT_RATE, reg);
}

/*******************************************************************************/
ReturnCode st25r3916PerformCollisionAvoidance(
    uint8_t FieldONCmd,
    uint8_t pdThreshold,
    uint8_t caThreshold,
    uint8_t nTRFW) {
    uint8_t treMask;
    uint32_t irqs;
    ReturnCode err;

    if((FieldONCmd != ST25R3916_CMD_INITIAL_RF_COLLISION) &&
       (FieldONCmd != ST25R3916_CMD_RESPONSE_RF_COLLISION_N)) {
        return ERR_PARAM;
    }

    err = ERR_INTERNAL;

    /* Check if new thresholds are to be applied */
    if((pdThreshold != ST25R3916_THRESHOLD_DO_NOT_SET) ||
       (caThreshold != ST25R3916_THRESHOLD_DO_NOT_SET)) {
        treMask = 0;

        if(pdThreshold != ST25R3916_THRESHOLD_DO_NOT_SET) {
            treMask |= ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_mask;
        }

        if(caThreshold != ST25R3916_THRESHOLD_DO_NOT_SET) {
            treMask |= ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_mask;
        }

        /* Set Detection Threshold and|or Collision Avoidance Threshold */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_FIELD_THRESHOLD_ACTV,
            treMask,
            (pdThreshold & ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_mask) |
                (caThreshold & ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_mask));
    }

    /* Set n x TRFW */
    st25r3916ChangeRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_nfc_n_mask, nTRFW);

    /*******************************************************************************/
    /* Enable and clear CA specific interrupts and execute command */
    st25r3916GetInterrupt(
        (ST25R3916_IRQ_MASK_CAC | ST25R3916_IRQ_MASK_CAT | ST25R3916_IRQ_MASK_APON));
    st25r3916EnableInterrupts(
        (ST25R3916_IRQ_MASK_CAC | ST25R3916_IRQ_MASK_CAT | ST25R3916_IRQ_MASK_APON));

    st25r3916ExecuteCommand(FieldONCmd);

    /*******************************************************************************/
    /* Wait for initial APON interrupt, indicating anticollision avoidance done and ST25R3916's 
     * field is now on, or a CAC indicating a collision */
    irqs = st25r3916WaitForInterruptsTimed(
        (ST25R3916_IRQ_MASK_CAC | ST25R3916_IRQ_MASK_APON), ST25R3916_TOUT_CA);

    if((ST25R3916_IRQ_MASK_CAC & irqs) != 0U) /* Collision occurred */
    {
        err = ERR_RF_COLLISION;
    } else if((ST25R3916_IRQ_MASK_APON & irqs) != 0U) {
        /* After APON wait for CAT interrupt, indication field was switched on minimum guard time has been fulfilled */
        irqs = st25r3916WaitForInterruptsTimed((ST25R3916_IRQ_MASK_CAT), ST25R3916_TOUT_CA);

        if((ST25R3916_IRQ_MASK_CAT & irqs) != 0U) /* No Collision detected, Field On */
        {
            err = ERR_NONE;
        }
    } else {
        /* MISRA 15.7 - Empty else */
    }

    /* Clear any previous External Field events and disable CA specific interrupts */
    st25r3916GetInterrupt((ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_EON));
    st25r3916DisableInterrupts(
        (ST25R3916_IRQ_MASK_CAC | ST25R3916_IRQ_MASK_CAT | ST25R3916_IRQ_MASK_APON));

    return err;
}

/*******************************************************************************/
void st25r3916SetNumTxBits(uint16_t nBits) {
    st25r3916WriteRegister(ST25R3916_REG_NUM_TX_BYTES2, (uint8_t)((nBits >> 0) & 0xFFU));
    st25r3916WriteRegister(ST25R3916_REG_NUM_TX_BYTES1, (uint8_t)((nBits >> 8) & 0xFFU));
}

/*******************************************************************************/
uint16_t st25r3916GetNumFIFOBytes(void) {
    uint8_t reg;
    uint16_t result;

    st25r3916ReadRegister(ST25R3916_REG_FIFO_STATUS2, &reg);
    reg =
        ((reg & ST25R3916_REG_FIFO_STATUS2_fifo_b_mask) >>
         ST25R3916_REG_FIFO_STATUS2_fifo_b_shift);
    result = ((uint16_t)reg << 8);

    st25r3916ReadRegister(ST25R3916_REG_FIFO_STATUS1, &reg);
    result |= (((uint16_t)reg) & 0x00FFU);

    return result;
}

/*******************************************************************************/
uint8_t st25r3916GetNumFIFOLastBits(void) {
    uint8_t reg;

    st25r3916ReadRegister(ST25R3916_REG_FIFO_STATUS2, &reg);

    return (
        (reg & ST25R3916_REG_FIFO_STATUS2_fifo_lb_mask) >>
        ST25R3916_REG_FIFO_STATUS2_fifo_lb_shift);
}

/*******************************************************************************/
uint32_t st25r3916GetNoResponseTime(void) {
    return gST25R3916NRT_64fcs;
}

/*******************************************************************************/
ReturnCode st25r3916SetNoResponseTime(uint32_t nrt_64fcs) {
    ReturnCode err;
    uint8_t nrt_step;
    uint32_t tmpNRT;

    tmpNRT = nrt_64fcs; /* MISRA 17.8 */
    err = ERR_NONE;

    gST25R3916NRT_64fcs = tmpNRT; /* Store given NRT value in 64/fc into local var       */
    nrt_step =
        ST25R3916_REG_TIMER_EMV_CONTROL_nrt_step_64fc; /* Set default NRT in steps of 64/fc                   */

    if(tmpNRT > ST25R3916_NRT_MAX) /* Check if the given NRT value fits using 64/fc steps */
    {
        nrt_step =
            ST25R3916_REG_TIMER_EMV_CONTROL_nrt_step_4096_fc; /* If not, change NRT set to 4096/fc                   */
        tmpNRT = ((tmpNRT + 63U) / 64U); /* Calculate number of steps in 4096/fc                */

        if(tmpNRT > ST25R3916_NRT_MAX) /* Check if the NRT value fits using 64/fc steps       */
        {
            tmpNRT = ST25R3916_NRT_MAX; /* Assign the maximum possible                         */
            err = ERR_PARAM; /* Signal parameter error                              */
        }
        gST25R3916NRT_64fcs = (64U * tmpNRT);
    }

    /* Set the ST25R3916 NRT step units and the value */
    st25r3916ChangeRegisterBits(
        ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_step, nrt_step);
    st25r3916WriteRegister(ST25R3916_REG_NO_RESPONSE_TIMER1, (uint8_t)(tmpNRT >> 8U));
    st25r3916WriteRegister(ST25R3916_REG_NO_RESPONSE_TIMER2, (uint8_t)(tmpNRT & 0xFFU));

    return err;
}

/*******************************************************************************/
ReturnCode st25r3916SetStartNoResponseTimer(uint32_t nrt_64fcs) {
    ReturnCode err;

    err = st25r3916SetNoResponseTime(nrt_64fcs);
    if(err == ERR_NONE) {
        st25r3916ExecuteCommand(ST25R3916_CMD_START_NO_RESPONSE_TIMER);
    }

    return err;
}

/*******************************************************************************/
void st25r3916SetGPTime(uint16_t gpt_8fcs) {
    st25r3916WriteRegister(ST25R3916_REG_GPT1, (uint8_t)(gpt_8fcs >> 8));
    st25r3916WriteRegister(ST25R3916_REG_GPT2, (uint8_t)(gpt_8fcs & 0xFFU));
}

/*******************************************************************************/
ReturnCode st25r3916SetStartGPTimer(uint16_t gpt_8fcs, uint8_t trigger_source) {
    st25r3916SetGPTime(gpt_8fcs);
    st25r3916ChangeRegisterBits(
        ST25R3916_REG_TIMER_EMV_CONTROL,
        ST25R3916_REG_TIMER_EMV_CONTROL_gptc_mask,
        trigger_source);

    /* If there's no trigger source, start GPT immediately */
    if(trigger_source == ST25R3916_REG_TIMER_EMV_CONTROL_gptc_no_trigger) {
        st25r3916ExecuteCommand(ST25R3916_CMD_START_GP_TIMER);
    }

    return ERR_NONE;
}

/*******************************************************************************/
bool st25r3916CheckChipID(uint8_t* rev) {
    uint8_t ID;

    ID = 0;
    st25r3916ReadRegister(ST25R3916_REG_IC_IDENTITY, &ID);

    /* Check if IC Identity Register contains ST25R3916's IC type code */
    if((ID & ST25R3916_REG_IC_IDENTITY_ic_type_mask) !=
       ST25R3916_REG_IC_IDENTITY_ic_type_st25r3916) {
        return false;
    }

    if(rev != NULL) {
        *rev = (ID & ST25R3916_REG_IC_IDENTITY_ic_rev_mask);
    }

    return true;
}

/*******************************************************************************/
ReturnCode st25r3916GetRegsDump(t_st25r3916Regs* regDump) {
    uint8_t regIt;

    if(regDump == NULL) {
        return ERR_PARAM;
    }

    /* Dump Registers on space A */
    for(regIt = ST25R3916_REG_IO_CONF1; regIt <= ST25R3916_REG_IC_IDENTITY; regIt++) {
        st25r3916ReadRegister(regIt, &regDump->RsA[regIt]);
    }

    regIt = 0;

    /* Read non-consecutive Registers on space B */
    st25r3916ReadRegister(ST25R3916_REG_EMD_SUP_CONF, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_SUBC_START_TIME, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_P2P_RX_CONF, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_CORR_CONF1, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_CORR_CONF2, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_SQUELCH_TIMER, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_FIELD_ON_GT, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_AUX_MOD, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_TX_DRIVER_TIMING, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_RES_AM_MOD, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_TX_DRIVER_STATUS, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_REGULATOR_RESULT, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_OVERSHOOT_CONF1, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_OVERSHOOT_CONF2, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_UNDERSHOOT_CONF1, &regDump->RsB[regIt++]);
    st25r3916ReadRegister(ST25R3916_REG_UNDERSHOOT_CONF2, &regDump->RsB[regIt++]);

    return ERR_NONE;
}

/*******************************************************************************/
bool st25r3916IsCmdValid(uint8_t cmd) {
    if(!((cmd >= ST25R3916_CMD_SET_DEFAULT) && (cmd <= ST25R3916_CMD_RESPONSE_RF_COLLISION_N)) &&
       !((cmd >= ST25R3916_CMD_GOTO_SENSE) && (cmd <= ST25R3916_CMD_GOTO_SLEEP)) &&
       !((cmd >= ST25R3916_CMD_MASK_RECEIVE_DATA) && (cmd <= ST25R3916_CMD_MEASURE_AMPLITUDE)) &&
       !((cmd >= ST25R3916_CMD_RESET_RXGAIN) && (cmd <= ST25R3916_CMD_ADJUST_REGULATORS)) &&
       !((cmd >= ST25R3916_CMD_CALIBRATE_DRIVER_TIMING) &&
         (cmd <= ST25R3916_CMD_START_PPON2_TIMER)) &&
       (cmd != ST25R3916_CMD_SPACE_B_ACCESS) && (cmd != ST25R3916_CMD_STOP_NRT)) {
        return false;
    }
    return true;
}

/*******************************************************************************/
ReturnCode st25r3916StreamConfigure(const struct st25r3916StreamConfig* config) {
    uint8_t smd;
    uint8_t mode;

    smd = 0;

    if(config->useBPSK != 0U) {
        mode = ST25R3916_REG_MODE_om_bpsk_stream;
        if((config->din < 2U) || (config->din > 4U)) /* not in fc/4 .. fc/16 */
        {
            return ERR_PARAM;
        }
        smd |= ((4U - config->din) << ST25R3916_REG_STREAM_MODE_scf_shift);
    } else {
        mode = ST25R3916_REG_MODE_om_subcarrier_stream;
        if((config->din < 3U) || (config->din > 6U)) /* not in fc/8 .. fc/64 */
        {
            return ERR_PARAM;
        }
        smd |= ((6U - config->din) << ST25R3916_REG_STREAM_MODE_scf_shift);
        if(config->report_period_length == 0U) {
            return ERR_PARAM;
        }
    }

    if((config->dout < 1U) || (config->dout > 7U)) /* not in fc/2 .. fc/128 */
    {
        return ERR_PARAM;
    }
    smd |= (7U - config->dout) << ST25R3916_REG_STREAM_MODE_stx_shift;

    if(config->report_period_length > 3U) {
        return ERR_PARAM;
    }
    smd |= (config->report_period_length << ST25R3916_REG_STREAM_MODE_scp_shift);

    st25r3916WriteRegister(ST25R3916_REG_STREAM_MODE, smd);
    st25r3916ChangeRegisterBits(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_mask, mode);

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916GetRSSI(uint16_t* amRssi, uint16_t* pmRssi) {
    /*******************************************************************************/
    /* MISRA 8.9 An object should be defined at block scope if its identifier only appears in a single function */
    /*< ST25R3916  RSSI Display Reg values:      0   1   2   3   4   5   6    7    8   9    a     b    c    d  e  f */
    static const uint16_t st25r3916Rssi2mV[] = {
        0, 20, 27, 37, 52, 72, 99, 136, 190, 262, 357, 500, 686, 950, 1150, 1150};

    /* ST25R3916 2/3 stage gain reduction [dB]          0    0    0    0    0    3    6    9   12   15   18  na na na na na */
    static const uint16_t st25r3916Gain2Percent[] = {
        100, 100, 100, 100, 100, 141, 200, 281, 398, 562, 794, 1, 1, 1, 1, 1};
    /*******************************************************************************/

    uint8_t rssi;
    uint8_t gainRed;

    st25r3916ReadRegister(ST25R3916_REG_RSSI_RESULT, &rssi);
    st25r3916ReadRegister(ST25R3916_REG_GAIN_RED_STATE, &gainRed);

    if(amRssi != NULL) {
        *amRssi =
            (uint16_t)(((uint32_t)st25r3916Rssi2mV[(rssi >> ST25R3916_REG_RSSI_RESULT_rssi_am_shift)] * (uint32_t)st25r3916Gain2Percent[(gainRed >> ST25R3916_REG_GAIN_RED_STATE_gs_am_shift)]) / 100U);
    }

    if(pmRssi != NULL) {
        *pmRssi =
            (uint16_t)(((uint32_t)st25r3916Rssi2mV[(rssi & ST25R3916_REG_RSSI_RESULT_rssi_pm_mask)] * (uint32_t)st25r3916Gain2Percent[(gainRed & ST25R3916_REG_GAIN_RED_STATE_gs_pm_mask)]) / 100U);
    }

    return ERR_NONE;
}
