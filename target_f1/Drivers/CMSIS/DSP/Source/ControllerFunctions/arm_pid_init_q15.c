/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_pid_init_q15.c
 * Description:  Q15 PID Control initialization function
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
 * @details
 * @param[in,out] *S points to an instance of the Q15 PID structure.
 * @param[in]     resetStateFlag  flag to reset the state. 0 = no change in state 1 = reset the state.
 * @return none.
 * \par Description:
 * \par
 * The <code>resetStateFlag</code> specifies whether to set state to zero or not. \n
 * The function computes the structure fields: <code>A0</code>, <code>A1</code> <code>A2</code>
 * using the proportional gain( \c Kp), integral gain( \c Ki) and derivative gain( \c Kd)
 * also sets the state variables to all zeros.
 */

void arm_pid_init_q15(
  arm_pid_instance_q15 * S,
  int32_t resetStateFlag)
{

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  /* Derived coefficient A0 */
  S->A0 = __QADD16(__QADD16(S->Kp, S->Ki), S->Kd);

  /* Derived coefficients and pack into A1 */

#ifndef  ARM_MATH_BIG_ENDIAN

  S->A1 = __PKHBT(-__QADD16(__QADD16(S->Kd, S->Kd), S->Kp), S->Kd, 16);

#else

  S->A1 = __PKHBT(S->Kd, -__QADD16(__QADD16(S->Kd, S->Kd), S->Kp), 16);

#endif /*      #ifndef  ARM_MATH_BIG_ENDIAN    */

  /* Check whether state needs reset or not */
  if (resetStateFlag)
  {
    /* Clear the state buffer.  The size will be always 3 samples */
    memset(S->state, 0, 3U * sizeof(q15_t));
  }

#else

  /* Run the below code for Cortex-M0 */

  q31_t temp;                                    /*to store the sum */

  /* Derived coefficient A0 */
  temp = S->Kp + S->Ki + S->Kd;
  S->A0 = (q15_t) __SSAT(temp, 16);

  /* Derived coefficients and pack into A1 */
  temp = -(S->Kd + S->Kd + S->Kp);
  S->A1 = (q15_t) __SSAT(temp, 16);
  S->A2 = S->Kd;



  /* Check whether state needs reset or not */
  if (resetStateFlag)
  {
    /* Clear the state buffer.  The size will be always 3 samples */
    memset(S->state, 0, 3U * sizeof(q15_t));
  }

#endif /* #if defined (ARM_MATH_DSP) */

}

/**
 * @} end of PID group
 */
