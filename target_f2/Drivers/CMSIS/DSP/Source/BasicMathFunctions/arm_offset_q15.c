/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_offset_q15.c
 * Description:  Q15 vector offset
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
 * @addtogroup offset
 * @{
 */

/**
 * @brief  Adds a constant offset to a Q15 vector.
 * @param[in]  *pSrc points to the input vector
 * @param[in]  offset is the offset to be added
 * @param[out]  *pDst points to the output vector
 * @param[in]  blockSize number of samples in the vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q15 range [0x8000 0x7FFF] are saturated.
 */

void arm_offset_q15(
  q15_t * pSrc,
  q15_t offset,
  q15_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */
  q31_t offset_packed;                           /* Offset packed to 32 bit */


  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* Offset is packed to 32 bit in order to use SIMD32 for addition */
  offset_packed = __PKHBT(offset, offset, 16);

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = A + offset */
    /* Add offset and then store the results in the destination buffer, 2 samples at a time. */
    *__SIMD32(pDst)++ = __QADD16(*__SIMD32(pSrc)++, offset_packed);
    *__SIMD32(pDst)++ = __QADD16(*__SIMD32(pSrc)++, offset_packed);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

  while (blkCnt > 0U)
  {
    /* C = A + offset */
    /* Add offset and then store the results in the destination buffer. */
    *pDst++ = (q15_t) __QADD16(*pSrc++, offset);

    /* Decrement the loop counter */
    blkCnt--;
  }

#else

  /* Run the below code for Cortex-M0 */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* C = A + offset */
    /* Add offset and then store the results in the destination buffer. */
    *pDst++ = (q15_t) __SSAT(((q31_t) * pSrc++ + offset), 16);

    /* Decrement the loop counter */
    blkCnt--;
  }

#endif /* #if defined (ARM_MATH_DSP) */

}

/**
 * @} end of offset group
 */
