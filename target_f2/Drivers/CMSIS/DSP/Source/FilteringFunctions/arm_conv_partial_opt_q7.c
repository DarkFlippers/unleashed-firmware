/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_conv_partial_opt_q7.c
 * Description:  Partial convolution of Q7 sequences
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
 * @brief Partial convolution of Q7 sequences.
 * @param[in]       *pSrcA points to the first input sequence.
 * @param[in]       srcALen length of the first input sequence.
 * @param[in]       *pSrcB points to the second input sequence.
 * @param[in]       srcBLen length of the second input sequence.
 * @param[out]      *pDst points to the location where the output result is written.
 * @param[in]       firstIndex is the first output sample to start with.
 * @param[in]       numPoints is the number of output points to be computed.
 * @param[in]      *pScratch1 points to scratch buffer(of type q15_t) of size max(srcALen, srcBLen) + 2*min(srcALen, srcBLen) - 2.
 * @param[in]      *pScratch2 points to scratch buffer (of type q15_t) of size min(srcALen, srcBLen).
 * @return  Returns either ARM_MATH_SUCCESS if the function completed correctly or ARM_MATH_ARGUMENT_ERROR if the requested subset is not in the range [0 srcALen+srcBLen-2].
 *
 * \par Restrictions
 *  If the silicon does not support unaligned memory access enable the macro UNALIGNED_SUPPORT_DISABLE
 *	In this case input, output, scratch1 and scratch2 buffers should be aligned by 32-bit
 *
 *
 *
 */


#ifndef UNALIGNED_SUPPORT_DISABLE

