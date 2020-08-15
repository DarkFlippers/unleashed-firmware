/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fir_q15.c
 * Description:  Q15 FIR filter processing function
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
 * @addtogroup FIR
 * @{
 */

/**
 * @brief Processing function for the Q15 FIR filter.
 * @param[in] *S points to an instance of the Q15 FIR structure.
 * @param[in] *pSrc points to the block of input data.
 * @param[out] *pDst points to the block of output data.
 * @param[in]  blockSize number of samples to process per call.
 * @return none.
 *
 *
 * \par Restrictions
 *  If the silicon does not support unaligned memory access enable the macro UNALIGNED_SUPPORT_DISABLE
 *	In this case input, output, state buffers should be aligned by 32-bit
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using a 64-bit internal accumulator.
 * Both coefficients and state variables are represented in 1.15 format and multiplications yield a 2.30 result.
 * The 2.30 intermediate results are accumulated in a 64-bit accumulator in 34.30 format.
 * There is no risk of internal overflow with this approach and the full precision of intermediate multiplications is preserved.
 * After all additions have been performed, the accumulator is truncated to 34.15 format by discarding low 15 bits.
 * Lastly, the accumulator is saturated to yield a result in 1.15 format.
 *
 * \par
 * Refer to the function <code>arm_fir_fast_q15()</code> for a faster but less precise implementation of this function.
 */

#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */

#ifndef UNALIGNED_SUPPORT_DISABLE


void arm_fir_q15(
  const arm_fir_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state */
  q15_t *px1;                                    /* Temporary q15 pointer for state buffer */
  q15_t *pb;                                     /* Temporary pointer for coefficient buffer */
  q31_t x0, x1, x2, x3, c0;                      /* Temporary variables to hold SIMD state and coefficient values */
  q63_t acc0, acc1, acc2, acc3;                  /* Accumulators */
  uint32_t numTaps = S->numTaps;                 /* Number of taps in the filter */
  uint32_t tapCnt, blkCnt;                       /* Loop counters */


  /* S->pState points to state array which contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = &(S->pState[(numTaps - 1U)]);

  /* Apply loop unrolling and compute 4 output values simultaneously.
   * The variables acc0 ... acc3 hold output values that are being computed:
   *
   *    acc0 =  b[numTaps-1] * x[n-numTaps-1] + b[numTaps-2] * x[n-numTaps-2] + b[numTaps-3] * x[n-numTaps-3] +...+ b[0] * x[0]
   *    acc1 =  b[numTaps-1] * x[n-numTaps] +   b[numTaps-2] * x[n-numTaps-1] + b[numTaps-3] * x[n-numTaps-2] +...+ b[0] * x[1]
   *    acc2 =  b[numTaps-1] * x[n-numTaps+1] + b[numTaps-2] * x[n-numTaps] +   b[numTaps-3] * x[n-numTaps-1] +...+ b[0] * x[2]
   *    acc3 =  b[numTaps-1] * x[n-numTaps+2] + b[numTaps-2] * x[n-numTaps+1] + b[numTaps-3] * x[n-numTaps]   +...+ b[0] * x[3]
   */

  blkCnt = blockSize >> 2;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* Copy four new input samples into the state buffer.
     ** Use 32-bit SIMD to move the 16-bit data.  Only requires two copies. */
    *__SIMD32(pStateCurnt)++ = *__SIMD32(pSrc)++;
    *__SIMD32(pStateCurnt)++ = *__SIMD32(pSrc)++;

    /* Set all accumulators to zero */
    acc0 = 0;
    acc1 = 0;
    acc2 = 0;
    acc3 = 0;

    /* Initialize state pointer of type q15 */
    px1 = pState;

    /* Initialize coeff pointer of type q31 */
    pb = pCoeffs;

    /* Read the first two samples from the state buffer:  x[n-N], x[n-N-1] */
    x0 = _SIMD32_OFFSET(px1);

    /* Read the third and forth samples from the state buffer: x[n-N-1], x[n-N-2] */
    x1 = _SIMD32_OFFSET(px1 + 1U);

    px1 += 2U;

    /* Loop over the number of taps.  Unroll by a factor of 4.
     ** Repeat until we've computed numTaps-4 coefficients. */
    tapCnt = numTaps >> 2;

    while (tapCnt > 0U)
    {
      /* Read the first two coefficients using SIMD:  b[N] and b[N-1] coefficients */
      c0 = *__SIMD32(pb)++;

      /* acc0 +=  b[N] * x[n-N] + b[N-1] * x[n-N-1] */
      acc0 = __SMLALD(x0, c0, acc0);

      /* acc1 +=  b[N] * x[n-N-1] + b[N-1] * x[n-N-2] */
      acc1 = __SMLALD(x1, c0, acc1);

      /* Read state x[n-N-2], x[n-N-3] */
      x2 = _SIMD32_OFFSET(px1);

      /* Read state x[n-N-3], x[n-N-4] */
      x3 = _SIMD32_OFFSET(px1 + 1U);

      /* acc2 +=  b[N] * x[n-N-2] + b[N-1] * x[n-N-3] */
      acc2 = __SMLALD(x2, c0, acc2);

      /* acc3 +=  b[N] * x[n-N-3] + b[N-1] * x[n-N-4] */
      acc3 = __SMLALD(x3, c0, acc3);

      /* Read coefficients b[N-2], b[N-3] */
      c0 = *__SIMD32(pb)++;

      /* acc0 +=  b[N-2] * x[n-N-2] + b[N-3] * x[n-N-3] */
      acc0 = __SMLALD(x2, c0, acc0);

      /* acc1 +=  b[N-2] * x[n-N-3] + b[N-3] * x[n-N-4] */
      acc1 = __SMLALD(x3, c0, acc1);

      /* Read state x[n-N-4], x[n-N-5] */
      x0 = _SIMD32_OFFSET(px1 + 2U);

      /* Read state x[n-N-5], x[n-N-6] */
      x1 = _SIMD32_OFFSET(px1 + 3U);

      /* acc2 +=  b[N-2] * x[n-N-4] + b[N-3] * x[n-N-5] */
      acc2 = __SMLALD(x0, c0, acc2);

      /* acc3 +=  b[N-2] * x[n-N-5] + b[N-3] * x[n-N-6] */
      acc3 = __SMLALD(x1, c0, acc3);

      px1 += 4U;

      tapCnt--;

    }


    /* If the filter length is not a multiple of 4, compute the remaining filter taps.
     ** This is always be 2 taps since the filter length is even. */
    if ((numTaps & 0x3U) != 0U)
    {
      /* Read 2 coefficients */
      c0 = *__SIMD32(pb)++;

      /* Fetch 4 state variables */
      x2 = _SIMD32_OFFSET(px1);

      x3 = _SIMD32_OFFSET(px1 + 1U);

      /* Perform the multiply-accumulates */
      acc0 = __SMLALD(x0, c0, acc0);

      px1 += 2U;

      acc1 = __SMLALD(x1, c0, acc1);
      acc2 = __SMLALD(x2, c0, acc2);
      acc3 = __SMLALD(x3, c0, acc3);
    }

    /* The results in the 4 accumulators are in 2.30 format.  Convert to 1.15 with saturation.
     ** Then store the 4 outputs in the destination buffer. */

#ifndef ARM_MATH_BIG_ENDIAN

    *__SIMD32(pDst)++ =
      __PKHBT(__SSAT((acc0 >> 15), 16), __SSAT((acc1 >> 15), 16), 16);
    *__SIMD32(pDst)++ =
      __PKHBT(__SSAT((acc2 >> 15), 16), __SSAT((acc3 >> 15), 16), 16);

#else

    *__SIMD32(pDst)++ =
      __PKHBT(__SSAT((acc1 >> 15), 16), __SSAT((acc0 >> 15), 16), 16);
    *__SIMD32(pDst)++ =
      __PKHBT(__SSAT((acc3 >> 15), 16), __SSAT((acc2 >> 15), 16), 16);

#endif /*      #ifndef ARM_MATH_BIG_ENDIAN       */



    /* Advance the state pointer by 4 to process the next group of 4 samples */
    pState = pState + 4;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;
  while (blkCnt > 0U)
  {
    /* Copy two samples into state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Set the accumulator to zero */
    acc0 = 0;

    /* Initialize state pointer of type q15 */
    px1 = pState;

    /* Initialize coeff pointer of type q31 */
    pb = pCoeffs;

    tapCnt = numTaps >> 1;

    do
    {

      c0 = *__SIMD32(pb)++;
      x0 = *__SIMD32(px1)++;

      acc0 = __SMLALD(x0, c0, acc0);
      tapCnt--;
    }
    while (tapCnt > 0U);

    /* The result is in 2.30 format.  Convert to 1.15 with saturation.
     ** Then store the output in the destination buffer. */
    *pDst++ = (q15_t) (__SSAT((acc0 >> 15), 16));

    /* Advance state pointer by 1 for the next sample */
    pState = pState + 1;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Processing is complete.
   ** Now copy the last numTaps - 1 samples to the satrt of the state buffer.
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  /* Calculation of count for copying integer writes */
  tapCnt = (numTaps - 1U) >> 2;

  while (tapCnt > 0U)
  {

    /* Copy state values to start of state buffer */
    *__SIMD32(pStateCurnt)++ = *__SIMD32(pState)++;
    *__SIMD32(pStateCurnt)++ = *__SIMD32(pState)++;

    tapCnt--;

  }

  /* Calculation of count for remaining q15_t data */
  tapCnt = (numTaps - 1U) % 0x4U;

  /* copy remaining data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }
}

#else /* UNALIGNED_SUPPORT_DISABLE */

void arm_fir_q15(
  const arm_fir_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state */
  q63_t acc0, acc1, acc2, acc3;                  /* Accumulators */
  q15_t *pb;                                     /* Temporary pointer for coefficient buffer */
  q15_t *px;                                     /* Temporary q31 pointer for SIMD state buffer accesses */
  q31_t x0, x1, x2, c0;                          /* Temporary variables to hold SIMD state and coefficient values */
  uint32_t numTaps = S->numTaps;                 /* Number of taps in the filter */
  uint32_t tapCnt, blkCnt;                       /* Loop counters */


  /* S->pState points to state array which contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = &(S->pState[(numTaps - 1U)]);

  /* Apply loop unrolling and compute 4 output values simultaneously.
   * The variables acc0 ... acc3 hold output values that are being computed:
   *
   *    acc0 =  b[numTaps-1] * x[n-numTaps-1] + b[numTaps-2] * x[n-numTaps-2] + b[numTaps-3] * x[n-numTaps-3] +...+ b[0] * x[0]
   *    acc1 =  b[numTaps-1] * x[n-numTaps] +   b[numTaps-2] * x[n-numTaps-1] + b[numTaps-3] * x[n-numTaps-2] +...+ b[0] * x[1]
   *    acc2 =  b[numTaps-1] * x[n-numTaps+1] + b[numTaps-2] * x[n-numTaps] +   b[numTaps-3] * x[n-numTaps-1] +...+ b[0] * x[2]
   *    acc3 =  b[numTaps-1] * x[n-numTaps+2] + b[numTaps-2] * x[n-numTaps+1] + b[numTaps-3] * x[n-numTaps]   +...+ b[0] * x[3]
   */

  blkCnt = blockSize >> 2;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* Copy four new input samples into the state buffer.
     ** Use 32-bit SIMD to move the 16-bit data.  Only requires two copies. */
    *pStateCurnt++ = *pSrc++;
    *pStateCurnt++ = *pSrc++;
    *pStateCurnt++ = *pSrc++;
    *pStateCurnt++ = *pSrc++;


    /* Set all accumulators to zero */
    acc0 = 0;
    acc1 = 0;
    acc2 = 0;
    acc3 = 0;

    /* Typecast q15_t pointer to q31_t pointer for state reading in q31_t */
    px = pState;

    /* Typecast q15_t pointer to q31_t pointer for coefficient reading in q31_t */
    pb = pCoeffs;

    /* Read the first two samples from the state buffer:  x[n-N], x[n-N-1] */
    x0 = *__SIMD32(px)++;

    /* Read the third and forth samples from the state buffer: x[n-N-2], x[n-N-3] */
    x2 = *__SIMD32(px)++;

    /* Loop over the number of taps.  Unroll by a factor of 4.
     ** Repeat until we've computed numTaps-(numTaps%4) coefficients. */
    tapCnt = numTaps >> 2;

    while (tapCnt > 0)
    {
      /* Read the first two coefficients using SIMD:  b[N] and b[N-1] coefficients */
      c0 = *__SIMD32(pb)++;

      /* acc0 +=  b[N] * x[n-N] + b[N-1] * x[n-N-1] */
      acc0 = __SMLALD(x0, c0, acc0);

      /* acc2 +=  b[N] * x[n-N-2] + b[N-1] * x[n-N-3] */
      acc2 = __SMLALD(x2, c0, acc2);

      /* pack  x[n-N-1] and x[n-N-2] */
#ifndef ARM_MATH_BIG_ENDIAN
      x1 = __PKHBT(x2, x0, 0);
#else
      x1 = __PKHBT(x0, x2, 0);
#endif

      /* Read state x[n-N-4], x[n-N-5] */
      x0 = _SIMD32_OFFSET(px);

      /* acc1 +=  b[N] * x[n-N-1] + b[N-1] * x[n-N-2] */
      acc1 = __SMLALDX(x1, c0, acc1);

      /* pack  x[n-N-3] and x[n-N-4] */
#ifndef ARM_MATH_BIG_ENDIAN
      x1 = __PKHBT(x0, x2, 0);
#else
      x1 = __PKHBT(x2, x0, 0);
#endif

      /* acc3 +=  b[N] * x[n-N-3] + b[N-1] * x[n-N-4] */
      acc3 = __SMLALDX(x1, c0, acc3);

      /* Read coefficients b[N-2], b[N-3] */
      c0 = *__SIMD32(pb)++;

      /* acc0 +=  b[N-2] * x[n-N-2] + b[N-3] * x[n-N-3] */
      acc0 = __SMLALD(x2, c0, acc0);

      /* Read state x[n-N-6], x[n-N-7] with offset */
      x2 = _SIMD32_OFFSET(px + 2U);

      /* acc2 +=  b[N-2] * x[n-N-4] + b[N-3] * x[n-N-5] */
      acc2 = __SMLALD(x0, c0, acc2);

      /* acc1 +=  b[N-2] * x[n-N-3] + b[N-3] * x[n-N-4] */
      acc1 = __SMLALDX(x1, c0, acc1);

      /* pack  x[n-N-5] and x[n-N-6] */
#ifndef ARM_MATH_BIG_ENDIAN
      x1 = __PKHBT(x2, x0, 0);
#else
      x1 = __PKHBT(x0, x2, 0);
#endif

      /* acc3 +=  b[N-2] * x[n-N-5] + b[N-3] * x[n-N-6] */
      acc3 = __SMLALDX(x1, c0, acc3);

      /* Update state pointer for next state reading */
      px += 4U;

      /* Decrement tap count */
      tapCnt--;

    }

    /* If the filter length is not a multiple of 4, compute the remaining filter taps.
     ** This is always be 2 taps since the filter length is even. */
    if ((numTaps & 0x3U) != 0U)
    {

      /* Read last two coefficients */
      c0 = *__SIMD32(pb)++;

      /* Perform the multiply-accumulates */
      acc0 = __SMLALD(x0, c0, acc0);
      acc2 = __SMLALD(x2, c0, acc2);

      /* pack state variables */
#ifndef ARM_MATH_BIG_ENDIAN
      x1 = __PKHBT(x2, x0, 0);
#else
      x1 = __PKHBT(x0, x2, 0);
#endif

      /* Read last state variables */
      x0 = *__SIMD32(px);

      /* Perform the multiply-accumulates */
      acc1 = __SMLALDX(x1, c0, acc1);

      /* pack state variables */
#ifndef ARM_MATH_BIG_ENDIAN
      x1 = __PKHBT(x0, x2, 0);
#else
      x1 = __PKHBT(x2, x0, 0);
#endif

      /* Perform the multiply-accumulates */
      acc3 = __SMLALDX(x1, c0, acc3);
    }

    /* The results in the 4 accumulators are in 2.30 format.  Convert to 1.15 with saturation.
     ** Then store the 4 outputs in the destination buffer. */

#ifndef ARM_MATH_BIG_ENDIAN

    *__SIMD32(pDst)++ =
      __PKHBT(__SSAT((acc0 >> 15), 16), __SSAT((acc1 >> 15), 16), 16);

    *__SIMD32(pDst)++ =
      __PKHBT(__SSAT((acc2 >> 15), 16), __SSAT((acc3 >> 15), 16), 16);

#else

    *__SIMD32(pDst)++ =
      __PKHBT(__SSAT((acc1 >> 15), 16), __SSAT((acc0 >> 15), 16), 16);

    *__SIMD32(pDst)++ =
      __PKHBT(__SSAT((acc3 >> 15), 16), __SSAT((acc2 >> 15), 16), 16);

#endif /*      #ifndef ARM_MATH_BIG_ENDIAN       */

    /* Advance the state pointer by 4 to process the next group of 4 samples */
    pState = pState + 4;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;
  while (blkCnt > 0U)
  {
    /* Copy two samples into state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Set the accumulator to zero */
    acc0 = 0;

    /* Use SIMD to hold states and coefficients */
    px = pState;
    pb = pCoeffs;

    tapCnt = numTaps >> 1U;

    do
    {
      acc0 += (q31_t) * px++ * *pb++;
	  acc0 += (q31_t) * px++ * *pb++;
      tapCnt--;
    }
    while (tapCnt > 0U);

    /* The result is in 2.30 format.  Convert to 1.15 with saturation.
     ** Then store the output in the destination buffer. */
    *pDst++ = (q15_t) (__SSAT((acc0 >> 15), 16));

    /* Advance state pointer by 1 for the next sample */
    pState = pState + 1U;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Processing is complete.
   ** Now copy the last numTaps - 1 samples to the satrt of the state buffer.
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  /* Calculation of count for copying integer writes */
  tapCnt = (numTaps - 1U) >> 2;

  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;

    tapCnt--;

  }

  /* Calculation of count for remaining q15_t data */
  tapCnt = (numTaps - 1U) % 0x4U;

  /* copy remaining data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }
}


#endif /* #ifndef UNALIGNED_SUPPORT_DISABLE */

#else /* ARM_MATH_CM0_FAMILY */


/* Run the below code for Cortex-M0 */

void arm_fir_q15(
  const arm_fir_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state */



  q15_t *px;                                     /* Temporary pointer for state buffer */
  q15_t *pb;                                     /* Temporary pointer for coefficient buffer */
  q63_t acc;                                     /* Accumulator */
  uint32_t numTaps = S->numTaps;                 /* Number of nTaps in the filter */
  uint32_t tapCnt, blkCnt;                       /* Loop counters */

  /* S->pState buffer contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = &(S->pState[(numTaps - 1U)]);

  /* Initialize blkCnt with blockSize */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* Copy one sample at a time into state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Set the accumulator to zero */
    acc = 0;

    /* Initialize state pointer */
    px = pState;

    /* Initialize Coefficient pointer */
    pb = pCoeffs;

    tapCnt = numTaps;

    /* Perform the multiply-accumulates */
    do
    {
      /* acc =  b[numTaps-1] * x[n-numTaps-1] + b[numTaps-2] * x[n-numTaps-2] + b[numTaps-3] * x[n-numTaps-3] +...+ b[0] * x[0] */
      acc += (q31_t) * px++ * *pb++;
      tapCnt--;
    } while (tapCnt > 0U);

    /* The result is in 2.30 format.  Convert to 1.15
     ** Then store the output in the destination buffer. */
    *pDst++ = (q15_t) __SSAT((acc >> 15U), 16);

    /* Advance state pointer by 1 for the next sample */
    pState = pState + 1;

    /* Decrement the samples loop counter */
    blkCnt--;
  }

  /* Processing is complete.
   ** Now copy the last numTaps - 1 samples to the satrt of the state buffer.
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  /* Copy numTaps number of values */
  tapCnt = (numTaps - 1U);

  /* copy data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }

}

#endif /* #if defined (ARM_MATH_DSP) */




/**
 * @} end of FIR group
 */
