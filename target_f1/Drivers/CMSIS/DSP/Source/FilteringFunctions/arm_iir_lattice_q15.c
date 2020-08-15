/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_iir_lattice_q15.c
 * Description:  Q15 IIR lattice filter processing function
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
 * @addtogroup IIR_Lattice
 * @{
 */

/**
 * @brief Processing function for the Q15 IIR lattice filter.
 * @param[in] *S points to an instance of the Q15 IIR lattice structure.
 * @param[in] *pSrc points to the block of input data.
 * @param[out] *pDst points to the block of output data.
 * @param[in] blockSize number of samples to process.
 * @return none.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using a 64-bit internal accumulator.
 * Both coefficients and state variables are represented in 1.15 format and multiplications yield a 2.30 result.
 * The 2.30 intermediate results are accumulated in a 64-bit accumulator in 34.30 format.
 * There is no risk of internal overflow with this approach and the full precision of intermediate multiplications is preserved.
 * After all additions have been performed, the accumulator is truncated to 34.15 format by discarding low 15 bits.
 * Lastly, the accumulator is saturated to yield a result in 1.15 format.
 */

void arm_iir_lattice_q15(
  const arm_iir_lattice_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{


#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  q31_t fcurr, fnext, gcurr = 0, gnext;          /* Temporary variables for lattice stages */
  q15_t gnext1, gnext2;                          /* Temporary variables for lattice stages */
  uint32_t stgCnt;                               /* Temporary variables for counts */
  q63_t acc;                                     /* Accumlator */
  uint32_t blkCnt, tapCnt;                       /* Temporary variables for counts */
  q15_t *px1, *px2, *pk, *pv;                    /* temporary pointers for state and coef */
  uint32_t numStages = S->numStages;             /* number of stages */
  q15_t *pState;                                 /* State pointer */
  q15_t *pStateCurnt;                            /* State current pointer */
  q15_t out;                                     /* Temporary variable for output */
  q31_t v;                                       /* Temporary variable for ladder coefficient */
#ifdef UNALIGNED_SUPPORT_DISABLE
	q15_t v1, v2;
#endif


  blkCnt = blockSize;

  pState = &S->pState[0];

  /* Sample processing */
  while (blkCnt > 0U)
  {
    /* Read Sample from input buffer */
    /* fN(n) = x(n) */
    fcurr = *pSrc++;

    /* Initialize state read pointer */
    px1 = pState;
    /* Initialize state write pointer */
    px2 = pState;
    /* Set accumulator to zero */
    acc = 0;
    /* Initialize Ladder coeff pointer */
    pv = &S->pvCoeffs[0];
    /* Initialize Reflection coeff pointer */
    pk = &S->pkCoeffs[0];


    /* Process sample for first tap */
    gcurr = *px1++;
    /* fN-1(n) = fN(n) - kN * gN-1(n-1) */
    fnext = fcurr - (((q31_t) gcurr * (*pk)) >> 15);
    fnext = __SSAT(fnext, 16);
    /* gN(n) = kN * fN-1(n) + gN-1(n-1) */
    gnext = (((q31_t) fnext * (*pk++)) >> 15) + gcurr;
    gnext = __SSAT(gnext, 16);
    /* write gN(n) into state for next sample processing */
    *px2++ = (q15_t) gnext;
    /* y(n) += gN(n) * vN  */
    acc += (q31_t) ((gnext * (*pv++)));


    /* Update f values for next coefficient processing */
    fcurr = fnext;

    /* Loop unrolling.  Process 4 taps at a time. */
    tapCnt = (numStages - 1U) >> 2;

    while (tapCnt > 0U)
    {

      /* Process sample for 2nd, 6th ...taps */
      /* Read gN-2(n-1) from state buffer */
      gcurr = *px1++;
      /* Process sample for 2nd, 6th .. taps */
      /* fN-2(n) = fN-1(n) - kN-1 * gN-2(n-1) */
      fnext = fcurr - (((q31_t) gcurr * (*pk)) >> 15);
      fnext = __SSAT(fnext, 16);
      /* gN-1(n) = kN-1 * fN-2(n) + gN-2(n-1) */
      gnext = (((q31_t) fnext * (*pk++)) >> 15) + gcurr;
      gnext1 = (q15_t) __SSAT(gnext, 16);
      /* write gN-1(n) into state */
      *px2++ = (q15_t) gnext1;


      /* Process sample for 3nd, 7th ...taps */
      /* Read gN-3(n-1) from state */
      gcurr = *px1++;
      /* Process sample for 3rd, 7th .. taps */
      /* fN-3(n) = fN-2(n) - kN-2 * gN-3(n-1) */
      fcurr = fnext - (((q31_t) gcurr * (*pk)) >> 15);
      fcurr = __SSAT(fcurr, 16);
      /* gN-2(n) = kN-2 * fN-3(n) + gN-3(n-1) */
      gnext = (((q31_t) fcurr * (*pk++)) >> 15) + gcurr;
      gnext2 = (q15_t) __SSAT(gnext, 16);
      /* write gN-2(n) into state */
      *px2++ = (q15_t) gnext2;

      /* Read vN-1 and vN-2 at a time */
#ifndef UNALIGNED_SUPPORT_DISABLE

      v = *__SIMD32(pv)++;

#else

	  v1 = *pv++;
	  v2 = *pv++;

#ifndef ARM_MATH_BIG_ENDIAN

	  v = __PKHBT(v1, v2, 16);

#else

	  v = __PKHBT(v2, v1, 16);

#endif	/* 	#ifndef ARM_MATH_BIG_ENDIAN		*/

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE */


      /* Pack gN-1(n) and gN-2(n) */

#ifndef  ARM_MATH_BIG_ENDIAN

      gnext = __PKHBT(gnext1, gnext2, 16);

#else

      gnext = __PKHBT(gnext2, gnext1, 16);

#endif /*   #ifndef  ARM_MATH_BIG_ENDIAN    */

      /* y(n) += gN-1(n) * vN-1  */
      /* process for gN-5(n) * vN-5, gN-9(n) * vN-9 ... */
      /* y(n) += gN-2(n) * vN-2  */
      /* process for gN-6(n) * vN-6, gN-10(n) * vN-10 ... */
      acc = __SMLALD(gnext, v, acc);


      /* Process sample for 4th, 8th ...taps */
      /* Read gN-4(n-1) from state */
      gcurr = *px1++;
      /* Process sample for 4th, 8th .. taps */
      /* fN-4(n) = fN-3(n) - kN-3 * gN-4(n-1) */
      fnext = fcurr - (((q31_t) gcurr * (*pk)) >> 15);
      fnext = __SSAT(fnext, 16);
      /* gN-3(n) = kN-3 * fN-1(n) + gN-1(n-1) */
      gnext = (((q31_t) fnext * (*pk++)) >> 15) + gcurr;
      gnext1 = (q15_t) __SSAT(gnext, 16);
      /* write  gN-3(n) for the next sample process */
      *px2++ = (q15_t) gnext1;


      /* Process sample for 5th, 9th ...taps */
      /* Read gN-5(n-1) from state */
      gcurr = *px1++;
      /* Process sample for 5th, 9th .. taps */
      /* fN-5(n) = fN-4(n) - kN-4 * gN-5(n-1) */
      fcurr = fnext - (((q31_t) gcurr * (*pk)) >> 15);
      fcurr = __SSAT(fcurr, 16);
      /* gN-4(n) = kN-4 * fN-5(n) + gN-5(n-1) */
      gnext = (((q31_t) fcurr * (*pk++)) >> 15) + gcurr;
      gnext2 = (q15_t) __SSAT(gnext, 16);
      /* write      gN-4(n) for the next sample process */
      *px2++ = (q15_t) gnext2;

      /* Read vN-3 and vN-4 at a time */
#ifndef UNALIGNED_SUPPORT_DISABLE

      v = *__SIMD32(pv)++;

#else

	  v1 = *pv++;
	  v2 = *pv++;

#ifndef ARM_MATH_BIG_ENDIAN

	  v = __PKHBT(v1, v2, 16);

#else

	  v = __PKHBT(v2, v1, 16);

#endif	/* #ifndef ARM_MATH_BIG_ENDIAN	 */

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE */


      /* Pack gN-3(n) and gN-4(n) */
#ifndef  ARM_MATH_BIG_ENDIAN

      gnext = __PKHBT(gnext1, gnext2, 16);

#else

      gnext = __PKHBT(gnext2, gnext1, 16);

#endif /*      #ifndef  ARM_MATH_BIG_ENDIAN    */

      /* y(n) += gN-4(n) * vN-4  */
      /* process for gN-8(n) * vN-8, gN-12(n) * vN-12 ... */
      /* y(n) += gN-3(n) * vN-3  */
      /* process for gN-7(n) * vN-7, gN-11(n) * vN-11 ... */
      acc = __SMLALD(gnext, v, acc);

      tapCnt--;

    }

    fnext = fcurr;

    /* If the filter length is not a multiple of 4, compute the remaining filter taps */
    tapCnt = (numStages - 1U) % 0x4U;

    while (tapCnt > 0U)
    {
      gcurr = *px1++;
      /* Process sample for last taps */
      fnext = fcurr - (((q31_t) gcurr * (*pk)) >> 15);
      fnext = __SSAT(fnext, 16);
      gnext = (((q31_t) fnext * (*pk++)) >> 15) + gcurr;
      gnext = __SSAT(gnext, 16);
      /* Output samples for last taps */
      acc += (q31_t) (((q31_t) gnext * (*pv++)));
      *px2++ = (q15_t) gnext;
      fcurr = fnext;

      tapCnt--;
    }

    /* y(n) += g0(n) * v0 */
    acc += (q31_t) (((q31_t) fnext * (*pv++)));

    out = (q15_t) __SSAT(acc >> 15, 16);
    *px2++ = (q15_t) fnext;

    /* write out into pDst */
    *pDst++ = out;

    /* Advance the state pointer by 4 to process the next group of 4 samples */
    pState = pState + 1U;
    blkCnt--;

  }

  /* Processing is complete. Now copy last S->numStages samples to start of the buffer
     for the preperation of next frame process */
  /* Points to the start of the state buffer */
  pStateCurnt = &S->pState[0];
  pState = &S->pState[blockSize];

  stgCnt = (numStages >> 2U);

  /* copy data */
  while (stgCnt > 0U)
  {
#ifndef UNALIGNED_SUPPORT_DISABLE

    *__SIMD32(pStateCurnt)++ = *__SIMD32(pState)++;
    *__SIMD32(pStateCurnt)++ = *__SIMD32(pState)++;

#else

    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;

#endif /*	#ifndef UNALIGNED_SUPPORT_DISABLE */

    /* Decrement the loop counter */
    stgCnt--;

  }

  /* Calculation of count for remaining q15_t data */
  stgCnt = (numStages) % 0x4U;

  /* copy data */
  while (stgCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    stgCnt--;
  }

#else

  /* Run the below code for Cortex-M0 */

  q31_t fcurr, fnext = 0, gcurr = 0, gnext;      /* Temporary variables for lattice stages */
  uint32_t stgCnt;                               /* Temporary variables for counts */
  q63_t acc;                                     /* Accumlator */
  uint32_t blkCnt, tapCnt;                       /* Temporary variables for counts */
  q15_t *px1, *px2, *pk, *pv;                    /* temporary pointers for state and coef */
  uint32_t numStages = S->numStages;             /* number of stages */
  q15_t *pState;                                 /* State pointer */
  q15_t *pStateCurnt;                            /* State current pointer */
  q15_t out;                                     /* Temporary variable for output */


  blkCnt = blockSize;

  pState = &S->pState[0];

  /* Sample processing */
  while (blkCnt > 0U)
  {
    /* Read Sample from input buffer */
    /* fN(n) = x(n) */
    fcurr = *pSrc++;

    /* Initialize state read pointer */
    px1 = pState;
    /* Initialize state write pointer */
    px2 = pState;
    /* Set accumulator to zero */
    acc = 0;
    /* Initialize Ladder coeff pointer */
    pv = &S->pvCoeffs[0];
    /* Initialize Reflection coeff pointer */
    pk = &S->pkCoeffs[0];

    tapCnt = numStages;

    while (tapCnt > 0U)
    {
      gcurr = *px1++;
      /* Process sample */
      /* fN-1(n) = fN(n) - kN * gN-1(n-1) */
      fnext = fcurr - ((gcurr * (*pk)) >> 15);
      fnext = __SSAT(fnext, 16);
      /* gN(n) = kN * fN-1(n) + gN-1(n-1) */
      gnext = ((fnext * (*pk++)) >> 15) + gcurr;
      gnext = __SSAT(gnext, 16);
      /* Output samples */
      /* y(n) += gN(n) * vN */
      acc += (q31_t) ((gnext * (*pv++)));
      /* write gN(n) into state for next sample processing */
      *px2++ = (q15_t) gnext;
      /* Update f values for next coefficient processing */
      fcurr = fnext;

      tapCnt--;
    }

    /* y(n) += g0(n) * v0 */
    acc += (q31_t) ((fnext * (*pv++)));

    out = (q15_t) __SSAT(acc >> 15, 16);
    *px2++ = (q15_t) fnext;

    /* write out into pDst */
    *pDst++ = out;

    /* Advance the state pointer by 1 to process the next group of samples */
    pState = pState + 1U;
    blkCnt--;

  }

  /* Processing is complete. Now copy last S->numStages samples to start of the buffer
     for the preperation of next frame process */
  /* Points to the start of the state buffer */
  pStateCurnt = &S->pState[0];
  pState = &S->pState[blockSize];

  stgCnt = numStages;

  /* copy data */
  while (stgCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    stgCnt--;
  }

#endif /*   #if defined (ARM_MATH_DSP) */

}




/**
 * @} end of IIR_Lattice group
 */
