/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_lms_init_f32.c
 * Description:  Floating-point LMS filter initialization function
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
 * @addtogroup LMS
 * @{
 */

  /**
   * @brief Initialization function for floating-point LMS filter.
   * @param[in] *S points to an instance of the floating-point LMS filter structure.
   * @param[in] numTaps  number of filter coefficients.
   * @param[in] *pCoeffs points to the coefficient buffer.
   * @param[in] *pState points to state buffer.
   * @param[in] mu step size that controls filter coefficient updates.
   * @param[in] blockSize number of samples to process.
   * @return none.
   */

/**
 * \par Description:
 * <code>pCoeffs</code> points to the array of filter coefficients stored in time reversed order:
 * <pre>
 *    {b[numTaps-1], b[numTaps-2], b[N-2], ..., b[1], b[0]}
 * </pre>
 * The initial filter coefficients serve as a starting point for the adaptive filter.
 * <code>pState</code> points to an array of length <code>numTaps+blockSize-1</code> samples, where <code>blockSize</code> is the number of input samples processed by each call to <code>arm_lms_f32()</code>.
 */

void arm_lms_init_f32(
  arm_lms_instance_f32 * S,
  uint16_t numTaps,
  float32_t * pCoeffs,
  float32_t * pState,
  float32_t mu,
  uint32_t blockSize)
{
  /* Assign filter taps */
  S->numTaps = numTaps;

  /* Assign coefficient pointer */
  S->pCoeffs = pCoeffs;

  /* Clear state buffer and size is always blockSize + numTaps */
  memset(pState, 0, (numTaps + (blockSize - 1)) * sizeof(float32_t));

  /* Assign state pointer */
  S->pState = pState;

  /* Assign Step size value */
  S->mu = mu;
}

/**
 * @} end of LMS group
 */
