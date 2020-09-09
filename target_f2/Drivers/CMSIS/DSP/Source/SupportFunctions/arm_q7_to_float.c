/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_q7_to_float.c
 * Description:  Converts the elements of the Q7 vector to floating-point vector
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
 * @defgroup q7_to_x  Convert 8-bit Integer value
 */

/**
 * @addtogroup q7_to_x
 * @{
 */

/**
 * @brief Converts the elements of the Q7 vector to floating-point vector.
 * @param[in]       *pSrc points to the Q7 input vector
 * @param[out]      *pDst points to the floating-point output vector
 * @param[in]       blockSize length of the input vector
 * @return none.
 *
 * \par Description:
 *
 * The equation used for the conversion process is:
 *
 * <pre>
 * 	pDst[n] = (float32_t) pSrc[n] / 128;   0 <= n < blockSize.
 * </pre>
 *
 */


void arm_q7_to_float(
  q7_t * pSrc,
  float32_t * pDst,
  uint32_t blockSize)
{
  q7_t *pIn = pSrc;                              /* Src pointer */
  uint32_t blkCnt;                               /* loop counter */


#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = (float32_t) A / 128 */
    /* convert from q7 to float and then store the results in the destination buffer */
    *pDst++ = ((float32_t) * pIn++ / 128.0f);
    *pDst++ = ((float32_t) * pIn++ / 128.0f);
    *pDst++ = ((float32_t) * pIn++ / 128.0f);
    *pDst++ = ((float32_t) * pIn++ / 128.0f);

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
    /* C = (float32_t) A / 128 */
    /* convert from q7 to float and then store the results in the destination buffer */
    *pDst++ = ((float32_t) * pIn++ / 128.0f);

    /* Decrement the loop counter */
    blkCnt--;
  }
}

/**
 * @} end of q7_to_x group
 */
