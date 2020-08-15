/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mat_mult_q15.c
 * Description:  Q15 matrix multiplication
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
 * @brief Q15 matrix multiplication
 * @param[in]       *pSrcA points to the first input matrix structure
 * @param[in]       *pSrcB points to the second input matrix structure
 * @param[out]      *pDst points to output matrix structure
 * @param[in]       *pState points to the array for storing intermediate results (Unused)
 * @return          The function returns either
 * <code>ARM_MATH_SIZE_MISMATCH</code> or <code>ARM_MATH_SUCCESS</code> based on the outcome of size checking.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * The function is implemented using a 64-bit internal accumulator. The inputs to the
 * multiplications are in 1.15 format and multiplications yield a 2.30 result.
 * The 2.30 intermediate
 * results are accumulated in a 64-bit accumulator in 34.30 format. This approach
 * provides 33 guard bits and there is no risk of overflow. The 34.30 result is then
 * truncated to 34.15 format by discarding the low 15 bits and then saturated to
 * 1.15 format.
 *
 * \par
 * Refer to <code>arm_mat_mult_fast_q15()</code> for a faster but less precise version of this function for Cortex-M3 and Cortex-M4.
 *
 */

arm_status arm_mat_mult_q15(
  const arm_matrix_instance_q15 * pSrcA,
  const arm_matrix_instance_q15 * pSrcB,
  arm_matrix_instance_q15 * pDst,
  q15_t * pState)
{
  q63_t sum;                                     /* accumulator */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  q15_t *pSrcBT = pState;                        /* input data matrix pointer for transpose */
  q15_t *pInA = pSrcA->pData;                    /* input data matrix pointer A of Q15 type */
  q15_t *pInB = pSrcB->pData;                    /* input data matrix pointer B of Q15 type */
  q15_t *px;                                     /* Temporary output data matrix pointer */
  uint16_t numRowsA = pSrcA->numRows;            /* number of rows of input matrix A    */
  uint16_t numColsB = pSrcB->numCols;            /* number of columns of input matrix B */
  uint16_t numColsA = pSrcA->numCols;            /* number of columns of input matrix A */
  uint16_t numRowsB = pSrcB->numRows;            /* number of rows of input matrix A    */
  uint16_t col, i = 0U, row = numRowsB, colCnt;  /* loop counters */
  arm_status status;                             /* status of matrix multiplication */

#ifndef UNALIGNED_SUPPORT_DISABLE

  q31_t in;                                      /* Temporary variable to hold the input value */
  q31_t pSourceA1, pSourceB1, pSourceA2, pSourceB2;

#else

  q15_t in;                                      /* Temporary variable to hold the input value */
  q15_t inA1, inB1, inA2, inB2;

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
#endif /*    #ifdef ARM_MATH_MATRIX_CHECK    */
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

#endif /* #ifndef ARM_MATH_BIG_ENDIAN */

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Unpack and store the second element in the destination */
#ifndef ARM_MATH_BIG_ENDIAN

        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#else

        *px = (q15_t) in;

#endif /* #ifndef ARM_MATH_BIG_ENDIAN */

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Read two elements from the row */
        in = *__SIMD32(pInB)++;

        /* Unpack and store one element in the destination */
#ifndef ARM_MATH_BIG_ENDIAN

        *px = (q15_t) in;

#else

        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#endif /* #ifndef ARM_MATH_BIG_ENDIAN */

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Unpack and store the second element in the destination */

#ifndef ARM_MATH_BIG_ENDIAN

        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#else

        *px = (q15_t) in;

#endif /* #ifndef ARM_MATH_BIG_ENDIAN */

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

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

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

#endif /* #ifndef UNALIGNED_SUPPORT_DISABLE */

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

    /* The following loop performs the dot-product of each row in pSrcA with each column in pSrcB */
    /* row loop */
    do
    {
      /* For every row wise process, the column loop counter is to be initiated */
      col = numColsB;

      /* For every row wise process, the pIn2 pointer is set
       ** to the starting address of the transposed pSrcB data */
      pInB = pSrcBT;

      /* column loop */
      do
      {
        /* Set the variable sum, that acts as accumulator, to zero */
        sum = 0;

        /* Apply loop unrolling and compute 2 MACs simultaneously. */
        colCnt = numColsA >> 2;

        /* Initiate the pointer pIn1 to point to the starting address of the column being processed */
        pInA = pSrcA->pData + i;


        /* matrix multiplication */
        while (colCnt > 0U)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */
#ifndef UNALIGNED_SUPPORT_DISABLE

          /* read real and imag values from pSrcA and pSrcB buffer */
          pSourceA1 = *__SIMD32(pInA)++;
          pSourceB1 = *__SIMD32(pInB)++;

          pSourceA2 = *__SIMD32(pInA)++;
          pSourceB2 = *__SIMD32(pInB)++;

          /* Multiply and Accumlates */
          sum = __SMLALD(pSourceA1, pSourceB1, sum);
          sum = __SMLALD(pSourceA2, pSourceB2, sum);

#else
          /* read real and imag values from pSrcA and pSrcB buffer */
          inA1 = *pInA++;
          inB1 = *pInB++;
          inA2 = *pInA++;
          /* Multiply and Accumlates */
          sum += inA1 * inB1;
          inB2 = *pInB++;

          inA1 = *pInA++;
          inB1 = *pInB++;
          /* Multiply and Accumlates */
          sum += inA2 * inB2;
          inA2 = *pInA++;
          inB2 = *pInB++;

          /* Multiply and Accumlates */
          sum += inA1 * inB1;
          sum += inA2 * inB2;

#endif /* #ifndef UNALIGNED_SUPPORT_DISABLE */

          /* Decrement the loop counter */
          colCnt--;
        }

        /* process remaining column samples */
        colCnt = numColsA & 3U;

        while (colCnt > 0U)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */
          sum += *pInA++ * *pInB++;

          /* Decrement the loop counter */
          colCnt--;
        }

        /* Saturate and store the result in the destination buffer */
        *px = (q15_t) (__SSAT((sum >> 15), 16));
        px++;

        /* Decrement the column loop counter */
        col--;

      } while (col > 0U);

      i = i + numColsA;

      /* Decrement the row loop counter */
      row--;

    } while (row > 0U);

