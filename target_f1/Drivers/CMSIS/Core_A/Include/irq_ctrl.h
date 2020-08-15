/**************************************************************************//**
 * @file     irq_ctrl.h
 * @brief    Interrupt Controller API header file
 * @version  V1.0.0
 * @date     23. June 2017
 ******************************************************************************/
/*
 * Copyright (c) 2017 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if   defined ( __ICCARM__ )
  #pragma system_include         /* treat file as system include file for MISRA check */
#elif defined (__clang__)
  #pragma clang system_header   /* treat file as system include file */
#endif

#ifndef IRQ_CTRL_H_
#define IRQ_CTRL_H_

#include <stdint.h>

#ifndef IRQHANDLER_T
#define IRQHANDLER_T
/// Interrupt handler data type
typedef void (*IRQHandler_t) (void);
#endif

#ifndef IRQN_ID_T
#define IRQN_ID_T
/// Interrupt ID number data type
typedef int32_t IRQn_ID_t;
#endif

/* Interrupt mode bit-masks */
#define IRQ_MODE_TRIG_Pos           (0U)
#define IRQ_MODE_TRIG_Msk           (0x07UL /*<< IRQ_MODE_TRIG_Pos*/)
#define IRQ_MODE_TRIG_LEVEL         (0x00UL /*<< IRQ_MODE_TRIG_Pos*/) ///< Trigger: level triggered interrupt
#define IRQ_MODE_TRIG_LEVEL_LOW     (0x01UL /*<< IRQ_MODE_TRIG_Pos*/) ///< Trigger: low level triggered interrupt
#define IRQ_MODE_TRIG_LEVEL_HIGH    (0x02UL /*<< IRQ_MODE_TRIG_Pos*/) ///< Trigger: high level triggered interrupt
#define IRQ_MODE_TRIG_EDGE          (0x04UL /*<< IRQ_MODE_TRIG_Pos*/) ///< Trigger: edge triggered interrupt
#define IRQ_MODE_TRIG_EDGE_RISING   (0x05UL /*<< IRQ_MODE_TRIG_Pos*/) ///< Trigger: rising edge triggered interrupt
#define IRQ_MODE_TRIG_EDGE_FALLING  (0x06UL /*<< IRQ_MODE_TRIG_Pos*/) ///< Trigger: falling edge triggered interrupt
#define IRQ_MODE_TRIG_EDGE_BOTH     (0x07UL /*<< IRQ_MODE_TRIG_Pos*/) ///< Trigger: rising and falling edge triggered interrupt

#define IRQ_MODE_TYPE_Pos           (3U)
#define IRQ_MODE_TYPE_Msk           (0x01UL << IRQ_MODE_TYPE_Pos)
#define IRQ_MODE_TYPE_IRQ           (0x00UL << IRQ_MODE_TYPE_Pos)     ///< Type: interrupt source triggers CPU IRQ line
#define IRQ_MODE_TYPE_FIQ           (0x01UL << IRQ_MODE_TYPE_Pos)     ///< Type: interrupt source triggers CPU FIQ line

#define IRQ_MODE_DOMAIN_Pos         (4U)
#define IRQ_MODE_DOMAIN_Msk         (0x01UL << IRQ_MODE_DOMAIN_Pos)
#define IRQ_MODE_DOMAIN_NONSECURE   (0x00UL << IRQ_MODE_DOMAIN_Pos)   ///< Domain: interrupt is targeting non-secure domain
#define IRQ_MODE_DOMAIN_SECURE      (0x01UL << IRQ_MODE_DOMAIN_Pos)   ///< Domain: interrupt is targeting secure domain

#define IRQ_MODE_CPU_Pos            (5U)
#define IRQ_MODE_CPU_Msk            (0xFFUL << IRQ_MODE_CPU_Pos)
#define IRQ_MODE_CPU_ALL            (0x00UL << IRQ_MODE_CPU_Pos)      ///< CPU: interrupt targets all CPUs
#define IRQ_MODE_CPU_0              (0x01UL << IRQ_MODE_CPU_Pos)      ///< CPU: interrupt targets CPU 0
#define IRQ_MODE_CPU_1              (0x02UL << IRQ_MODE_CPU_Pos)      ///< CPU: interrupt targets CPU 1
#define IRQ_MODE_CPU_2              (0x04UL << IRQ_MODE_CPU_Pos)      ///< CPU: interrupt targets CPU 2
#define IRQ_MODE_CPU_3              (0x08UL << IRQ_MODE_CPU_Pos)      ///< CPU: interrupt targets CPU 3
#define IRQ_MODE_CPU_4              (0x10UL << IRQ_MODE_CPU_Pos)      ///< CPU: interrupt targets CPU 4
#define IRQ_MODE_CPU_5              (0x20UL << IRQ_MODE_CPU_Pos)      ///< CPU: interrupt targets CPU 5
#define IRQ_MODE_CPU_6              (0x40UL << IRQ_MODE_CPU_Pos)      ///< CPU: interrupt targets CPU 6
#define IRQ_MODE_CPU_7              (0x80UL << IRQ_MODE_CPU_Pos)      ///< CPU: interrupt targets CPU 7

