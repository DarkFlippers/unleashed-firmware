
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
 *  \brief Implementation of ST25R3916 communication
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
#include "st_errno.h"
#include "platform.h"
#include "utils.h"

/*
******************************************************************************
* LOCAL DEFINES
******************************************************************************
*/

#define ST25R3916_OPTIMIZE \
    true /*!< Optimization switch: false always write value to register      */
#define ST25R3916_I2C_ADDR \
    (0xA0U >> 1) /*!< ST25R3916's default I2C address                                */
#define ST25R3916_REG_LEN 1U /*!< Byte length of a ST25R3916 register                            */

#define ST25R3916_WRITE_MODE \
    (0U << 6) /*!< ST25R3916 Operation Mode: Write                                */
#define ST25R3916_READ_MODE \
    (1U << 6) /*!< ST25R3916 Operation Mode: Read                                 */
#define ST25R3916_CMD_MODE \
    (3U << 6) /*!< ST25R3916 Operation Mode: Direct Command                       */
#define ST25R3916_FIFO_LOAD \
    (0x80U) /*!< ST25R3916 Operation Mode: FIFO Load                            */
#define ST25R3916_FIFO_READ \
    (0x9FU) /*!< ST25R3916 Operation Mode: FIFO Read                            */
#define ST25R3916_PT_A_CONFIG_LOAD \
    (0xA0U) /*!< ST25R3916 Operation Mode: Passive Target Memory A-Config Load  */
#define ST25R3916_PT_F_CONFIG_LOAD \
    (0xA8U) /*!< ST25R3916 Operation Mode: Passive Target Memory F-Config Load  */
#define ST25R3916_PT_TSN_DATA_LOAD \
    (0xACU) /*!< ST25R3916 Operation Mode: Passive Target Memory TSN Load       */
#define ST25R3916_PT_MEM_READ \
    (0xBFU) /*!< ST25R3916 Operation Mode: Passive Target Memory Read           */

#define ST25R3916_CMD_LEN \
    (1U) /*!< ST25R3916 CMD length                                           */
#define ST25R3916_BUF_LEN \
    (ST25R3916_CMD_LEN +  \
     ST25R3916_FIFO_DEPTH) /*!< ST25R3916 communication buffer: CMD + FIFO length    */

/*
******************************************************************************
* MACROS
******************************************************************************
*/
#ifdef RFAL_USE_I2C
#define st25r3916I2CStart() \
    platformI2CStart() /*!< ST25R3916 HAL I2C driver macro to start a I2C transfer         */
#define st25r3916I2CStop() \
    platformI2CStop() /*!< ST25R3916 HAL I2C driver macro to stop a I2C transfer          */
#define st25r3916I2CRepeatStart() \
    platformI2CRepeatStart() /*!< ST25R3916 HAL I2C driver macro to repeat Start                 */
#define st25r3916I2CSlaveAddrWR(sA) \
    platformI2CSlaveAddrWR(         \
        sA) /*!< ST25R3916 HAL I2C driver macro to repeat Start                 */
#define st25r3916I2CSlaveAddrRD(sA) \
    platformI2CSlaveAddrRD(         \
        sA) /*!< ST25R3916 HAL I2C driver macro to repeat Start                 */
#endif /* RFAL_USE_I2C */

#if defined(ST25R_COM_SINGLETXRX) && !defined(RFAL_USE_I2C)
static uint8_t
    comBuf[ST25R3916_BUF_LEN]; /*!< ST25R3916 communication buffer                                 */
static uint16_t comBufIt; /*!< ST25R3916 communication buffer iterator                        */
#endif /* ST25R_COM_SINGLETXRX */

/*
 ******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 ******************************************************************************
 */

/*!
 ******************************************************************************
 * \brief ST25R3916 communication Start
 * 
 * This method performs the required actions to start communications with 
 * ST25R3916, either by SPI or I2C 
 ******************************************************************************
 */
static void st25r3916comStart(void);

