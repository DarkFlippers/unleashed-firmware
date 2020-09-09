#include "ref.h"

void ref_fir_lattice_f32(
  const arm_fir_lattice_instance_f32 * S,
  float32_t * pSrc,
  float32_t * pDst,
  uint32_t blockSize)
{
  float32_t *pState;                             /* State pointer */
  float32_t *pCoeffs = S->pCoeffs;               /* Coefficient pointer */
  float32_t *px;                                 /* temporary state pointer */
  float32_t *pk;                                 /* temporary coefficient pointer */
  float32_t fcurr, fnext, gcurr, gnext;          /* temporary variables */
  uint32_t numStages = S->numStages;             /* Length of the filter */
  uint32_t blkCnt, stageCnt;                     /* temporary variables for counts */

  pState = &S->pState[0];

  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* f0(n) = x(n) */
    fcurr = *pSrc++;

    /* Initialize coeff pointer */
    pk = pCoeffs;

    /* Initialize state pointer */
    px = pState;

    /* read g0(n-1) from state buffer */
    gcurr = *px;

    /* for sample 1 processing */
    /* f1(n) = f0(n) +  K1 * g0(n-1) */
    fnext = fcurr + ((*pk) * gcurr);
    /* g1(n) = f0(n) * K1  +  g0(n-1) */
    gnext = (fcurr * (*pk++)) + gcurr;

    /* save f0(n) in state buffer */
    *px++ = fcurr;

    /* f1(n) is saved in fcurr            
       for next stage processing */
    fcurr = fnext;

    stageCnt = (numStages - 1U);

    /* stage loop */
    while (stageCnt > 0U)
    {
      /* read g2(n) from state buffer */
      gcurr = *px;

      /* save g1(n) in state buffer */
      *px++ = gnext;

      /* Sample processing for K2, K3.... */
      /* f2(n) = f1(n) +  K2 * g1(n-1) */
      fnext = fcurr + ((*pk) * gcurr);
      /* g2(n) = f1(n) * K2  +  g1(n-1) */
      gnext = (fcurr * (*pk++)) + gcurr;

      /* f1(n) is saved in fcurr1            
         for next stage processing */
      fcurr = fnext;

      stageCnt--;
    }

    /* y(n) = fN(n) */
    *pDst++ = fcurr;

    blkCnt--;
  }
}

void ref_fir_lattice_q31(
  const arm_fir_lattice_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t *pState;                                 /* State pointer */
  q31_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q31_t *px;                                     /* temporary state pointer */
  q31_t *pk;                                     /* temporary coefficient pointer */
  q31_t fcurr, fnext, gcurr, gnext;              /* temporary variables */
  uint32_t numStages = S->numStages;             /* Length of the filter */
  uint32_t blkCnt, stageCnt;                     /* temporary variables for counts */

  pState = &S->pState[0];

  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* f0(n) = x(n) */
    fcurr = *pSrc++;

    /* Initialize coeff pointer */
    pk = pCoeffs;

    /* Initialize state pointer */
    px = pState;

    /* read g0(n-1) from state buffer */
    gcurr = *px;

    /* for sample 1 processing */
    /* f1(n) = f0(n) +  K1 * g0(n-1) */
    fnext = (q31_t) (((q63_t) gcurr * (*pk)) >> 31) + fcurr;
    /* g1(n) = f0(n) * K1  +  g0(n-1) */
    gnext = (q31_t) (((q63_t) fcurr * (*pk++)) >> 31) + gcurr;
    /* save g1(n) in state buffer */
    *px++ = fcurr;

    /* f1(n) is saved in fcurr1            
       for next stage processing */
    fcurr = fnext;

    stageCnt = (numStages - 1U);

    /* stage loop */
    while (stageCnt > 0U)
    {
      /* read g2(n) from state buffer */
      gcurr = *px;

      /* save g1(n) in state buffer */
      *px++ = gnext;

      /* Sample processing for K2, K3.... */
      /* f2(n) = f1(n) +  K2 * g1(n-1) */
      fnext = (q31_t) (((q63_t) gcurr * (*pk)) >> 31) + fcurr;
      /* g2(n) = f1(n) * K2  +  g1(n-1) */
      gnext = (q31_t) (((q63_t) fcurr * (*pk++)) >> 31) + gcurr;

      /* f1(n) is saved in fcurr1            
         for next stage processing */
      fcurr = fnext;

      stageCnt--;

    }

    /* y(n) = fN(n) */
    *pDst++ = fcurr;

    blkCnt--;

  }
}

void ref_fir_lattice_q15(
  const arm_fir_lattice_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState;                                 /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *px;                                     /* temporary state pointer */
  q15_t *pk;                                     /* temporary coefficient pointer */
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
    fnext = ref_sat_q15(fnext);


    /* g1(n) = f0(n) * K1  +  g0(n-1) */
    gnext = ((fcurnt * (*pk++)) >> 15U) + gcurnt;
    gnext = ref_sat_q15(gnext);

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
      fnext = ref_sat_q15(fnext);

      /* g2(n) = f1(n) * K2  +  g1(n-1) */
      gnext = ((fcurnt * (*pk++)) >> 15U) + gcurnt;
      gnext = ref_sat_q15(gnext);


      /* f1(n) is saved in fcurnt            
         for next stage processing */
      fcurnt = fnext;

      stageCnt--;

    }

    /* y(n) = fN(n) */
    *pDst++ = ref_sat_q15(fcurnt);


    blkCnt--;

  }
}
