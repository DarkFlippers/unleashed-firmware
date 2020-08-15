/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_sin_q31.c
 * Description:  Fast sine calculation for Q31 values
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
 * @addtogroup sin
 * @{
 */

/**
 * @brief Fast approximation to the trigonometric sine function for Q31 data.
 * @param[in] x Scaled input value in radians.
 * @return  sin(x).
 *
 * The Q31 input value is in the range [0 +0.9999] and is mapped to a radian value in the range [0 2*pi). */

q31_t arm_sin_q31(
  q31_t x)
{
  q31_t sinVal;                                  /* Temporary variables for input, output */
  int32_t index;                                 /* Index variables */
  q31_t a, b;                                    /* Four nearest output values */
  q31_t fract;                                   /* Temporary values for fractional values */

  /* Calculate the nearest index */
  index = (uint32_t)x >> FAST_MATH_Q31_SHIFT;

  /* Calculation of fractional value */
  fract = (x - (index << FAST_MATH_Q31_SHIFT)) << 9;

  /* Read two nearest values of input value from the sin table */
  a = sinTable_q31[index];
  b = sinTable_q31[index+1];

  /* Linear interpolation process */
  sinVal = (q63_t)(0x80000000-fract)*a >> 32;
  sinVal = (q31_t)((((q63_t)sinVal << 32) + ((q63_t)fract*b)) >> 32);

  return sinVal << 1;
}

/**
 * @} end of sin group
 */
