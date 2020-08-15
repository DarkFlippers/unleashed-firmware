#include "ref.h"

void ref_lms_f32(
  const arm_lms_instance_f32 * S,
  float32_t * pSrc,
  float32_t * pRef,
  float32_t * pOut,
  float32_t * pErr,
  uint32_t blockSize)
{
  float32_t *pState = S->pState;                 /* State pointer */
  float32_t *pCoeffs = S->pCoeffs;               /* Coefficient pointer */
  float32_t *pStateCurnt;                        /* Points to the current sample of the state */
  float32_t mu = S->mu;                          /* Adaptive factor */
  uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
  uint32_t i, blkCnt;                     			 /* Loop counters */
  float32_t sum, e, d;                           /* accumulator, error, reference data sample */
  float32_t w = 0.0f;                            /* weight factor */

  e = 0.0f;
  d = 0.0f;

  /* S->pState points to state array which contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = &(S->pState[numTaps - 1U]);

  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* Copy the new input sample into the state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Set the accumulator to zero */
    sum = 0.0f;
		
		for(i=0;i<numTaps;i++)
		{ /* Perform the multiply-accumulate */
      sum += pState[i] * pCoeffs[i];
		}

    /* The result is stored in the destination buffer. */
    *pOut++ = sum;

    /* Compute and store error */
    d = *pRef++;
    e = d - sum;
    *pErr++ = e;

    /* Weighting factor for the LMS version */
    w = e * mu;
		
		for(i=0;i<numTaps;i++)
		{ /* Perform the multiply-accumulate */
      pCoeffs[i] += w * pState[i];
		}

    /* Advance state pointer by 1 for the next sample */
    pState++;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Processing is complete. Now copy the last numTaps - 1 samples to the        
   * start of the state buffer. This prepares the state buffer for the        
   * next function call. */
	for(i=0;i<numTaps-1;i++)
  {
    S->pState[i] = pState[i];
  }
}

void ref_lms_norm_f32(
  arm_lms_norm_instance_f32 * S,
  float32_t * pSrc,
  float32_t * pRef,
  float32_t * pOut,
  float32_t * pErr,
  uint32_t blockSize)
{
  float32_t *pState = S->pState;                 /* State pointer */
  float32_t *pCoeffs = S->pCoeffs;               /* Coefficient pointer */
  float32_t *pStateCurnt;                        /* Points to the current sample of the state */
  float32_t mu = S->mu;                          /* Adaptive factor */
  uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
  uint32_t i, blkCnt;                    				 /* Loop counters */
  float32_t energy;                              /* Energy of the input */
  float32_t sum, e, d;                           /* accumulator, error, reference data sample */
  float32_t w, x0, in;                           /* weight factor, temporary variable to hold input sample and state */

  /* Initializations of error,  difference, Coefficient update */
  e = 0.0f;
  d = 0.0f;
  w = 0.0f;

  energy = S->energy;
  x0 = S->x0;

  /* S->pState points to buffer which contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = &(S->pState[numTaps - 1U]);

  for(blkCnt = blockSize; blkCnt > 0U; blkCnt--)
  {
    /* Copy the new input sample into the state buffer */
    *pStateCurnt++ = *pSrc;

    /* Read the sample from input buffer */
    in = *pSrc++;

    /* Update the energy calculation */
    energy -= x0 * x0;
    energy += in * in;

    /* Set the accumulator to zero */
    sum = 0.0f;
		
		for(i=0;i<numTaps;i++)
		{ /* Perform the multiply-accumulate */
      sum += pState[i] * pCoeffs[i];
		}

    /* The result in the accumulator is stored in the destination buffer. */
    *pOut++ = sum;

    /* Compute and store error */
    d = *pRef++;
    e = d - sum;
    *pErr++ = e;

    /* Calculation of Weighting factor for updating filter coefficients */
    /* epsilon value 0.000000119209289f */
    w = e * mu / (energy + 0.000000119209289f);

		for(i=0;i<numTaps;i++)
    {
      /* Perform the multiply-accumulate */
      pCoeffs[i] += w * pState[i];
    }

    x0 = *pState;

    /* Advance state pointer by 1 for the next sample */
    pState++;
  }

  S->energy = energy;
  S->x0 = x0;

  /* Processing is complete. Now copy the last numTaps - 1 samples to the        
   * start of the state buffer. This prepares the state buffer for the        
   * next function call. */
	for(i=0;i<numTaps-1;i++)
  {
    S->pState[i] = pState[i];
  }
}

