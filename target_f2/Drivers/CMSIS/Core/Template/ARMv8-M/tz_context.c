/******************************************************************************
 * @file     tz_context.c
 * @brief    Context Management for Armv8-M TrustZone - Sample implementation
 * @version  V1.1.1
 * @date     10. January 2018
 ******************************************************************************/
/*
 * Copyright (c) 2016-2018 Arm Limited. All rights reserved.
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

#include "RTE_Components.h"
#include CMSIS_device_header
#include "tz_context.h"

/// Number of process slots (threads may call secure library code)
#ifndef TZ_PROCESS_STACK_SLOTS
#define TZ_PROCESS_STACK_SLOTS     8U
#endif

/// Stack size of the secure library code
#ifndef TZ_PROCESS_STACK_SIZE
#define TZ_PROCESS_STACK_SIZE      256U
#endif

typedef struct {
  uint32_t sp_top;      // stack space top
  uint32_t sp_limit;    // stack space limit
  uint32_t sp;          // current stack pointer
} stack_info_t;

static stack_info_t ProcessStackInfo  [TZ_PROCESS_STACK_SLOTS];
static uint64_t     ProcessStackMemory[TZ_PROCESS_STACK_SLOTS][TZ_PROCESS_STACK_SIZE/8U];
static uint32_t     ProcessStackFreeSlot = 0xFFFFFFFFU;


/// Initialize secure context memory system
/// \return execution status (1: success, 0: error)
__attribute__((cmse_nonsecure_entry))
uint32_t TZ_InitContextSystem_S (void) {
  uint32_t n;

  if (__get_IPSR() == 0U) {
    return 0U;  // Thread Mode
  }

  for (n = 0U; n < TZ_PROCESS_STACK_SLOTS; n++) {
    ProcessStackInfo[n].sp = 0U;
    ProcessStackInfo[n].sp_limit = (uint32_t)&ProcessStackMemory[n];
    ProcessStackInfo[n].sp_top   = (uint32_t)&ProcessStackMemory[n] + TZ_PROCESS_STACK_SIZE;
    *((uint32_t *)ProcessStackMemory[n]) = n + 1U;
  }
  *((uint32_t *)ProcessStackMemory[--n]) = 0xFFFFFFFFU;

  ProcessStackFreeSlot = 0U;

  // Default process stack pointer and stack limit
  __set_PSPLIM((uint32_t)ProcessStackMemory);
  __set_PSP   ((uint32_t)ProcessStackMemory);

  // Privileged Thread Mode using PSP
  __set_CONTROL(0x02U);

  return 1U;    // Success
}


/// Allocate context memory for calling secure software modules in TrustZone
/// \param[in]  module   identifies software modules called from non-secure mode
/// \return value != 0 id TrustZone memory slot identifier
/// \return value 0    no memory available or internal error
__attribute__((cmse_nonsecure_entry))
TZ_MemoryId_t TZ_AllocModuleContext_S (TZ_ModuleId_t module) {
  uint32_t slot;

  (void)module; // Ignore (fixed Stack size)

  if (__get_IPSR() == 0U) {
    return 0U;  // Thread Mode
  }

  if (ProcessStackFreeSlot == 0xFFFFFFFFU) {
    return 0U;  // No slot available
  }

  slot = ProcessStackFreeSlot;
  ProcessStackFreeSlot = *((uint32_t *)ProcessStackMemory[slot]);

  ProcessStackInfo[slot].sp = ProcessStackInfo[slot].sp_top;

  return (slot + 1U);
}


/// Free context memory that was previously allocated with \ref TZ_AllocModuleContext_S
/// \param[in]  id  TrustZone memory slot identifier
/// \return execution status (1: success, 0: error)
__attribute__((cmse_nonsecure_entry))
uint32_t TZ_FreeModuleContext_S (TZ_MemoryId_t id) {
  uint32_t slot;

  if (__get_IPSR() == 0U) {
    return 0U;  // Thread Mode
  }

  if ((id == 0U) || (id > TZ_PROCESS_STACK_SLOTS)) {
    return 0U;  // Invalid ID
  }

  slot = id - 1U;

  if (ProcessStackInfo[slot].sp == 0U) {
    return 0U;  // Inactive slot
  }
  ProcessStackInfo[slot].sp = 0U;

  *((uint32_t *)ProcessStackMemory[slot]) = ProcessStackFreeSlot;
  ProcessStackFreeSlot = slot;

  return 1U;    // Success
}


/// Load secure context (called on RTOS thread context switch)
/// \param[in]  id  TrustZone memory slot identifier
/// \return execution status (1: success, 0: error)
__attribute__((cmse_nonsecure_entry))
uint32_t TZ_LoadContext_S (TZ_MemoryId_t id) {
  uint32_t slot;

  if ((__get_IPSR() == 0U) || ((__get_CONTROL() & 2U) == 0U)) {
    return 0U;  // Thread Mode or using Main Stack for threads
  }

  if ((id == 0U) || (id > TZ_PROCESS_STACK_SLOTS)) {
    return 0U;  // Invalid ID
  }

  slot = id - 1U;

  if (ProcessStackInfo[slot].sp == 0U) {
    return 0U;  // Inactive slot
  }

  // Setup process stack pointer and stack limit
  __set_PSPLIM(ProcessStackInfo[slot].sp_limit);
  __set_PSP   (ProcessStackInfo[slot].sp);

  return 1U;    // Success
}


/// Store secure context (called on RTOS thread context switch)
/// \param[in]  id  TrustZone memory slot identifier
/// \return execution status (1: success, 0: error)
__attribute__((cmse_nonsecure_entry))
uint32_t TZ_StoreContext_S (TZ_MemoryId_t id) {
  uint32_t slot;
  uint32_t sp;

  if ((__get_IPSR() == 0U) || ((__get_CONTROL() & 2U) == 0U)) {
    return 0U;  // Thread Mode or using Main Stack for threads
  }

  if ((id == 0U) || (id > TZ_PROCESS_STACK_SLOTS)) {
    return 0U;  // Invalid ID
  }

  slot = id - 1U;

  if (ProcessStackInfo[slot].sp == 0U) {
    return 0U;  // Inactive slot
  }

  sp = __get_PSP();
  if ((sp < ProcessStackInfo[slot].sp_limit) ||
      (sp > ProcessStackInfo[slot].sp_top)) {
    return 0U;  // SP out of range
  }
  ProcessStackInfo[slot].sp = sp;

  // Default process stack pointer and stack limit
  __set_PSPLIM((uint32_t)ProcessStackMemory);
  __set_PSP   ((uint32_t)ProcessStackMemory);

  return 1U;    // Success
}
