/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_conv_partial_f32.c
 * Description:  Partial convolution of floating-point sequences
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
 * @ingroup groupFilters
 */

/**
 * @defgroup PartialConv Partial Convolution
 *
 * Partial Convolution is equivalent to Convolution except that a subset of the output samples is generated.
 * Each function has two additional arguments.
 * <code>firstIndex</code> specifies the starting index of the subset of output samples.
 * <code>numPoints</code> is the number of output samples to compute.
 * The function computes the output in the range
 * <code>[firstIndex, ..., firstIndex+numPoints-1]</code>.
 * The output array <code>pDst</code> contains <code>numPoints</code> values.
 *
 * The allowable range of output indices is [0 srcALen+srcBLen-2].
 * If the requested subset does not fall in this range then the functions return ARM_MATH_ARGUMENT_ERROR.
 * Otherwise the functions return ARM_MATH_SUCCESS.
 * \note Refer arm_conv_f32() for details on fixed point behavior.
 *
 *
 * <b>Fast Versions</b>
 *
 * \par
 * Fast versions are supported for Q31 and Q15 of partial convolution.  Cycles for Fast versions are less compared to Q31 and Q15 of partial conv and the design requires
 * the input signals should be scaled down to avoid intermediate overflows.
 *
 *
 * <b>Opt Versions</b>
 *
 * \par
 * Opt versions are supported for Q15 and Q7.  Design uses internal scratch buffer for getting good optimisation.
 * These versions are optimised in cycles and consumes more memory(Scratch memory) compared to Q15 and Q7 versions of partial convolution
 */

/**
 * @addtogroup PartialConv
 * @{
 */

/**
 * @brief Partial convolution of floating-point sequences.
 * @param[in]       *pSrcA points to the first input sequence.
 * @param[in]       srcALen length of the first input sequence.
 * @param[in]       *pSrcB points to the second input sequence.
 * @param[in]       srcBLen length of the second input sequence.
 * @param[out]      *pDst points to the location where the output result is written.
 * @param[in]       firstIndex is the first output sample to start with.
 * @param[in]       numPoints is the number of output points to be computed.
 * @return  Returns either ARM_MATH_SUCCESS if the function completed correctly or ARM_MATH_ARGUMENT_ERROR if the requested subset is not in the range [0 srcALen+srcBLen-2].
 */