arm_status arm_conv_partial_opt_q7(
  q7_t * pSrcA,
  uint32_t srcALen,
  q7_t * pSrcB,
  uint32_t srcBLen,
  q7_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints,
  q15_t * pScratch1,
  q15_t * pScratch2)
{

  q15_t *pScr2, *pScr1;                          /* Intermediate pointers for scratch pointers */
  q15_t x4;                                      /* Temporary input variable */
  q7_t *pIn1, *pIn2;                             /* inputA and inputB pointer */
  uint32_t j, k, blkCnt, tapCnt;                 /* loop counter */
  q7_t *px;                                      /* Temporary input1 pointer */
  q15_t *py;                                     /* Temporary input2 pointer */
  q31_t acc0, acc1, acc2, acc3;                  /* Accumulator */
  q31_t x1, x2, x3, y1;                          /* Temporary input variables */
  arm_status status;
  q7_t *pOut = pDst;                             /* output pointer */
  q7_t out0, out1, out2, out3;                   /* temporary variables */

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

    /* pointer to take end of scratch2 buffer */
    pScr2 = pScratch2;

    /* points to smaller length sequence */
    px = pIn2 + srcBLen - 1;

    /* Apply loop unrolling and do 4 Copies simultaneously. */
    k = srcBLen >> 2U;

    /* First part of the processing with loop unrolling copies 4 data points at a time.
     ** a second loop below copies for the remaining 1 to 3 samples. */
    while (k > 0U)
    {
      /* copy second buffer in reversal manner */
      x4 = (q15_t) * px--;
      *pScr2++ = x4;
      x4 = (q15_t) * px--;
      *pScr2++ = x4;
      x4 = (q15_t) * px--;
      *pScr2++ = x4;
      x4 = (q15_t) * px--;
      *pScr2++ = x4;

      /* Decrement the loop counter */
      k--;
    }

    /* If the count is not a multiple of 4, copy remaining samples here.
     ** No loop unrolling is used. */
    k = srcBLen % 0x4U;

    while (k > 0U)
    {
      /* copy second buffer in reversal manner for remaining samples */
      x4 = (q15_t) * px--;
      *pScr2++ = x4;

      /* Decrement the loop counter */
      k--;
    }

    /* Initialze temporary scratch pointer */
    pScr1 = pScratch1;

    /* Fill (srcBLen - 1U) zeros in scratch buffer */
    arm_fill_q15(0, pScr1, (srcBLen - 1U));

    /* Update temporary scratch pointer */
    pScr1 += (srcBLen - 1U);

    /* Copy (srcALen) samples in scratch buffer */
    /* Apply loop unrolling and do 4 Copies simultaneously. */
    k = srcALen >> 2U;

    /* First part of the processing with loop unrolling copies 4 data points at a time.
     ** a second loop below copies for the remaining 1 to 3 samples. */
    while (k > 0U)
    {
      /* copy second buffer in reversal manner */
      x4 = (q15_t) * pIn1++;
      *pScr1++ = x4;
      x4 = (q15_t) * pIn1++;
      *pScr1++ = x4;
      x4 = (q15_t) * pIn1++;
      *pScr1++ = x4;
      x4 = (q15_t) * pIn1++;
      *pScr1++ = x4;

      /* Decrement the loop counter */
      k--;
    }

    /* If the count is not a multiple of 4, copy remaining samples here.
     ** No loop unrolling is used. */
    k = srcALen % 0x4U;

    while (k > 0U)
    {
      /* copy second buffer in reversal manner for remaining samples */
      x4 = (q15_t) * pIn1++;
      *pScr1++ = x4;

      /* Decrement the loop counter */
      k--;
    }

    /* Fill (srcBLen - 1U) zeros at end of scratch buffer */
    arm_fill_q15(0, pScr1, (srcBLen - 1U));

    /* Update pointer */
    pScr1 += (srcBLen - 1U);


    /* Temporary pointer for scratch2 */
    py = pScratch2;

    /* Initialization of pIn2 pointer */
    pIn2 = (q7_t *) py;

    pScr2 = py;

    pOut = pDst + firstIndex;

    pScratch1 += firstIndex;

    /* Actual convolution process starts here */
    blkCnt = (numPoints) >> 2;


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

        /* Read four samples from smaller buffer */
        y1 = _SIMD32_OFFSET(pScr2);

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
        x1 = *__SIMD32(pScr1)++;

        /* pack input data */
#ifndef ARM_MATH_BIG_ENDIAN
        x3 = __PKHBT(x1, x2, 0);
#else
        x3 = __PKHBT(x2, x1, 0);
#endif

        acc3 = __SMLADX(x3, y1, acc3);

        /* Read four samples from smaller buffer */
        y1 = _SIMD32_OFFSET(pScr2 + 2U);

        acc0 = __SMLAD(x2, y1, acc0);

        acc2 = __SMLAD(x1, y1, acc2);

        acc1 = __SMLADX(x3, y1, acc1);

        x2 = *__SIMD32(pScr1)++;

#ifndef ARM_MATH_BIG_ENDIAN
        x3 = __PKHBT(x2, x1, 0);
#else
        x3 = __PKHBT(x1, x2, 0);
#endif

        acc3 = __SMLADX(x3, y1, acc3);

        pScr2 += 4U;


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
        acc0 += (*pScr1++ * *pScr2);
        acc1 += (*pScr1++ * *pScr2);
        acc2 += (*pScr1++ * *pScr2);
        acc3 += (*pScr1++ * *pScr2++);

        pScr1 -= 3U;

        /* Decrement the loop counter */
        tapCnt--;
      }

      blkCnt--;

      /* Store the result in the accumulator in the destination buffer. */
      out0 = (q7_t) (__SSAT(acc0 >> 7U, 8));
      out1 = (q7_t) (__SSAT(acc1 >> 7U, 8));
      out2 = (q7_t) (__SSAT(acc2 >> 7U, 8));
      out3 = (q7_t) (__SSAT(acc3 >> 7U, 8));

      *__SIMD32(pOut)++ = __PACKq7(out0, out1, out2, out3);

      /* Initialization of inputB pointer */
      pScr2 = py;

      pScratch1 += 4U;

    }

    blkCnt = (numPoints) & 0x3;

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

        /* Read next two samples from scratch1 buffer */
        x1 = *__SIMD32(pScr1)++;

        /* Read two samples from smaller buffer */
        y1 = *__SIMD32(pScr2)++;

        acc0 = __SMLAD(x1, y1, acc0);

        /* Decrement the loop counter */
        tapCnt--;
      }

      tapCnt = (srcBLen) & 1U;

      /* apply same above for remaining samples of smaller length sequence */
      while (tapCnt > 0U)
      {

        /* accumlate the results */
        acc0 += (*pScr1++ * *pScr2++);

        /* Decrement the loop counter */
        tapCnt--;
      }

      blkCnt--;

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = (q7_t) (__SSAT(acc0 >> 7U, 8));

      /* Initialization of inputB pointer */
      pScr2 = py;

      pScratch1 += 1U;

    }

    /* set status as ARM_MATH_SUCCESS */
    status = ARM_MATH_SUCCESS;


  }

  return (status);

}

