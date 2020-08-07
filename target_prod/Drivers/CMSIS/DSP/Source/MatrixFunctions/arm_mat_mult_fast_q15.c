/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mat_mult_fast_q15.c
 * Description:  Q15 matrix multiplication (fast variant)
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
 * @brief Q15 matrix multiplication (fast variant) for Cortex-M3 and Cortex-M4
 * @param[in]       *pSrcA points to the first input matrix structure
 * @param[in]       *pSrcB points to the second input matrix structure
 * @param[out]      *pDst points to output matrix structure
 * @param[in]       *pState points to the array for storing intermediate results
 * @return          The function returns either
 * <code>ARM_MATH_SIZE_MISMATCH</code> or <code>ARM_MATH_SUCCESS</code> based on the outcome of size checking.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * The difference between the function arm_mat_mult_q15() and this fast variant is that
 * the fast variant use a 32-bit rather than a 64-bit accumulator.
 * The result of each 1.15 x 1.15 multiplication is truncated to
 * 2.30 format. These intermediate results are accumulated in a 32-bit register in 2.30
 * format. Finally, the accumulator is saturated and converted to a 1.15 result.
 *
 * \par
 * The fast version has the same overflow behavior as the standard version but provides
 * less precision since it discards the low 16 bits of each multiplication result.
 * In order to avoid overflows completely the input signals must be scaled down.
 * Scale down one of the input matrices by log2(numColsA) bits to
 * avoid overflows, as a total of numColsA additions are computed internally for each
 * output element.
 *
 * \par
 * See <code>arm_mat_mult_q15()</code> for a slower implementation of this function
 * which uses 64-bit accumulation to provide higher precision.
 */

