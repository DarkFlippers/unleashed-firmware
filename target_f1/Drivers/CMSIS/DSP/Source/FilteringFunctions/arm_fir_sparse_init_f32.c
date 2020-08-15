/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fir_sparse_init_f32.c
 * Description:  Floating-point sparse FIR filter initialization function
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
 * @addtogroup FIR_Sparse
 * @{
 */

/**
 * @brief  Initialization function for the floating-point sparse FIR filter.
 * @param[in,out] *S         points to an instance of the floating-point sparse FIR structure.
 * @param[in]     numTaps    number of nonzero coefficients in the filter.
 * @param[in]     *pCoeffs   points to the array of filter coefficients.
 * @param[in]     *pState    points to the state buffer.
 * @param[in]     *pTapDelay points to the array of offset times.
 * @param[in]     maxDelay   maximum offset time supported.
 * @param[in]     blockSize  number of samples that will be processed per block.
 * @return none
 *
 * <b>Description:</b>
 * \par
 * <code>pCoeffs</code> holds the filter coefficients and has length <code>numTaps</code>.
 * <code>pState</code> holds the filter's state variables and must be of length
 * <code>maxDelay + blockSize</code>, where <code>maxDelay</code>
 * is the maximum number of delay line values.
 * <code>blockSize</code> is the
 * number of samples processed by the <code>arm_fir_sparse_f32()</code> function.
 */

void arm_fir_sparse_init_f32(
  arm_fir_sparse_instance_f32 * S,
  uint16_t numTaps,
  float32_t * pCoeffs,
  float32_t * pState,
  int32_t * pTapDelay,
  uint16_t maxDelay,
  uint32_t blockSize)
{
  /* Assign filter taps */
  S->numTaps = numTaps;

  /* Assign coefficient pointer */
  S->pCoeffs = pCoeffs;

  /* Assign TapDelay pointer */
  S->pTapDelay = pTapDelay;

  /* Assign MaxDelay */
  S->maxDelay = maxDelay;

  /* reset the stateIndex to 0 */
  S->stateIndex = 0U;

  /* Clear state buffer and size is always maxDelay + blockSize */
  memset(pState, 0, (maxDelay + blockSize) * sizeof(float32_t));

  /* Assign state pointer */
  S->pState = pState;

}

/**
 * @} end of FIR_Sparse group
 */