#else

arm_status arm_conv_partial_opt_q7(
  q7_t * pSrcA,
  uint32_t srcALen,
  q7_t * pSrcB,
  uint32_t srcBLen,
  q7_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints,
  q15_t * pScratch1,
  q15_t * pScratch2)
{

  q15_t *pScr2, *pScr1;                          /* Intermediate pointers for scratch pointers */
  q15_t x4;                                      /* Temporary input variable */
  q7_t *pIn1, *pIn2;                             /* inputA and inputB pointer */
  uint32_t j, k, blkCnt, tapCnt;                 /* loop counter */
  q7_t *px;                                      /* Temporary input1 pointer */
  q15_t *py;                                     /* Temporary input2 pointer */
  q31_t acc0, acc1, acc2, acc3;                  /* Accumulator */
  arm_status status;
  q7_t *pOut = pDst;                             /* output pointer */
  q15_t x10, x11, x20, x21;                      /* Temporary input variables */
  q15_t y10, y11;                                /* Temporary input variables */

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

    /* pointer to take end of scratch2 buffer */
    pScr2 = pScratch2;

    /* points to smaller length sequence */
    px = pIn2 + srcBLen - 1;

    /* Apply loop unrolling and do 4 Copies simultaneously. */
    k = srcBLen >> 2U;

    /* First part of the processing with loop unrolling copies 4 data points at a time.
     ** a second loop below copies for the remaining 1 to 3 samples. */
    while (k > 0U)
    {
      /* copy second buffer in reversal manner */
      x4 = (q15_t) * px--;
      *pScr2++ = x4;
      x4 = (q15_t) * px--;
      *pScr2++ = x4;
      x4 = (q15_t) * px--;
      *pScr2++ = x4;
      x4 = (q15_t) * px--;
      *pScr2++ = x4;

      /* Decrement the loop counter */
      k--;
    }

    /* If the count is not a multiple of 4, copy remaining samples here.
     ** No loop unrolling is used. */
    k = srcBLen % 0x4U;

    while (k > 0U)
    {
      /* copy second buffer in reversal manner for remaining samples */
      x4 = (q15_t) * px--;
      *pScr2++ = x4;

      /* Decrement the loop counter */
      k--;
    }

    /* Initialze temporary scratch pointer */
    pScr1 = pScratch1;

    /* Fill (srcBLen - 1U) zeros in scratch buffer */
    arm_fill_q15(0, pScr1, (srcBLen - 1U));

    /* Update temporary scratch pointer */
    pScr1 += (srcBLen - 1U);

    /* Copy (srcALen) samples in scratch buffer */
    /* Apply loop unrolling and do 4 Copies simultaneously. */
    k = srcALen >> 2U;

    /* First part of the processing with loop unrolling copies 4 data points at a time.
     ** a second loop below copies for the remaining 1 to 3 samples. */
    while (k > 0U)
    {
      /* copy second buffer in reversal manner */
      x4 = (q15_t) * pIn1++;
      *pScr1++ = x4;
      x4 = (q15_t) * pIn1++;
      *pScr1++ = x4;
      x4 = (q15_t) * pIn1++;
      *pScr1++ = x4;
      x4 = (q15_t) * pIn1++;
      *pScr1++ = x4;

      /* Decrement the loop counter */
      k--;
    }

    /* If the count is not a multiple of 4, copy remaining samples here.
     ** No loop unrolling is used. */
    k = srcALen % 0x4U;

    while (k > 0U)
    {
      /* copy second buffer in reversal manner for remaining samples */
      x4 = (q15_t) * pIn1++;
      *pScr1++ = x4;

      /* Decrement the loop counter */
      k--;
    }

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


    /* Temporary pointer for scratch2 */
    py = pScratch2;

    /* Initialization of pIn2 pointer */
    pIn2 = (q7_t *) py;

    pScr2 = py;

    pOut = pDst + firstIndex;

    pScratch1 += firstIndex;

    /* Actual convolution process starts here */
    blkCnt = (numPoints) >> 2;


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
      x10 = *pScr1++;
      x11 = *pScr1++;

      /* Read next two samples from scratch1 buffer */
      x20 = *pScr1++;
      x21 = *pScr1++;

      tapCnt = (srcBLen) >> 2U;

      while (tapCnt > 0U)
      {

        /* Read four samples from smaller buffer */
        y10 = *pScr2;
        y11 = *(pScr2 + 1U);

        /* multiply and accumlate */
        acc0 += (q31_t) x10 *y10;
        acc0 += (q31_t) x11 *y11;
        acc2 += (q31_t) x20 *y10;
        acc2 += (q31_t) x21 *y11;


        acc1 += (q31_t) x11 *y10;
        acc1 += (q31_t) x20 *y11;

        /* Read next two samples from scratch1 buffer */
        x10 = *pScr1;
        x11 = *(pScr1 + 1U);

        /* multiply and accumlate */
        acc3 += (q31_t) x21 *y10;
        acc3 += (q31_t) x10 *y11;

        /* Read next two samples from scratch2 buffer */
        y10 = *(pScr2 + 2U);
        y11 = *(pScr2 + 3U);

        /* multiply and accumlate */
        acc0 += (q31_t) x20 *y10;
        acc0 += (q31_t) x21 *y11;
        acc2 += (q31_t) x10 *y10;
        acc2 += (q31_t) x11 *y11;
        acc1 += (q31_t) x21 *y10;
        acc1 += (q31_t) x10 *y11;

        /* Read next two samples from scratch1 buffer */
        x20 = *(pScr1 + 2);
        x21 = *(pScr1 + 3);

        /* multiply and accumlate */
        acc3 += (q31_t) x11 *y10;
        acc3 += (q31_t) x20 *y11;

        /* update scratch pointers */

        pScr1 += 4U;
        pScr2 += 4U;

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
        acc0 += (*pScr1++ * *pScr2);
        acc1 += (*pScr1++ * *pScr2);
        acc2 += (*pScr1++ * *pScr2);
        acc3 += (*pScr1++ * *pScr2++);

        pScr1 -= 3U;

        /* Decrement the loop counter */
        tapCnt--;
      }

      blkCnt--;

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = (q7_t) (__SSAT(acc0 >> 7U, 8));
      *pOut++ = (q7_t) (__SSAT(acc1 >> 7U, 8));
      *pOut++ = (q7_t) (__SSAT(acc2 >> 7U, 8));
      *pOut++ = (q7_t) (__SSAT(acc3 >> 7U, 8));

      /* Initialization of inputB pointer */
      pScr2 = py;

      pScratch1 += 4U;

    }

    blkCnt = (numPoints) & 0x3;

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

        /* Read next two samples from scratch1 buffer */
        x10 = *pScr1++;
        x11 = *pScr1++;

        /* Read two samples from smaller buffer */
        y10 = *pScr2++;
        y11 = *pScr2++;

        /* multiply and accumlate */
        acc0 += (q31_t) x10 *y10;
        acc0 += (q31_t) x11 *y11;

        /* Decrement the loop counter */
        tapCnt--;
      }

      tapCnt = (srcBLen) & 1U;

      /* apply same above for remaining samples of smaller length sequence */
      while (tapCnt > 0U)
      {

        /* accumlate the results */
        acc0 += (*pScr1++ * *pScr2++);

        /* Decrement the loop counter */
        tapCnt--;
      }

      blkCnt--;

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = (q7_t) (__SSAT(acc0 >> 7U, 8));

      /* Initialization of inputB pointer */
      pScr2 = py;

      pScratch1 += 1U;

    }

    /* set status as ARM_MATH_SUCCESS */
    status = ARM_MATH_SUCCESS;

  }

  return (status);

}

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE	*/



/**
 * @} end of PartialConv group
 */
