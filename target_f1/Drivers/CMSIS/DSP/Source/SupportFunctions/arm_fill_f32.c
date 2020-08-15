/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fill_f32.c
 * Description:  Fills a constant value into a floating-point vector
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
 * @ingroup groupSupport
 */

/**
 * @defgroup Fill Vector Fill
 *
 * Fills the destination vector with a constant value.
 *
 * <pre>
 * 	pDst[n] = value;   0 <= n < blockSize.
 * </pre>
 *
 * There are separate functions for floating point, Q31, Q15, and Q7 data types.
 */

/**
 * @addtogroup Fill
 * @{
 */

/**
 * @brief Fills a constant value into a floating-point vector.
 * @param[in]       value input value to be filled
 * @param[out]      *pDst points to output vector
 * @param[in]       blockSize length of the output vector
 * @return none.
 *
 */


void arm_fill_f32(
  float32_t value,
  float32_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */
  float32_t in1 = value;
  float32_t in2 = value;
  float32_t in3 = value;
  float32_t in4 = value;

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = value */
    /* Fill the value in the destination buffer */
    *pDst++ = in1;
    *pDst++ = in2;
    *pDst++ = in3;
    *pDst++ = in4;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

#else

  /* Run the below code for Cortex-M0 */

  /* Loop over blockSize number of values */
  blkCnt = blockSize;

#endif /* #if defined (ARM_MATH_DSP) */


  while (blkCnt > 0U)
  {
    /* C = value */
    /* Fill the value in the destination buffer */
    *pDst++ = value;

    /* Decrement the loop counter */
    blkCnt--;
  }
}

/**
 * @} end of Fill group
 */