arm_status arm_conv_partial_f32(
  float32_t * pSrcA,
  uint32_t srcALen,
  float32_t * pSrcB,
  uint32_t srcBLen,
  float32_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints)
{


#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  float32_t *pIn1 = pSrcA;                       /* inputA pointer */
  float32_t *pIn2 = pSrcB;                       /* inputB pointer */
  float32_t *pOut = pDst;                        /* output pointer */
  float32_t *px;                                 /* Intermediate inputA pointer */
  float32_t *py;                                 /* Intermediate inputB pointer */
  float32_t *pSrc1, *pSrc2;                      /* Intermediate pointers */
  float32_t sum, acc0, acc1, acc2, acc3;         /* Accumulator */
  float32_t x0, x1, x2, x3, c0;                  /* Temporary variables to hold state and coefficient values */
  uint32_t j, k, count = 0U, blkCnt, check;
  int32_t blockSize1, blockSize2, blockSize3;    /* loop counters */
  arm_status status;                             /* status of Partial convolution */


  /* Check for range of output samples to be calculated */
  if ((firstIndex + numPoints) > ((srcALen + (srcBLen - 1U))))
  {
    /* Set status as ARM_MATH_ARGUMENT_ERROR */
    status = ARM_MATH_ARGUMENT_ERROR;
  }
  else
  {

    /* The algorithm implementation is based on the lengths of the inputs. */
    /* srcB is always made to slide across srcA. */
    /* So srcBLen is always considered as shorter or equal to srcALen */
    if (srcALen >= srcBLen)
    {
      /* Initialization of inputA pointer */
      pIn1 = pSrcA;

      /* Initialization of inputB pointer */
      pIn2 = pSrcB;
    }
    else
    {
      /* Initialization of inputA pointer */
      pIn1 = pSrcB;

      /* Initialization of inputB pointer */
      pIn2 = pSrcA;

      /* srcBLen is always considered as shorter or equal to srcALen */
      j = srcBLen;
      srcBLen = srcALen;
      srcALen = j;
    }

    /* Conditions to check which loopCounter holds
     * the first and last indices of the output samples to be calculated. */
    check = firstIndex + numPoints;
    blockSize3 = ((int32_t)check > (int32_t)srcALen) ? (int32_t)check - (int32_t)srcALen : 0;
    blockSize3 = ((int32_t)firstIndex > (int32_t)srcALen - 1) ? blockSize3 - (int32_t)firstIndex + (int32_t)srcALen : blockSize3;
    blockSize1 = ((int32_t) srcBLen - 1) - (int32_t) firstIndex;
    blockSize1 = (blockSize1 > 0) ? ((check > (srcBLen - 1U)) ? blockSize1 :
                                     (int32_t) numPoints) : 0;
    blockSize2 = ((int32_t) check - blockSize3) -
      (blockSize1 + (int32_t) firstIndex);
    blockSize2 = (blockSize2 > 0) ? blockSize2 : 0;

    /* conv(x,y) at n = x[n] * y[0] + x[n-1] * y[1] + x[n-2] * y[2] + ...+ x[n-N+1] * y[N -1] */
    /* The function is internally
     * divided into three stages according to the number of multiplications that has to be
     * taken place between inputA samples and inputB samples. In the first stage of the
     * algorithm, the multiplications increase by one for every iteration.
     * In the second stage of the algorithm, srcBLen number of multiplications are done.
     * In the third stage of the algorithm, the multiplications decrease by one
     * for every iteration. */

    /* Set the output pointer to point to the firstIndex
     * of the output sample to be calculated. */
    pOut = pDst + firstIndex;

    /* --------------------------
     * Initializations of stage1
     * -------------------------*/

    /* sum = x[0] * y[0]
     * sum = x[0] * y[1] + x[1] * y[0]
     * ....
     * sum = x[0] * y[srcBlen - 1] + x[1] * y[srcBlen - 2] +...+ x[srcBLen - 1] * y[0]
     */

    /* In this stage the MAC operations are increased by 1 for every iteration.
       The count variable holds the number of MAC operations performed.
       Since the partial convolution starts from from firstIndex
       Number of Macs to be performed is firstIndex + 1 */
    count = 1U + firstIndex;

    /* Working pointer of inputA */
    px = pIn1;

    /* Working pointer of inputB */
    pSrc1 = pIn2 + firstIndex;
    py = pSrc1;

    /* ------------------------
     * Stage1 process
     * ----------------------*/

    /* The first stage starts here */
    while (blockSize1 > 0)
    {
      /* Accumulator is made zero for every iteration */
      sum = 0.0f;

      /* Apply loop unrolling and compute 4 MACs simultaneously. */
      k = count >> 2U;

      /* First part of the processing with loop unrolling.  Compute 4 MACs at a time.
       ** a second loop below computes MACs for the remaining 1 to 3 samples. */
      while (k > 0U)
      {
        /* x[0] * y[srcBLen - 1] */
        sum += *px++ * *py--;

        /* x[1] * y[srcBLen - 2] */
        sum += *px++ * *py--;

        /* x[2] * y[srcBLen - 3] */
        sum += *px++ * *py--;

        /* x[3] * y[srcBLen - 4] */
        sum += *px++ * *py--;

        /* Decrement the loop counter */
        k--;
      }

      /* If the count is not a multiple of 4, compute any remaining MACs here.
       ** No loop unrolling is used. */
      k = count % 0x4U;

      while (k > 0U)
      {
        /* Perform the multiply-accumulates */
        sum += *px++ * *py--;

        /* Decrement the loop counter */
        k--;
      }

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = sum;

      /* Update the inputA and inputB pointers for next MAC calculation */
      py = ++pSrc1;
      px = pIn1;

      /* Increment the MAC count */
      count++;

      /* Decrement the loop counter */
      blockSize1--;
    }

    /* --------------------------
     * Initializations of stage2
     * ------------------------*/

    /* sum = x[0] * y[srcBLen-1] + x[1] * y[srcBLen-2] +...+ x[srcBLen-1] * y[0]
     * sum = x[1] * y[srcBLen-1] + x[2] * y[srcBLen-2] +...+ x[srcBLen] * y[0]
     * ....
     * sum = x[srcALen-srcBLen-2] * y[srcBLen-1] + x[srcALen] * y[srcBLen-2] +...+ x[srcALen-1] * y[0]
     */

    /* Working pointer of inputA */
    if ((int32_t)firstIndex - (int32_t)srcBLen + 1 > 0)
    {
      px = pIn1 + firstIndex - srcBLen + 1;
    }
    else
    {
      px = pIn1;
    }

    /* Working pointer of inputB */
    pSrc2 = pIn2 + (srcBLen - 1U);
    py = pSrc2;

    /* count is index by which the pointer pIn1 to be incremented */
    count = 0U;

    /* -------------------
     * Stage2 process
     * ------------------*/

    /* Stage2 depends on srcBLen as in this stage srcBLen number of MACS are performed.
     * So, to loop unroll over blockSize2,
     * srcBLen should be greater than or equal to 4 */
    if (srcBLen >= 4U)
    {
      /* Loop unroll over blockSize2, by 4 */
      blkCnt = ((uint32_t) blockSize2 >> 2U);

      while (blkCnt > 0U)
      {
        /* Set all accumulators to zero */
        acc0 = 0.0f;
        acc1 = 0.0f;
        acc2 = 0.0f;
        acc3 = 0.0f;

        /* read x[0], x[1], x[2] samples */
        x0 = *(px++);
        x1 = *(px++);
        x2 = *(px++);

        /* Apply loop unrolling and compute 4 MACs simultaneously. */
        k = srcBLen >> 2U;

        /* First part of the processing with loop unrolling.  Compute 4 MACs at a time.
         ** a second loop below computes MACs for the remaining 1 to 3 samples. */
        do
        {
          /* Read y[srcBLen - 1] sample */
          c0 = *(py--);

          /* Read x[3] sample */
          x3 = *(px++);

          /* Perform the multiply-accumulate */
          /* acc0 +=  x[0] * y[srcBLen - 1] */
          acc0 += x0 * c0;

          /* acc1 +=  x[1] * y[srcBLen - 1] */
          acc1 += x1 * c0;

          /* acc2 +=  x[2] * y[srcBLen - 1] */
          acc2 += x2 * c0;

          /* acc3 +=  x[3] * y[srcBLen - 1] */
          acc3 += x3 * c0;

          /* Read y[srcBLen - 2] sample */
          c0 = *(py--);

          /* Read x[4] sample */
          x0 = *(px++);

          /* Perform the multiply-accumulate */
          /* acc0 +=  x[1] * y[srcBLen - 2] */
          acc0 += x1 * c0;
          /* acc1 +=  x[2] * y[srcBLen - 2] */
          acc1 += x2 * c0;
          /* acc2 +=  x[3] * y[srcBLen - 2] */
          acc2 += x3 * c0;
          /* acc3 +=  x[4] * y[srcBLen - 2] */
          acc3 += x0 * c0;

          /* Read y[srcBLen - 3] sample */
          c0 = *(py--);

          /* Read x[5] sample */
          x1 = *(px++);

          /* Perform the multiply-accumulates */
          /* acc0 +=  x[2] * y[srcBLen - 3] */
          acc0 += x2 * c0;
          /* acc1 +=  x[3] * y[srcBLen - 2] */
          acc1 += x3 * c0;
          /* acc2 +=  x[4] * y[srcBLen - 2] */
          acc2 += x0 * c0;
          /* acc3 +=  x[5] * y[srcBLen - 2] */
          acc3 += x1 * c0;

          /* Read y[srcBLen - 4] sample */
          c0 = *(py--);

          /* Read x[6] sample */
          x2 = *(px++);

          /* Perform the multiply-accumulates */
          /* acc0 +=  x[3] * y[srcBLen - 4] */
          acc0 += x3 * c0;
          /* acc1 +=  x[4] * y[srcBLen - 4] */
          acc1 += x0 * c0;
          /* acc2 +=  x[5] * y[srcBLen - 4] */
          acc2 += x1 * c0;
          /* acc3 +=  x[6] * y[srcBLen - 4] */
          acc3 += x2 * c0;


        } while (--k);

        /* If the srcBLen is not a multiple of 4, compute any remaining MACs here.
         ** No loop unrolling is used. */
        k = srcBLen % 0x4U;

        while (k > 0U)
        {
          /* Read y[srcBLen - 5] sample */
          c0 = *(py--);

          /* Read x[7] sample */
          x3 = *(px++);

          /* Perform the multiply-accumulates */
          /* acc0 +=  x[4] * y[srcBLen - 5] */
          acc0 += x0 * c0;
          /* acc1 +=  x[5] * y[srcBLen - 5] */
          acc1 += x1 * c0;
          /* acc2 +=  x[6] * y[srcBLen - 5] */
          acc2 += x2 * c0;
          /* acc3 +=  x[7] * y[srcBLen - 5] */
          acc3 += x3 * c0;

          /* Reuse the present samples for the next MAC */
          x0 = x1;
          x1 = x2;
          x2 = x3;

          /* Decrement the loop counter */
          k--;
        }

        /* Store the result in the accumulator in the destination buffer. */
        *pOut++ = acc0;
        *pOut++ = acc1;
        *pOut++ = acc2;
        *pOut++ = acc3;

        /* Increment the pointer pIn1 index, count by 1 */
        count += 4U;

        /* Update the inputA and inputB pointers for next MAC calculation */
        if ((int32_t)firstIndex - (int32_t)srcBLen + 1 > 0)
        {
          px = pIn1 + firstIndex - srcBLen + 1 + count;
        }
        else
        {
          px = pIn1 + count;
        }
        py = pSrc2;

        /* Decrement the loop counter */
        blkCnt--;
      }

      /* If the blockSize2 is not a multiple of 4, compute any remaining output samples here.
       ** No loop unrolling is used. */
      blkCnt = (uint32_t) blockSize2 % 0x4U;

      while (blkCnt > 0U)
      {
        /* Accumulator is made zero for every iteration */
        sum = 0.0f;

        /* Apply loop unrolling and compute 4 MACs simultaneously. */
        k = srcBLen >> 2U;

        /* First part of the processing with loop unrolling.  Compute 4 MACs at a time.
         ** a second loop below computes MACs for the remaining 1 to 3 samples. */
        while (k > 0U)
        {
          /* Perform the multiply-accumulates */
          sum += *px++ * *py--;
          sum += *px++ * *py--;
          sum += *px++ * *py--;
          sum += *px++ * *py--;

          /* Decrement the loop counter */
          k--;
        }

        /* If the srcBLen is not a multiple of 4, compute any remaining MACs here.
         ** No loop unrolling is used. */
        k = srcBLen % 0x4U;

        while (k > 0U)
        {
          /* Perform the multiply-accumulate */
          sum += *px++ * *py--;

          /* Decrement the loop counter */
          k--;
        }

        /* Store the result in the accumulator in the destination buffer. */
        *pOut++ = sum;

        /* Increment the MAC count */
        count++;

        /* Update the inputA and inputB pointers for next MAC calculation */
        if ((int32_t)firstIndex - (int32_t)srcBLen + 1 > 0)
        {
          px = pIn1 + firstIndex - srcBLen + 1 + count;
        }
        else
        {
          px = pIn1 + count;
        }
        py = pSrc2;

        /* Decrement the loop counter */
        blkCnt--;
      }
    }
    else
    {
      /* If the srcBLen is not a multiple of 4,
       * the blockSize2 loop cannot be unrolled by 4 */
      blkCnt = (uint32_t) blockSize2;

      while (blkCnt > 0U)
      {
        /* Accumulator is made zero for every iteration */
        sum = 0.0f;

        /* srcBLen number of MACS should be performed */
        k = srcBLen;

        while (k > 0U)
        {
          /* Perform the multiply-accumulate */
          sum += *px++ * *py--;

          /* Decrement the loop counter */
          k--;
        }

        /* Store the result in the accumulator in the destination buffer. */
        *pOut++ = sum;

        /* Increment the MAC count */
        count++;

        /* Update the inputA and inputB pointers for next MAC calculation */
        if ((int32_t)firstIndex - (int32_t)srcBLen + 1 > 0)
        {
          px = pIn1 + firstIndex - srcBLen + 1 + count;
        }
        else
        {
          px = pIn1 + count;
        }
        py = pSrc2;

        /* Decrement the loop counter */
        blkCnt--;
      }
    }


    /* --------------------------
     * Initializations of stage3
     * -------------------------*/

    /* sum += x[srcALen-srcBLen+1] * y[srcBLen-1] + x[srcALen-srcBLen+2] * y[srcBLen-2] +...+ x[srcALen-1] * y[1]
     * sum += x[srcALen-srcBLen+2] * y[srcBLen-1] + x[srcALen-srcBLen+3] * y[srcBLen-2] +...+ x[srcALen-1] * y[2]
     * ....
     * sum +=  x[srcALen-2] * y[srcBLen-1] + x[srcALen-1] * y[srcBLen-2]
     * sum +=  x[srcALen-1] * y[srcBLen-1]
     */

    /* In this stage the MAC operations are decreased by 1 for every iteration.
       The count variable holds the number of MAC operations performed */
    count = srcBLen - 1U;

    /* Working pointer of inputA */
    pSrc1 = (pIn1 + srcALen) - (srcBLen - 1U);
    px = pSrc1;

    /* Working pointer of inputB */
    pSrc2 = pIn2 + (srcBLen - 1U);
    py = pSrc2;

    while (blockSize3 > 0)
    {
      /* Accumulator is made zero for every iteration */
      sum = 0.0f;

      /* Apply loop unrolling and compute 4 MACs simultaneously. */
      k = count >> 2U;

      /* First part of the processing with loop unrolling.  Compute 4 MACs at a time.
       ** a second loop below computes MACs for the remaining 1 to 3 samples. */
      while (k > 0U)
      {
        /* sum += x[srcALen - srcBLen + 1] * y[srcBLen - 1] */
        sum += *px++ * *py--;

        /* sum += x[srcALen - srcBLen + 2] * y[srcBLen - 2] */
        sum += *px++ * *py--;

        /* sum += x[srcALen - srcBLen + 3] * y[srcBLen - 3] */
        sum += *px++ * *py--;

        /* sum += x[srcALen - srcBLen + 4] * y[srcBLen - 4] */
        sum += *px++ * *py--;

        /* Decrement the loop counter */
        k--;
      }

      /* If the count is not a multiple of 4, compute any remaining MACs here.
       ** No loop unrolling is used. */
      k = count % 0x4U;

      while (k > 0U)
      {
        /* Perform the multiply-accumulates */
        /* sum +=  x[srcALen-1] * y[srcBLen-1] */
        sum += *px++ * *py--;

        /* Decrement the loop counter */
        k--;
      }

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = sum;

      /* Update the inputA and inputB pointers for next MAC calculation */
      px = ++pSrc1;
      py = pSrc2;

      /* Decrement the MAC count */
      count--;

      /* Decrement the loop counter */
      blockSize3--;

    }

    /* set status as ARM_MATH_SUCCESS */
    status = ARM_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);

#else

  /* Run the below code for Cortex-M0 */

  float32_t *pIn1 = pSrcA;                       /* inputA pointer */
  float32_t *pIn2 = pSrcB;                       /* inputB pointer */
  float32_t sum;                                 /* Accumulator */
  uint32_t i, j;                                 /* loop counters */
  arm_status status;                             /* status of Partial convolution */

  /* Check for range of output samples to be calculated */
  if ((firstIndex + numPoints) > ((srcALen + (srcBLen - 1U))))
  {
    /* Set status as ARM_ARGUMENT_ERROR */
    status = ARM_MATH_ARGUMENT_ERROR;
  }
  else
  {
    /* Loop to calculate convolution for output length number of values */
    for (i = firstIndex; i <= (firstIndex + numPoints - 1); i++)
    {
      /* Initialize sum with zero to carry on MAC operations */
      sum = 0.0f;

      /* Loop to perform MAC operations according to convolution equation */
      for (j = 0U; j <= i; j++)
      {
        /* Check the array limitations for inputs */
        if ((((i - j) < srcBLen) && (j < srcALen)))
        {
          /* z[i] += x[i-j] * y[j] */
          sum += pIn1[j] * pIn2[i - j];
        }
      }
      /* Store the output in the destination buffer */
      pDst[i] = sum;
    }
    /* set status as ARM_SUCCESS as there are no argument errors */
    status = ARM_MATH_SUCCESS;
  }
  return (status);

#endif /*   #if defined (ARM_MATH_DSP) */

}

/**
 * @} end of PartialConv group
 */
