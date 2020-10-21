
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
 *      PROJECT:   ST25R391x firmware
 *      Revision:
 *      LANGUAGE:  ISO C99
 */

/*! \file rfal_chip.h
 *
 *  \author Gustavo Patricio 
 *
 *  \brief RF Chip specific Layer
 *  
 *  \warning This layer, which provides direct access to RF chip, should 
 *           only be used for debug purposes and/or advanced features
 *
 *
 * \addtogroup RFAL
 * @{
 *
 * \addtogroup RFAL-HAL
 * \brief RFAL Hardware Abstraction Layer
 * @{
 *
 * \addtogroup Chip
 * \brief RFAL RF Chip Module
 * @{
 * 
 */


#ifndef RFAL_CHIP_H
#define RFAL_CHIP_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"
#include "st_errno.h"
#include "rfal_rf.h"


/*****************************************************************************
 *  RF Chip                                                                  *  
 *****************************************************************************/

/*!
 *****************************************************************************
 * \brief Writes a register on the RF Chip
 *
 * Checks if the given register is valid and if so, writes the value(s)
 * on the RF Chip register
 * 
 * \param[in] reg: register address to be written, or the first if len > 1
 * \param[in] values: pointer with content to be written on the register(s)
 * \param[in] len: number of consecutive registers to be written
 *  
 * 
 * \return ERR_PARAM    : Invalid register or bad request
 * \return ERR_NOTSUPP  : Feature not supported
 * \return ERR_NONE     : Write done with no error
 *****************************************************************************
 */
ReturnCode rfalChipWriteReg( uint16_t reg, const uint8_t* values, uint8_t len );

/*!
 *****************************************************************************
 * \brief Reads a register on the RF Chip
 *
 * Checks if the given register is valid and if so, reads the value(s)
 * of the RF Chip register(s)
 * 
 * \param[in]  reg: register address to be read, or the first if len > 1
 * \param[out] values: pointer where the register(s) read content will be placed 
 * \param[in]  len: number of consecutive registers to be read  
 * 
 * \return ERR_PARAM    : Invalid register or bad request
 * \return ERR_NOTSUPP  : Feature not supported
 * \return ERR_NONE     : Read done with no error
 *****************************************************************************
 */
ReturnCode rfalChipReadReg( uint16_t reg, uint8_t* values, uint8_t len );

/*!
 *****************************************************************************
 * \brief Change a register on the RF Chip
 *
 * Change the value of the register bits on the RF Chip Test set in the valueMask. 
 * 
 * \param[in] reg: register address to be modified
 * \param[in] valueMask: mask value of the register bits to be changed
 * \param[in] value: register value to be set
 * 
 * \return ERR_PARAM    : Invalid register or bad request
 * \return ERR_NOTSUPP  : Feature not supported
 * \return ERR_OK       : Change done with no error
 *****************************************************************************
 */
ReturnCode rfalChipChangeRegBits( uint16_t reg, uint8_t valueMask, uint8_t value );

/*!
 *****************************************************************************
 * \brief Writes a Test register on the RF Chip
 *
 * Writes the value on the RF Chip Test register
 * 
 * \param[in] reg: register address to be written
 * \param[in] value: value to be written on the register
 *  
 * 
 * \return ERR_PARAM    : Invalid register or bad request
 * \return ERR_NOTSUPP  : Feature not supported
 * \return ERR_NONE     : Write done with no error
 *****************************************************************************
 */
ReturnCode rfalChipWriteTestReg( uint16_t reg, uint8_t value );

/*!
 *****************************************************************************
 * \brief Reads a Test register on the RF Chip
 *
 * Reads the value of the RF Chip Test register
 * 
 * \param[in]  reg: register address to be read
 * \param[out] value: pointer where the register content will be placed  
 * 
 * \return ERR_PARAM    :Invalid register or bad request
 * \return ERR_NOTSUPP  : Feature not supported
 * \return ERR_NONE     : Read done with no error
 *****************************************************************************
 */
ReturnCode rfalChipReadTestReg( uint16_t reg, uint8_t* value );

/*!
 *****************************************************************************
 * \brief Change a Test register on the RF Chip
 *
 * Change the value of the register bits on the RF Chip Test set in the valueMask. 
 * 
 * \param[in] reg: test register address to be modified
 * \param[in] valueMask: mask value of the register bits to be changed
 * \param[in] value: register value to be set
 * 
 * \return ERR_PARAM     : Invalid register or bad request
 * \return ERR_NOTSUPP   : Feature not supported
 * \return ERR_OK        : Change done with no error
 *****************************************************************************
 */
ReturnCode rfalChipChangeTestRegBits( uint16_t reg, uint8_t valueMask, uint8_t value );

/*!
 *****************************************************************************
 * \brief Execute command on the RF Chip
 *
 * Checks if the given command is valid and if so, executes it on 
 * the RF Chip
 * 
 * \param[in] cmd: direct command to be executed
 * 
 * \return ERR_PARAM     : Invalid command or bad request
 * \return  ERR_NOTSUPP  : Feature not supported
 * \return ERR_NONE      : Direct command executed with no error
 *****************************************************************************
 */
ReturnCode rfalChipExecCmd( uint16_t cmd );

/*! 
 *****************************************************************************
 * \brief  Set RFO
 *
 * Sets the RFO value to be used when the field is on (unmodulated/active)
 * 
 * \param[in] rfo : the RFO value to be used
 *
 * \return  ERR_IO           : Internal error
 * \return  ERR_NOTSUPP      : Feature not supported
 * \return  ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalChipSetRFO( uint8_t rfo );


/*! 
 *****************************************************************************
 * \brief  Get RFO
 *
 * Gets the RFO value used used when the field is on (unmodulated/active)
 *
 * \param[out] result : the current RFO value 
 *
 * \return  ERR_IO           : Internal error
 * \return  ERR_NOTSUPP      : Feature not supported
 * \return  ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalChipGetRFO( uint8_t* result );


/*! 
 *****************************************************************************
 * \brief  Measure Amplitude
 *
 * Measures the RF Amplitude
 *
 * \param[out] result : result of RF measurement
 *
 * \return  ERR_IO           : Internal error
 * \return  ERR_NOTSUPP      : Feature not supported
 * \return  ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalChipMeasureAmplitude( uint8_t* result );


/*! 
 *****************************************************************************
 * \brief  Measure Phase
 *
 * Measures the Phase
 *
 * \param[out] result : result of Phase measurement
 *
 * \return  ERR_IO           : Internal error
 * \return  ERR_NOTSUPP      : Feature not supported
 * \return  ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalChipMeasurePhase( uint8_t* result );


/*! 
 *****************************************************************************
 * \brief  Measure Capacitance
 *
 * Measures the Capacitance
 *
 * \param[out] result : result of Capacitance measurement
 *
 * \return  ERR_IO           : Internal error
 * \return  ERR_NOTSUPP      : Feature not supported
 * \return  ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalChipMeasureCapacitance( uint8_t* result );


/*! 
 *****************************************************************************
 * \brief  Measure Power Supply
 *
 * Measures the Power Supply
 *
 * \param[in]   param : measurement parameter (chip specific)
 * \param[out] result : result of the measurement
 *
 * \return  ERR_IO           : Internal error
 * \return  ERR_NOTSUPP      : Feature not supported
 * \return  ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalChipMeasurePowerSupply( uint8_t param, uint8_t* result );


#endif /* RFAL_CHIP_H */

/**
  * @}
  *
  * @}
  *
  * @}
  */
