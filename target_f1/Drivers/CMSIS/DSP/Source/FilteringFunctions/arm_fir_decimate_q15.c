/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fir_decimate_q15.c
 * Description:  Q15 FIR Decimator
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
 * @addtogroup FIR_decimate
 * @{
 */

/**
 * @brief Processing function for the Q15 FIR decimator.
 * @param[in] *S points to an instance of the Q15 FIR decimator structure.
 * @param[in] *pSrc points to the block of input data.
 * @param[out] *pDst points to the location where the output result is written.
 * @param[in] blockSize number of input samples to process per call.
 * @return none.
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
 * Refer to the function <code>arm_fir_decimate_fast_q15()</code> for a faster but less precise implementation of this function for Cortex-M3 and Cortex-M4.
 */

#if defined (ARM_MATH_DSP)

#ifndef UNALIGNED_SUPPORT_DISABLE

void arm_fir_decimate_q15(
  const arm_fir_decimate_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state */
  q15_t *px;                                     /* Temporary pointer for state buffer */
  q15_t *pb;                                     /* Temporary pointer coefficient buffer */
  q31_t x0, x1, c0, c1;                          /* Temporary variables to hold state and coefficient values */
  q63_t sum0;                                    /* Accumulators */
  q63_t acc0, acc1;
  q15_t *px0, *px1;
  uint32_t blkCntN3;
  uint32_t numTaps = S->numTaps;                 /* Number of taps */
  uint32_t i, blkCnt, tapCnt, outBlockSize = blockSize / S->M;  /* Loop counters */


  /* S->pState buffer contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + (numTaps - 1U);


  /* Total number of output samples to be computed */
  blkCnt = outBlockSize / 2;
  blkCntN3 = outBlockSize - (2 * blkCnt);


  while (blkCnt > 0U)
  {
    /* Copy decimation factor number of new input samples into the state buffer */
    i = 2 * S->M;

    do
    {
      *pStateCurnt++ = *pSrc++;

    } while (--i);

    /* Set accumulator to zero */
    acc0 = 0;
    acc1 = 0;

    /* Initialize state pointer */
    px0 = pState;

    px1 = pState + S->M;


    /* Initialize coeff pointer */
    pb = pCoeffs;

    /* Loop unrolling.  Process 4 taps at a time. */
    tapCnt = numTaps >> 2;

    /* Loop over the number of taps.  Unroll by a factor of 4.
     ** Repeat until we've computed numTaps-4 coefficients. */
    while (tapCnt > 0U)
    {
      /* Read the Read b[numTaps-1] and b[numTaps-2]  coefficients */
      c0 = *__SIMD32(pb)++;

      /* Read x[n-numTaps-1] and x[n-numTaps-2]sample */
      x0 = *__SIMD32(px0)++;

      x1 = *__SIMD32(px1)++;

      /* Perform the multiply-accumulate */
      acc0 = __SMLALD(x0, c0, acc0);

      acc1 = __SMLALD(x1, c0, acc1);

      /* Read the b[numTaps-3] and b[numTaps-4] coefficient */
      c0 = *__SIMD32(pb)++;

      /* Read x[n-numTaps-2] and x[n-numTaps-3] sample */
      x0 = *__SIMD32(px0)++;

      x1 = *__SIMD32(px1)++;

      /* Perform the multiply-accumulate */
      acc0 = __SMLALD(x0, c0, acc0);

      acc1 = __SMLALD(x1, c0, acc1);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* If the filter length is not a multiple of 4, compute the remaining filter taps */
    tapCnt = numTaps % 0x4U;

    while (tapCnt > 0U)
    {
      /* Read coefficients */
      c0 = *pb++;

      /* Fetch 1 state variable */
      x0 = *px0++;

      x1 = *px1++;

      /* Perform the multiply-accumulate */
      acc0 = __SMLALD(x0, c0, acc0);
      acc1 = __SMLALD(x1, c0, acc1);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Advance the state pointer by the decimation factor
     * to process the next group of decimation factor number samples */
    pState = pState + S->M * 2;

    /* Store filter output, smlad returns the values in 2.14 format */
    /* so downsacle by 15 to get output in 1.15 */
    *pDst++ = (q15_t) (__SSAT((acc0 >> 15), 16));
    *pDst++ = (q15_t) (__SSAT((acc1 >> 15), 16));

    /* Decrement the loop counter */
    blkCnt--;
  }



  while (blkCntN3 > 0U)
  {
    /* Copy decimation factor number of new input samples into the state buffer */
    i = S->M;

    do
    {
      *pStateCurnt++ = *pSrc++;

    } while (--i);

    /*Set sum to zero */
    sum0 = 0;

    /* Initialize state pointer */
    px = pState;

    /* Initialize coeff pointer */
    pb = pCoeffs;

    /* Loop unrolling.  Process 4 taps at a time. */
    tapCnt = numTaps >> 2;

    /* Loop over the number of taps.  Unroll by a factor of 4.
     ** Repeat until we've computed numTaps-4 coefficients. */
    while (tapCnt > 0U)
    {
      /* Read the Read b[numTaps-1] and b[numTaps-2]  coefficients */
      c0 = *__SIMD32(pb)++;

      /* Read x[n-numTaps-1] and x[n-numTaps-2]sample */
      x0 = *__SIMD32(px)++;

      /* Read the b[numTaps-3] and b[numTaps-4] coefficient */
      c1 = *__SIMD32(pb)++;

      /* Perform the multiply-accumulate */
      sum0 = __SMLALD(x0, c0, sum0);

      /* Read x[n-numTaps-2] and x[n-numTaps-3] sample */
      x0 = *__SIMD32(px)++;

      /* Perform the multiply-accumulate */
      sum0 = __SMLALD(x0, c1, sum0);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* If the filter length is not a multiple of 4, compute the remaining filter taps */
    tapCnt = numTaps % 0x4U;

    while (tapCnt > 0U)
    {
      /* Read coefficients */
      c0 = *pb++;

      /* Fetch 1 state variable */
      x0 = *px++;

      /* Perform the multiply-accumulate */
      sum0 = __SMLALD(x0, c0, sum0);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Advance the state pointer by the decimation factor
     * to process the next group of decimation factor number samples */
    pState = pState + S->M;

    /* Store filter output, smlad returns the values in 2.14 format */
    /* so downsacle by 15 to get output in 1.15 */
    *pDst++ = (q15_t) (__SSAT((sum0 >> 15), 16));

    /* Decrement the loop counter */
    blkCntN3--;
  }

  /* Processing is complete.
   ** Now copy the last numTaps - 1 samples to the satrt of the state buffer.
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  i = (numTaps - 1U) >> 2U;

  /* copy data */
  while (i > 0U)
  {
    *__SIMD32(pStateCurnt)++ = *__SIMD32(pState)++;
    *__SIMD32(pStateCurnt)++ = *__SIMD32(pState)++;

    /* Decrement the loop counter */
    i--;
  }

  i = (numTaps - 1U) % 0x04U;

  /* copy data */
  while (i > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    i--;
  }
}

#else


void arm_fir_decimate_q15(
  const arm_fir_decimate_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state */
  q15_t *px;                                     /* Temporary pointer for state buffer */
  q15_t *pb;                                     /* Temporary pointer coefficient buffer */
  q15_t x0, x1, c0;                              /* Temporary variables to hold state and coefficient values */
  q63_t sum0;                                    /* Accumulators */
  q63_t acc0, acc1;
  q15_t *px0, *px1;
  uint32_t blkCntN3;
  uint32_t numTaps = S->numTaps;                 /* Number of taps */
  uint32_t i, blkCnt, tapCnt, outBlockSize = blockSize / S->M;  /* Loop counters */


  /* S->pState buffer contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + (numTaps - 1U);


  /* Total number of output samples to be computed */
  blkCnt = outBlockSize / 2;
  blkCntN3 = outBlockSize - (2 * blkCnt);

  while (blkCnt > 0U)
  {
    /* Copy decimation factor number of new input samples into the state buffer */
    i = 2 * S->M;

    do
    {
      *pStateCurnt++ = *pSrc++;

    } while (--i);

    /* Set accumulator to zero */
    acc0 = 0;
    acc1 = 0;

    /* Initialize state pointer */
    px0 = pState;

    px1 = pState + S->M;


    /* Initialize coeff pointer */
    pb = pCoeffs;

    /* Loop unrolling.  Process 4 taps at a time. */
    tapCnt = numTaps >> 2;

    /* Loop over the number of taps.  Unroll by a factor of 4.
     ** Repeat until we've computed numTaps-4 coefficients. */
    while (tapCnt > 0U)
    {
      /* Read the Read b[numTaps-1] coefficients */
      c0 = *pb++;

      /* Read x[n-numTaps-1] for sample 0 and for sample 1 */
      x0 = *px0++;
      x1 = *px1++;

      /* Perform the multiply-accumulate */
      acc0 += x0 * c0;
      acc1 += x1 * c0;

      /* Read the b[numTaps-2] coefficient */
      c0 = *pb++;

      /* Read x[n-numTaps-2] for sample 0 and sample 1 */
      x0 = *px0++;
      x1 = *px1++;

      /* Perform the multiply-accumulate */
      acc0 += x0 * c0;
      acc1 += x1 * c0;

      /* Read the b[numTaps-3] coefficients */
      c0 = *pb++;

      /* Read x[n-numTaps-3] for sample 0 and sample 1 */
      x0 = *px0++;
      x1 = *px1++;

      /* Perform the multiply-accumulate */
      acc0 += x0 * c0;
      acc1 += x1 * c0;

      /* Read the b[numTaps-4] coefficient */
      c0 = *pb++;

      /* Read x[n-numTaps-4] for sample 0 and sample 1 */
      x0 = *px0++;
      x1 = *px1++;

      /* Perform the multiply-accumulate */
      acc0 += x0 * c0;
      acc1 += x1 * c0;

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* If the filter length is not a multiple of 4, compute the remaining filter taps */
    tapCnt = numTaps % 0x4U;

    while (tapCnt > 0U)
    {
      /* Read coefficients */
      c0 = *pb++;

      /* Fetch 1 state variable */
      x0 = *px0++;
      x1 = *px1++;

      /* Perform the multiply-accumulate */
      acc0 += x0 * c0;
      acc1 += x1 * c0;

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Advance the state pointer by the decimation factor
     * to process the next group of decimation factor number samples */
    pState = pState + S->M * 2;

    /* Store filter output, smlad returns the values in 2.14 format */
    /* so downsacle by 15 to get output in 1.15 */

    *pDst++ = (q15_t) (__SSAT((acc0 >> 15), 16));
    *pDst++ = (q15_t) (__SSAT((acc1 >> 15), 16));

    /* Decrement the loop counter */
    blkCnt--;
  }

  while (blkCntN3 > 0U)
  {
    /* Copy decimation factor number of new input samples into the state buffer */
    i = S->M;

    do
    {
      *pStateCurnt++ = *pSrc++;

    } while (--i);

    /*Set sum to zero */
    sum0 = 0;

    /* Initialize state pointer */
    px = pState;

    /* Initialize coeff pointer */
    pb = pCoeffs;

    /* Loop unrolling.  Process 4 taps at a time. */
    tapCnt = numTaps >> 2;

    /* Loop over the number of taps.  Unroll by a factor of 4.
     ** Repeat until we've computed numTaps-4 coefficients. */
    while (tapCnt > 0U)
    {
      /* Read the Read b[numTaps-1] coefficients */
      c0 = *pb++;

      /* Read x[n-numTaps-1] and sample */
      x0 = *px++;

      /* Perform the multiply-accumulate */
      sum0 += x0 * c0;

      /* Read the b[numTaps-2] coefficient */
      c0 = *pb++;

      /* Read x[n-numTaps-2] and  sample */
      x0 = *px++;

      /* Perform the multiply-accumulate */
      sum0 += x0 * c0;

      /* Read the b[numTaps-3]  coefficients */
      c0 = *pb++;

      /* Read x[n-numTaps-3] sample */
      x0 = *px++;

      /* Perform the multiply-accumulate */
      sum0 += x0 * c0;

      /* Read the b[numTaps-4] coefficient */
      c0 = *pb++;

      /* Read x[n-numTaps-4] sample */
      x0 = *px++;

      /* Perform the multiply-accumulate */
      sum0 += x0 * c0;

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* If the filter length is not a multiple of 4, compute the remaining filter taps */
    tapCnt = numTaps % 0x4U;

    while (tapCnt > 0U)
    {
      /* Read coefficients */
      c0 = *pb++;

      /* Fetch 1 state variable */
      x0 = *px++;

      /* Perform the multiply-accumulate */
      sum0 += x0 * c0;

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Advance the state pointer by the decimation factor
     * to process the next group of decimation factor number samples */
    pState = pState + S->M;

    /* Store filter output, smlad returns the values in 2.14 format */
    /* so downsacle by 15 to get output in 1.15 */
    *pDst++ = (q15_t) (__SSAT((sum0 >> 15), 16));

    /* Decrement the loop counter */
    blkCntN3--;
  }

  /* Processing is complete.
   ** Now copy the last numTaps - 1 samples to the satrt of the state buffer.
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  i = (numTaps - 1U) >> 2U;

  /* copy data */
  while (i > 0U)
  {
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    i--;
  }

  i = (numTaps - 1U) % 0x04U;

  /* copy data */
  while (i > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    i--;
  }
}


#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE	*/

#else


void arm_fir_decimate_q15(
  const arm_fir_decimate_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state */
  q15_t *px;                                     /* Temporary pointer for state buffer */
  q15_t *pb;                                     /* Temporary pointer coefficient buffer */
  q31_t x0, c0;                                  /* Temporary variables to hold state and coefficient values */
  q63_t sum0;                                    /* Accumulators */
  uint32_t numTaps = S->numTaps;                 /* Number of taps */
  uint32_t i, blkCnt, tapCnt, outBlockSize = blockSize / S->M;  /* Loop counters */



/* Run the below code for Cortex-M0 */

  /* S->pState buffer contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + (numTaps - 1U);

  /* Total number of output samples to be computed */
  blkCnt = outBlockSize;

  while (blkCnt > 0U)
  {
    /* Copy decimation factor number of new input samples into the state buffer */
    i = S->M;

    do
    {
      *pStateCurnt++ = *pSrc++;

    } while (--i);

    /*Set sum to zero */
    sum0 = 0;

    /* Initialize state pointer */
    px = pState;

    /* Initialize coeff pointer */
    pb = pCoeffs;

    tapCnt = numTaps;

    while (tapCnt > 0U)
    {
      /* Read coefficients */
      c0 = *pb++;

      /* Fetch 1 state variable */
      x0 = *px++;

      /* Perform the multiply-accumulate */
      sum0 += (q31_t) x0 *c0;

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Advance the state pointer by the decimation factor
     * to process the next group of decimation factor number samples */
    pState = pState + S->M;

    /*Store filter output , smlad will return the values in 2.14 format */
    /* so downsacle by 15 to get output in 1.15 */
    *pDst++ = (q15_t) (__SSAT((sum0 >> 15), 16));

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Processing is complete.
   ** Now copy the last numTaps - 1 samples to the start of the state buffer.
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  i = numTaps - 1U;

  /* copy data */
  while (i > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    i--;
  }


}
#endif /*   #if defined (ARM_MATH_DSP) */


/**
 * @} end of FIR_decimate group
 */
