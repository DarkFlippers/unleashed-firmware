/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mat_mult_fast_q31.c
 * Description:  Q31 matrix multiplication (fast variant)
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
 * @addtogroup MatrixMult
 * @{
 */

/**
 * @brief Q31 matrix multiplication (fast variant) for Cortex-M3 and Cortex-M4
 * @param[in]       *pSrcA points to the first input matrix structure
 * @param[in]       *pSrcB points to the second input matrix structure
 * @param[out]      *pDst points to output matrix structure
 * @return          The function returns either
 * <code>ARM_MATH_SIZE_MISMATCH</code> or <code>ARM_MATH_SUCCESS</code> based on the outcome of size checking.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * The difference between the function arm_mat_mult_q31() and this fast variant is that
 * the fast variant use a 32-bit rather than a 64-bit accumulator.
 * The result of each 1.31 x 1.31 multiplication is truncated to
 * 2.30 format. These intermediate results are accumulated in a 32-bit register in 2.30
 * format. Finally, the accumulator is saturated and converted to a 1.31 result.
 *
 * \par
 * The fast version has the same overflow behavior as the standard version but provides
 * less precision since it discards the low 32 bits of each multiplication result.
 * In order to avoid overflows completely the input signals must be scaled down.
 * Scale down one of the input matrices by log2(numColsA) bits to
 * avoid overflows, as a total of numColsA additions are computed internally for each
 * output element.
 *
 * \par
 * See <code>arm_mat_mult_q31()</code> for a slower implementation of this function
 * which uses 64-bit accumulation to provide higher precision.
 */

arm_status arm_mat_mult_fast_q31(
  const arm_matrix_instance_q31 * pSrcA,
  const arm_matrix_instance_q31 * pSrcB,
  arm_matrix_instance_q31 * pDst)
{
  q31_t *pInA = pSrcA->pData;                    /* input data matrix pointer A */
  q31_t *pInB = pSrcB->pData;                    /* input data matrix pointer B */
  q31_t *px;                                     /* Temporary output data matrix pointer */
  q31_t sum;                                     /* Accumulator */
  uint16_t numRowsA = pSrcA->numRows;            /* number of rows of input matrix A    */
  uint16_t numColsB = pSrcB->numCols;            /* number of columns of input matrix B */
  uint16_t numColsA = pSrcA->numCols;            /* number of columns of input matrix A */
  uint32_t col, i = 0U, j, row = numRowsA, colCnt;  /* loop counters */
  arm_status status;                             /* status of matrix multiplication */
  q31_t inA1, inB1;

#if defined (ARM_MATH_DSP)

  q31_t sum2, sum3, sum4;
  q31_t inA2, inB2;
  q31_t *pInA2;
  q31_t *px2;

#endif

#ifdef ARM_MATH_MATRIX_CHECK

  /* Check for matrix mismatch condition */
  if ((pSrcA->numCols != pSrcB->numRows) ||
     (pSrcA->numRows != pDst->numRows) || (pSrcB->numCols != pDst->numCols))
  {
    /* Set status as ARM_MATH_SIZE_MISMATCH */
    status = ARM_MATH_SIZE_MISMATCH;
  }
  else
#endif /*      #ifdef ARM_MATH_MATRIX_CHECK    */

  {

    px = pDst->pData;

#if defined (ARM_MATH_DSP)
    row = row >> 1;
    px2 = px + numColsB;
#endif

    /* The following loop performs the dot-product of each row in pSrcA with each column in pSrcB */
    /* row loop */
    while (row > 0U)
    {

      /* For every row wise process, the column loop counter is to be initiated */
      col = numColsB;

      /* For every row wise process, the pIn2 pointer is set
       ** to the starting address of the pSrcB data */
      pInB = pSrcB->pData;

      j = 0U;

#if defined (ARM_MATH_DSP)
      col = col >> 1;
#endif

      /* column loop */
      while (col > 0U)
      {
        /* Set the variable sum, that acts as accumulator, to zero */
        sum = 0;

        /* Initiate data pointers */
        pInA = pSrcA->pData + i;
        pInB  = pSrcB->pData + j;

#if defined (ARM_MATH_DSP)
        sum2 = 0;
        sum3 = 0;
        sum4 = 0;
        pInA2 = pInA + numColsA;
        colCnt = numColsA;
#else
        colCnt = numColsA >> 2;
#endif

        /* matrix multiplication */
        while (colCnt > 0U)
        {

#if defined (ARM_MATH_DSP)
          inA1 = *pInA++;
          inB1 = pInB[0];
          inA2 = *pInA2++;
          inB2 = pInB[1];
          pInB += numColsB;

          sum  = __SMMLA(inA1, inB1, sum);
          sum2 = __SMMLA(inA1, inB2, sum2);
          sum3 = __SMMLA(inA2, inB1, sum3);
          sum4 = __SMMLA(inA2, inB2, sum4);
#else
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */
          /* Perform the multiply-accumulates */
          inB1 = *pInB;
          pInB += numColsB;
          inA1 = pInA[0];
          sum = __SMMLA(inA1, inB1, sum);

          inB1 = *pInB;
          pInB += numColsB;
          inA1 = pInA[1];
          sum = __SMMLA(inA1, inB1, sum);

          inB1 = *pInB;
          pInB += numColsB;
          inA1 = pInA[2];
          sum = __SMMLA(inA1, inB1, sum);

          inB1 = *pInB;
          pInB += numColsB;
          inA1 = pInA[3];
          sum = __SMMLA(inA1, inB1, sum);

          pInA += 4U;
#endif

          /* Decrement the loop counter */
          colCnt--;
        }

#ifdef ARM_MATH_CM0_FAMILY
        /* If the columns of pSrcA is not a multiple of 4, compute any remaining output samples here. */
        colCnt = numColsA % 0x4U;
        while (colCnt > 0U)
        {
          sum = __SMMLA(*pInA++, *pInB, sum);
          pInB += numColsB;
          colCnt--;
        }
        j++;
#endif

        /* Convert the result from 2.30 to 1.31 format and store in destination buffer */
        *px++  = sum << 1;

#if defined (ARM_MATH_DSP)
        *px++  = sum2 << 1;
        *px2++ = sum3 << 1;
        *px2++ = sum4 << 1;
        j += 2;
#endif

        /* Decrement the column loop counter */
        col--;

      }

      i = i + numColsA;

#if defined (ARM_MATH_DSP)
      i = i + numColsA;
      px = px2 + (numColsB & 1U);
      px2 = px + numColsB;
#endif

      /* Decrement the row loop counter */
      row--;

    }

    /* Compute any remaining odd row/column below */

#if defined (ARM_MATH_DSP)

    /* Compute remaining output column */
    if (numColsB & 1U) {

      /* Avoid redundant computation of last element */
      row = numRowsA & (~0x1);

      /* Point to remaining unfilled column in output matrix */
      px = pDst->pData+numColsB-1;
      pInA = pSrcA->pData;

      /* row loop */
      while (row > 0)
      {

        /* point to last column in matrix B */
        pInB  = pSrcB->pData + numColsB-1;

        /* Set the variable sum, that acts as accumulator, to zero */
        sum  = 0;

        /* Compute 4 columns at once */
        colCnt = numColsA >> 2;

        /* matrix multiplication */
        while (colCnt > 0U)
        {
          inA1 = *pInA++;
          inA2 = *pInA++;
          inB1 = *pInB;
          pInB += numColsB;
          inB2 = *pInB;
          pInB += numColsB;
          sum = __SMMLA(inA1, inB1, sum);
          sum = __SMMLA(inA2, inB2, sum);

          inA1 = *pInA++;
          inA2 = *pInA++;
          inB1 = *pInB;
          pInB += numColsB;
          inB2 = *pInB;
          pInB += numColsB;
          sum = __SMMLA(inA1, inB1, sum);
          sum = __SMMLA(inA2, inB2, sum);

          /* Decrement the loop counter */
          colCnt--;
        }

        colCnt = numColsA & 3U;
        while (colCnt > 0U) {
          sum = __SMMLA(*pInA++, *pInB, sum);
          pInB += numColsB;
          colCnt--;
        }

        /* Convert the result from 2.30 to 1.31 format and store in destination buffer */
        *px = sum << 1;
        px += numColsB;

        /* Decrement the row loop counter */
        row--;
      }
    }

    /* Compute remaining output row */
    if (numRowsA & 1U) {

      /* point to last row in output matrix */
      px = pDst->pData+(numColsB)*(numRowsA-1);

      col = numColsB;
      i = 0U;

      /* col loop */
      while (col > 0)
      {

        /* point to last row in matrix A */
        pInA = pSrcA->pData + (numRowsA-1)*numColsA;
        pInB  = pSrcB->pData + i;

        /* Set the variable sum, that acts as accumulator, to zero */
        sum  = 0;

        /* Compute 4 columns at once */
        colCnt = numColsA >> 2;

        /* matrix multiplication */
        while (colCnt > 0U)
        {
          inA1 = *pInA++;
          inA2 = *pInA++;
          inB1 = *pInB;
          pInB += numColsB;
          inB2 = *pInB;
          pInB += numColsB;
          sum = __SMMLA(inA1, inB1, sum);
          sum = __SMMLA(inA2, inB2, sum);

          inA1 = *pInA++;
          inA2 = *pInA++;
          inB1 = *pInB;
          pInB += numColsB;
          inB2 = *pInB;
          pInB += numColsB;
          sum = __SMMLA(inA1, inB1, sum);
          sum = __SMMLA(inA2, inB2, sum);

          /* Decrement the loop counter */
          colCnt--;
        }

        colCnt = numColsA & 3U;
        while (colCnt > 0U) {
          sum = __SMMLA(*pInA++, *pInB, sum);
          pInB += numColsB;
          colCnt--;
        }

        /* Saturate and store the result in the destination buffer */
        *px++ = sum << 1;
        i++;

        /* Decrement the col loop counter */
        col--;
      }
    }

#endif /* #if defined (ARM_MATH_DSP) */

    /* set status as ARM_MATH_SUCCESS */
    status = ARM_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);
}

/**
 * @} end of MatrixMult group
 */
