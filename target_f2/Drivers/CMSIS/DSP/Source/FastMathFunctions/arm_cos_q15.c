/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cos_q15.c
 * Description:  Fast cosine calculation for Q15 values
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
#include "arm_common_tables.h"

/**
 * @ingroup groupFastMath
 */

 /**
 * @addtogroup cos
 * @{
 */

/**
 * @brief Fast approximation to the trigonometric cosine function for Q15 data.
 * @param[in] x Scaled input value in radians.
 * @return  cos(x).
 *
 * The Q15 input value is in the range [0 +0.9999] and is mapped to a radian
 * value in the range [0 2*pi).
 */

q15_t arm_cos_q15(
  q15_t x)
{
  q15_t cosVal;                                  /* Temporary variables for input, output */
  int32_t index;                                 /* Index variables */
  q15_t a, b;                                    /* Four nearest output values */
  q15_t fract;                                   /* Temporary values for fractional values */

  /* add 0.25 (pi/2) to read sine table */
  x = (uint16_t)x + 0x2000;
  if (x < 0)
  {   /* convert negative numbers to corresponding positive ones */
      x = (uint16_t)x + 0x8000;
  }

  /* Calculate the nearest index */
  index = (uint32_t)x >> FAST_MATH_Q15_SHIFT;

  /* Calculation of fractional value */
  fract = (x - (index << FAST_MATH_Q15_SHIFT)) << 9;

  /* Read two nearest values of input value from the sin table */
  a = sinTable_q15[index];
  b = sinTable_q15[index+1];

  /* Linear interpolation process */
  cosVal = (q31_t)(0x8000-fract)*a >> 16;
  cosVal = (q15_t)((((q31_t)cosVal << 16) + ((q31_t)fract*b)) >> 16);

  return cosVal << 1;
}

/**
 * @} end of cos group
 */
