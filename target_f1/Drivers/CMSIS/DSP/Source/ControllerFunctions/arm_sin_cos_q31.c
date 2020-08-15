/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_sin_cos_q31.c
 * Description:  Cosine & Sine calculation for Q31 values
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
 * @ingroup groupController
 */

 /**
 * @addtogroup SinCos
 * @{
 */

/**
 * @brief  Q31 sin_cos function.
 * @param[in]  theta    scaled input value in degrees
 * @param[out] *pSinVal points to the processed sine output.
 * @param[out] *pCosVal points to the processed cosine output.
 * @return none.
 *
 * The Q31 input value is in the range [-1 0.999999] and is mapped to a degree value in the range [-180 179].
 *
 */

void arm_sin_cos_q31(
  q31_t theta,
  q31_t * pSinVal,
  q31_t * pCosVal)
{
  q31_t fract;                                 /* Temporary variables for input, output */
  uint16_t indexS, indexC;                     /* Index variable */
  q31_t f1, f2, d1, d2;                        /* Two nearest output values */
  q31_t Dn, Df;
  q63_t temp;

  /* Calculate the nearest index */
  indexS = (uint32_t)theta >> CONTROLLER_Q31_SHIFT;
  indexC = (indexS + 128) & 0x1ff;

  /* Calculation of fractional value */
  fract = (theta - (indexS << CONTROLLER_Q31_SHIFT)) << 8;

  /* Read two nearest values of input value from the cos & sin tables */
  f1 = sinTable_q31[indexC+0];
  f2 = sinTable_q31[indexC+1];
  d1 = -sinTable_q31[indexS+0];
  d2 = -sinTable_q31[indexS+1];

  Dn = 0x1921FB5; // delta between the two points (fixed), in this case 2*pi/FAST_MATH_TABLE_SIZE
  Df = f2 - f1; // delta between the values of the functions
  temp = Dn*((q63_t)d1 + d2);
  temp = temp - ((q63_t)Df << 32);
  temp = (q63_t)fract*(temp >> 31);
  temp = temp + ((3*(q63_t)Df << 31) - (d2 + ((q63_t)d1 << 1))*Dn);
  temp = (q63_t)fract*(temp >> 31);
  temp = temp + (q63_t)d1*Dn;
  temp = (q63_t)fract*(temp >> 31);

  /* Calculation of cosine value */
  *pCosVal = clip_q63_to_q31((temp >> 31) + (q63_t)f1);

  /* Read two nearest values of input value from the cos & sin tables */
  f1 = sinTable_q31[indexS+0];
  f2 = sinTable_q31[indexS+1];
  d1 = sinTable_q31[indexC+0];
  d2 = sinTable_q31[indexC+1];

  Df = f2 - f1; // delta between the values of the functions
  temp = Dn*((q63_t)d1 + d2);
  temp = temp - ((q63_t)Df << 32);
  temp = (q63_t)fract*(temp >> 31);
  temp = temp + ((3*(q63_t)Df << 31) - (d2 + ((q63_t)d1 << 1))*Dn);
  temp = (q63_t)fract*(temp >> 31);
  temp = temp + (q63_t)d1*Dn;
  temp = (q63_t)fract*(temp >> 31);

  /* Calculation of sine value */
  *pSinVal = clip_q63_to_q31((temp >> 31) + (q63_t)f1);
}

/**
 * @} end of SinCos group
 */
