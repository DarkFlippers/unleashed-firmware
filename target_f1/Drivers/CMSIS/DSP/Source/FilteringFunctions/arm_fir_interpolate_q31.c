/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fir_interpolate_q31.c
 * Description:  Q31 FIR interpolation
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
 * @addtogroup FIR_Interpolate
 * @{
 */

/**
 * @brief Processing function for the Q31 FIR interpolator.
 * @param[in] *S        points to an instance of the Q31 FIR interpolator structure.
 * @param[in] *pSrc     points to the block of input data.
 * @param[out] *pDst    points to the block of output data.
 * @param[in] blockSize number of input samples to process per call.
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using an internal 64-bit accumulator.
 * The accumulator has a 2.62 format and maintains full precision of the intermediate multiplication results but provides only a single guard bit.
 * Thus, if the accumulator result overflows it wraps around rather than clip.
 * In order to avoid overflows completely the input signal must be scaled down by <code>1/(numTaps/L)</code>.
 * since <code>numTaps/L</code> additions occur per output sample.
 * After all multiply-accumulates are performed, the 2.62 accumulator is truncated to 1.32 format and then saturated to 1.31 format.
 */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

void arm_fir_interpolate_q31(
  const arm_fir_interpolate_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t *pState = S->pState;                     /* State pointer */
  q31_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q31_t *pStateCurnt;                            /* Points to the current sample of the state */
  q31_t *ptr1, *ptr2;                            /* Temporary pointers for state and coefficient buffers */
  q63_t sum0;                                    /* Accumulators */
  q31_t x0, c0;                                  /* Temporary variables to hold state and coefficient values */
  uint32_t i, blkCnt, j;                         /* Loop counters */
  uint16_t phaseLen = S->phaseLength, tapCnt;    /* Length of each polyphase filter component */

  uint32_t blkCntN2;
  q63_t acc0, acc1;
  q31_t x1;

  /* S->pState buffer contains previous frame (phaseLen - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + ((q31_t) phaseLen - 1);

  /* Initialise  blkCnt */
  blkCnt = blockSize / 2;
  blkCntN2 = blockSize - (2 * blkCnt);

  /* Samples loop unrolled by 2 */
  while (blkCnt > 0U)
  {
    /* Copy new input sample into the state buffer */
    *pStateCurnt++ = *pSrc++;
    *pStateCurnt++ = *pSrc++;

    /* Address modifier index of coefficient buffer */
    j = 1U;

    /* Loop over the Interpolation factor. */
    i = (S->L);

    while (i > 0U)
    {
      /* Set accumulator to zero */
      acc0 = 0;
      acc1 = 0;

      /* Initialize state pointer */
      ptr1 = pState;

      /* Initialize coefficient pointer */
      ptr2 = pCoeffs + (S->L - j);

      /* Loop over the polyPhase length. Unroll by a factor of 4.
       ** Repeat until we've computed numTaps-(4*S->L) coefficients. */
      tapCnt = phaseLen >> 2U;

      x0 = *(ptr1++);

      while (tapCnt > 0U)
      {

        /* Read the input sample */
        x1 = *(ptr1++);

        /* Read the coefficient */
        c0 = *(ptr2);

        /* Perform the multiply-accumulate */
        acc0 += (q63_t) x0 *c0;
        acc1 += (q63_t) x1 *c0;


        /* Read the coefficient */
        c0 = *(ptr2 + S->L);

        /* Read the input sample */
        x0 = *(ptr1++);

        /* Perform the multiply-accumulate */
        acc0 += (q63_t) x1 *c0;
        acc1 += (q63_t) x0 *c0;


        /* Read the coefficient */
        c0 = *(ptr2 + S->L * 2);

        /* Read the input sample */
        x1 = *(ptr1++);

        /* Perform the multiply-accumulate */
        acc0 += (q63_t) x0 *c0;
        acc1 += (q63_t) x1 *c0;

        /* Read the coefficient */
        c0 = *(ptr2 + S->L * 3);

        /* Read the input sample */
        x0 = *(ptr1++);

        /* Perform the multiply-accumulate */
        acc0 += (q63_t) x1 *c0;
        acc1 += (q63_t) x0 *c0;


        /* Upsampling is done by stuffing L-1 zeros between each sample.
         * So instead of multiplying zeros with coefficients,
         * Increment the coefficient pointer by interpolation factor times. */
        ptr2 += 4 * S->L;

        /* Decrement the loop counter */
        tapCnt--;
      }

      /* If the polyPhase length is not a multiple of 4, compute the remaining filter taps */
      tapCnt = phaseLen % 0x4U;

      while (tapCnt > 0U)
      {

        /* Read the input sample */
        x1 = *(ptr1++);

        /* Read the coefficient */
        c0 = *(ptr2);

        /* Perform the multiply-accumulate */
        acc0 += (q63_t) x0 *c0;
        acc1 += (q63_t) x1 *c0;

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* update states for next sample processing */
        x0 = x1;

        /* Decrement the loop counter */
        tapCnt--;
      }

      /* The result is in the accumulator, store in the destination buffer. */
      *pDst = (q31_t) (acc0 >> 31);
      *(pDst + S->L) = (q31_t) (acc1 >> 31);


      pDst++;

      /* Increment the address modifier index of coefficient buffer */
      j++;

      /* Decrement the loop counter */
      i--;
    }

    /* Advance the state pointer by 1
     * to process the next group of interpolation factor number samples */
    pState = pState + 2;

    pDst += S->L;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 2, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blkCntN2;

  /* Loop over the blockSize. */
  while (blkCnt > 0U)
  {
    /* Copy new input sample into the state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Address modifier index of coefficient buffer */
    j = 1U;

    /* Loop over the Interpolation factor. */
    i = S->L;
    while (i > 0U)
    {
      /* Set accumulator to zero */
      sum0 = 0;

      /* Initialize state pointer */
      ptr1 = pState;

      /* Initialize coefficient pointer */
      ptr2 = pCoeffs + (S->L - j);

      /* Loop over the polyPhase length. Unroll by a factor of 4.
       ** Repeat until we've computed numTaps-(4*S->L) coefficients. */
      tapCnt = phaseLen >> 2;
      while (tapCnt > 0U)
      {

        /* Read the coefficient */
        c0 = *(ptr2);

        /* Upsampling is done by stuffing L-1 zeros between each sample.
         * So instead of multiplying zeros with coefficients,
         * Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *(ptr1++);

        /* Perform the multiply-accumulate */
        sum0 += (q63_t) x0 *c0;

        /* Read the coefficient */
        c0 = *(ptr2);

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *(ptr1++);

        /* Perform the multiply-accumulate */
        sum0 += (q63_t) x0 *c0;

        /* Read the coefficient */
        c0 = *(ptr2);

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *(ptr1++);

        /* Perform the multiply-accumulate */
        sum0 += (q63_t) x0 *c0;

        /* Read the coefficient */
        c0 = *(ptr2);

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *(ptr1++);

        /* Perform the multiply-accumulate */
        sum0 += (q63_t) x0 *c0;

        /* Decrement the loop counter */
        tapCnt--;
      }

      /* If the polyPhase length is not a multiple of 4, compute the remaining filter taps */
      tapCnt = phaseLen & 0x3U;

      while (tapCnt > 0U)
      {
        /* Read the coefficient */
        c0 = *(ptr2);

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *(ptr1++);

        /* Perform the multiply-accumulate */
        sum0 += (q63_t) x0 *c0;

        /* Decrement the loop counter */
        tapCnt--;
      }

      /* The result is in the accumulator, store in the destination buffer. */
      *pDst++ = (q31_t) (sum0 >> 31);

      /* Increment the address modifier index of coefficient buffer */
      j++;

      /* Decrement the loop counter */
      i--;
    }

    /* Advance the state pointer by 1
     * to process the next group of interpolation factor number samples */
    pState = pState + 1;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Processing is complete.
   ** Now copy the last phaseLen - 1 samples to the satrt of the state buffer.
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  tapCnt = (phaseLen - 1U) >> 2U;

  /* copy data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }

  tapCnt = (phaseLen - 1U) % 0x04U;

  /* copy data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }

}


#else

void arm_fir_interpolate_q31(
  const arm_fir_interpolate_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t *pState = S->pState;                     /* State pointer */
  q31_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q31_t *pStateCurnt;                            /* Points to the current sample of the state */
  q31_t *ptr1, *ptr2;                            /* Temporary pointers for state and coefficient buffers */

  /* Run the below code for Cortex-M0 */

  q63_t sum;                                     /* Accumulator */
  q31_t x0, c0;                                  /* Temporary variables to hold state and coefficient values */
  uint32_t i, blkCnt;                            /* Loop counters */
  uint16_t phaseLen = S->phaseLength, tapCnt;    /* Length of each polyphase filter component */


  /* S->pState buffer contains previous frame (phaseLen - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + ((q31_t) phaseLen - 1);

  /* Total number of intput samples */
  blkCnt = blockSize;

  /* Loop over the blockSize. */
  while (blkCnt > 0U)
  {
    /* Copy new input sample into the state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Loop over the Interpolation factor. */
    i = S->L;

    while (i > 0U)
    {
      /* Set accumulator to zero */
      sum = 0;

      /* Initialize state pointer */
      ptr1 = pState;

      /* Initialize coefficient pointer */
      ptr2 = pCoeffs + (i - 1U);

      tapCnt = phaseLen;

      while (tapCnt > 0U)
      {
        /* Read the coefficient */
        c0 = *(ptr2);

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *ptr1++;

        /* Perform the multiply-accumulate */
        sum += (q63_t) x0 *c0;

        /* Decrement the loop counter */
        tapCnt--;
      }

      /* The result is in the accumulator, store in the destination buffer. */
      *pDst++ = (q31_t) (sum >> 31);

      /* Decrement the loop counter */
      i--;
    }

    /* Advance the state pointer by 1
     * to process the next group of interpolation factor number samples */
    pState = pState + 1;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Processing is complete.
   ** Now copy the last phaseLen - 1 samples to the satrt of the state buffer.
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  tapCnt = phaseLen - 1U;

  /* copy data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }

}

#endif /*   #if defined (ARM_MATH_DSP) */

 /**
  * @} end of FIR_Interpolate group
  */