void ref_lms_q31(
  const arm_lms_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pRef,
  q31_t * pOut,
  q31_t * pErr,
  uint32_t blockSize)
{
  q31_t *pState = S->pState;                     /* State pointer */
  uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
  q31_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q31_t *pStateCurnt;                            /* Points to the current sample of the state */
  q31_t mu = S->mu;                              /* Adaptive factor */
  q31_t *px;                                     /* Temporary pointer for state */
  q31_t *pb;                                     /* Temporary pointer for coefficient buffer */
  uint32_t tapCnt, blkCnt;                       /* Loop counters */
  q63_t acc;                                     /* Accumulator */
  q31_t e = 0;                                   /* error of data sample */
  q31_t alpha;                                   /* Intermediate constant for taps update */
  q31_t coef;                                    /* Temporary variable for coef */
  q31_t acc_l, acc_h;                            /*  temporary input */
  uint32_t uShift = (uint32_t)S->postShift + 1;
  uint32_t lShift = 32U - uShift;                /*  Shift to be applied to the output */

  /* S->pState points to buffer which contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = &(S->pState[(numTaps - 1U)]);

  for(blkCnt = blockSize; blkCnt > 0U; blkCnt--)
  {
    /* Copy the new input sample into the state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Initialize pState pointer */
    px = pState;

    /* Initialize pCoeffs pointer */
    pb = pCoeffs;

    /* Set the accumulator to zero */
    acc = 0;

    /* Loop over numTaps number of values */
    tapCnt = numTaps;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      acc += (q63_t)(*px++) * (*pb++);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Converting the result to 1.31 format */
    /* Store the result from accumulator into the destination buffer. */
    /* Calc lower part of acc */
    acc_l = acc & 0xffffffff;

    /* Calc upper part of acc */
    acc_h = (acc >> 32) & 0xffffffff;

    acc = (uint32_t)acc_l >> lShift | acc_h << uShift;

    *pOut++ = (q31_t)acc;

    /* Compute and store error */
    e = *pRef++ - (q31_t)acc;

    *pErr++ = (q31_t)e;

    /* Weighting factor for the LMS version */
    alpha = (q31_t)(((q63_t)e * mu) >> 31);

    /* Initialize pState pointer */
    /* Advance state pointer by 1 for the next sample */
    px = pState++;

    /* Initialize pCoeffs pointer */
    pb = pCoeffs;

    /* Loop over numTaps number of values */
    tapCnt = numTaps;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      coef = (q31_t)(((q63_t) alpha * (*px++)) >> 32);
      *pb = ref_sat_q31((q63_t)*pb + (coef << 1));
      pb++;

      /* Decrement the loop counter */
      tapCnt--;
    }
  }

  /* Processing is complete. Now copy the last numTaps - 1 samples to the     
     start of the state buffer. This prepares the state buffer for the   
     next function call. */

  /* Points to the start of the pState buffer */
  pStateCurnt = S->pState;

  /*  Copy (numTaps - 1U) samples  */
  tapCnt = numTaps - 1;

  /* Copy the data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }
}

void ref_lms_norm_q31(
  arm_lms_norm_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pRef,
  q31_t * pOut,
  q31_t * pErr,
  uint32_t blockSize)
{
  q31_t *pState = S->pState;                     /* State pointer */
  q31_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q31_t *pStateCurnt;                            /* Points to the current sample of the state */
  q31_t *px, *pb;                                /* Temporary pointers for state and coefficient buffers */
  q31_t mu = S->mu;                              /* Adaptive factor */
  uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
  uint32_t tapCnt, blkCnt;                       /* Loop counters */
  q63_t energy;                                  /* Energy of the input */
  q63_t acc;                                     /* Accumulator */
  q31_t e = 0, d = 0;                            /* error, reference data sample */
  q31_t w = 0, in;                               /* weight factor and state */
  q31_t x0;                                      /* temporary variable to hold input sample */   
  q63_t errorXmu;                   				 /* Temporary variables to store error and mu product and reciprocal of energy */
  q31_t coef;                                    /* Temporary variable for coef */
  q31_t acc_l, acc_h;                            /*  temporary input */
  uint32_t uShift = ((uint32_t) S->postShift + 1U);
  uint32_t lShift = 32U - uShift;                /*  Shift to be applied to the output */

  energy = S->energy;
  x0 = S->x0;

  /* S->pState points to buffer which contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = &(S->pState[(numTaps - 1U)]);

  for(blkCnt = blockSize; blkCnt > 0U; blkCnt--)
  {

    /* Copy the new input sample into the state buffer */
    *pStateCurnt++ = *pSrc;

    /* Initialize pState pointer */
    px = pState;

    /* Initialize pCoeffs pointer */
    pb = pCoeffs;

    /* Read the sample from input buffer */
    in = *pSrc++;

    /* Update the energy calculation */
    energy = (q31_t)((((q63_t)energy << 32) - (((q63_t)x0 * x0) << 1)) >> 32) & 0xffffffff;
    energy = (q31_t)(((((q63_t)in * in) << 1) + ((q63_t)energy << 32)) >> 32) & 0xffffffff;

    /* Set the accumulator to zero */
    acc = 0;

    /* Loop over numTaps number of values */
    tapCnt = numTaps;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      acc += ((q63_t) (*px++)) * (*pb++);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Converting the result to 1.31 format */
    /* Calc lower part of acc */
    acc_l = acc & 0xffffffff;

    /* Calc upper part of acc */
    acc_h = (acc >> 32) & 0xffffffff;

    acc = (uint32_t)acc_l >> lShift | acc_h << uShift;

    /* Store the result from accumulator into the destination buffer. */
    *pOut++ = (q31_t)acc;

    /* Compute and store error */
    d = *pRef++;
    e = d - (q31_t)acc;
    *pErr++ = e;

    /* Calculation of product of (e * mu) */
    errorXmu = (q63_t)e * mu;

    /* Weighting factor for the normalized version */
    w = ref_sat_q31(errorXmu / (energy + DELTA_Q31));

    /* Initialize pState pointer */
    px = pState;

    /* Initialize coeff pointer */
    pb = pCoeffs;

    /* Loop over numTaps number of values */
    tapCnt = numTaps;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      /* coef is in 2.30 format */
      coef = (q31_t)(((q63_t)w * (*px++)) >> 32);
      /* get coef in 1.31 format by left shifting */
      *pb = ref_sat_q31((q63_t)*pb + (coef << 1U));
      /* update coefficient buffer to next coefficient */
      pb++;

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Read the sample from state buffer */
    x0 = *pState;

    /* Advance state pointer by 1 for the next sample */
    pState++;
  }

  /* Save energy and x0 values for the next frame */
  S->energy = (q31_t)energy;
  S->x0 = x0;

  /* Processing is complete. Now copy the last numTaps - 1 samples to the     
     start of the state buffer. This prepares the state buffer for the        
     next function call. */

  /* Points to the start of the pState buffer */
  pStateCurnt = S->pState;

  /* Loop for (numTaps - 1U) samples copy */
  tapCnt = numTaps - 1;

  /* Copy the remaining q31_t data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }
}

void ref_lms_q15(
  const arm_lms_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pRef,
  q15_t * pOut,
  q15_t * pErr,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer */
  uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state */
  q15_t mu = S->mu;                              /* Adaptive factor */
  q15_t *px;                                     /* Temporary pointer for state */
  q15_t *pb;                                     /* Temporary pointer for coefficient buffer */
  uint32_t tapCnt, blkCnt;                       /* Loop counters */
  q63_t acc;                                     /* Accumulator */
  q15_t e = 0;                                   /* error of data sample */
  q15_t alpha;                                   /* Intermediate constant for taps update */
  q31_t coef;                                    /* Teporary variable for coefficient */
  q31_t acc_l, acc_h;
  int32_t lShift = 15 - (int32_t)S->postShift;   /*  Post shift  */
  int32_t uShift = 32 - lShift;

  /* S->pState points to buffer which contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = &(S->pState[(numTaps - 1U)]);

  for(blkCnt = blockSize; blkCnt > 0U; blkCnt--)
  {
    /* Copy the new input sample into the state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Initialize pState pointer */
    px = pState;

    /* Initialize pCoeffs pointer */
    pb = pCoeffs;

    /* Set the accumulator to zero */
    acc = 0;

    /* Loop over numTaps number of values */
    tapCnt = numTaps;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      acc += (q63_t)((q31_t)(*px++) * (*pb++));

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Calc lower part of acc */
    acc_l = acc & 0xffffffff;

    /* Calc upper part of acc */
    acc_h = (acc >> 32) & 0xffffffff;

    /* Apply shift for lower part of acc and upper part of acc */
    acc = (uint32_t)acc_l >> lShift | acc_h << uShift;

    /* Converting the result to 1.15 format and saturate the output */
    acc = ref_sat_q15(acc);

    /* Store the result from accumulator into the destination buffer. */
    *pOut++ = (q15_t)acc;

    /* Compute and store error */
    e = *pRef++ - (q15_t)acc;

    *pErr++ = (q15_t)e;

    /* Compute alpha i.e. intermediate constant for taps update */
    alpha = (q15_t)(((q31_t)e * mu) >> 15);

    /* Initialize pState pointer */
    /* Advance state pointer by 1 for the next sample */
    px = pState++;

    /* Initialize pCoeffs pointer */
    pb = pCoeffs;

    /* Loop over numTaps number of values */
    tapCnt = numTaps;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      coef = (q31_t) * pb + (((q31_t) alpha * (*px++)) >> 15);
      *pb++ = (q15_t) ref_sat_q15(coef);

      /* Decrement the loop counter */
      tapCnt--;
    }
  }

  /* Processing is complete. Now copy the last numTaps - 1 samples to the        
     start of the state buffer. This prepares the state buffer for the   
     next function call. */

  /* Points to the start of the pState buffer */
  pStateCurnt = S->pState;

  /*  Copy (numTaps - 1U) samples  */
  tapCnt = numTaps - 1;

  /* Copy the data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }
}

void ref_lms_norm_q15(
  arm_lms_norm_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pRef,
  q15_t * pOut,
  q15_t * pErr,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state */
  q15_t *px, *pb;                                /* Temporary pointers for state and coefficient buffers */
  q15_t mu = S->mu;                              /* Adaptive factor */
  uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
  uint32_t tapCnt, blkCnt;                       /* Loop counters */
  q31_t energy;                                  /* Energy of the input */
  q63_t acc;                                     /* Accumulator */
  q15_t e = 0, d = 0;                            /* error, reference data sample */
  q15_t w = 0, in;                               /* weight factor and state */
  q15_t x0;                                      /* temporary variable to hold input sample */
  q15_t errorXmu, oneByEnergy;                   /* Temporary variables to store error and mu product and reciprocal of energy */
  //q31_t errorXmu;                   				 /* Temporary variables to store error and mu product and reciprocal of energy */
  q15_t postShift;                               /* Post shift to be applied to weight after reciprocal calculation */
  q31_t coef;                                    /* Teporary variable for coefficient */
  q31_t acc_l, acc_h;
  int32_t lShift = 15 - (int32_t)S->postShift;  /*  Post shift  */
  int32_t uShift = 32 - lShift;

  energy = S->energy;
  x0 = S->x0;

  /* S->pState points to buffer which contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = &(S->pState[(numTaps - 1U)]);

  for(blkCnt = blockSize; blkCnt > 0U; blkCnt--)
  {
    /* Copy the new input sample into the state buffer */
    *pStateCurnt++ = *pSrc;

    /* Initialize pState pointer */
    px = pState;

    /* Initialize pCoeffs pointer */
    pb = pCoeffs;

    /* Read the sample from input buffer */
    in = *pSrc++;

    /* Update the energy calculation */
    energy -= (((q31_t)x0 * x0) >> 15) & 0xffff;
    energy += (((q31_t)in * in) >> 15) & 0xffff;

    /* Set the accumulator to zero */
    acc = 0;

    /* Loop over numTaps number of values */
    tapCnt = numTaps;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      acc += (q31_t)*px++ * (*pb++);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Calc lower part of acc */
    acc_l = acc & 0xffffffff;

    /* Calc upper part of acc */
    acc_h = (acc >> 32) & 0xffffffff;

    /* Apply shift for lower part of acc and upper part of acc */
    acc = (uint32_t) acc_l >> lShift | acc_h << uShift;

    /* Converting the result to 1.15 format and saturate the output */
    acc = ref_sat_q15(acc);

    /* Store the result from accumulator into the destination buffer. */
    *pOut++ = (q15_t) acc;

    /* Compute and store error */
    d = *pRef++;
    e = d - (q15_t) acc;
    *pErr++ = e;
    
