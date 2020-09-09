/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_sub_f32.c
 * Description:  Floating-point vector subtraction.
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
 * @ingroup groupMath
 */

/**
 * @defgroup BasicSub Vector Subtraction
 *
 * Element-by-element subtraction of two vectors.
 *
 * <pre>
 *     pDst[n] = pSrcA[n] - pSrcB[n],   0 <= n < blockSize.
 * </pre>
 *
 * There are separate functions for floating-point, Q7, Q15, and Q31 data types.
 */

/**
 * @addtogroup BasicSub
 * @{
 */


/**
 * @brief Floating-point vector subtraction.
 * @param[in]       *pSrcA points to the first input vector
 * @param[in]       *pSrcB points to the second input vector
 * @param[out]      *pDst points to the output vector
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 */

void arm_sub_f32(
  float32_t * pSrcA,
  float32_t * pSrcB,
  float32_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */
  float32_t inA1, inA2, inA3, inA4;              /* temporary variables */
  float32_t inB1, inB2, inB3, inB4;              /* temporary variables */

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = A - B */
    /* Subtract and then store the results in the destination buffer. */
    /* Read 4 input samples from sourceA and sourceB */
    inA1 = *pSrcA;
    inB1 = *pSrcB;
    inA2 = *(pSrcA + 1);
    inB2 = *(pSrcB + 1);
    inA3 = *(pSrcA + 2);
    inB3 = *(pSrcB + 2);
    inA4 = *(pSrcA + 3);
    inB4 = *(pSrcB + 3);

    /* dst = srcA - srcB */
    /* subtract and store the result */
    *pDst = inA1 - inB1;
    *(pDst + 1) = inA2 - inB2;
    *(pDst + 2) = inA3 - inB3;
    *(pDst + 3) = inA4 - inB4;


    /* Update pointers to process next sampels */
    pSrcA += 4U;
    pSrcB += 4U;
    pDst += 4U;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

#else

  /* Run the below code for Cortex-M0 */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

#endif /* #if defined (ARM_MATH_DSP) */

  while (blkCnt > 0U)
  {
    /* C = A - B */
    /* Subtract and then store the results in the destination buffer. */
    *pDst++ = (*pSrcA++) - (*pSrcB++);

    /* Decrement the loop counter */
    blkCnt--;
  }
}

/**
 * @} end of BasicSub group
 */
