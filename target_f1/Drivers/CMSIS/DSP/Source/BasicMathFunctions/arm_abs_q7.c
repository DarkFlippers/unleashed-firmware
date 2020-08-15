/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_abs_q7.c
 * Description:  Q7 vector absolute value
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
 * @addtogroup BasicAbs
 * @{
 */

/**
 * @brief Q7 vector absolute value.
 * @param[in]       *pSrc points to the input buffer
 * @param[out]      *pDst points to the output buffer
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 *
 * \par Conditions for optimum performance
 *  Input and output buffers should be aligned by 32-bit
 *
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * The Q7 value -1 (0x80) will be saturated to the maximum allowable positive value 0x7F.
 */

void arm_abs_q7(
  q7_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  q7_t in;                                       /* Input value1 */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */
  q31_t in1, in2, in3, in4;                      /* temporary input variables */
  q31_t out1, out2, out3, out4;                  /* temporary output variables */

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = |A| */
    /* Read inputs */
    in1 = (q31_t) * pSrc;
    in2 = (q31_t) * (pSrc + 1);
    in3 = (q31_t) * (pSrc + 2);

    /* find absolute value */
    out1 = (in1 > 0) ? in1 : (q31_t)__QSUB8(0, in1);

    /* read input */
    in4 = (q31_t) * (pSrc + 3);

    /* find absolute value */
    out2 = (in2 > 0) ? in2 : (q31_t)__QSUB8(0, in2);

    /* store result to destination */
    *pDst = (q7_t) out1;

    /* find absolute value */
    out3 = (in3 > 0) ? in3 : (q31_t)__QSUB8(0, in3);

    /* find absolute value */
    out4 = (in4 > 0) ? in4 : (q31_t)__QSUB8(0, in4);

    /* store result to destination */
    *(pDst + 1) = (q7_t) out2;

    /* store result to destination */
    *(pDst + 2) = (q7_t) out3;

    /* store result to destination */
    *(pDst + 3) = (q7_t) out4;

    /* update pointers to process next samples */
    pSrc += 4U;
    pDst += 4U;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;
#else

  /* Run the below code for Cortex-M0 */
  blkCnt = blockSize;

#endif /* #define ARM_MATH_CM0_FAMILY */

  while (blkCnt > 0U)
  {
    /* C = |A| */
    /* Read the input */
    in = *pSrc++;

    /* Store the Absolute result in the destination buffer */
    *pDst++ = (in > 0) ? in : ((in == (q7_t) 0x80) ? 0x7f : -in);

    /* Decrement the loop counter */
    blkCnt--;
  }
}

/**
 * @} end of BasicAbs group
 */
