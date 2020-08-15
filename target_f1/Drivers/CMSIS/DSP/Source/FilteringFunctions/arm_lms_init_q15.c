/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_lms_init_q15.c
 * Description:  Q15 LMS filter initialization function
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
 * @addtogroup LMS
 * @{
 */

/**
* @brief Initialization function for the Q15 LMS filter.
* @param[in] *S points to an instance of the Q15 LMS filter structure.
* @param[in] numTaps  number of filter coefficients.
* @param[in] *pCoeffs points to the coefficient buffer.
* @param[in] *pState points to the state buffer.
* @param[in] mu step size that controls filter coefficient updates.
* @param[in] blockSize number of samples to process.
* @param[in] postShift bit shift applied to coefficients.
* @return    none.
*
* \par Description:
* <code>pCoeffs</code> points to the array of filter coefficients stored in time reversed order:
* <pre>
*    {b[numTaps-1], b[numTaps-2], b[N-2], ..., b[1], b[0]}
* </pre>
* The initial filter coefficients serve as a starting point for the adaptive filter.
* <code>pState</code> points to the array of state variables and size of array is
* <code>numTaps+blockSize-1</code> samples, where <code>blockSize</code> is the number of
* input samples processed by each call to <code>arm_lms_q15()</code>.
*/

void arm_lms_init_q15(
  arm_lms_instance_q15 * S,
  uint16_t numTaps,
  q15_t * pCoeffs,
  q15_t * pState,
  q15_t mu,
  uint32_t blockSize,
  uint32_t postShift)
{
  /* Assign filter taps */
  S->numTaps = numTaps;

  /* Assign coefficient pointer */
  S->pCoeffs = pCoeffs;

  /* Clear state buffer and size is always blockSize + numTaps - 1 */
  memset(pState, 0, (numTaps + (blockSize - 1U)) * sizeof(q15_t));

  /* Assign state pointer */
  S->pState = pState;

  /* Assign Step size value */
  S->mu = mu;

  /* Assign postShift value to be applied */
  S->postShift = postShift;

}

/**
 * @} end of LMS group
 */