arm_status arm_mat_mult_fast_q15(
  const arm_matrix_instance_q15 * pSrcA,
  const arm_matrix_instance_q15 * pSrcB,
  arm_matrix_instance_q15 * pDst,
  q15_t * pState)
{
  q31_t sum;                                     /* accumulator */
  q15_t *pSrcBT = pState;                        /* input data matrix pointer for transpose */
  q15_t *pInA = pSrcA->pData;                    /* input data matrix pointer A of Q15 type */
  q15_t *pInB = pSrcB->pData;                    /* input data matrix pointer B of Q15 type */
  q15_t *px;                                     /* Temporary output data matrix pointer */
  uint16_t numRowsA = pSrcA->numRows;            /* number of rows of input matrix A    */
  uint16_t numColsB = pSrcB->numCols;            /* number of columns of input matrix B */
  uint16_t numColsA = pSrcA->numCols;            /* number of columns of input matrix A */
  uint16_t numRowsB = pSrcB->numRows;            /* number of rows of input matrix A    */
  uint32_t col, i = 0U, row = numRowsB, colCnt;  /* loop counters */
  arm_status status;                             /* status of matrix multiplication */

#ifndef UNALIGNED_SUPPORT_DISABLE

  q31_t in;                                      /* Temporary variable to hold the input value */
  q31_t inA1, inA2, inB1, inB2;
  q31_t sum2, sum3, sum4;
  q15_t *pInA2, *pInB2, *px2;
  uint32_t j = 0;

#else

  q15_t in;                                      /* Temporary variable to hold the input value */
  q15_t inA1, inA2, inB1, inB2;

#endif /* #ifndef UNALIGNED_SUPPORT_DISABLE */

#ifdef ARM_MATH_MATRIX_CHECK
  /* Check for matrix mismatch condition */
  if ((pSrcA->numCols != pSrcB->numRows) ||
     (pSrcA->numRows != pDst->numRows) || (pSrcB->numCols != pDst->numCols))
  {
    /* Set status as ARM_MATH_SIZE_MISMATCH */
    status = ARM_MATH_SIZE_MISMATCH;
  }
  else
#endif
  {
    /* Matrix transpose */
    do
    {
      /* Apply loop unrolling and exchange the columns with row elements */
      col = numColsB >> 2;

      /* The pointer px is set to starting address of the column being processed */
      px = pSrcBT + i;

      /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
       ** a second loop below computes the remaining 1 to 3 samples. */
      while (col > 0U)
      {
#ifndef UNALIGNED_SUPPORT_DISABLE
        /* Read two elements from the row */
        in = *__SIMD32(pInB)++;

        /* Unpack and store one element in the destination */
#ifndef ARM_MATH_BIG_ENDIAN

        *px = (q15_t) in;

#else

        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#endif /*    #ifndef ARM_MATH_BIG_ENDIAN    */

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Unpack and store the second element in the destination */
#ifndef ARM_MATH_BIG_ENDIAN

        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#else

        *px = (q15_t) in;

#endif /*    #ifndef ARM_MATH_BIG_ENDIAN    */

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Read two elements from the row */
        in = *__SIMD32(pInB)++;

        /* Unpack and store one element in the destination */
#ifndef ARM_MATH_BIG_ENDIAN

        *px = (q15_t) in;

#else

        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#endif /*    #ifndef ARM_MATH_BIG_ENDIAN    */

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Unpack and store the second element in the destination */

#ifndef ARM_MATH_BIG_ENDIAN

        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#else

        *px = (q15_t) in;

#endif /*    #ifndef ARM_MATH_BIG_ENDIAN    */

#else

        /* Read one element from the row */
        in = *pInB++;

        /* Store one element in the destination */
        *px = in;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Read one element from the row */
        in = *pInB++;

        /* Store one element in the destination */
        *px = in;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Read one element from the row */
        in = *pInB++;

        /* Store one element in the destination */
        *px = in;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Read one element from the row */
        in = *pInB++;

        /* Store one element in the destination */
        *px = in;

#endif /* #ifndef UNALIGNED_SUPPORT_DISABLE */

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Decrement the column loop counter */
        col--;
      }

      /* If the columns of pSrcB is not a multiple of 4, compute any remaining output samples here.
       ** No loop unrolling is used. */
      col = numColsB % 0x4U;

      while (col > 0U)
      {
        /* Read and store the input element in the destination */
        *px = *pInB++;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Decrement the column loop counter */
        col--;
      }

      i++;

      /* Decrement the row loop counter */
      row--;

    } while (row > 0U);

    /* Reset the variables for the usage in the following multiplication process */
    row = numRowsA;
    i = 0U;
    px = pDst->pData;

#ifndef UNALIGNED_SUPPORT_DISABLE
    /* Process two rows from matrix A at a time and output two rows at a time */
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
       ** to the starting address of the transposed pSrcB data */
      pInB = pSrcBT;

#ifndef UNALIGNED_SUPPORT_DISABLE
      /* Process two (transposed) columns from matrix B at a time */
      col = col >> 1;
      j = 0;
#endif

      /* column loop */
      while (col > 0U)
      {
        /* Set the variable sum, that acts as accumulator, to zero */
        sum = 0;

        /* Initiate the pointer pInA to point to the starting address of the column being processed */
        pInA = pSrcA->pData + i;

#ifndef UNALIGNED_SUPPORT_DISABLE
        sum2 = 0;
        sum3 = 0;
        sum4 = 0;
        pInB  = pSrcBT + j;
        pInA2 = pInA + numColsA;
        pInB2 = pInB + numRowsB;

        /* Read in two elements at once - alows dual MAC instruction */
        colCnt = numColsA >> 1;
#else
        colCnt = numColsA >> 2;
#endif

        /* matrix multiplication */
        while (colCnt > 0U)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */
#ifndef UNALIGNED_SUPPORT_DISABLE

          inA1 = *__SIMD32(pInA)++;
          inB1 = *__SIMD32(pInB)++;
          inA2 = *__SIMD32(pInA2)++;
          inB2 = *__SIMD32(pInB2)++;

          sum  = __SMLAD(inA1, inB1, sum);
          sum2 = __SMLAD(inA1, inB2, sum2);
          sum3 = __SMLAD(inA2, inB1, sum3);
          sum4 = __SMLAD(inA2, inB2, sum4);

#else

          inA1 = *pInA;
          inB1 = *pInB;
          sum += inA1 * inB1;

          inA2 = pInA[1];
          inB2 = pInB[1];
          sum += inA2 * inB2;

          inA1 = pInA[2];
          inB1 = pInB[2];
          sum += inA1 * inB1;

          inA2 = pInA[3];
          inB2 = pInB[3];
          sum += inA2 * inB2;

          pInA += 4;
          pInB += 4;

#endif /* #ifndef UNALIGNED_SUPPORT_DISABLE */

          /* Decrement the loop counter */
          colCnt--;
        }

        /* process odd column samples */
#ifndef UNALIGNED_SUPPORT_DISABLE
        if (numColsA & 1U) {
          inA1 = *pInA++;
          inB1 = *pInB++;
          inA2 = *pInA2++;
          inB2 = *pInB2++;
          sum  += inA1 * inB1;
          sum2 += inA1 * inB2;
          sum3 += inA2 * inB1;
          sum4 += inA2 * inB2;
        }
#else
        colCnt = numColsA % 0x4U;

        while (colCnt > 0U)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */
          sum += (q31_t) (*pInA++) * (*pInB++);

          colCnt--;
        }
#endif

        /* Saturate and store the result in the destination buffer */
        *px++  = (q15_t) (sum >> 15);

#ifndef UNALIGNED_SUPPORT_DISABLE
        *px++  = (q15_t) (sum2 >> 15);
        *px2++ = (q15_t) (sum3 >> 15);
        *px2++ = (q15_t) (sum4 >> 15);
        j += numRowsB * 2;
#endif

        /* Decrement the column loop counter */
        col--;

      }

      i = i + numColsA;