/*!
 ******************************************************************************
 * \brief ST25R3916 communication Stop
 * 
 * This method performs the required actions to terminate communications with 
 * ST25R3916, either by SPI or I2C 
 ******************************************************************************
 */
static void st25r3916comStop(void);

/*!
 ******************************************************************************
 * \brief ST25R3916 communication Repeat Start
 * 
 * This method performs the required actions to repeat start a transmission
 * with ST25R3916, either by SPI or I2C 
 ******************************************************************************
 */
#ifdef RFAL_USE_I2C
static void st25r3916comRepeatStart(void);
#else
#define st25r3916comRepeatStart()
#endif /* RFAL_USE_I2C */

/*!
 ******************************************************************************
 * \brief ST25R3916 communication Tx
 * 
 * This method performs the required actions to transmit the given buffer
 * to ST25R3916, either by SPI or I2C
 * 
 * \param[in]  txBuf : the buffer to transmit
 * \param[in]  txLen : the length of the buffer to transmit
 * \param[in]  last   : true if last data to be transmitted
 * \param[in]  txOnly : true no reception is to be performed
 *  
 ******************************************************************************
 */
static void st25r3916comTx(const uint8_t* txBuf, uint16_t txLen, bool last, bool txOnly);

/*!
 ******************************************************************************
 * \brief ST25R3916 communication Rx
 * 
 * This method performs the required actions to receive from ST25R3916 the given 
 * amount of bytes, either by SPI or I2C
 * 
 * \param[out]  rxBuf : the buffer place the received bytes
 * \param[in]   rxLen : the length to receive
 *  
 ******************************************************************************
 */
static void st25r3916comRx(uint8_t* rxBuf, uint16_t rxLen);

/*!
 ******************************************************************************
 * \brief ST25R3916 communication Tx Byte
 * 
 * This helper method transmits a byte passed by value and not by reference
 * 
 * \param[in]   txByte : the value of the byte to be transmitted
 * \param[in]   last   : true if last byte to be transmitted
 * \param[in]   txOnly : true no reception is to be performed
 *  
 ******************************************************************************
 */
static void st25r3916comTxByte(uint8_t txByte, bool last, bool txOnly);

/*
 ******************************************************************************
 * LOCAL FUNCTION
 ******************************************************************************
 */
static void st25r3916comStart(void) {
    /* Make this operation atomic, disabling ST25R3916 interrupt during communications*/
    platformProtectST25RComm();

#ifdef RFAL_USE_I2C
    /* I2C Start and send Slave Address */
    st25r3916I2CStart();
    st25r3916I2CSlaveAddrWR(ST25R3916_I2C_ADDR);
#else
    /* Perform the chip select */
    platformSpiSelect();

#if defined(ST25R_COM_SINGLETXRX)
    comBufIt = 0; /* reset local buffer position   */
#endif /* ST25R_COM_SINGLETXRX */

#endif /* RFAL_USE_I2C */
}

/*******************************************************************************/
static void st25r3916comStop(void) {
#ifdef RFAL_USE_I2C
    /* Generate Stop signal */
    st25r3916I2CStop();
#else
    /* Release the chip select */
    platformSpiDeselect();
#endif /* RFAL_USE_I2C */

    /* reEnable the ST25R3916 interrupt */
    platformUnprotectST25RComm();
}

/*******************************************************************************/
#ifdef RFAL_USE_I2C
static void st25r3916comRepeatStart(void) {
    st25r3916I2CRepeatStart();
    st25r3916I2CSlaveAddrRD(ST25R3916_I2C_ADDR);
}
#endif /* RFAL_USE_I2C */

