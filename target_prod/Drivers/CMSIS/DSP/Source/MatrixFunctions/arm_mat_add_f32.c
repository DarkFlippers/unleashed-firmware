/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mat_add_f32.c
 * Description:  Floating-point matrix addition
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
 * @defgroup MatrixAdd Matrix Addition
 *
 * Adds two matrices.
 * \image html MatrixAddition.gif "Addition of two 3 x 3 matrices"
 *
 * The functions check to make sure that
 * <code>pSrcA</code>, <code>pSrcB</code>, and <code>pDst</code> have the same
 * number of rows and columns.
 */

/**
 * @addtogroup MatrixAdd
 * @{
 */


/**
 * @brief Floating-point matrix addition.
 * @param[in]       *pSrcA points to the first input matrix structure
 * @param[in]       *pSrcB points to the second input matrix structure
 * @param[out]      *pDst points to output matrix structure
 * @return     		The function returns either
 * <code>ARM_MATH_SIZE_MISMATCH</code> or <code>ARM_MATH_SUCCESS</code> based on the outcome of size checking.
 */

arm_status arm_mat_add_f32(
  const arm_matrix_instance_f32 * pSrcA,
  const arm_matrix_instance_f32 * pSrcB,
  arm_matrix_instance_f32 * pDst)
{
  float32_t *pIn1 = pSrcA->pData;                /* input data matrix pointer A  */
  float32_t *pIn2 = pSrcB->pData;                /* input data matrix pointer B  */
  float32_t *pOut = pDst->pData;                 /* output data matrix pointer   */

#if defined (ARM_MATH_DSP)

  float32_t inA1, inA2, inB1, inB2, out1, out2;  /* temporary variables */

#endif //      #if defined (ARM_MATH_DSP)

  uint32_t numSamples;                           /* total number of elements in the matrix  */
  uint32_t blkCnt;                               /* loop counters */
  arm_status status;                             /* status of matrix addition */

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
#endif
  {

    /* Total number of samples in the input matrix */
    numSamples = (uint32_t) pSrcA->numRows * pSrcA->numCols;

#if defined (ARM_MATH_DSP)

    /* Loop unrolling */
    blkCnt = numSamples >> 2U;

    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U)
    {
      /* C(m,n) = A(m,n) + B(m,n) */
      /* Add and then store the results in the destination buffer. */
      /* Read values from source A */
      inA1 = pIn1[0];

      /* Read values from source B */
      inB1 = pIn2[0];

      /* Read values from source A */
      inA2 = pIn1[1];

      /* out = sourceA + sourceB */
      out1 = inA1 + inB1;

      /* Read values from source B */
      inB2 = pIn2[1];

      /* Read values from source A */
      inA1 = pIn1[2];

      /* out = sourceA + sourceB */
      out2 = inA2 + inB2;

      /* Read values from source B */
      inB1 = pIn2[2];

      /* Store result in destination */
      pOut[0] = out1;
      pOut[1] = out2;

      /* Read values from source A */
      inA2 = pIn1[3];

      /* Read values from source B */
      inB2 = pIn2[3];

      /* out = sourceA + sourceB */
      out1 = inA1 + inB1;

      /* out = sourceA + sourceB */
      out2 = inA2 + inB2;

      /* Store result in destination */
      pOut[2] = out1;

      /* Store result in destination */
      pOut[3] = out2;


      /* update pointers to process next sampels */
      pIn1 += 4U;
      pIn2 += 4U;
      pOut += 4U;
      /* Decrement the loop counter */
      blkCnt--;
    }

    /* If the numSamples is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = numSamples % 0x4U;

#else

    /* Run the below code for Cortex-M0 */

    /* Initialize blkCnt with number of samples */
    blkCnt = numSamples;

#endif /* #if defined (ARM_MATH_DSP) */

    while (blkCnt > 0U)
    {
      /* C(m,n) = A(m,n) + B(m,n) */
      /* Add and then store the results in the destination buffer. */
      *pOut++ = (*pIn1++) + (*pIn2++);

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* set status as ARM_MATH_SUCCESS */
    status = ARM_MATH_SUCCESS;

  }

  /* Return to application */
  return (status);
}

/**
 * @} end of MatrixAdd group
 */
