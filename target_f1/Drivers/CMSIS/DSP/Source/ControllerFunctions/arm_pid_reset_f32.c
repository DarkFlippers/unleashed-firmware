/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_pid_reset_f32.c
 * Description:  Floating-point PID Control reset function
 *
 * $Date:        27. January 2017
 * $Revision:    V.1.5.1
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2017 ARM Limited or its affiliates. All rights reserved.
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

#include "arm_math.h"

 /**
 * @addtogroup PID
 * @{
 */

/**
* @brief  Reset function for the floating-point PID Control.
* @param[in] *S	Instance pointer of PID control data structure.
* @return none.
* \par Description:
* The function resets the state buffer to zeros.
*/
void arm_pid_reset_f32(
  arm_pid_instance_f32 * S)
{

  /* Clear the state buffer.  The size will be always 3 samples */
  memset(S->state, 0, 3U * sizeof(float32_t));
}

/**
 * @} end of PID group
 */
