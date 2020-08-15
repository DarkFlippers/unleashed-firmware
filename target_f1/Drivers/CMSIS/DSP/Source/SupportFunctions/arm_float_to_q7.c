/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_float_to_q7.c
 * Description:  Converts the elements of the floating-point vector to Q7 vector
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
 * @addtogroup float_to_x
 * @{
 */

/**
 * @brief Converts the elements of the floating-point vector to Q7 vector.
 * @param[in]       *pSrc points to the floating-point input vector
 * @param[out]      *pDst points to the Q7 output vector
 * @param[in]       blockSize length of the input vector
 * @return none.
 *
 *\par Description:
 * \par
 * The equation used for the conversion process is:
 * <pre>
 * 	pDst[n] = (q7_t)(pSrc[n] * 128);   0 <= n < blockSize.
 * </pre>
 * \par Scaling and Overflow Behavior:
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q7 range [0x80 0x7F] will be saturated.
 * \note
 * In order to apply rounding, the library should be rebuilt with the ROUNDING macro
 * defined in the preprocessor section of project options.
 */


void arm_float_to_q7(
  float32_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
  float32_t *pIn = pSrc;                         /* Src pointer */
  uint32_t blkCnt;                               /* loop counter */

#ifdef ARM_MATH_ROUNDING

  float32_t in;

#endif /*      #ifdef ARM_MATH_ROUNDING        */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {

#ifdef ARM_MATH_ROUNDING
    /* C = A * 128 */
    /* convert from float to q7 and then store the results in the destination buffer */
    in = *pIn++;
    in = (in * 128);
    in += in > 0.0f ? 0.5f : -0.5f;
    *pDst++ = (q7_t) (__SSAT((q15_t) (in), 8));

    in = *pIn++;
    in = (in * 128);
    in += in > 0.0f ? 0.5f : -0.5f;
    *pDst++ = (q7_t) (__SSAT((q15_t) (in), 8));

    in = *pIn++;
    in = (in * 128);
    in += in > 0.0f ? 0.5f : -0.5f;
    *pDst++ = (q7_t) (__SSAT((q15_t) (in), 8));

    in = *pIn++;
    in = (in * 128);
    in += in > 0.0f ? 0.5f : -0.5f;
    *pDst++ = (q7_t) (__SSAT((q15_t) (in), 8));

#else

    /* C = A * 128 */
    /* convert from float to q7 and then store the results in the destination buffer */
    *pDst++ = __SSAT((q31_t) (*pIn++ * 128.0f), 8);
    *pDst++ = __SSAT((q31_t) (*pIn++ * 128.0f), 8);
    *pDst++ = __SSAT((q31_t) (*pIn++ * 128.0f), 8);
    *pDst++ = __SSAT((q31_t) (*pIn++ * 128.0f), 8);

#endif /*      #ifdef ARM_MATH_ROUNDING        */

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

  while (blkCnt > 0U)
  {

#ifdef ARM_MATH_ROUNDING
    /* C = A * 128 */
    /* convert from float to q7 and then store the results in the destination buffer */
    in = *pIn++;
    in = (in * 128);
    in += in > 0.0f ? 0.5f : -0.5f;
    *pDst++ = (q7_t) (__SSAT((q15_t) (in), 8));

#else

    /* C = A * 128 */
    /* convert from float to q7 and then store the results in the destination buffer */
    *pDst++ = __SSAT((q31_t) (*pIn++ * 128.0f), 8);

#endif /*      #ifdef ARM_MATH_ROUNDING        */

    /* Decrement the loop counter */
    blkCnt--;
  }


#else

  /* Run the below code for Cortex-M0 */


  /* Loop over blockSize number of values */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
#ifdef ARM_MATH_ROUNDING
    /* C = A * 128 */
    /* convert from float to q7 and then store the results in the destination buffer */
    in = *pIn++;
    in = (in * 128.0f);
    in += in > 0 ? 0.5f : -0.5f;
    *pDst++ = (q7_t) (__SSAT((q31_t) (in), 8));

#else

    /* C = A * 128 */
    /* convert from float to q7 and then store the results in the destination buffer */
    *pDst++ = (q7_t) __SSAT((q31_t) (*pIn++ * 128.0f), 8);

#endif /*      #ifdef ARM_MATH_ROUNDING        */

    /* Decrement the loop counter */
    blkCnt--;
  }

#endif /* #if defined (ARM_MATH_DSP) */

}

/**
 * @} end of float_to_x group
 */
