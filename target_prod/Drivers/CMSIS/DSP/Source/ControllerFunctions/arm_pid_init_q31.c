/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_pid_init_q31.c
 * Description:  Q31 PID Control initialization function
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
 * @brief  Initialization function for the Q31 PID Control.
 * @param[in,out] *S points to an instance of the Q31 PID structure.
 * @param[in]     resetStateFlag  flag to reset the state. 0 = no change in state 1 = reset the state.
 * @return none.
 * \par Description:
 * \par
 * The <code>resetStateFlag</code> specifies whether to set state to zero or not. \n
 * The function computes the structure fields: <code>A0</code>, <code>A1</code> <code>A2</code>
 * using the proportional gain( \c Kp), integral gain( \c Ki) and derivative gain( \c Kd)
 * also sets the state variables to all zeros.
 */

void arm_pid_init_q31(
  arm_pid_instance_q31 * S,
  int32_t resetStateFlag)
{

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  /* Derived coefficient A0 */
  S->A0 = __QADD(__QADD(S->Kp, S->Ki), S->Kd);

  /* Derived coefficient A1 */
  S->A1 = -__QADD(__QADD(S->Kd, S->Kd), S->Kp);


#else

  /* Run the below code for Cortex-M0 */

  q31_t temp;

  /* Derived coefficient A0 */
  temp = clip_q63_to_q31((q63_t) S->Kp + S->Ki);
  S->A0 = clip_q63_to_q31((q63_t) temp + S->Kd);

  /* Derived coefficient A1 */
  temp = clip_q63_to_q31((q63_t) S->Kd + S->Kd);
  S->A1 = -clip_q63_to_q31((q63_t) temp + S->Kp);

#endif /* #if defined (ARM_MATH_DSP) */

  /* Derived coefficient A2 */
  S->A2 = S->Kd;

  /* Check whether state needs reset or not */
  if (resetStateFlag)
  {
    /* Clear the state buffer.  The size will be always 3 samples */
    memset(S->state, 0, 3U * sizeof(q31_t));
  }

}

/**
 * @} end of PID group
 */