/*******************************************************************************/
static void st25r3916comTx(const uint8_t* txBuf, uint16_t txLen, bool last, bool txOnly) {
    NO_WARNING(last);
    NO_WARNING(txOnly);

    if(txLen > 0U) {
#ifdef RFAL_USE_I2C
        platformI2CTx(txBuf, txLen, last, txOnly);
#else /* RFAL_USE_I2C */

#ifdef ST25R_COM_SINGLETXRX

        ST_MEMCPY(
            &comBuf[comBufIt],
            txBuf,
            MIN(txLen,
                (ST25R3916_BUF_LEN -
                 comBufIt))); /* copy tx data to local buffer                      */
        comBufIt +=
            MIN(txLen,
                (ST25R3916_BUF_LEN -
                 comBufIt)); /* store position on local buffer                    */

        if(last && txOnly) /* only perform SPI transaction if no Rx will follow */
        {
            platformSpiTxRx(comBuf, NULL, comBufIt);
        }

#else
        platformSpiTxRx(txBuf, NULL, txLen);
#endif /* ST25R_COM_SINGLETXRX */

#endif /* RFAL_USE_I2C */
    }
}

/*******************************************************************************/
static void st25r3916comRx(uint8_t* rxBuf, uint16_t rxLen) {
    if(rxLen > 0U) {
#ifdef RFAL_USE_I2C
        platformI2CRx(rxBuf, rxLen);
#else /* RFAL_USE_I2C */

#ifdef ST25R_COM_SINGLETXRX
        ST_MEMSET(
            &comBuf[comBufIt],
            0x00,
            MIN(rxLen,
                (ST25R3916_BUF_LEN -
                 comBufIt))); /* clear outgoing buffer                                  */
        platformSpiTxRx(
            comBuf,
            comBuf,
            MIN((comBufIt + rxLen),
                ST25R3916_BUF_LEN)); /* transceive as a single SPI call                        */
        ST_MEMCPY(
            rxBuf,
            &comBuf[comBufIt],
            MIN(rxLen,
                (ST25R3916_BUF_LEN -
                 comBufIt))); /* copy from local buf to output buffer and skip cmd byte */
#else
        if(rxBuf != NULL) {
            ST_MEMSET(
                rxBuf, 0x00, rxLen); /* clear outgoing buffer                                  */
        }
        platformSpiTxRx(NULL, rxBuf, rxLen);
#endif /* ST25R_COM_SINGLETXRX */
#endif /* RFAL_USE_I2C */
    }
}

/*******************************************************************************/
static void st25r3916comTxByte(uint8_t txByte, bool last, bool txOnly) {
    uint8_t val = txByte; /* MISRA 17.8: use intermediate variable */
    st25r3916comTx(&val, ST25R3916_REG_LEN, last, txOnly);
}

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
ReturnCode st25r3916ReadRegister(uint8_t reg, uint8_t* val) {
    return st25r3916ReadMultipleRegisters(reg, val, ST25R3916_REG_LEN);
}

