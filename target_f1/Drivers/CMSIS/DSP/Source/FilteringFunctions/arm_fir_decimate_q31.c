/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fir_decimate_q31.c
 * Description:  Q31 FIR Decimator
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
 * @brief Processing function for the Q31 FIR decimator.
 * @param[in] *S points to an instance of the Q31 FIR decimator structure.
 * @param[in] *pSrc points to the block of input data.
 * @param[out] *pDst points to the block of output data
 * @param[in] blockSize number of input samples to process per call.
 * @return none
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using an internal 64-bit accumulator.
 * The accumulator has a 2.62 format and maintains full precision of the intermediate multiplication results but provides only a single guard bit.
 * Thus, if the accumulator result overflows it wraps around rather than clip.
 * In order to avoid overflows completely the input signal must be scaled down by log2(numTaps) bits (where log2 is read as log to the base 2).
 * After all multiply-accumulates are performed, the 2.62 accumulator is truncated to 1.32 format and then saturated to 1.31 format.
 *
 * \par
 * Refer to the function <code>arm_fir_decimate_fast_q31()</code> for a faster but less precise implementation of this function for Cortex-M3 and Cortex-M4.
 */

void arm_fir_decimate_q31(
  const arm_fir_decimate_instance_q31 * S,
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
  q63_t sum0;                                    /* Accumulator */
  uint32_t numTaps = S->numTaps;                 /* Number of taps */
  uint32_t i, tapCnt, blkCnt, outBlockSize = blockSize / S->M;  /* Loop counters */


#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

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
      sum0 += (q63_t) x0 *c0;

      /* Read the b[numTaps-2] coefficient */
      c0 = *(pb++);

      /* Read x[n-numTaps-2] sample */
      x0 = *(px++);

      /* Perform the multiply-accumulate */
      sum0 += (q63_t) x0 *c0;

      /* Read the b[numTaps-3] coefficient */
      c0 = *(pb++);

      /* Read x[n-numTaps-3] sample */
      x0 = *(px++);

      /* Perform the multiply-accumulate */
      sum0 += (q63_t) x0 *c0;

      /* Read the b[numTaps-4] coefficient */
      c0 = *(pb++);

      /* Read x[n-numTaps-4] sample */
      x0 = *(px++);

      /* Perform the multiply-accumulate */
      sum0 += (q63_t) x0 *c0;

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
      sum0 += (q63_t) x0 *c0;

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Advance the state pointer by the decimation factor
     * to process the next group of decimation factor number samples */
    pState = pState + S->M;

    /* The result is in the accumulator, store in the destination buffer. */
    *pDst++ = (q31_t) (sum0 >> 31);

    /* Decrement the loop counter */
    blkCnt--;
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

#else

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

    /* Set accumulator to zero */
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
      sum0 += (q63_t) x0 *c0;

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Advance the state pointer by the decimation factor
     * to process the next group of decimation factor number samples */
    pState = pState + S->M;

    /* The result is in the accumulator, store in the destination buffer. */
    *pDst++ = (q31_t) (sum0 >> 31);

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

#endif /*   #if defined (ARM_MATH_DSP) */

}

/**
 * @} end of FIR_decimate group
 */
