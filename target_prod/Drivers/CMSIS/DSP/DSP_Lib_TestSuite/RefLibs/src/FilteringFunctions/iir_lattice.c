#include "ref.h"

void ref_iir_lattice_f32(
  const arm_iir_lattice_instance_f32 * S,
  float32_t * pSrc,
  float32_t * pDst,
  uint32_t blockSize)
{
  float32_t fcurr, fnext = 0, gcurr, gnext;      /* Temporary variables for lattice stages */
  float32_t acc;                                 /* Accumlator */
  uint32_t blkCnt, tapCnt;                       /* temporary variables for counts */
  float32_t *px1, *px2, *pk, *pv;                /* temporary pointers for state and coef */
  uint32_t numStages = S->numStages;             /* number of stages */
  float32_t *pState;                             /* State pointer */
  float32_t *pStateCurnt;                        /* State current pointer */

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
    acc = 0.0f;
    /* Initialize Ladder coeff pointer */
    pv = &S->pvCoeffs[0];
    /* Initialize Reflection coeff pointer */
    pk = &S->pkCoeffs[0];

    /* Process sample for numStages */
    tapCnt = numStages;

    while (tapCnt > 0U)
    {
      gcurr = *px1++;
      /* Process sample for last taps */
      fnext = fcurr - (*pk) * gcurr;
      gnext = fnext * (*pk++) + gcurr;

      /* Output samples for last taps */
      acc += gnext * (*pv++);
      *px2++ = gnext;
      fcurr = fnext;

      /* Decrementing loop counter */
      tapCnt--;
    }

    /* y(n) += g0(n) * v0 */
    acc += fnext * (*pv);

    *px2++ = fnext;

    /* write out into pDst */
    *pDst++ = acc;

    /* Advance the state pointer by 1 to process the next group of samples */
    pState = pState + 1U;
    blkCnt--;
  }

  /* Processing is complete. Now copy last S->numStages samples to start of the buffer           
     for the preperation of next frame process */

  /* Points to the start of the state buffer */
  pStateCurnt = &S->pState[0];
  pState = &S->pState[blockSize];

  tapCnt = numStages;

  /* Copy the data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }
}

void ref_iir_lattice_q31(
  const arm_iir_lattice_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t fcurr, fnext = 0, gcurr = 0, gnext;      /* Temporary variables for lattice stages */
  q63_t acc;                                     /* Accumlator */
  uint32_t blkCnt, tapCnt;                       /* Temporary variables for counts */
  q31_t *px1, *px2, *pk, *pv;                    /* Temporary pointers for state and coef */
  uint32_t numStages = S->numStages;             /* number of stages */
  q31_t *pState;                                 /* State pointer */
  q31_t *pStateCurnt;                            /* State current pointer */

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
      fnext =
        ref_sat_q31(((q63_t) fcurr -
                         ((q31_t) (((q63_t) gcurr * (*pk)) >> 31))));
      /* gN(n) = kN * fN-1(n) + gN-1(n-1) */
      gnext =
        ref_sat_q31(((q63_t) gcurr +
                         ((q31_t) (((q63_t) fnext * (*pk++)) >> 31))));
      /* Output samples */
      /* y(n) += gN(n) * vN  */
      acc += ((q63_t) gnext * *pv++);
      /* write gN-1(n-1) into state for next sample processing */
      *px2++ = gnext;
      /* Update f values for next coefficient processing */
      fcurr = fnext;

      tapCnt--;
    }

    /* y(n) += g0(n) * v0 */
    acc += (q63_t) fnext *(*pv++);

    *px2++ = fnext;

    /* write out into pDst */
    *pDst++ = (q31_t) (acc >> 31U);

    /* Advance the state pointer by 1 to process the next group of samples */
    pState = pState + 1U;
    blkCnt--;
  }

  /* Processing is complete. Now copy last S->numStages samples to start of the buffer           
     for the preperation of next frame process */

  /* Points to the start of the state buffer */
  pStateCurnt = &S->pState[0];
  pState = &S->pState[blockSize];

  tapCnt = numStages;

  /* Copy the remaining q31_t data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }
}

void ref_iir_lattice_q15(
  const arm_iir_lattice_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
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
      fnext = ref_sat_q15(fnext);
      /* gN(n) = kN * fN-1(n) + gN-1(n-1) */
      gnext = ((fnext * (*pk++)) >> 15) + gcurr;
      gnext = ref_sat_q15(gnext);
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

    out = ref_sat_q15(acc >> 15);
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
}