/*******************************************************************************/
ReturnCode st25r3916ReadMultipleRegisters(uint8_t reg, uint8_t* values, uint8_t length) {
    if(length > 0U) {
        st25r3916comStart();

        /* If is a space-B register send a direct command first */
        if((reg & ST25R3916_SPACE_B) != 0U) {
            st25r3916comTxByte(ST25R3916_CMD_SPACE_B_ACCESS, false, false);
        }

        st25r3916comTxByte(((reg & ~ST25R3916_SPACE_B) | ST25R3916_READ_MODE), true, false);
        st25r3916comRepeatStart();
        st25r3916comRx(values, length);
        st25r3916comStop();
    }

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916WriteRegister(uint8_t reg, uint8_t val) {
    uint8_t value = val; /* MISRA 17.8: use intermediate variable */
    return st25r3916WriteMultipleRegisters(reg, &value, ST25R3916_REG_LEN);
}

/*******************************************************************************/
ReturnCode st25r3916WriteMultipleRegisters(uint8_t reg, const uint8_t* values, uint8_t length) {
    if(length > 0U) {
        st25r3916comStart();

        if((reg & ST25R3916_SPACE_B) != 0U) {
            st25r3916comTxByte(ST25R3916_CMD_SPACE_B_ACCESS, false, true);
        }

        st25r3916comTxByte(((reg & ~ST25R3916_SPACE_B) | ST25R3916_WRITE_MODE), false, true);
        st25r3916comTx(values, length, true, true);
        st25r3916comStop();

        /* Send a WriteMultiReg event to LED handling */
        st25r3916ledEvtWrMultiReg(reg, values, length);
    }

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916WriteFifo(const uint8_t* values, uint16_t length) {
    if(length > ST25R3916_FIFO_DEPTH) {
        return ERR_PARAM;
    }

    if(length > 0U) {
        st25r3916comStart();
        st25r3916comTxByte(ST25R3916_FIFO_LOAD, false, true);
        st25r3916comTx(values, length, true, true);
        st25r3916comStop();
    }

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916ReadFifo(uint8_t* buf, uint16_t length) {
    if(length > 0U) {
        st25r3916comStart();
        st25r3916comTxByte(ST25R3916_FIFO_READ, true, false);

        st25r3916comRepeatStart();
        st25r3916comRx(buf, length);
        st25r3916comStop();
    }

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916WritePTMem(const uint8_t* values, uint16_t length) {
    if(length > ST25R3916_PTM_LEN) {
        return ERR_PARAM;
    }

    if(length > 0U) {
        st25r3916comStart();
        st25r3916comTxByte(ST25R3916_PT_A_CONFIG_LOAD, false, true);
        st25r3916comTx(values, length, true, true);
        st25r3916comStop();
    }

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916ReadPTMem(uint8_t* values, uint16_t length) {
    uint8_t
        tmp[ST25R3916_REG_LEN +
            ST25R3916_PTM_LEN]; /* local buffer to handle prepended byte on I2C and SPI */

    if(length > 0U) {
        if(length > ST25R3916_PTM_LEN) {
            return ERR_PARAM;
        }

        st25r3916comStart();
        st25r3916comTxByte(ST25R3916_PT_MEM_READ, true, false);

        st25r3916comRepeatStart();
        st25r3916comRx(tmp, (ST25R3916_REG_LEN + length)); /* skip prepended byte */
        st25r3916comStop();

        /* Copy PTMem content without prepended byte */
        ST_MEMCPY(values, (tmp + ST25R3916_REG_LEN), length);
    }

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916WritePTMemF(const uint8_t* values, uint16_t length) {
    if(length > (ST25R3916_PTM_F_LEN + ST25R3916_PTM_TSN_LEN)) {
        return ERR_PARAM;
    }

    if(length > 0U) {
        st25r3916comStart();
        st25r3916comTxByte(ST25R3916_PT_F_CONFIG_LOAD, false, true);
        st25r3916comTx(values, length, true, true);
        st25r3916comStop();
    }

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916WritePTMemTSN(const uint8_t* values, uint16_t length) {
    if(length > ST25R3916_PTM_TSN_LEN) {
        return ERR_PARAM;
    }

    if(length > 0U) {
        st25r3916comStart();
        st25r3916comTxByte(ST25R3916_PT_TSN_DATA_LOAD, false, true);
        st25r3916comTx(values, length, true, true);
        st25r3916comStop();
    }

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916ExecuteCommand(uint8_t cmd) {
    st25r3916comStart();
    st25r3916comTxByte((cmd | ST25R3916_CMD_MODE), true, true);
    st25r3916comStop();

    /* Send a cmd event to LED handling */
    st25r3916ledEvtCmd(cmd);

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916ReadTestRegister(uint8_t reg, uint8_t* val) {
    st25r3916comStart();
    st25r3916comTxByte(ST25R3916_CMD_TEST_ACCESS, false, false);
    st25r3916comTxByte((reg | ST25R3916_READ_MODE), true, false);
    st25r3916comRepeatStart();
    st25r3916comRx(val, ST25R3916_REG_LEN);
    st25r3916comStop();

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916WriteTestRegister(uint8_t reg, uint8_t val) {
    uint8_t value = val; /* MISRA 17.8: use intermediate variable */

    st25r3916comStart();
    st25r3916comTxByte(ST25R3916_CMD_TEST_ACCESS, false, true);
    st25r3916comTxByte((reg | ST25R3916_WRITE_MODE), false, true);
    st25r3916comTx(&value, ST25R3916_REG_LEN, true, true);
    st25r3916comStop();

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode st25r3916ClrRegisterBits(uint8_t reg, uint8_t clr_mask) {
    ReturnCode ret;
    uint8_t rdVal;

    /* Read current reg value */
    EXIT_ON_ERR(ret, st25r3916ReadRegister(reg, &rdVal));

    /* Only perform a Write if value to be written is different */
    if(ST25R3916_OPTIMIZE && (rdVal == (uint8_t)(rdVal & ~clr_mask))) {
        return ERR_NONE;
    }

    /* Write new reg value */
    return st25r3916WriteRegister(reg, (uint8_t)(rdVal & ~clr_mask));
}

/*******************************************************************************/
ReturnCode st25r3916SetRegisterBits(uint8_t reg, uint8_t set_mask) {
    ReturnCode ret;
    uint8_t rdVal;

    /* Read current reg value */
    EXIT_ON_ERR(ret, st25r3916ReadRegister(reg, &rdVal));

    /* Only perform a Write if the value to be written is different */
    if(ST25R3916_OPTIMIZE && (rdVal == (rdVal | set_mask))) {
        return ERR_NONE;
    }

    /* Write new reg value */
    return st25r3916WriteRegister(reg, (rdVal | set_mask));
}

/*******************************************************************************/
ReturnCode st25r3916ChangeRegisterBits(uint8_t reg, uint8_t valueMask, uint8_t value) {
    return st25r3916ModifyRegister(reg, valueMask, (valueMask & value));
}

/*******************************************************************************/
ReturnCode st25r3916ModifyRegister(uint8_t reg, uint8_t clr_mask, uint8_t set_mask) {
    ReturnCode ret;
    uint8_t rdVal;
    uint8_t wrVal;

    /* Read current reg value */
    EXIT_ON_ERR(ret, st25r3916ReadRegister(reg, &rdVal));

    /* Compute new value */
    wrVal = (uint8_t)(rdVal & ~clr_mask);
    wrVal |= set_mask;

    /* Only perform a Write if the value to be written is different */
    if(ST25R3916_OPTIMIZE && (rdVal == wrVal)) {
        return ERR_NONE;
    }

    /* Write new reg value */
    return st25r3916WriteRegister(reg, wrVal);
}

/*******************************************************************************/
ReturnCode st25r3916ChangeTestRegisterBits(uint8_t reg, uint8_t valueMask, uint8_t value) {
    ReturnCode ret;
    uint8_t rdVal;
    uint8_t wrVal;

    /* Read current reg value */
    EXIT_ON_ERR(ret, st25r3916ReadTestRegister(reg, &rdVal));

    /* Compute new value */
    wrVal = (uint8_t)(rdVal & ~valueMask);
    wrVal |= (uint8_t)(value & valueMask);

    /* Only perform a Write if the value to be written is different */
    if(ST25R3916_OPTIMIZE && (rdVal == wrVal)) {
        return ERR_NONE;
    }

    /* Write new reg value */
    return st25r3916WriteTestRegister(reg, wrVal);
}

/*******************************************************************************/
bool st25r3916CheckReg(uint8_t reg, uint8_t mask, uint8_t val) {
    uint8_t regVal;

    regVal = 0;
    st25r3916ReadRegister(reg, &regVal);

    return ((regVal & mask) == val);
}

/*******************************************************************************/
bool st25r3916IsRegValid(uint8_t reg) {
#pragma GCC diagnostic ignored "-Wtype-limits"
    if(!(((int16_t)reg >= (int32_t)ST25R3916_REG_IO_CONF1) &&
         (reg <= (ST25R3916_SPACE_B | ST25R3916_REG_IC_IDENTITY)))) {
        return false;
    }
    return true;
}