#if 0
    /* Calculation of e * mu value */
    errorXmu = (q31_t) e * mu;

    /* Calculation of (e * mu) /energy value */
    acc = errorXmu / (energy + DELTA_Q15);
#endif

    /* Calculation of 1/energy */
    postShift = arm_recip_q15((q15_t) energy + DELTA_Q15,
                              &oneByEnergy, S->recipTable);

    /* Calculation of e * mu value */
    errorXmu = (q15_t) (((q31_t) e * mu) >> 15);

    /* Calculation of (e * mu) * (1/energy) value */
    acc = (((q31_t) errorXmu * oneByEnergy) >> (15 - postShift));
    
    /* Weighting factor for the normalized version */
    w = ref_sat_q15((q31_t)acc);

    /* Initialize pState pointer */
    px = pState;

    /* Initialize coeff pointer */
    pb = pCoeffs;

    /* Loop over numTaps number of values */
    tapCnt = numTaps;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      coef = *pb + (((q31_t)w * (*px++)) >> 15);
      *pb++ = ref_sat_q15(coef);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Read the sample from state buffer */
    x0 = *pState;

    /* Advance state pointer by 1 for the next sample */
    pState = pState + 1U;
  }

  /* Save energy and x0 values for the next frame */
  S->energy = (q15_t)energy;
  S->x0 = x0;

  /* Processing is complete. Now copy the last numTaps - 1 samples to the        
     satrt of the state buffer. This prepares the state buffer for the        
     next function call. */

  /* Points to the start of the pState buffer */
  pStateCurnt = S->pState;

  /* copy (numTaps - 1U) data */
  tapCnt = numTaps - 1;

  /* copy data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }
}