#define IRQ_MODE_ERROR              (0x80000000UL)                    ///< Bit indicating mode value error

/* Interrupt priority bit-masks */
#define IRQ_PRIORITY_Msk            (0x0000FFFFUL)                    ///< Interrupt priority value bit-mask
#define IRQ_PRIORITY_ERROR          (0x80000000UL)                    ///< Bit indicating priority value error

/// Initialize interrupt controller.
/// \return 0 on success, -1 on error.
int32_t IRQ_Initialize (void);

/// Register interrupt handler.
/// \param[in]     irqn          interrupt ID number
/// \param[in]     handler       interrupt handler function address
/// \return 0 on success, -1 on error.
int32_t IRQ_SetHandler (IRQn_ID_t irqn, IRQHandler_t handler);

/// Get the registered interrupt handler.
/// \param[in]     irqn          interrupt ID number
/// \return registered interrupt handler function address.
IRQHandler_t IRQ_GetHandler (IRQn_ID_t irqn);

/// Enable interrupt.
/// \param[in]     irqn          interrupt ID number
/// \return 0 on success, -1 on error.
int32_t IRQ_Enable (IRQn_ID_t irqn);

/// Disable interrupt.
/// \param[in]     irqn          interrupt ID number
/// \return 0 on success, -1 on error.
int32_t IRQ_Disable (IRQn_ID_t irqn);

/// Get interrupt enable state.
/// \param[in]     irqn          interrupt ID number
/// \return 0 - interrupt is disabled, 1 - interrupt is enabled.
uint32_t IRQ_GetEnableState (IRQn_ID_t irqn);

/// Configure interrupt request mode.
/// \param[in]     irqn          interrupt ID number
/// \param[in]     mode          mode configuration
/// \return 0 on success, -1 on error.
int32_t IRQ_SetMode (IRQn_ID_t irqn, uint32_t mode);

/// Get interrupt mode configuration.
/// \param[in]     irqn          interrupt ID number
/// \return current interrupt mode configuration with optional IRQ_MODE_ERROR bit set.
uint32_t IRQ_GetMode (IRQn_ID_t irqn);

/// Get ID number of current interrupt request (IRQ).
/// \return interrupt ID number.
IRQn_ID_t IRQ_GetActiveIRQ (void);

/// Get ID number of current fast interrupt request (FIQ).
/// \return interrupt ID number.
IRQn_ID_t IRQ_GetActiveFIQ (void);

/// Signal end of interrupt processing.
/// \param[in]     irqn          interrupt ID number
/// \return 0 on success, -1 on error.
int32_t IRQ_EndOfInterrupt (IRQn_ID_t irqn);

/// Set interrupt pending flag.
/// \param[in]     irqn          interrupt ID number
/// \return 0 on success, -1 on error.
int32_t IRQ_SetPending (IRQn_ID_t irqn);

/// Get interrupt pending flag.
/// \param[in]     irqn          interrupt ID number
/// \return 0 - interrupt is not pending, 1 - interrupt is pending.
uint32_t IRQ_GetPending (IRQn_ID_t irqn);

/// Clear interrupt pending flag.
/// \param[in]     irqn          interrupt ID number
/// \return 0 on success, -1 on error.
int32_t IRQ_ClearPending (IRQn_ID_t irqn);

/// Set interrupt priority value.
/// \param[in]     irqn          interrupt ID number
/// \param[in]     priority      interrupt priority value
/// \return 0 on success, -1 on error.
int32_t IRQ_SetPriority (IRQn_ID_t irqn, uint32_t priority);

/// Get interrupt priority.
/// \param[in]     irqn          interrupt ID number
/// \return current interrupt priority value with optional IRQ_PRIORITY_ERROR bit set.
uint32_t IRQ_GetPriority (IRQn_ID_t irqn);

/// Set priority masking threshold.
/// \param[in]     priority      priority masking threshold value
/// \return 0 on success, -1 on error.
int32_t IRQ_SetPriorityMask (uint32_t priority);

/// Get priority masking threshold
/// \return current priority masking threshold value with optional IRQ_PRIORITY_ERROR bit set.
uint32_t IRQ_GetPriorityMask (void);

/// Set priority grouping field split point
/// \param[in]     bits          number of MSB bits included in the group priority field comparison
/// \return 0 on success, -1 on error.
int32_t IRQ_SetPriorityGroupBits (uint32_t bits);

/// Get priority grouping field split point
/// \return current number of MSB bits included in the group priority field comparison with
///         optional IRQ_PRIORITY_ERROR bit set.
uint32_t IRQ_GetPriorityGroupBits (void);

#endif  // IRQ_CTRL_H_
