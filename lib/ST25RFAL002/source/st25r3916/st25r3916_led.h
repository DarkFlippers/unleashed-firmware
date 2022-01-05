
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
 *  \brief ST25R3916 LEDs handling
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
 * \addtogroup ST25R3916_LED
 * \brief RFAL ST25R3916 LED
 * @{
 * 
 */

#ifndef ST25R3916_LED_H
#define ST25R3916_LED_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/

#include "platform.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*! 
 *****************************************************************************
 *  \brief  ST25R3916 LED Initialize
 *
 *  This function initializes the LEDs that represent ST25R3916 activity 
 *
 *****************************************************************************
 */
void st25r3916ledInit(void);

/*! 
 *****************************************************************************
 *  \brief   ST25R3916 LED Event Interrupt
 *
 *  This function should be called upon a ST25R3916 Interrupt, providing 
 *  the interrupt event with ST25R3916 irq flags to update LEDs 
 *
 *  \param[in] irqs: ST25R3916 irqs mask
 *
 *****************************************************************************
 */
void st25r3916ledEvtIrq(uint32_t irqs);

/*! 
 *****************************************************************************
 *  \brief   ST25R3916 LED Event Write Register
 *
 *  This function should be called on a ST25R3916 Write Register operation
 *  providing the event with the register and value to update LEDs 
 *
 *  \param[in] reg: ST25R3916 register to be written
 *  \param[in] val: value to be written on the register
 *
 *****************************************************************************
 */
void st25r3916ledEvtWrReg(uint8_t reg, uint8_t val);

/*! 
 *****************************************************************************
 *  \brief   ST25R3916 LED Event Write Multiple Register
 *
 *  This function should be called upon a ST25R3916 Write Multiple Registers, 
 *  providing the event with the registers and values to update LEDs 
 *
 *  \param[in] reg : ST25R3916 first register written
 *  \param[in] vals: pointer to the values written
 *  \param[in] len : number of registers written
 *
 *****************************************************************************
 */
void st25r3916ledEvtWrMultiReg(uint8_t reg, const uint8_t* vals, uint8_t len);

/*! 
 *****************************************************************************
 *  \brief   ST25R3916 LED Event Direct Command
 *
 *  This function should be called upon a ST25R3916 direct command, providing 
 *  the event with the command executed
 *
 *  \param[in] cmd: ST25R3916 cmd executed
 *
 *****************************************************************************
 */
void st25r3916ledEvtCmd(uint8_t cmd);

#endif /* ST25R3916_LED_H */

/**
  * @}
  *
  * @}
  *
  * @}
  * 
  * @}
  */
