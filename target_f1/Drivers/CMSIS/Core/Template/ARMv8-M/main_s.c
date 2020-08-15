/******************************************************************************
 * @file     main_s.c
 * @brief    Code template for secure main function
 * @version  V1.1.1
 * @date     10. January 2018
 ******************************************************************************/
/*
 * Copyright (c) 2013-2018 Arm Limited. All rights reserved.
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

/* Use CMSE intrinsics */
#include <arm_cmse.h>
 
#include "RTE_Components.h"
#include CMSIS_device_header
 
/* TZ_START_NS: Start address of non-secure application */
#ifndef TZ_START_NS
#define TZ_START_NS (0x200000U)
#endif
 
/* typedef for non-secure callback functions */
typedef void (*funcptr_void) (void) __attribute__((cmse_nonsecure_call));
 
/* Secure main() */
int main(void) {
  funcptr_void NonSecure_ResetHandler;
 
  /* Add user setup code for secure part here*/
 
  /* Set non-secure main stack (MSP_NS) */
  __TZ_set_MSP_NS(*((uint32_t *)(TZ_START_NS)));
 
  /* Get non-secure reset handler */
  NonSecure_ResetHandler = (funcptr_void)(*((uint32_t *)((TZ_START_NS) + 4U)));
 
  /* Start non-secure state software application */
  NonSecure_ResetHandler();
 
  /* Non-secure software does not return, this code is not executed */
  while (1) {
    __NOP();
  }
}
