/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_conv_fast_opt_q15.c
 * Description:  Fast Q15 Convolution
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
 * @addtogroup Conv
 * @{
 */

/**
 * @brief Convolution of Q15 sequences (fast version) for Cortex-M3 and Cortex-M4.
 * @param[in] *pSrcA points to the first input sequence.
 * @param[in] srcALen length of the first input sequence.
 * @param[in] *pSrcB points to the second input sequence.
 * @param[in] srcBLen length of the second input sequence.
 * @param[out] *pDst points to the location where the output result is written.  Length srcALen+srcBLen-1.
 * @param[in]  *pScratch1 points to scratch buffer of size max(srcALen, srcBLen) + 2*min(srcALen, srcBLen) - 2.
 * @param[in]  *pScratch2 points to scratch buffer of size min(srcALen, srcBLen).
 * @return none.
 *
 * \par Restrictions
 *  If the silicon does not support unaligned memory access enable the macro UNALIGNED_SUPPORT_DISABLE
 *	In this case input, output, scratch1 and scratch2 buffers should be aligned by 32-bit
 *
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * This fast version uses a 32-bit accumulator with 2.30 format.
 * The accumulator maintains full precision of the intermediate multiplication results
 * but provides only a single guard bit. There is no saturation on intermediate additions.
 * Thus, if the accumulator overflows it wraps around and distorts the result.
 * The input signals should be scaled down to avoid intermediate overflows.
 * Scale down the inputs by log2(min(srcALen, srcBLen)) (log2 is read as log to the base 2) times to avoid overflows,
 * as maximum of min(srcALen, srcBLen) number of additions are carried internally.
 * The 2.30 accumulator is right shifted by 15 bits and then saturated to 1.15 format to yield the final result.
 *
 * \par
 * See <code>arm_conv_q15()</code> for a slower implementation of this function which uses 64-bit accumulation to avoid wrap around distortion.
 */

void arm_conv_fast_opt_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst,
  q15_t * pScratch1,
  q15_t * pScratch2)
{
  q31_t acc0, acc1, acc2, acc3;                  /* Accumulators */
  q31_t x1, x2, x3;                              /* Temporary variables to hold state and coefficient values */
  q31_t y1, y2;                                  /* State variables */
  q15_t *pOut = pDst;                            /* output pointer */
  q15_t *pScr1 = pScratch1;                      /* Temporary pointer for scratch1 */
  q15_t *pScr2 = pScratch2;                      /* Temporary pointer for scratch1 */
  q15_t *pIn1;                                   /* inputA pointer */
  q15_t *pIn2;                                   /* inputB pointer */
  q15_t *px;                                     /* Intermediate inputA pointer  */
  q15_t *py;                                     /* Intermediate inputB pointer  */
  uint32_t j, k, blkCnt;                         /* loop counter */
  uint32_t tapCnt;                               /* loop count */
#ifdef UNALIGNED_SUPPORT_DISABLE

  q15_t a, b;

#endif	/*	#ifdef UNALIGNED_SUPPORT_DISABLE	*/

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

  /* Pointer to take end of scratch2 buffer */
  pScr2 = pScratch2 + srcBLen - 1;

  /* points to smaller length sequence */
  px = pIn2;

  /* Apply loop unrolling and do 4 Copies simultaneously. */
  k = srcBLen >> 2U;

  /* First part of the processing with loop unrolling copies 4 data points at a time.
   ** a second loop below copies for the remaining 1 to 3 samples. */

  /* Copy smaller length input sequence in reverse order into second scratch buffer */
  while (k > 0U)
  {
    /* copy second buffer in reversal manner */
    *pScr2-- = *px++;
    *pScr2-- = *px++;
    *pScr2-- = *px++;
    *pScr2-- = *px++;

    /* Decrement the loop counter */
    k--;
  }

  /* If the count is not a multiple of 4, copy remaining samples here.
   ** No loop unrolling is used. */
  k = srcBLen % 0x4U;

  while (k > 0U)
  {
    /* copy second buffer in reversal manner for remaining samples */
    *pScr2-- = *px++;

    /* Decrement the loop counter */
    k--;
  }

  /* Initialze temporary scratch pointer */
  pScr1 = pScratch1;

  /* Assuming scratch1 buffer is aligned by 32-bit */
  /* Fill (srcBLen - 1U) zeros in scratch1 buffer */
  arm_fill_q15(0, pScr1, (srcBLen - 1U));

  /* Update temporary scratch pointer */
  pScr1 += (srcBLen - 1U);

  /* Copy bigger length sequence(srcALen) samples in scratch1 buffer */

#ifndef UNALIGNED_SUPPORT_DISABLE

  /* Copy (srcALen) samples in scratch buffer */
  arm_copy_q15(pIn1, pScr1, srcALen);

  /* Update pointers */
  pScr1 += srcALen;

#else

  /* Apply loop unrolling and do 4 Copies simultaneously. */
  k = srcALen >> 2U;

  /* First part of the processing with loop unrolling copies 4 data points at a time.
   ** a second loop below copies for the remaining 1 to 3 samples. */
  while (k > 0U)
  {
    /* copy second buffer in reversal manner */
    *pScr1++ = *pIn1++;
    *pScr1++ = *pIn1++;
    *pScr1++ = *pIn1++;
    *pScr1++ = *pIn1++;

    /* Decrement the loop counter */
    k--;
  }

  /* If the count is not a multiple of 4, copy remaining samples here.
   ** No loop unrolling is used. */
  k = srcALen % 0x4U;

  while (k > 0U)
  {
    /* copy second buffer in reversal manner for remaining samples */
    *pScr1++ = *pIn1++;

    /* Decrement the loop counter */
    k--;
  }

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE	*/


#ifndef UNALIGNED_SUPPORT_DISABLE

  /* Fill (srcBLen - 1U) zeros at end of scratch buffer */
  arm_fill_q15(0, pScr1, (srcBLen - 1U));

  /* Update pointer */
  pScr1 += (srcBLen - 1U);

#else

  /* Apply loop unrolling and do 4 Copies simultaneously. */
  k = (srcBLen - 1U) >> 2U;

  /* First part of the processing with loop unrolling copies 4 data points at a time.
   ** a second loop below copies for the remaining 1 to 3 samples. */
  while (k > 0U)
  {
    /* copy second buffer in reversal manner */
    *pScr1++ = 0;
    *pScr1++ = 0;
    *pScr1++ = 0;
    *pScr1++ = 0;

    /* Decrement the loop counter */
    k--;
  }

  /* If the count is not a multiple of 4, copy remaining samples here.
   ** No loop unrolling is used. */
  k = (srcBLen - 1U) % 0x4U;

  while (k > 0U)
  {
    /* copy second buffer in reversal manner for remaining samples */
    *pScr1++ = 0;

    /* Decrement the loop counter */
    k--;
  }

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE	*/

  /* Temporary pointer for scratch2 */
  py = pScratch2;


  /* Initialization of pIn2 pointer */
  pIn2 = py;

  /* First part of the processing with loop unrolling process 4 data points at a time.
   ** a second loop below process for the remaining 1 to 3 samples. */

  /* Actual convolution process starts here */
  blkCnt = (srcALen + srcBLen - 1U) >> 2;

  while (blkCnt > 0)
  {
    /* Initialze temporary scratch pointer as scratch1 */
    pScr1 = pScratch1;

    /* Clear Accumlators */
    acc0 = 0;
    acc1 = 0;
    acc2 = 0;
    acc3 = 0;

    /* Read two samples from scratch1 buffer */
    x1 = *__SIMD32(pScr1)++;

    /* Read next two samples from scratch1 buffer */
    x2 = *__SIMD32(pScr1)++;

    tapCnt = (srcBLen) >> 2U;

    while (tapCnt > 0U)
    {

#ifndef UNALIGNED_SUPPORT_DISABLE

      /* Read four samples from smaller buffer */
      y1 = _SIMD32_OFFSET(pIn2);
      y2 = _SIMD32_OFFSET(pIn2 + 2U);

      /* multiply and accumlate */
      acc0 = __SMLAD(x1, y1, acc0);
      acc2 = __SMLAD(x2, y1, acc2);

      /* pack input data */
#ifndef ARM_MATH_BIG_ENDIAN
      x3 = __PKHBT(x2, x1, 0);
#else
      x3 = __PKHBT(x1, x2, 0);
#endif

      /* multiply and accumlate */
      acc1 = __SMLADX(x3, y1, acc1);

      /* Read next two samples from scratch1 buffer */
      x1 = _SIMD32_OFFSET(pScr1);

      /* multiply and accumlate */
      acc0 = __SMLAD(x2, y2, acc0);
      acc2 = __SMLAD(x1, y2, acc2);

      /* pack input data */
#ifndef ARM_MATH_BIG_ENDIAN
      x3 = __PKHBT(x1, x2, 0);
#else
      x3 = __PKHBT(x2, x1, 0);
#endif

      acc3 = __SMLADX(x3, y1, acc3);
      acc1 = __SMLADX(x3, y2, acc1);

      x2 = _SIMD32_OFFSET(pScr1 + 2U);

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

	  a = *pScr1;
	  b = *(pScr1 + 1);

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

	  a = *(pScr1 + 2);
	  b = *(pScr1 + 3);

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

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE	*/

      /* update scratch pointers */
      pIn2 += 4U;
      pScr1 += 4U;


      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Update scratch pointer for remaining samples of smaller length sequence */
    pScr1 -= 4U;

    /* apply same above for remaining samples of smaller length sequence */
    tapCnt = (srcBLen) & 3U;

    while (tapCnt > 0U)
    {

      /* accumlate the results */
      acc0 += (*pScr1++ * *pIn2);
      acc1 += (*pScr1++ * *pIn2);
      acc2 += (*pScr1++ * *pIn2);
      acc3 += (*pScr1++ * *pIn2++);

      pScr1 -= 3U;

      /* Decrement the loop counter */
      tapCnt--;
    }

    blkCnt--;


    /* Store the results in the accumulators in the destination buffer. */

#ifndef ARM_MATH_BIG_ENDIAN

    *__SIMD32(pOut)++ =
      __PKHBT(__SSAT((acc0 >> 15), 16), __SSAT((acc1 >> 15), 16), 16);

    *__SIMD32(pOut)++ =
      __PKHBT(__SSAT((acc2 >> 15), 16), __SSAT((acc3 >> 15), 16), 16);


#else

    *__SIMD32(pOut)++ =
      __PKHBT(__SSAT((acc1 >> 15), 16), __SSAT((acc0 >> 15), 16), 16);

    *__SIMD32(pOut)++ =
      __PKHBT(__SSAT((acc3 >> 15), 16), __SSAT((acc2 >> 15), 16), 16);



#endif /*      #ifndef ARM_MATH_BIG_ENDIAN       */

    /* Initialization of inputB pointer */
    pIn2 = py;

    pScratch1 += 4U;

  }


  blkCnt = (srcALen + srcBLen - 1U) & 0x3;

  /* Calculate convolution for remaining samples of Bigger length sequence */
  while (blkCnt > 0)
  {
    /* Initialze temporary scratch pointer as scratch1 */
    pScr1 = pScratch1;

    /* Clear Accumlators */
    acc0 = 0;

    tapCnt = (srcBLen) >> 1U;

    while (tapCnt > 0U)
    {

      acc0 += (*pScr1++ * *pIn2++);
      acc0 += (*pScr1++ * *pIn2++);

      /* Decrement the loop counter */
      tapCnt--;
    }

    tapCnt = (srcBLen) & 1U;

    /* apply same above for remaining samples of smaller length sequence */
    while (tapCnt > 0U)
    {

      /* accumlate the results */
      acc0 += (*pScr1++ * *pIn2++);

      /* Decrement the loop counter */
      tapCnt--;
    }

    blkCnt--;

    /* The result is in 2.30 format.  Convert to 1.15 with saturation.
     ** Then store the output in the destination buffer. */
    *pOut++ = (q15_t) (__SSAT((acc0 >> 15), 16));

    /* Initialization of inputB pointer */
    pIn2 = py;

    pScratch1 += 1U;

  }

}

/**
 * @} end of Conv group
 */