#ifndef UNALIGNED_SUPPORT_DISABLE
      i = i + numColsA;
      px = px2 + (numColsB & 1U);
      px2 = px + numColsB;
#endif

      /* Decrement the row loop counter */
      row--;

    }

    /* Compute any remaining odd row/column below */

#ifndef UNALIGNED_SUPPORT_DISABLE

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
        pInB  = pSrcBT + numRowsB*(numColsB-1);

        /* Set the variable sum, that acts as accumulator, to zero */
        sum  = 0;

        /* Compute 4 columns at once */
        colCnt = numColsA >> 2;

        /* matrix multiplication */
        while (colCnt > 0U)
        {
          inA1 = *__SIMD32(pInA)++;
          inA2 = *__SIMD32(pInA)++;
          inB1 = *__SIMD32(pInB)++;
          inB2 = *__SIMD32(pInB)++;

          sum  = __SMLAD(inA1, inB1, sum);
          sum  = __SMLAD(inA2, inB2, sum);

          /* Decrement the loop counter */
          colCnt--;
        }

        colCnt = numColsA & 3U;
        while (colCnt > 0U) {
          sum += (q31_t) (*pInA++) * (*pInB++);
          colCnt--;
        }

        /* Store the result in the destination buffer */
        *px  = (q15_t) (sum  >> 15);
        px += numColsB;

        /* Decrement the row loop counter */
        row--;
      }
    }

    /* Compute remaining output row */
    if (numRowsA & 1U) {

      /* point to last row in output matrix */
      px = pDst->pData+(numColsB)*(numRowsA-1);

      pInB  = pSrcBT;
      col = numColsB;
      i = 0U;

      /* col loop */
      while (col > 0)
      {

        /* point to last row in matrix A */
        pInA = pSrcA->pData + (numRowsA-1)*numColsA;

        /* Set the variable sum, that acts as accumulator, to zero */
        sum  = 0;

        /* Compute 4 columns at once */
        colCnt = numColsA >> 2;

        /* matrix multiplication */
        while (colCnt > 0U)
        {
          inA1 = *__SIMD32(pInA)++;
          inA2 = *__SIMD32(pInA)++;
          inB1 = *__SIMD32(pInB)++;
          inB2 = *__SIMD32(pInB)++;

          sum  = __SMLAD(inA1, inB1, sum);
          sum  = __SMLAD(inA2, inB2, sum);

          /* Decrement the loop counter */
          colCnt--;
        }

        colCnt = numColsA & 3U;
        while (colCnt > 0U) {
          sum += (q31_t) (*pInA++) * (*pInB++);
          colCnt--;
        }

        /* Store the result in the destination buffer */
        *px++  = (q15_t) (sum  >> 15);

        /* Decrement the col loop counter */
        col--;
      }
    }

#endif /* #ifndef UNALIGNED_SUPPORT_DISABLE */

    /* set status as ARM_MATH_SUCCESS */
    status = ARM_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);
}

/**
 * @} end of MatrixMult group
 */
