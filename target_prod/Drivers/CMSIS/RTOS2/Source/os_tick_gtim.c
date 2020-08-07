/**************************************************************************//**
 * @file     os_tick_gtim.c
 * @brief    CMSIS OS Tick implementation for Generic Timer
 * @version  V1.0.1
 * @date     24. November 2017
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

#include "os_tick.h"
#include "irq_ctrl.h"

#include "RTE_Components.h"
#include CMSIS_device_header

#ifndef GTIM_IRQ_PRIORITY
#define GTIM_IRQ_PRIORITY           0xFFU
#endif

#ifndef GTIM_IRQ_NUM
#define GTIM_IRQ_NUM                SecurePhyTimer_IRQn
#endif

// Timer interrupt pending flag
static uint8_t GTIM_PendIRQ;

// Timer tick frequency
static uint32_t GTIM_Clock;

// Timer load value
static uint32_t GTIM_Load;

// Setup OS Tick.
int32_t OS_Tick_Setup (uint32_t freq, IRQHandler_t handler) {
  uint32_t prio, bits;

  if (freq == 0U) {
    return (-1);
  }

  GTIM_PendIRQ = 0U;

  // Get timer clock
#ifdef SCTR_BASE
  GTIM_Clock = *(uint32_t*)(SCTR_BASE+0x20);
#else
  // FVP REFCLK CNTControl 100MHz
  GTIM_Clock = 100000000UL;
#endif

  PL1_SetCounterFrequency(GTIM_Clock);

  // Calculate load value
  GTIM_Load = (GTIM_Clock / freq) - 1U;

  // Disable Generic Timer and set load value
  PL1_SetControl(0U);
  PL1_SetLoadValue(GTIM_Load);

  // Disable corresponding IRQ
  IRQ_Disable(GTIM_IRQ_NUM);
  IRQ_ClearPending(GTIM_IRQ_NUM);

  // Determine number of implemented priority bits
  IRQ_SetPriority(GTIM_IRQ_NUM, 0xFFU);

  prio = IRQ_GetPriority(GTIM_IRQ_NUM);

  // At least bits [7:4] must be implemented
  if ((prio & 0xF0U) == 0U) {
    return (-1);
  }

  for (bits = 0; bits < 4; bits++) {
    if ((prio & 0x01) != 0) {
      break;
    }
    prio >>= 1;
  }
  
  // Adjust configured priority to the number of implemented priority bits
  prio = (GTIM_IRQ_PRIORITY << bits) & 0xFFUL;

  // Set Private Timer interrupt priority
  IRQ_SetPriority(GTIM_IRQ_NUM, prio-1U);

  // Set edge-triggered IRQ
  IRQ_SetMode(GTIM_IRQ_NUM, IRQ_MODE_TRIG_EDGE);

  // Register tick interrupt handler function
  IRQ_SetHandler(GTIM_IRQ_NUM, handler);

  // Enable corresponding interrupt
  IRQ_Enable(GTIM_IRQ_NUM);

  // Enable system counter and timer control
#ifdef SCTR_BASE
  *(uint32_t*)SCTR_BASE |= 3U;
#endif

  // Enable timer control
  PL1_SetControl(1U);

  return (0);
}

/// Enable OS Tick.
void OS_Tick_Enable (void) {
  uint32_t ctrl;

  // Set pending interrupt if flag set
  if (GTIM_PendIRQ != 0U) {
    GTIM_PendIRQ = 0U;
    IRQ_SetPending (GTIM_IRQ_NUM);
  }

  // Start the Private Timer
  ctrl = PL1_GetControl();
  // Set bit: Timer enable
  ctrl |= 1U;
  PL1_SetControl(ctrl);
}

/// Disable OS Tick.
void OS_Tick_Disable (void) {
  uint32_t ctrl;

  // Stop the Private Timer
  ctrl = PL1_GetControl();
  // Clear bit: Timer enable
  ctrl &= ~1U;
  PL1_SetControl(ctrl);

  // Remember pending interrupt flag
  if (IRQ_GetPending(GTIM_IRQ_NUM) != 0) {
    IRQ_ClearPending(GTIM_IRQ_NUM);
    GTIM_PendIRQ = 1U;
  }
}

// Acknowledge OS Tick IRQ.
void OS_Tick_AcknowledgeIRQ (void) {
  IRQ_ClearPending (GTIM_IRQ_NUM);
  PL1_SetLoadValue(GTIM_Load);
}

// Get OS Tick IRQ number.
int32_t  OS_Tick_GetIRQn (void) {
  return (GTIM_IRQ_NUM);
}

// Get OS Tick clock.
uint32_t OS_Tick_GetClock (void) {
  return (GTIM_Clock);
}

// Get OS Tick interval.
uint32_t OS_Tick_GetInterval (void) {
  return (GTIM_Load + 1U);
}

// Get OS Tick count value.
uint32_t OS_Tick_GetCount (void) {
  return (GTIM_Load - PL1_GetCurrentValue());
}

// Get OS Tick overflow status.
uint32_t OS_Tick_GetOverflow (void) {
  CNTP_CTL_Type cntp_ctl;
  cntp_ctl.w = PL1_GetControl();
  return (cntp_ctl.b.ISTATUS);
}
