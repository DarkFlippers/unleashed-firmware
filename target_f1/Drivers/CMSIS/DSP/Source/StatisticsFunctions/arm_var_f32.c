/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_var_f32.c
 * Description:  Variance of the elements of a floating-point vector
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
 * @ingroup groupStats
 */

/**
 * @defgroup variance  Variance
 *
 * Calculates the variance of the elements in the input vector.
 * The underlying algorithm used is the direct method sometimes referred to as the two-pass method:
 *
 * <pre>
 *   Result = sum(element - meanOfElements)^2) / numElement - 1
 *
 *     where, meanOfElements = ( pSrc[0] * pSrc[0] + pSrc[1] * pSrc[1] + ... + pSrc[blockSize-1] ) / blockSize
 *
 * </pre>
 *
 * There are separate functions for floating point, Q31, and Q15 data types.
 */

/**
 * @addtogroup variance
 * @{
 */


/**
 * @brief Variance of the elements of a floating-point vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult variance value returned here
 * @return none.
 */

void arm_var_f32(
                 float32_t * pSrc,
                 uint32_t blockSize,
                 float32_t * pResult)
{
    float32_t fMean, fValue;
    uint32_t blkCnt;            /* loop counter */
    float32_t * pInput = pSrc;
    float32_t sum = 0.0f;
    float32_t fSum = 0.0f;
    #if defined(ARM_MATH_DSP)
    float32_t in1, in2, in3, in4;
    #endif

    if (blockSize <= 1U)
    {
        *pResult = 0;
        return;
    }

    #if defined(ARM_MATH_DSP)
        /* Run the below code for Cortex-M4 and Cortex-M7 */

        /*loop Unrolling */
        blkCnt = blockSize >> 2U;

        /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
        ** a second loop below computes the remaining 1 to 3 samples. */
        while (blkCnt > 0U)
        {
            /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) */
            in1 = *pInput++;
            in2 = *pInput++;
            in3 = *pInput++;
            in4 = *pInput++;

            sum += in1;
            sum += in2;
            sum += in3;
            sum += in4;

            /* Decrement the loop counter */
            blkCnt--;
        }

        /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
        ** No loop unrolling is used. */
        blkCnt = blockSize % 0x4U;

    #else
        /* Run the below code for Cortex-M0 or Cortex-M3 */

        /* Loop over blockSize number of values */
        blkCnt = blockSize;

    #endif

    while (blkCnt > 0U)
    {
        /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) */
        sum += *pInput++;

        /* Decrement the loop counter */
        blkCnt--;
    }

    /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) / blockSize  */
    fMean = sum / (float32_t) blockSize;

    pInput = pSrc;

    #if defined(ARM_MATH_DSP)

        /*loop Unrolling */
        blkCnt = blockSize >> 2U;

        /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
        ** a second loop below computes the remaining 1 to 3 samples. */
        while (blkCnt > 0U)
        {
            fValue = *pInput++ - fMean;
            fSum += fValue * fValue;
            fValue = *pInput++ - fMean;
            fSum += fValue * fValue;
            fValue = *pInput++ - fMean;
            fSum += fValue * fValue;
            fValue = *pInput++ - fMean;
            fSum += fValue * fValue;

            /* Decrement the loop counter */
            blkCnt--;
        }

        blkCnt = blockSize % 0x4U;
    #else
        /* Run the below code for Cortex-M0 or Cortex-M3 */

        /* Loop over blockSize number of values */
        blkCnt = blockSize;
    #endif

    while (blkCnt > 0U)
    {
        fValue = *pInput++ - fMean;
        fSum += fValue * fValue;

        /* Decrement the loop counter */
        blkCnt--;
    }

    /* Variance */
    *pResult = fSum / (float32_t)(blockSize - 1.0f);
}

/**
 * @} end of variance group
 */
