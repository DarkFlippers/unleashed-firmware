/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mat_add_q15.c
 * Description:  Q15 matrix addition
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
 * @ingroup groupMatrix
 */

/**
 * @addtogroup MatrixAdd
 * @{
 */

/**
 * @brief Q15 matrix addition.
 * @param[in]       *pSrcA points to the first input matrix structure
 * @param[in]       *pSrcB points to the second input matrix structure
 * @param[out]      *pDst points to output matrix structure
 * @return     		The function returns either
 * <code>ARM_MATH_SIZE_MISMATCH</code> or <code>ARM_MATH_SUCCESS</code> based on the outcome of size checking.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q15 range [0x8000 0x7FFF] will be saturated.
 */

arm_status arm_mat_add_q15(
  const arm_matrix_instance_q15 * pSrcA,
  const arm_matrix_instance_q15 * pSrcB,
  arm_matrix_instance_q15 * pDst)
{
  q15_t *pInA = pSrcA->pData;                    /* input data matrix pointer A  */
  q15_t *pInB = pSrcB->pData;                    /* input data matrix pointer B */
  q15_t *pOut = pDst->pData;                     /* output data matrix pointer */
  uint16_t numSamples;                           /* total number of elements in the matrix  */
  uint32_t blkCnt;                               /* loop counters  */
  arm_status status;                             /* status of matrix addition  */

#ifdef ARM_MATH_MATRIX_CHECK


  /* Check for matrix mismatch condition */
  if ((pSrcA->numRows != pSrcB->numRows) ||
     (pSrcA->numCols != pSrcB->numCols) ||
     (pSrcA->numRows != pDst->numRows) || (pSrcA->numCols != pDst->numCols))
  {
    /* Set status as ARM_MATH_SIZE_MISMATCH */
    status = ARM_MATH_SIZE_MISMATCH;
  }
  else
#endif /*    #ifdef ARM_MATH_MATRIX_CHECK    */

  {
    /* Total number of samples in the input matrix */
    numSamples = (uint16_t) (pSrcA->numRows * pSrcA->numCols);

#if defined (ARM_MATH_DSP)

    /* Run the below code for Cortex-M4 and Cortex-M3 */

    /* Loop unrolling */
    blkCnt = (uint32_t) numSamples >> 2U;

    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U)
    {
      /* C(m,n) = A(m,n) + B(m,n) */
      /* Add, Saturate and then store the results in the destination buffer. */
      *__SIMD32(pOut)++ = __QADD16(*__SIMD32(pInA)++, *__SIMD32(pInB)++);
      *__SIMD32(pOut)++ = __QADD16(*__SIMD32(pInA)++, *__SIMD32(pInB)++);

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = (uint32_t) numSamples % 0x4U;

    /* q15 pointers of input and output are initialized */

    while (blkCnt > 0U)
    {
      /* C(m,n) = A(m,n) + B(m,n) */
      /* Add, Saturate and then store the results in the destination buffer. */
      *pOut++ = (q15_t) __QADD16(*pInA++, *pInB++);

      /* Decrement the loop counter */
      blkCnt--;
    }

#else

    /* Run the below code for Cortex-M0 */

    /* Initialize blkCnt with number of samples */
    blkCnt = (uint32_t) numSamples;


    /* q15 pointers of input and output are initialized */
    while (blkCnt > 0U)
    {
      /* C(m,n) = A(m,n) + B(m,n) */
      /* Add, Saturate and then store the results in the destination buffer. */
      *pOut++ = (q15_t) __SSAT(((q31_t) * pInA++ + *pInB++), 16);

      /* Decrement the loop counter */
      blkCnt--;
    }

#endif /* #if defined (ARM_MATH_DSP) */

    /* set status as ARM_MATH_SUCCESS */
    status = ARM_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);
}

/**
 * @} end of MatrixAdd group
 */
