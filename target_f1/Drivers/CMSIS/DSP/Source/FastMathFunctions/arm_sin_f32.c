/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_sin_f32.c
 * Description:  Fast sine calculation for floating-point values
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
#include <math.h>

/**
 * @ingroup groupFastMath
 */

/**
 * @defgroup sin Sine
 *
 * Computes the trigonometric sine function using a combination of table lookup
 * and linear interpolation.  There are separate functions for
 * Q15, Q31, and floating-point data types.
 * The input to the floating-point version is in radians and in the range [0 2*pi) while the
 * fixed-point Q15 and Q31 have a scaled input with the range
 * [0 +0.9999] mapping to [0 2*pi).  The fixed-point range is chosen so that a
 * value of 2*pi wraps around to 0.
 *
 * The implementation is based on table lookup using 256 values together with linear interpolation.
 * The steps used are:
 *  -# Calculation of the nearest integer table index
 *  -# Compute the fractional portion (fract) of the table index.
 *  -# The final result equals <code>(1.0f-fract)*a + fract*b;</code>
 *
 * where
 * <pre>
 *    b=Table[index+0];
 *    c=Table[index+1];
 * </pre>
 */

/**
 * @addtogroup sin
 * @{
 */

/**
 * @brief  Fast approximation to the trigonometric sine function for floating-point data.
 * @param[in] x input value in radians.
 * @return  sin(x).
 */

float32_t arm_sin_f32(
  float32_t x)
{
  float32_t sinVal, fract, in;                           /* Temporary variables for input, output */
  uint16_t index;                                        /* Index variable */
  float32_t a, b;                                        /* Two nearest output values */
  int32_t n;
  float32_t findex;

  /* Special case for small negative inputs */
  if ((x < 0.0f) && (x >= -1.9e-7f)) {
     return x;
  }

  /* input x is in radians */
  /* Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi */
  in = x * 0.159154943092f;

  /* Calculation of floor value of input */
  n = (int32_t) in;

  /* Make negative values towards -infinity */
  if (x < 0.0f)
  {
    n--;
  }

  /* Map input value to [0 1] */
  in = in - (float32_t) n;

  /* Calculation of index of the table */
  findex = (float32_t) FAST_MATH_TABLE_SIZE * in;

  index = ((uint16_t)findex) & 0x1ff;

  /* fractional value calculation */
  fract = findex - (float32_t) index;

  /* Read two nearest values of input value from the sin table */
  a = sinTable_f32[index];
  b = sinTable_f32[index+1];

  /* Linear interpolation process */
  sinVal = (1.0f-fract)*a + fract*b;

  /* Return the output value */
  return (sinVal);
}

/**
 * @} end of sin group
 */
