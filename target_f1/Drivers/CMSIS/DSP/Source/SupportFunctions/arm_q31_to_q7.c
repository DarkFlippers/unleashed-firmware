/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_q31_to_q7.c
 * Description:  Converts the elements of the Q31 vector to Q7 vector
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
 * @addtogroup q31_to_x
 * @{
 */

/**
 * @brief Converts the elements of the Q31 vector to Q7 vector.
 * @param[in]       *pSrc points to the Q31 input vector
 * @param[out]      *pDst points to the Q7 output vector
 * @param[in]       blockSize length of the input vector
 * @return none.
 *
 * \par Description:
 *
 * The equation used for the conversion process is:
 *
 * <pre>
 * 	pDst[n] = (q7_t) pSrc[n] >> 24;   0 <= n < blockSize.
 * </pre>
 *
 */


void arm_q31_to_q7(
  q31_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
  q31_t *pIn = pSrc;                             /* Src pointer */
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */
  q31_t in1, in2, in3, in4;
  q7_t out1, out2, out3, out4;

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = (q7_t) A >> 24 */
    /* convert from q31 to q7 and then store the results in the destination buffer */
    in1 = *pIn++;
    in2 = *pIn++;
    in3 = *pIn++;
    in4 = *pIn++;

    out1 = (q7_t) (in1 >> 24);
    out2 = (q7_t) (in2 >> 24);
    out3 = (q7_t) (in3 >> 24);
    out4 = (q7_t) (in4 >> 24);

    *__SIMD32(pDst)++ = __PACKq7(out1, out2, out3, out4);

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
    /* C = (q7_t) A >> 24 */
    /* convert from q31 to q7 and then store the results in the destination buffer */
    *pDst++ = (q7_t) (*pIn++ >> 24);

    /* Decrement the loop counter */
    blkCnt--;
  }

}

/**
 * @} end of q31_to_x group
 */
