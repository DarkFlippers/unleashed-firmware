/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_correlate_fast_opt_q15.c
 * Description:  Fast Q15 Correlation
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
 * @addtogroup Corr
 * @{
 */

/**
 * @brief Correlation of Q15 sequences (fast version) for Cortex-M3 and Cortex-M4.
 * @param[in] *pSrcA points to the first input sequence.
 * @param[in] srcALen length of the first input sequence.
 * @param[in] *pSrcB points to the second input sequence.
 * @param[in] srcBLen length of the second input sequence.
 * @param[out] *pDst points to the location where the output result is written.  Length 2 * max(srcALen, srcBLen) - 1.
 * @param[in]  *pScratch points to scratch buffer of size max(srcALen, srcBLen) + 2*min(srcALen, srcBLen) - 2.
 * @return none.
 *
 *
 * \par Restrictions
 *  If the silicon does not support unaligned memory access enable the macro UNALIGNED_SUPPORT_DISABLE
 *  In this case input, output, scratch buffers should be aligned by 32-bit
 *
 *
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * This fast version uses a 32-bit accumulator with 2.30 format.
 * The accumulator maintains full precision of the intermediate multiplication results but provides only a single guard bit.
 * There is no saturation on intermediate additions.
 * Thus, if the accumulator overflows it wraps around and distorts the result.
 * The input signals should be scaled down to avoid intermediate overflows.
 * Scale down one of the inputs by 1/min(srcALen, srcBLen) to avoid overflow since a
 * maximum of min(srcALen, srcBLen) number of additions is carried internally.
 * The 2.30 accumulator is right shifted by 15 bits and then saturated to 1.15 format to yield the final result.
 *
 * \par
 * See <code>arm_correlate_q15()</code> for a slower implementation of this function which uses a 64-bit accumulator to avoid wrap around distortion.
 */

void arm_correlate_fast_opt_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst,
  q15_t * pScratch)
{
  q15_t *pIn1;                                   /* inputA pointer               */
  q15_t *pIn2;                                   /* inputB pointer               */
  q31_t acc0, acc1, acc2, acc3;                  /* Accumulators                  */
  q15_t *py;                                     /* Intermediate inputB pointer  */
  q31_t x1, x2, x3;                              /* temporary variables for holding input and coefficient values */
  uint32_t j, blkCnt, outBlockSize;              /* loop counter                 */
  int32_t inc = 1;                               /* Destination address modifier */
  uint32_t tapCnt;
  q31_t y1, y2;
  q15_t *pScr;                                   /* Intermediate pointers        */
  q15_t *pOut = pDst;                            /* output pointer               */
#ifdef UNALIGNED_SUPPORT_DISABLE

  q15_t a, b;

#endif /* #ifndef UNALIGNED_SUPPORT_DISABLE */

  /* The algorithm implementation is based on the lengths of the inputs. */
  /* srcB is always made to slide across srcA. */
  /* So srcBLen is always considered as shorter or equal to srcALen */
  /* But CORR(x, y) is reverse of CORR(y, x) */
  /* So, when srcBLen > srcALen, output pointer is made to point to the end of the output buffer */
  /* and the destination pointer modifier, inc is set to -1 */
  /* If srcALen > srcBLen, zero pad has to be done to srcB to make the two inputs of same length */
  /* But to improve the performance,
   * we include zeroes in the output instead of zero padding either of the the inputs*/
  /* If srcALen > srcBLen,
   * (srcALen - srcBLen) zeroes has to included in the starting of the output buffer */
  /* If srcALen < srcBLen,
   * (srcALen - srcBLen) zeroes has to included in the ending of the output buffer */
  if (srcALen >= srcBLen)
  {
    /* Initialization of inputA pointer */
    pIn1 = (pSrcA);

    /* Initialization of inputB pointer */
    pIn2 = (pSrcB);

    /* Number of output samples is calculated */
    outBlockSize = (2U * srcALen) - 1U;

    /* When srcALen > srcBLen, zero padding is done to srcB
     * to make their lengths equal.
     * Instead, (outBlockSize - (srcALen + srcBLen - 1))
     * number of output samples are made zero */
    j = outBlockSize - (srcALen + (srcBLen - 1U));

    /* Updating the pointer position to non zero value */
    pOut += j;

  }
  else
  {
    /* Initialization of inputA pointer */
    pIn1 = (pSrcB);

    /* Initialization of inputB pointer */
    pIn2 = (pSrcA);

    /* srcBLen is always considered as shorter or equal to srcALen */
    j = srcBLen;
    srcBLen = srcALen;
    srcALen = j;

    /* CORR(x, y) = Reverse order(CORR(y, x)) */
    /* Hence set the destination pointer to point to the last output sample */
    pOut = pDst + ((srcALen + srcBLen) - 2U);

    /* Destination address modifier is set to -1 */
    inc = -1;

  }

  pScr = pScratch;

  /* Fill (srcBLen - 1U) zeros in scratch buffer */
  arm_fill_q15(0, pScr, (srcBLen - 1U));

  /* Update temporary scratch pointer */
  pScr += (srcBLen - 1U);

#ifndef UNALIGNED_SUPPORT_DISABLE

  /* Copy (srcALen) samples in scratch buffer */
  arm_copy_q15(pIn1, pScr, srcALen);

  /* Update pointers */
  pScr += srcALen;

#else

  /* Apply loop unrolling and do 4 Copies simultaneously. */
  j = srcALen >> 2U;

  /* First part of the processing with loop unrolling copies 4 data points at a time.
   ** a second loop below copies for the remaining 1 to 3 samples. */
  while (j > 0U)
  {
    /* copy second buffer in reversal manner */
    *pScr++ = *pIn1++;
    *pScr++ = *pIn1++;
    *pScr++ = *pIn1++;
    *pScr++ = *pIn1++;

    /* Decrement the loop counter */
    j--;
  }

  /* If the count is not a multiple of 4, copy remaining samples here.
   ** No loop unrolling is used. */
  j = srcALen % 0x4U;

  while (j > 0U)
  {
    /* copy second buffer in reversal manner for remaining samples */
    *pScr++ = *pIn1++;

    /* Decrement the loop counter */
    j--;
  }

#endif /* #ifndef UNALIGNED_SUPPORT_DISABLE */

#ifndef UNALIGNED_SUPPORT_DISABLE

  /* Fill (srcBLen - 1U) zeros at end of scratch buffer */
  arm_fill_q15(0, pScr, (srcBLen - 1U));

  /* Update pointer */
  pScr += (srcBLen - 1U);

#else

/* Apply loop unrolling and do 4 Copies simultaneously. */
  j = (srcBLen - 1U) >> 2U;

  /* First part of the processing with loop unrolling copies 4 data points at a time.
   ** a second loop below copies for the remaining 1 to 3 samples. */
  while (j > 0U)
  {
    /* copy second buffer in reversal manner */
    *pScr++ = 0;
    *pScr++ = 0;
    *pScr++ = 0;
    *pScr++ = 0;

    /* Decrement the loop counter */
    j--;
  }

  /* If the count is not a multiple of 4, copy remaining samples here.
   ** No loop unrolling is used. */
  j = (srcBLen - 1U) % 0x4U;

  while (j > 0U)
  {
    /* copy second buffer in reversal manner for remaining samples */
    *pScr++ = 0;

    /* Decrement the loop counter */
    j--;
  }

#endif /* #ifndef UNALIGNED_SUPPORT_DISABLE */

  /* Temporary pointer for scratch2 */
  py = pIn2;


  /* Actual correlation process starts here */
  blkCnt = (srcALen + srcBLen - 1U) >> 2;

  while (blkCnt > 0)
  {
    /* Initialze temporary scratch pointer as scratch1 */
    pScr = pScratch;

    /* Clear Accumlators */
    acc0 = 0;
    acc1 = 0;
    acc2 = 0;
    acc3 = 0;

    /* Read four samples from scratch1 buffer */
    x1 = *__SIMD32(pScr)++;

    /* Read next four samples from scratch1 buffer */
    x2 = *__SIMD32(pScr)++;

    tapCnt = (srcBLen) >> 2U;

    while (tapCnt > 0U)
    {

#ifndef UNALIGNED_SUPPORT_DISABLE

      /* Read four samples from smaller buffer */
      y1 = _SIMD32_OFFSET(pIn2);
      y2 = _SIMD32_OFFSET(pIn2 + 2U);

      acc0 = __SMLAD(x1, y1, acc0);

      acc2 = __SMLAD(x2, y1, acc2);

#ifndef ARM_MATH_BIG_ENDIAN
      x3 = __PKHBT(x2, x1, 0);
#else
      x3 = __PKHBT(x1, x2, 0);
#endif

      acc1 = __SMLADX(x3, y1, acc1);

      x1 = _SIMD32_OFFSET(pScr);

      acc0 = __SMLAD(x2, y2, acc0);

      acc2 = __SMLAD(x1, y2, acc2);

#ifndef ARM_MATH_BIG_ENDIAN
      x3 = __PKHBT(x1, x2, 0);
#else
      x3 = __PKHBT(x2, x1, 0);
#endif

      acc3 = __SMLADX(x3, y1, acc3);

      acc1 = __SMLADX(x3, y2, acc1);

      x2 = _SIMD32_OFFSET(pScr + 2U);

#ifndef ARM_MATH_BIG_ENDIAN
      x3 = __PKHBT(x2, x1, 0);
#else
      x3 = __PKHBT(x1, x2, 0);
#endif

      acc3 = __SMLADX(x3, y2, acc3);
#else

      /* Read four samples from smaller buffer */
      a = *pIn2;
      b = *(pIn2 + 1);

#ifndef ARM_MATH_BIG_ENDIAN
      y1 = __PKHBT(a, b, 16);
#else
      y1 = __PKHBT(b, a, 16);
#endif

      a = *(pIn2 + 2);
      b = *(pIn2 + 3);
#ifndef ARM_MATH_BIG_ENDIAN
      y2 = __PKHBT(a, b, 16);
#else
      y2 = __PKHBT(b, a, 16);
#endif

      acc0 = __SMLAD(x1, y1, acc0);

      acc2 = __SMLAD(x2, y1, acc2);

#ifndef ARM_MATH_BIG_ENDIAN
      x3 = __PKHBT(x2, x1, 0);
#else
      x3 = __PKHBT(x1, x2, 0);
#endif

      acc1 = __SMLADX(x3, y1, acc1);

      a = *pScr;
      b = *(pScr + 1);

#ifndef ARM_MATH_BIG_ENDIAN
      x1 = __PKHBT(a, b, 16);
#else
      x1 = __PKHBT(b, a, 16);
#endif

      acc0 = __SMLAD(x2, y2, acc0);

      acc2 = __SMLAD(x1, y2, acc2);

#ifndef ARM_MATH_BIG_ENDIAN
      x3 = __PKHBT(x1, x2, 0);
#else
      x3 = __PKHBT(x2, x1, 0);
#endif

      acc3 = __SMLADX(x3, y1, acc3);

      acc1 = __SMLADX(x3, y2, acc1);

      a = *(pScr + 2);
      b = *(pScr + 3);

#ifndef ARM_MATH_BIG_ENDIAN
      x2 = __PKHBT(a, b, 16);
#else
      x2 = __PKHBT(b, a, 16);
#endif

#ifndef ARM_MATH_BIG_ENDIAN
      x3 = __PKHBT(x2, x1, 0);
#else
      x3 = __PKHBT(x1, x2, 0);
#endif

      acc3 = __SMLADX(x3, y2, acc3);

#endif /* #ifndef UNALIGNED_SUPPORT_DISABLE */

      pIn2 += 4U;

      pScr += 4U;


      /* Decrement the loop counter */
      tapCnt--;
    }



    /* Update scratch pointer for remaining samples of smaller length sequence */
    pScr -= 4U;


    /* apply same above for remaining samples of smaller length sequence */
    tapCnt = (srcBLen) & 3U;

    while (tapCnt > 0U)
    {

      /* accumlate the results */
      acc0 += (*pScr++ * *pIn2);
      acc1 += (*pScr++ * *pIn2);
      acc2 += (*pScr++ * *pIn2);
      acc3 += (*pScr++ * *pIn2++);

      pScr -= 3U;

      /* Decrement the loop counter */
      tapCnt--;
    }

    blkCnt--;


    /* Store the results in the accumulators in the destination buffer. */
    *pOut = (__SSAT(acc0 >> 15U, 16));
    pOut += inc;
    *pOut = (__SSAT(acc1 >> 15U, 16));
    pOut += inc;
    *pOut = (__SSAT(acc2 >> 15U, 16));
    pOut += inc;
    *pOut = (__SSAT(acc3 >> 15U, 16));
    pOut += inc;


    /* Initialization of inputB pointer */
    pIn2 = py;

    pScratch += 4U;

  }


  blkCnt = (srcALen + srcBLen - 1U) & 0x3;

  /* Calculate correlation for remaining samples of Bigger length sequence */
  while (blkCnt > 0)
  {
    /* Initialze temporary scratch pointer as scratch1 */
    pScr = pScratch;

    /* Clear Accumlators */
    acc0 = 0;

    tapCnt = (srcBLen) >> 1U;

    while (tapCnt > 0U)
    {

      acc0 += (*pScr++ * *pIn2++);
      acc0 += (*pScr++ * *pIn2++);

      /* Decrement the loop counter */
      tapCnt--;
    }

    tapCnt = (srcBLen) & 1U;

    /* apply same above for remaining samples of smaller length sequence */
    while (tapCnt > 0U)
    {

      /* accumlate the results */
      acc0 += (*pScr++ * *pIn2++);

      /* Decrement the loop counter */
      tapCnt--;
    }

    blkCnt--;

    /* Store the result in the accumulator in the destination buffer. */

    *pOut = (q15_t) (__SSAT((acc0 >> 15), 16));

    pOut += inc;

    /* Initialization of inputB pointer */
    pIn2 = py;

    pScratch += 1U;

  }
}

/**
 * @} end of Corr group
 */
