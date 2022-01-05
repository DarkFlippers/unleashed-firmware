
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
 *  \brief ST25R3916 Interrupt handling
 *
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/

#include "st25r3916_irq.h"
#include "st25r3916_com.h"
#include "st25r3916_led.h"
#include "st25r3916.h"
#include "utils.h"

/*
 ******************************************************************************
 * LOCAL DATA TYPES
 ******************************************************************************
 */

/*! Holds current and previous interrupt callback pointer as well as current Interrupt status and mask */
typedef struct {
    void (*prevCallback)(void); /*!< call back function for ST25R3916 interrupt          */
    void (*callback)(void); /*!< call back function for ST25R3916 interrupt          */
    uint32_t status; /*!< latest interrupt status                             */
    uint32_t mask; /*!< Interrupt mask. Negative mask = ST25R3916 mask regs */
} st25r3916Interrupt;

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

/*! Length of the interrupt registers       */
#define ST25R3916_INT_REGS_LEN ((ST25R3916_REG_IRQ_TARGET - ST25R3916_REG_IRQ_MAIN) + 1U)

/*
******************************************************************************
* GLOBAL VARIABLES
******************************************************************************
*/

static volatile st25r3916Interrupt st25r3916interrupt; /*!< Instance of ST25R3916 interrupt */

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
void st25r3916InitInterrupts(void) {
    platformIrqST25RPinInitialize();
    platformIrqST25RSetCallback(st25r3916Isr);

    st25r3916interrupt.callback = NULL;
    st25r3916interrupt.prevCallback = NULL;
    st25r3916interrupt.status = ST25R3916_IRQ_MASK_NONE;
    st25r3916interrupt.mask = ST25R3916_IRQ_MASK_NONE;
}

/*******************************************************************************/
void st25r3916Isr(void) {
    st25r3916CheckForReceivedInterrupts();

    // Check if callback is set and run it
    if(NULL != st25r3916interrupt.callback) {
        st25r3916interrupt.callback();
    }
}

/*******************************************************************************/
void st25r3916CheckForReceivedInterrupts(void) {
    uint8_t iregs[ST25R3916_INT_REGS_LEN];
    uint32_t irqStatus;

    /* Initialize iregs */
    irqStatus = ST25R3916_IRQ_MASK_NONE;
    ST_MEMSET(iregs, (int32_t)(ST25R3916_IRQ_MASK_ALL & 0xFFU), ST25R3916_INT_REGS_LEN);

    /* In case the IRQ is Edge (not Level) triggered read IRQs until done */
    while(platformGpioIsHigh(ST25R_INT_PORT, ST25R_INT_PIN)) {
        st25r3916ReadMultipleRegisters(ST25R3916_REG_IRQ_MAIN, iregs, ST25R3916_INT_REGS_LEN);

        irqStatus |= (uint32_t)iregs[0];
        irqStatus |= (uint32_t)iregs[1] << 8;
        irqStatus |= (uint32_t)iregs[2] << 16;
        irqStatus |= (uint32_t)iregs[3] << 24;
    }

    /* Forward all interrupts, even masked ones to application */
    platformProtectST25RIrqStatus();
    st25r3916interrupt.status |= irqStatus;
    platformUnprotectST25RIrqStatus();

    /* Send an IRQ event to LED handling */
    st25r3916ledEvtIrq(st25r3916interrupt.status);
}

/*******************************************************************************/
void st25r3916ModifyInterrupts(uint32_t clr_mask, uint32_t set_mask) {
    uint8_t i;
    uint32_t old_mask;
    uint32_t new_mask;

    old_mask = st25r3916interrupt.mask;
    new_mask = ((~old_mask & set_mask) | (old_mask & clr_mask));
    st25r3916interrupt.mask &= ~clr_mask;
    st25r3916interrupt.mask |= set_mask;

    for(i = 0; i < ST25R3916_INT_REGS_LEN; i++) {
        if(((new_mask >> (8U * i)) & 0xFFU) == 0U) {
            continue;
        }

        st25r3916WriteRegister(
            ST25R3916_REG_IRQ_MASK_MAIN + i,
            (uint8_t)((st25r3916interrupt.mask >> (8U * i)) & 0xFFU));
    }
    return;
}

/*******************************************************************************/
uint32_t st25r3916WaitForInterruptsTimed(uint32_t mask, uint16_t tmo) {
    uint32_t tmrDelay;
    uint32_t status;

    tmrDelay = platformTimerCreate(tmo);

    /* Run until specific interrupt has happen or the timer has expired */
    do {
        status = (st25r3916interrupt.status & mask);
    } while((!platformTimerIsExpired(tmrDelay) || (tmo == 0U)) && (status == 0U));

    platformTimerDestroy(tmrDelay);

    status = st25r3916interrupt.status & mask;

    platformProtectST25RIrqStatus();
    st25r3916interrupt.status &= ~status;
    platformUnprotectST25RIrqStatus();

    return status;
}

/*******************************************************************************/
uint32_t st25r3916GetInterrupt(uint32_t mask) {
    uint32_t irqs;

    irqs = (st25r3916interrupt.status & mask);
    if(irqs != ST25R3916_IRQ_MASK_NONE) {
        platformProtectST25RIrqStatus();
        st25r3916interrupt.status &= ~irqs;
        platformUnprotectST25RIrqStatus();
    }

    return irqs;
}

/*******************************************************************************/
void st25r3916ClearAndEnableInterrupts(uint32_t mask) {
    st25r3916GetInterrupt(mask);
    st25r3916EnableInterrupts(mask);
}

/*******************************************************************************/
void st25r3916EnableInterrupts(uint32_t mask) {
    st25r3916ModifyInterrupts(mask, 0);
}

/*******************************************************************************/
void st25r3916DisableInterrupts(uint32_t mask) {
    st25r3916ModifyInterrupts(0, mask);
}

/*******************************************************************************/
void st25r3916ClearInterrupts(void) {
    uint8_t iregs[ST25R3916_INT_REGS_LEN];

    st25r3916ReadMultipleRegisters(ST25R3916_REG_IRQ_MAIN, iregs, ST25R3916_INT_REGS_LEN);

    platformProtectST25RIrqStatus();
    st25r3916interrupt.status = ST25R3916_IRQ_MASK_NONE;
    platformUnprotectST25RIrqStatus();
    return;
}

/*******************************************************************************/
void st25r3916IRQCallbackSet(void (*cb)(void)) {
    st25r3916interrupt.prevCallback = st25r3916interrupt.callback;
    st25r3916interrupt.callback = cb;
}

/*******************************************************************************/
void st25r3916IRQCallbackRestore(void) {
    st25r3916interrupt.callback = st25r3916interrupt.prevCallback;
    st25r3916interrupt.prevCallback = NULL;
}
