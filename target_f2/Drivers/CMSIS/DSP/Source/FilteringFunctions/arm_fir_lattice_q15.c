/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fir_lattice_q15.c
 * Description:  Q15 FIR lattice filter processing function
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
 * @addtogroup FIR_Lattice
 * @{
 */


/**
 * @brief Processing function for the Q15 FIR lattice filter.
 * @param[in]  *S        points to an instance of the Q15 FIR lattice structure.
 * @param[in]  *pSrc     points to the block of input data.
 * @param[out] *pDst     points to the block of output data
 * @param[in]  blockSize number of samples to process.
 * @return none.
 */

void arm_fir_lattice_q15(
  const arm_fir_lattice_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState;                                 /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *px;                                     /* temporary state pointer */
  q15_t *pk;                                     /* temporary coefficient pointer */


#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  q31_t fcurnt1, fnext1, gcurnt1 = 0, gnext1;    /* temporary variables for first sample in loop unrolling */
  q31_t fcurnt2, fnext2, gnext2;                 /* temporary variables for second sample in loop unrolling */
  q31_t fcurnt3, fnext3, gnext3;                 /* temporary variables for third sample in loop unrolling */
  q31_t fcurnt4, fnext4, gnext4;                 /* temporary variables for fourth sample in loop unrolling */
  uint32_t numStages = S->numStages;             /* Number of stages in the filter */
  uint32_t blkCnt, stageCnt;                     /* temporary variables for counts */

  pState = &S->pState[0];

  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {

    /* Read two samples from input buffer */
    /* f0(n) = x(n) */
    fcurnt1 = *pSrc++;
    fcurnt2 = *pSrc++;

    /* Initialize coeff pointer */
    pk = (pCoeffs);

    /* Initialize state pointer */
    px = pState;

    /* Read g0(n-1) from state */
    gcurnt1 = *px;

    /* Process first sample for first tap */
    /* f1(n) = f0(n) +  K1 * g0(n-1) */
    fnext1 = (q31_t) ((gcurnt1 * (*pk)) >> 15U) + fcurnt1;
    fnext1 = __SSAT(fnext1, 16);

    /* g1(n) = f0(n) * K1  +  g0(n-1) */
    gnext1 = (q31_t) ((fcurnt1 * (*pk)) >> 15U) + gcurnt1;
    gnext1 = __SSAT(gnext1, 16);

    /* Process second sample for first tap */
    /* for sample 2 processing */
    fnext2 = (q31_t) ((fcurnt1 * (*pk)) >> 15U) + fcurnt2;
    fnext2 = __SSAT(fnext2, 16);

    gnext2 = (q31_t) ((fcurnt2 * (*pk)) >> 15U) + fcurnt1;
    gnext2 = __SSAT(gnext2, 16);


    /* Read next two samples from input buffer */
    /* f0(n+2) = x(n+2) */
    fcurnt3 = *pSrc++;
    fcurnt4 = *pSrc++;

    /* Copy only last input samples into the state buffer
       which is used for next four samples processing */
    *px++ = (q15_t) fcurnt4;

    /* Process third sample for first tap */
    fnext3 = (q31_t) ((fcurnt2 * (*pk)) >> 15U) + fcurnt3;
    fnext3 = __SSAT(fnext3, 16);
    gnext3 = (q31_t) ((fcurnt3 * (*pk)) >> 15U) + fcurnt2;
    gnext3 = __SSAT(gnext3, 16);

    /* Process fourth sample for first tap */
    fnext4 = (q31_t) ((fcurnt3 * (*pk)) >> 15U) + fcurnt4;
    fnext4 = __SSAT(fnext4, 16);
    gnext4 = (q31_t) ((fcurnt4 * (*pk++)) >> 15U) + fcurnt3;
    gnext4 = __SSAT(gnext4, 16);

    /* Update of f values for next coefficient set processing */
    fcurnt1 = fnext1;
    fcurnt2 = fnext2;
    fcurnt3 = fnext3;
    fcurnt4 = fnext4;


    /* Loop unrolling.  Process 4 taps at a time . */
    stageCnt = (numStages - 1U) >> 2;


    /* Loop over the number of taps.  Unroll by a factor of 4.
     ** Repeat until we've computed numStages-3 coefficients. */

    /* Process 2nd, 3rd, 4th and 5th taps ... here */
    while (stageCnt > 0U)
    {
      /* Read g1(n-1), g3(n-1) .... from state */
      gcurnt1 = *px;

      /* save g1(n) in state buffer */
      *px++ = (q15_t) gnext4;

      /* Process first sample for 2nd, 6th .. tap */
      /* Sample processing for K2, K6.... */
      /* f1(n) = f0(n) +  K1 * g0(n-1) */
      fnext1 = (q31_t) ((gcurnt1 * (*pk)) >> 15U) + fcurnt1;
      fnext1 = __SSAT(fnext1, 16);


      /* Process second sample for 2nd, 6th .. tap */
      /* for sample 2 processing */
      fnext2 = (q31_t) ((gnext1 * (*pk)) >> 15U) + fcurnt2;
      fnext2 = __SSAT(fnext2, 16);
      /* Process third sample for 2nd, 6th .. tap */
      fnext3 = (q31_t) ((gnext2 * (*pk)) >> 15U) + fcurnt3;
      fnext3 = __SSAT(fnext3, 16);
      /* Process fourth sample for 2nd, 6th .. tap */
      /* fnext4 = fcurnt4 + (*pk) * gnext3; */
      fnext4 = (q31_t) ((gnext3 * (*pk)) >> 15U) + fcurnt4;
      fnext4 = __SSAT(fnext4, 16);

      /* g1(n) = f0(n) * K1  +  g0(n-1) */
      /* Calculation of state values for next stage */
      gnext4 = (q31_t) ((fcurnt4 * (*pk)) >> 15U) + gnext3;
      gnext4 = __SSAT(gnext4, 16);
      gnext3 = (q31_t) ((fcurnt3 * (*pk)) >> 15U) + gnext2;
      gnext3 = __SSAT(gnext3, 16);

      gnext2 = (q31_t) ((fcurnt2 * (*pk)) >> 15U) + gnext1;
      gnext2 = __SSAT(gnext2, 16);

      gnext1 = (q31_t) ((fcurnt1 * (*pk++)) >> 15U) + gcurnt1;
      gnext1 = __SSAT(gnext1, 16);


      /* Read g2(n-1), g4(n-1) .... from state */
      gcurnt1 = *px;

      /* save g1(n) in state buffer */
      *px++ = (q15_t) gnext4;

      /* Sample processing for K3, K7.... */
      /* Process first sample for 3rd, 7th .. tap */
      /* f3(n) = f2(n) +  K3 * g2(n-1) */
      fcurnt1 = (q31_t) ((gcurnt1 * (*pk)) >> 15U) + fnext1;
      fcurnt1 = __SSAT(fcurnt1, 16);

      /* Process second sample for 3rd, 7th .. tap */
      fcurnt2 = (q31_t) ((gnext1 * (*pk)) >> 15U) + fnext2;
      fcurnt2 = __SSAT(fcurnt2, 16);

      /* Process third sample for 3rd, 7th .. tap */
      fcurnt3 = (q31_t) ((gnext2 * (*pk)) >> 15U) + fnext3;
      fcurnt3 = __SSAT(fcurnt3, 16);

      /* Process fourth sample for 3rd, 7th .. tap */
      fcurnt4 = (q31_t) ((gnext3 * (*pk)) >> 15U) + fnext4;
      fcurnt4 = __SSAT(fcurnt4, 16);

      /* Calculation of state values for next stage */
      /* g3(n) = f2(n) * K3  +  g2(n-1) */
      gnext4 = (q31_t) ((fnext4 * (*pk)) >> 15U) + gnext3;
      gnext4 = __SSAT(gnext4, 16);

      gnext3 = (q31_t) ((fnext3 * (*pk)) >> 15U) + gnext2;
      gnext3 = __SSAT(gnext3, 16);

      gnext2 = (q31_t) ((fnext2 * (*pk)) >> 15U) + gnext1;
      gnext2 = __SSAT(gnext2, 16);

      gnext1 = (q31_t) ((fnext1 * (*pk++)) >> 15U) + gcurnt1;
      gnext1 = __SSAT(gnext1, 16);

      /* Read g1(n-1), g3(n-1) .... from state */
      gcurnt1 = *px;

      /* save g1(n) in state buffer */
      *px++ = (q15_t) gnext4;

      /* Sample processing for K4, K8.... */
      /* Process first sample for 4th, 8th .. tap */
      /* f4(n) = f3(n) +  K4 * g3(n-1) */
      fnext1 = (q31_t) ((gcurnt1 * (*pk)) >> 15U) + fcurnt1;
      fnext1 = __SSAT(fnext1, 16);

      /* Process second sample for 4th, 8th .. tap */
      /* for sample 2 processing */
      fnext2 = (q31_t) ((gnext1 * (*pk)) >> 15U) + fcurnt2;
      fnext2 = __SSAT(fnext2, 16);

      /* Process third sample for 4th, 8th .. tap */
      fnext3 = (q31_t) ((gnext2 * (*pk)) >> 15U) + fcurnt3;
      fnext3 = __SSAT(fnext3, 16);

      /* Process fourth sample for 4th, 8th .. tap */
      fnext4 = (q31_t) ((gnext3 * (*pk)) >> 15U) + fcurnt4;
      fnext4 = __SSAT(fnext4, 16);

      /* g4(n) = f3(n) * K4  +  g3(n-1) */
      /* Calculation of state values for next stage */
      gnext4 = (q31_t) ((fcurnt4 * (*pk)) >> 15U) + gnext3;
      gnext4 = __SSAT(gnext4, 16);

      gnext3 = (q31_t) ((fcurnt3 * (*pk)) >> 15U) + gnext2;
      gnext3 = __SSAT(gnext3, 16);

      gnext2 = (q31_t) ((fcurnt2 * (*pk)) >> 15U) + gnext1;
      gnext2 = __SSAT(gnext2, 16);
      gnext1 = (q31_t) ((fcurnt1 * (*pk++)) >> 15U) + gcurnt1;
      gnext1 = __SSAT(gnext1, 16);


      /* Read g2(n-1), g4(n-1) .... from state */
      gcurnt1 = *px;

      /* save g4(n) in state buffer */
      *px++ = (q15_t) gnext4;

      /* Sample processing for K5, K9.... */
      /* Process first sample for 5th, 9th .. tap */
      /* f5(n) = f4(n) +  K5 * g4(n-1) */
      fcurnt1 = (q31_t) ((gcurnt1 * (*pk)) >> 15U) + fnext1;
      fcurnt1 = __SSAT(fcurnt1, 16);

      /* Process second sample for 5th, 9th .. tap */
      fcurnt2 = (q31_t) ((gnext1 * (*pk)) >> 15U) + fnext2;
      fcurnt2 = __SSAT(fcurnt2, 16);

      /* Process third sample for 5th, 9th .. tap */
      fcurnt3 = (q31_t) ((gnext2 * (*pk)) >> 15U) + fnext3;
      fcurnt3 = __SSAT(fcurnt3, 16);

      /* Process fourth sample for 5th, 9th .. tap */
      fcurnt4 = (q31_t) ((gnext3 * (*pk)) >> 15U) + fnext4;
      fcurnt4 = __SSAT(fcurnt4, 16);

      /* Calculation of state values for next stage */
      /* g5(n) = f4(n) * K5  +  g4(n-1) */
      gnext4 = (q31_t) ((fnext4 * (*pk)) >> 15U) + gnext3;
      gnext4 = __SSAT(gnext4, 16);
      gnext3 = (q31_t) ((fnext3 * (*pk)) >> 15U) + gnext2;
      gnext3 = __SSAT(gnext3, 16);
      gnext2 = (q31_t) ((fnext2 * (*pk)) >> 15U) + gnext1;
      gnext2 = __SSAT(gnext2, 16);
      gnext1 = (q31_t) ((fnext1 * (*pk++)) >> 15U) + gcurnt1;
      gnext1 = __SSAT(gnext1, 16);

      stageCnt--;
    }

    /* If the (filter length -1) is not a multiple of 4, compute the remaining filter taps */
    stageCnt = (numStages - 1U) % 0x4U;

    while (stageCnt > 0U)
    {
      gcurnt1 = *px;

      /* save g value in state buffer */
      *px++ = (q15_t) gnext4;

      /* Process four samples for last three taps here */
      fnext1 = (q31_t) ((gcurnt1 * (*pk)) >> 15U) + fcurnt1;
      fnext1 = __SSAT(fnext1, 16);
      fnext2 = (q31_t) ((gnext1 * (*pk)) >> 15U) + fcurnt2;
      fnext2 = __SSAT(fnext2, 16);

      fnext3 = (q31_t) ((gnext2 * (*pk)) >> 15U) + fcurnt3;
      fnext3 = __SSAT(fnext3, 16);

      fnext4 = (q31_t) ((gnext3 * (*pk)) >> 15U) + fcurnt4;
      fnext4 = __SSAT(fnext4, 16);

      /* g1(n) = f0(n) * K1  +  g0(n-1) */
      gnext4 = (q31_t) ((fcurnt4 * (*pk)) >> 15U) + gnext3;
      gnext4 = __SSAT(gnext4, 16);
      gnext3 = (q31_t) ((fcurnt3 * (*pk)) >> 15U) + gnext2;
      gnext3 = __SSAT(gnext3, 16);
      gnext2 = (q31_t) ((fcurnt2 * (*pk)) >> 15U) + gnext1;
      gnext2 = __SSAT(gnext2, 16);
      gnext1 = (q31_t) ((fcurnt1 * (*pk++)) >> 15U) + gcurnt1;
      gnext1 = __SSAT(gnext1, 16);

      /* Update of f values for next coefficient set processing */
      fcurnt1 = fnext1;
      fcurnt2 = fnext2;
      fcurnt3 = fnext3;
      fcurnt4 = fnext4;

      stageCnt--;

    }

    /* The results in the 4 accumulators, store in the destination buffer. */
    /* y(n) = fN(n) */

#ifndef  ARM_MATH_BIG_ENDIAN

    *__SIMD32(pDst)++ = __PKHBT(fcurnt1, fcurnt2, 16);
    *__SIMD32(pDst)++ = __PKHBT(fcurnt3, fcurnt4, 16);

#else

    *__SIMD32(pDst)++ = __PKHBT(fcurnt2, fcurnt1, 16);
    *__SIMD32(pDst)++ = __PKHBT(fcurnt4, fcurnt3, 16);

#endif /*      #ifndef  ARM_MATH_BIG_ENDIAN    */

    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

  while (blkCnt > 0U)
  {
    /* f0(n) = x(n) */
    fcurnt1 = *pSrc++;

    /* Initialize coeff pointer */
    pk = (pCoeffs);

    /* Initialize state pointer */
    px = pState;

    /* read g2(n) from state buffer */
    gcurnt1 = *px;

    /* for sample 1 processing */
    /* f1(n) = f0(n) +  K1 * g0(n-1) */
    fnext1 = (((q31_t) gcurnt1 * (*pk)) >> 15U) + fcurnt1;
    fnext1 = __SSAT(fnext1, 16);


    /* g1(n) = f0(n) * K1  +  g0(n-1) */
    gnext1 = (((q31_t) fcurnt1 * (*pk++)) >> 15U) + gcurnt1;
    gnext1 = __SSAT(gnext1, 16);

    /* save g1(n) in state buffer */
    *px++ = (q15_t) fcurnt1;

    /* f1(n) is saved in fcurnt1
       for next stage processing */
    fcurnt1 = fnext1;

    stageCnt = (numStages - 1U);

    /* stage loop */
    while (stageCnt > 0U)
    {
      /* read g2(n) from state buffer */
      gcurnt1 = *px;

      /* save g1(n) in state buffer */
      *px++ = (q15_t) gnext1;

      /* Sample processing for K2, K3.... */
      /* f2(n) = f1(n) +  K2 * g1(n-1) */
      fnext1 = (((q31_t) gcurnt1 * (*pk)) >> 15U) + fcurnt1;
      fnext1 = __SSAT(fnext1, 16);

      /* g2(n) = f1(n) * K2  +  g1(n-1) */
      gnext1 = (((q31_t) fcurnt1 * (*pk++)) >> 15U) + gcurnt1;
      gnext1 = __SSAT(gnext1, 16);


      /* f1(n) is saved in fcurnt1
         for next stage processing */
      fcurnt1 = fnext1;

      stageCnt--;

    }

    /* y(n) = fN(n) */
    *pDst++ = __SSAT(fcurnt1, 16);


    blkCnt--;

  }

#else

  /* Run the below code for Cortex-M0 */

  q31_t fcurnt, fnext, gcurnt, gnext;            /* temporary variables */
  uint32_t numStages = S->numStages;             /* Length of the filter */
  uint32_t blkCnt, stageCnt;                     /* temporary variables for counts */

  pState = &S->pState[0];

  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* f0(n) = x(n) */
    fcurnt = *pSrc++;

    /* Initialize coeff pointer */
    pk = (pCoeffs);

    /* Initialize state pointer */
    px = pState;

    /* read g0(n-1) from state buffer */
    gcurnt = *px;

    /* for sample 1 processing */
    /* f1(n) = f0(n) +  K1 * g0(n-1) */
    fnext = ((gcurnt * (*pk)) >> 15U) + fcurnt;
    fnext = __SSAT(fnext, 16);


    /* g1(n) = f0(n) * K1  +  g0(n-1) */
    gnext = ((fcurnt * (*pk++)) >> 15U) + gcurnt;
    gnext = __SSAT(gnext, 16);

    /* save f0(n) in state buffer */
    *px++ = (q15_t) fcurnt;

    /* f1(n) is saved in fcurnt
       for next stage processing */
    fcurnt = fnext;

    stageCnt = (numStages - 1U);

    /* stage loop */
    while (stageCnt > 0U)
    {
      /* read g1(n-1) from state buffer */
      gcurnt = *px;

      /* save g0(n-1) in state buffer */
      *px++ = (q15_t) gnext;

      /* Sample processing for K2, K3.... */
      /* f2(n) = f1(n) +  K2 * g1(n-1) */
      fnext = ((gcurnt * (*pk)) >> 15U) + fcurnt;
      fnext = __SSAT(fnext, 16);

      /* g2(n) = f1(n) * K2  +  g1(n-1) */
      gnext = ((fcurnt * (*pk++)) >> 15U) + gcurnt;
      gnext = __SSAT(gnext, 16);


      /* f1(n) is saved in fcurnt
         for next stage processing */
      fcurnt = fnext;

      stageCnt--;

    }

    /* y(n) = fN(n) */
    *pDst++ = __SSAT(fcurnt, 16);


    blkCnt--;

  }

#endif /*   #if defined (ARM_MATH_DSP) */

}

/**
 * @} end of FIR_Lattice group
 */
