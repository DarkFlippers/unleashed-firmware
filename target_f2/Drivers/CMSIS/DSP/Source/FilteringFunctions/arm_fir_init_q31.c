/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fir_init_q31.c
 * Description:  Q31 FIR filter initialization function.
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
 * @ingroup groupFilters
 */

/**
 * @addtogroup FIR
 * @{
 */

/**
 * @details
 *
 * @param[in,out] *S points to an instance of the Q31 FIR filter structure.
 * @param[in] 	  numTaps  Number of filter coefficients in the filter.
 * @param[in] 	  *pCoeffs points to the filter coefficients buffer.
 * @param[in] 	  *pState points to the state buffer.
 * @param[in] 	  blockSize number of samples that are processed per call.
 * @return        none.
 *
 * <b>Description:</b>
 * \par
 * <code>pCoeffs</code> points to the array of filter coefficients stored in time reversed order:
 * <pre>
 *    {b[numTaps-1], b[numTaps-2], b[N-2], ..., b[1], b[0]}
 * </pre>
 * \par
 * <code>pState</code> points to the array of state variables.
 * <code>pState</code> is of length <code>numTaps+blockSize-1</code> samples, where <code>blockSize</code> is the number of input samples processed by each call to <code>arm_fir_q31()</code>.
 */

void arm_fir_init_q31(
  arm_fir_instance_q31 * S,
  uint16_t numTaps,
  q31_t * pCoeffs,
  q31_t * pState,
  uint32_t blockSize)
{
  /* Assign filter taps */
  S->numTaps = numTaps;

  /* Assign coefficient pointer */
  S->pCoeffs = pCoeffs;

  /* Clear state buffer and state array size is (blockSize + numTaps - 1) */
  memset(pState, 0, (blockSize + ((uint32_t) numTaps - 1U)) * sizeof(q31_t));

  /* Assign state pointer */
  S->pState = pState;

}

/**
 * @} end of FIR group
 */
