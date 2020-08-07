/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_biquad_cascade_df2T_init_f64.c
 * Description:  Initialization function for floating-point transposed direct form II Biquad cascade filter
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
 * @addtogroup BiquadCascadeDF2T
 * @{
 */

/**
 * @brief  Initialization function for the floating-point transposed direct form II Biquad cascade filter.
 * @param[in,out] *S           points to an instance of the filter data structure.
 * @param[in]     numStages    number of 2nd order stages in the filter.
 * @param[in]     *pCoeffs     points to the filter coefficients.
 * @param[in]     *pState      points to the state buffer.
 * @return        none
 *
 * <b>Coefficient and State Ordering:</b>
 * \par
 * The coefficients are stored in the array <code>pCoeffs</code> in the following order:
 * <pre>
 *     {b10, b11, b12, a11, a12, b20, b21, b22, a21, a22, ...}
 * </pre>
 *
 * \par
 * where <code>b1x</code> and <code>a1x</code> are the coefficients for the first stage,
 * <code>b2x</code> and <code>a2x</code> are the coefficients for the second stage,
 * and so on.  The <code>pCoeffs</code> array contains a total of <code>5*numStages</code> values.
 *
 * \par
 * The <code>pState</code> is a pointer to state array.
 * Each Biquad stage has 2 state variables <code>d1,</code> and <code>d2</code>.
 * The 2 state variables for stage 1 are first, then the 2 state variables for stage 2, and so on.
 * The state array has a total length of <code>2*numStages</code> values.
 * The state variables are updated after each block of data is processed; the coefficients are untouched.
 */

void arm_biquad_cascade_df2T_init_f64(
  arm_biquad_cascade_df2T_instance_f64 * S,
  uint8_t numStages,
  float64_t * pCoeffs,
  float64_t * pState)
{
  /* Assign filter stages */
  S->numStages = numStages;

  /* Assign coefficient pointer */
  S->pCoeffs = pCoeffs;

  /* Clear state buffer and size is always 2 * numStages */
  memset(pState, 0, (2U * (uint32_t) numStages) * sizeof(float64_t));

  /* Assign state pointer */
  S->pState = pState;
}

/**
 * @} end of BiquadCascadeDF2T group
 */