#else

  /* Run the below code for Cortex-M0 */

  q15_t *pIn1 = pSrcA->pData;                    /* input data matrix pointer A */
  q15_t *pIn2 = pSrcB->pData;                    /* input data matrix pointer B */
  q15_t *pInA = pSrcA->pData;                    /* input data matrix pointer A of Q15 type */
  q15_t *pInB = pSrcB->pData;                    /* input data matrix pointer B of Q15 type */
  q15_t *pOut = pDst->pData;                     /* output data matrix pointer */
  q15_t *px;                                     /* Temporary output data matrix pointer */
  uint16_t numColsB = pSrcB->numCols;            /* number of columns of input matrix B */
  uint16_t numColsA = pSrcA->numCols;            /* number of columns of input matrix A */
  uint16_t numRowsA = pSrcA->numRows;            /* number of rows of input matrix A    */
  uint16_t col, i = 0U, row = numRowsA, colCnt;  /* loop counters */
  arm_status status;                             /* status of matrix multiplication */

#ifdef ARM_MATH_MATRIX_CHECK

  /* Check for matrix mismatch condition */
  if ((pSrcA->numCols != pSrcB->numRows) ||
     (pSrcA->numRows != pDst->numRows) || (pSrcB->numCols != pDst->numCols))
  {
    /* Set status as ARM_MATH_SIZE_MISMATCH */
    status = ARM_MATH_SIZE_MISMATCH;
  }
  else
#endif /* #ifdef ARM_MATH_MATRIX_CHECK */

  {
    /* The following loop performs the dot-product of each row in pSrcA with each column in pSrcB */
    /* row loop */
    do
    {
      /* Output pointer is set to starting address of the row being processed */
      px = pOut + i;

      /* For every row wise process, the column loop counter is to be initiated */
      col = numColsB;

      /* For every row wise process, the pIn2 pointer is set
       ** to the starting address of the pSrcB data */
      pIn2 = pSrcB->pData;

      /* column loop */
      do
      {
        /* Set the variable sum, that acts as accumulator, to zero */
        sum = 0;

        /* Initiate the pointer pIn1 to point to the starting address of pSrcA */
        pIn1 = pInA;

        /* Matrix A columns number of MAC operations are to be performed */
        colCnt = numColsA;

        /* matrix multiplication */
        while (colCnt > 0U)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */
          /* Perform the multiply-accumulates */
          sum += (q31_t) * pIn1++ * *pIn2;
          pIn2 += numColsB;

          /* Decrement the loop counter */
          colCnt--;
        }

        /* Convert the result from 34.30 to 1.15 format and store the saturated value in destination buffer */
        /* Saturate and store the result in the destination buffer */
        *px++ = (q15_t) __SSAT((sum >> 15), 16);

        /* Decrement the column loop counter */
        col--;

        /* Update the pointer pIn2 to point to the  starting address of the next column */
        pIn2 = pInB + (numColsB - col);

      } while (col > 0U);

      /* Update the pointer pSrcA to point to the  starting address of the next row */
      i = i + numColsB;
      pInA = pInA + numColsA;

      /* Decrement the row loop counter */
      row--;

    } while (row > 0U);

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
