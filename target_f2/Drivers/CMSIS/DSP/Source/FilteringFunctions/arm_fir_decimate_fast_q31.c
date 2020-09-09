/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fir_decimate_fast_q31.c
 * Description:  Fast Q31 FIR Decimator
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
 * @brief Processing function for the Q31 FIR decimator (fast variant) for Cortex-M3 and Cortex-M4.
 * @param[in] *S points to an instance of the Q31 FIR decimator structure.
 * @param[in] *pSrc points to the block of input data.
 * @param[out] *pDst points to the block of output data
 * @param[in] blockSize number of input samples to process per call.
 * @return none
 *
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * This function is optimized for speed at the expense of fixed-point precision and overflow protection.
 * The result of each 1.31 x 1.31 multiplication is truncated to 2.30 format.
 * These intermediate results are added to a 2.30 accumulator.
 * Finally, the accumulator is saturated and converted to a 1.31 result.
 * The fast version has the same overflow behavior as the standard version and provides less precision since it discards the low 32 bits of each multiplication result.
 * In order to avoid overflows completely the input signal must be scaled down by log2(numTaps) bits (where log2 is read as log to the base 2).
 *
 * \par
 * Refer to the function <code>arm_fir_decimate_q31()</code> for a slower implementation of this function which uses a 64-bit accumulator to provide higher precision.
 * Both the slow and the fast versions use the same instance structure.
 * Use the function <code>arm_fir_decimate_init_q31()</code> to initialize the filter structure.
 */

void arm_fir_decimate_fast_q31(
  arm_fir_decimate_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t *pState = S->pState;                     /* State pointer */
  q31_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q31_t *pStateCurnt;                            /* Points to the current sample of the state */
  q31_t x0, c0;                                  /* Temporary variables to hold state and coefficient values */
  q31_t *px;                                     /* Temporary pointers for state buffer */
  q31_t *pb;                                     /* Temporary pointers for coefficient buffer */
  q31_t sum0;                                    /* Accumulator */
  uint32_t numTaps = S->numTaps;                 /* Number of taps */
  uint32_t i, tapCnt, blkCnt, outBlockSize = blockSize / S->M;  /* Loop counters */
  uint32_t blkCntN2;
  q31_t x1;
  q31_t acc0, acc1;
  q31_t *px0, *px1;

  /* S->pState buffer contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + (numTaps - 1U);

  /* Total number of output samples to be computed */

  blkCnt = outBlockSize / 2;
  blkCntN2 = outBlockSize - (2 * blkCnt);

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
      /* Read the b[numTaps-1] coefficient */
      c0 = *(pb);

      /* Read x[n-numTaps-1] for sample 0 sample 1 */
      x0 = *(px0);
      x1 = *(px1);

      /* Perform the multiply-accumulate */
      acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);
      acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);

      /* Read the b[numTaps-2] coefficient */
      c0 = *(pb + 1U);

      /* Read x[n-numTaps-2]  for sample 0 sample 1  */
      x0 = *(px0 + 1U);
      x1 = *(px1 + 1U);

      /* Perform the multiply-accumulate */
      acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);
      acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);

      /* Read the b[numTaps-3] coefficient */
      c0 = *(pb + 2U);

      /* Read x[n-numTaps-3]  for sample 0 sample 1 */
      x0 = *(px0 + 2U);
      x1 = *(px1 + 2U);
      pb += 4U;

      /* Perform the multiply-accumulate */
      acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);
      acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);

      /* Read the b[numTaps-4] coefficient */
      c0 = *(pb - 1U);

      /* Read x[n-numTaps-4] for sample 0 sample 1 */
      x0 = *(px0 + 3U);
      x1 = *(px1 + 3U);


      /* Perform the multiply-accumulate */
      acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);
      acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);

      /* update state pointers */
      px0 += 4U;
      px1 += 4U;

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* If the filter length is not a multiple of 4, compute the remaining filter taps */
    tapCnt = numTaps % 0x4U;

    while (tapCnt > 0U)
    {
      /* Read coefficients */
      c0 = *(pb++);

      /* Fetch 1 state variable */
      x0 = *(px0++);
      x1 = *(px1++);

      /* Perform the multiply-accumulate */
      acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);
      acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Advance the state pointer by the decimation factor
     * to process the next group of decimation factor number samples */
    pState = pState + S->M * 2;

    /* The result is in the accumulator, store in the destination buffer. */
    *pDst++ = (q31_t) (acc0 << 1);
    *pDst++ = (q31_t) (acc1 << 1);

    /* Decrement the loop counter */
    blkCnt--;
  }

  while (blkCntN2 > 0U)
  {
    /* Copy decimation factor number of new input samples into the state buffer */
    i = S->M;

    do
    {
      *pStateCurnt++ = *pSrc++;

    } while (--i);

    /* Set accumulator to zero */
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
      /* Read the b[numTaps-1] coefficient */
      c0 = *(pb++);

      /* Read x[n-numTaps-1] sample */
      x0 = *(px++);

      /* Perform the multiply-accumulate */
      sum0 = (q31_t) ((((q63_t) sum0 << 32) + ((q63_t) x0 * c0)) >> 32);

      /* Read the b[numTaps-2] coefficient */
      c0 = *(pb++);

      /* Read x[n-numTaps-2] sample */
      x0 = *(px++);

      /* Perform the multiply-accumulate */
      sum0 = (q31_t) ((((q63_t) sum0 << 32) + ((q63_t) x0 * c0)) >> 32);

      /* Read the b[numTaps-3] coefficient */
      c0 = *(pb++);

      /* Read x[n-numTaps-3] sample */
      x0 = *(px++);

      /* Perform the multiply-accumulate */
      sum0 = (q31_t) ((((q63_t) sum0 << 32) + ((q63_t) x0 * c0)) >> 32);

      /* Read the b[numTaps-4] coefficient */
      c0 = *(pb++);

      /* Read x[n-numTaps-4] sample */
      x0 = *(px++);

      /* Perform the multiply-accumulate */
      sum0 = (q31_t) ((((q63_t) sum0 << 32) + ((q63_t) x0 * c0)) >> 32);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* If the filter length is not a multiple of 4, compute the remaining filter taps */
    tapCnt = numTaps % 0x4U;

    while (tapCnt > 0U)
    {
      /* Read coefficients */
      c0 = *(pb++);

      /* Fetch 1 state variable */
      x0 = *(px++);

      /* Perform the multiply-accumulate */
      sum0 = (q31_t) ((((q63_t) sum0 << 32) + ((q63_t) x0 * c0)) >> 32);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Advance the state pointer by the decimation factor
     * to process the next group of decimation factor number samples */
    pState = pState + S->M;

    /* The result is in the accumulator, store in the destination buffer. */
    *pDst++ = (q31_t) (sum0 << 1);

    /* Decrement the loop counter */
    blkCntN2--;
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

/**
 * @} end of FIR_decimate group
 */
