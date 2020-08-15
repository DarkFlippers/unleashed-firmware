/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_sin_cos_f32.c
 * Description:  Sine and Cosine calculation for floating-point values
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
 * @defgroup SinCos Sine Cosine
 *
 * Computes the trigonometric sine and cosine values using a combination of table lookup
 * and linear interpolation.
 * There are separate functions for Q31 and floating-point data types.
 * The input to the floating-point version is in degrees while the
 * fixed-point Q31 have a scaled input with the range
 * [-1 0.9999] mapping to [-180 +180] degrees.
 *
 * The floating point function also allows values that are out of the usual range. When this happens, the function will
 * take extra time to adjust the input value to the range of [-180 180].
 *
 * The result is accurate to 5 digits after the decimal point.
 *
 * The implementation is based on table lookup using 360 values together with linear interpolation.
 * The steps used are:
 *  -# Calculation of the nearest integer table index.
 *  -# Compute the fractional portion (fract) of the input.
 *  -# Fetch the value corresponding to \c index from sine table to \c y0 and also value from \c index+1 to \c y1.
 *  -# Sine value is computed as <code> *psinVal = y0 + (fract * (y1 - y0))</code>.
 *  -# Fetch the value corresponding to \c index from cosine table to \c y0 and also value from \c index+1 to \c y1.
 *  -# Cosine value is computed as <code> *pcosVal = y0 + (fract * (y1 - y0))</code>.
 */

 /**
 * @addtogroup SinCos
 * @{
 */

/**
 * @brief  Floating-point sin_cos function.
 * @param[in]  theta    input value in degrees
 * @param[out] *pSinVal points to the processed sine output.
 * @param[out] *pCosVal points to the processed cos output.
 * @return none.
 */

void arm_sin_cos_f32(
                      float32_t theta,
                      float32_t * pSinVal,
                      float32_t * pCosVal)
{
    float32_t fract, in;                             /* Temporary variables for input, output */
    uint16_t indexS, indexC;                         /* Index variable */
    float32_t f1, f2, d1, d2;                        /* Two nearest output values */
    float32_t findex, Dn, Df, temp;

    /* input x is in degrees */
    /* Scale the input, divide input by 360, for cosine add 0.25 (pi/2) to read sine table */
    in = theta * 0.00277777777778f;

    if (in < 0.0f)
    {
        in = -in;
    }

    in = in - (int32_t)in;

    /* Calculation of index of the table */
    findex = (float32_t) FAST_MATH_TABLE_SIZE * in;
    indexS = ((uint16_t)findex) & 0x1ff;
    indexC = (indexS + (FAST_MATH_TABLE_SIZE / 4)) & 0x1ff;

    /* fractional value calculation */
    fract = findex - (float32_t) indexS;

    /* Read two nearest values of input value from the cos & sin tables */
    f1 = sinTable_f32[indexC+0];
    f2 = sinTable_f32[indexC+1];
    d1 = -sinTable_f32[indexS+0];
    d2 = -sinTable_f32[indexS+1];

    temp = (1.0f - fract) * f1 + fract * f2;

    Dn = 0.0122718463030f; // delta between the two points (fixed), in this case 2*pi/FAST_MATH_TABLE_SIZE
    Df = f2 - f1;          // delta between the values of the functions

    temp = Dn *(d1 + d2) - 2 * Df;
    temp = fract * temp + (3 * Df - (d2 + 2 * d1) * Dn);
    temp = fract * temp + d1 * Dn;

    /* Calculation of cosine value */
    *pCosVal = fract * temp + f1;

    /* Read two nearest values of input value from the cos & sin tables */
    f1 = sinTable_f32[indexS+0];
    f2 = sinTable_f32[indexS+1];
    d1 = sinTable_f32[indexC+0];
    d2 = sinTable_f32[indexC+1];

    temp = (1.0f - fract) * f1 + fract * f2;

    Df = f2 - f1; // delta between the values of the functions
    temp = Dn*(d1 + d2) - 2*Df;
    temp = fract*temp + (3*Df - (d2 + 2*d1)*Dn);
    temp = fract*temp + d1*Dn;

    /* Calculation of sine value */
    *pSinVal = fract*temp + f1;

    if (theta < 0.0f)
    {
        *pSinVal = -*pSinVal;
    }
}
/**
 * @} end of SinCos group
 */
