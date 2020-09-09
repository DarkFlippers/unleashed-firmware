/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_conv_partial_fast_q31.c
 * Description:  Fast Q31 Partial convolution
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
 * @addtogroup PartialConv
 * @{
 */

/**
 * @brief Partial convolution of Q31 sequences (fast version) for Cortex-M3 and Cortex-M4.
 * @param[in]       *pSrcA points to the first input sequence.
 * @param[in]       srcALen length of the first input sequence.
 * @param[in]       *pSrcB points to the second input sequence.
 * @param[in]       srcBLen length of the second input sequence.
 * @param[out]      *pDst points to the location where the output result is written.
 * @param[in]       firstIndex is the first output sample to start with.
 * @param[in]       numPoints is the number of output points to be computed.
 * @return Returns either ARM_MATH_SUCCESS if the function completed correctly or ARM_MATH_ARGUMENT_ERROR if the requested subset is not in the range [0 srcALen+srcBLen-2].
 *
 * \par
 * See <code>arm_conv_partial_q31()</code> for a slower implementation of this function which uses a 64-bit accumulator to provide higher precision.
 */

arm_status arm_conv_partial_fast_q31(
  q31_t * pSrcA,
  uint32_t srcALen,
  q31_t * pSrcB,
  uint32_t srcBLen,
  q31_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints)
{
  q31_t *pIn1;                                   /* inputA pointer               */
  q31_t *pIn2;                                   /* inputB pointer               */
  q31_t *pOut = pDst;                            /* output pointer               */
  q31_t *px;                                     /* Intermediate inputA pointer  */
  q31_t *py;                                     /* Intermediate inputB pointer  */
  q31_t *pSrc1, *pSrc2;                          /* Intermediate pointers        */
  q31_t sum, acc0, acc1, acc2, acc3;             /* Accumulators                  */
  q31_t x0, x1, x2, x3, c0;
  uint32_t j, k, count, check, blkCnt;
  int32_t blockSize1, blockSize2, blockSize3;    /* loop counters                 */
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
    blockSize1 = (((int32_t) srcBLen - 1) - (int32_t) firstIndex);
    blockSize1 = (blockSize1 > 0) ? ((check > (srcBLen - 1U)) ? blockSize1 :
                                     (int32_t) numPoints) : 0;
    blockSize2 = (int32_t) check - ((blockSize3 + blockSize1) +
                                    (int32_t) firstIndex);
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
       Since the partial convolution starts from firstIndex
       Number of Macs to be performed is firstIndex + 1 */
    count = 1U + firstIndex;

    /* Working pointer of inputA */
    px = pIn1;

    /* Working pointer of inputB */
    pSrc2 = pIn2 + firstIndex;
    py = pSrc2;

    /* ------------------------
     * Stage1 process
     * ----------------------*/

    /* The first loop starts here */
    while (blockSize1 > 0)
    {
      /* Accumulator is made zero for every iteration */
      sum = 0;

      /* Apply loop unrolling and compute 4 MACs simultaneously. */
      k = count >> 2U;

      /* First part of the processing with loop unrolling.  Compute 4 MACs at a time.
       ** a second loop below computes MACs for the remaining 1 to 3 samples. */
      while (k > 0U)
      {
        /* x[0] * y[srcBLen - 1] */
        sum = (q31_t) ((((q63_t) sum << 32) +
                        ((q63_t) * px++ * (*py--))) >> 32);

        /* x[1] * y[srcBLen - 2] */
        sum = (q31_t) ((((q63_t) sum << 32) +
                        ((q63_t) * px++ * (*py--))) >> 32);

        /* x[2] * y[srcBLen - 3] */
        sum = (q31_t) ((((q63_t) sum << 32) +
                        ((q63_t) * px++ * (*py--))) >> 32);

        /* x[3] * y[srcBLen - 4] */
        sum = (q31_t) ((((q63_t) sum << 32) +
                        ((q63_t) * px++ * (*py--))) >> 32);

        /* Decrement the loop counter */
        k--;
      }

      /* If the count is not a multiple of 4, compute any remaining MACs here.
       ** No loop unrolling is used. */
      k = count % 0x4U;

      while (k > 0U)
      {
        /* Perform the multiply-accumulates */
        sum = (q31_t) ((((q63_t) sum << 32) +
                        ((q63_t) * px++ * (*py--))) >> 32);

        /* Decrement the loop counter */
        k--;
      }

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = sum << 1;

      /* Update the inputA and inputB pointers for next MAC calculation */
      py = ++pSrc2;
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
      /* Loop unroll over blockSize2 */
      blkCnt = ((uint32_t) blockSize2 >> 2U);

      while (blkCnt > 0U)
      {
        /* Set all accumulators to zero */
        acc0 = 0;
        acc1 = 0;
        acc2 = 0;
        acc3 = 0;

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
          acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);

          /* acc1 +=  x[1] * y[srcBLen - 1] */
          acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);

          /* acc2 +=  x[2] * y[srcBLen - 1] */
          acc2 = (q31_t) ((((q63_t) acc2 << 32) + ((q63_t) x2 * c0)) >> 32);

          /* acc3 +=  x[3] * y[srcBLen - 1] */
          acc3 = (q31_t) ((((q63_t) acc3 << 32) + ((q63_t) x3 * c0)) >> 32);

          /* Read y[srcBLen - 2] sample */
          c0 = *(py--);

          /* Read x[4] sample */
          x0 = *(px++);

          /* Perform the multiply-accumulate */
          /* acc0 +=  x[1] * y[srcBLen - 2] */
          acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x1 * c0)) >> 32);
          /* acc1 +=  x[2] * y[srcBLen - 2] */
          acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x2 * c0)) >> 32);
          /* acc2 +=  x[3] * y[srcBLen - 2] */
          acc2 = (q31_t) ((((q63_t) acc2 << 32) + ((q63_t) x3 * c0)) >> 32);
          /* acc3 +=  x[4] * y[srcBLen - 2] */
          acc3 = (q31_t) ((((q63_t) acc3 << 32) + ((q63_t) x0 * c0)) >> 32);

          /* Read y[srcBLen - 3] sample */
          c0 = *(py--);

          /* Read x[5] sample */
          x1 = *(px++);

          /* Perform the multiply-accumulates */
          /* acc0 +=  x[2] * y[srcBLen - 3] */
          acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x2 * c0)) >> 32);
          /* acc1 +=  x[3] * y[srcBLen - 2] */
          acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x3 * c0)) >> 32);
          /* acc2 +=  x[4] * y[srcBLen - 2] */
          acc2 = (q31_t) ((((q63_t) acc2 << 32) + ((q63_t) x0 * c0)) >> 32);
          /* acc3 +=  x[5] * y[srcBLen - 2] */
          acc3 = (q31_t) ((((q63_t) acc3 << 32) + ((q63_t) x1 * c0)) >> 32);

          /* Read y[srcBLen - 4] sample */
          c0 = *(py--);

          /* Read x[6] sample */
          x2 = *(px++);

          /* Perform the multiply-accumulates */
          /* acc0 +=  x[3] * y[srcBLen - 4] */
          acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x3 * c0)) >> 32);
          /* acc1 +=  x[4] * y[srcBLen - 4] */
          acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x0 * c0)) >> 32);
          /* acc2 +=  x[5] * y[srcBLen - 4] */
          acc2 = (q31_t) ((((q63_t) acc2 << 32) + ((q63_t) x1 * c0)) >> 32);
          /* acc3 +=  x[6] * y[srcBLen - 4] */
          acc3 = (q31_t) ((((q63_t) acc3 << 32) + ((q63_t) x2 * c0)) >> 32);


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
          acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);
          /* acc1 +=  x[5] * y[srcBLen - 5] */
          acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);
          /* acc2 +=  x[6] * y[srcBLen - 5] */
          acc2 = (q31_t) ((((q63_t) acc2 << 32) + ((q63_t) x2 * c0)) >> 32);
          /* acc3 +=  x[7] * y[srcBLen - 5] */
          acc3 = (q31_t) ((((q63_t) acc3 << 32) + ((q63_t) x3 * c0)) >> 32);

          /* Reuse the present samples for the next MAC */
          x0 = x1;
          x1 = x2;
          x2 = x3;

          /* Decrement the loop counter */
          k--;
        }

        /* Store the result in the accumulator in the destination buffer. */
        *pOut++ = (q31_t) (acc0 << 1);
        *pOut++ = (q31_t) (acc1 << 1);
        *pOut++ = (q31_t) (acc2 << 1);
        *pOut++ = (q31_t) (acc3 << 1);

        /* Increment the pointer pIn1 index, count by 4 */
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
        sum = 0;

        /* Apply loop unrolling and compute 4 MACs simultaneously. */
        k = srcBLen >> 2U;

        /* First part of the processing with loop unrolling.  Compute 4 MACs at a time.
         ** a second loop below computes MACs for the remaining 1 to 3 samples. */
        while (k > 0U)
        {
          /* Perform the multiply-accumulates */
          sum = (q31_t) ((((q63_t) sum << 32) +
                          ((q63_t) * px++ * (*py--))) >> 32);
          sum = (q31_t) ((((q63_t) sum << 32) +
                          ((q63_t) * px++ * (*py--))) >> 32);
          sum = (q31_t) ((((q63_t) sum << 32) +
                          ((q63_t) * px++ * (*py--))) >> 32);
          sum = (q31_t) ((((q63_t) sum << 32) +
                          ((q63_t) * px++ * (*py--))) >> 32);

          /* Decrement the loop counter */
          k--;
        }

        /* If the srcBLen is not a multiple of 4, compute any remaining MACs here.
         ** No loop unrolling is used. */
        k = srcBLen % 0x4U;

        while (k > 0U)
        {
          /* Perform the multiply-accumulate */
          sum = (q31_t) ((((q63_t) sum << 32) +
                          ((q63_t) * px++ * (*py--))) >> 32);

          /* Decrement the loop counter */
          k--;
        }

        /* Store the result in the accumulator in the destination buffer. */
        *pOut++ = sum << 1;

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
        sum = 0;

        /* srcBLen number of MACS should be performed */
        k = srcBLen;

        while (k > 0U)
        {
          /* Perform the multiply-accumulate */
          sum = (q31_t) ((((q63_t) sum << 32) +
                          ((q63_t) * px++ * (*py--))) >> 32);

          /* Decrement the loop counter */
          k--;
        }

        /* Store the result in the accumulator in the destination buffer. */
        *pOut++ = sum << 1;

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

    /* -------------------
     * Stage3 process
     * ------------------*/

    while (blockSize3 > 0)
    {
      /* Accumulator is made zero for every iteration */
      sum = 0;

      /* Apply loop unrolling and compute 4 MACs simultaneously. */
      k = count >> 2U;

      /* First part of the processing with loop unrolling.  Compute 4 MACs at a time.
       ** a second loop below computes MACs for the remaining 1 to 3 samples. */
      while (k > 0U)
      {
        /* sum += x[srcALen - srcBLen + 1] * y[srcBLen - 1] */
        sum = (q31_t) ((((q63_t) sum << 32) +
                        ((q63_t) * px++ * (*py--))) >> 32);

        /* sum += x[srcALen - srcBLen + 2] * y[srcBLen - 2] */
        sum = (q31_t) ((((q63_t) sum << 32) +
                        ((q63_t) * px++ * (*py--))) >> 32);

        /* sum += x[srcALen - srcBLen + 3] * y[srcBLen - 3] */
        sum = (q31_t) ((((q63_t) sum << 32) +
                        ((q63_t) * px++ * (*py--))) >> 32);

        /* sum += x[srcALen - srcBLen + 4] * y[srcBLen - 4] */
        sum = (q31_t) ((((q63_t) sum << 32) +
                        ((q63_t) * px++ * (*py--))) >> 32);

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
        sum = (q31_t) ((((q63_t) sum << 32) +
                        ((q63_t) * px++ * (*py--))) >> 32);

        /* Decrement the loop counter */
        k--;
      }

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = sum << 1;

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

}

/**
 * @} end of PartialConv group
 */
