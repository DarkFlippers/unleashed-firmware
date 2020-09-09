#include "ref.h"

void ref_fir_decimate_f32(
  const arm_fir_decimate_instance_f32 * S,
  float32_t * pSrc,
  float32_t * pDst,
  uint32_t blockSize)
{
  float32_t *pState = S->pState;                 /* State pointer */
  float32_t *pCoeffs = S->pCoeffs;               /* Coefficient pointer */
  float32_t *pStateCurnt;                        /* Points to the current sample of the state */
  float32_t sum0;                                /* Accumulator */
  float32_t x0, c0;                              /* Temporary variables to hold state and coefficient values */
  uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
  uint32_t i, blkCnt;  									 				 /* Loop counters */

  /* S->pState buffer contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + numTaps - 1U;

  /* Total number of output samples to be computed */
  blkCnt = blockSize / S->M;

  while (blkCnt > 0U)
  {
    /* Copy decimation factor number of new input samples into the state buffer */
    i = S->M;

    do
    {
      *pStateCurnt++ = *pSrc++;
    } while (--i);

    /* Set accumulator to zero */
    sum0 = 0.0f;

		for(i=0;i<numTaps;i++)
		{
      /* Read coefficients */
      c0 = pCoeffs[i];

      /* Fetch 1 state variable */
      x0 = pState[i];

      /* Perform the multiply-accumulate */
      sum0 += x0 * c0;
		}

    /* Advance the state pointer by the decimation factor           
     * to process the next group of decimation factor number samples */
    pState += S->M;

    /* The result is in the accumulator, store in the destination buffer. */
    *pDst++ = sum0;

    /* Decrement the loop counter */
    blkCnt--;
  }
  /* Processing is complete.         
   ** Now copy the last numTaps - 1 samples to the start of the state buffer.       
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  /* Copy numTaps number of values */
  i = numTaps - 1U;

  /* copy data */
  while (i > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    i--;
  }
}

void ref_fir_decimate_q31(
  const arm_fir_decimate_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t *pState = S->pState;                     /* State pointer */
  q31_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q31_t *pStateCurnt;                            /* Points to the current sample of the state */
  q31_t x0, c0;                                  /* Temporary variables to hold state and coefficient values */
  q63_t sum0;                                    /* Accumulator */
  uint32_t numTaps = S->numTaps;                 /* Number of taps */
  uint32_t i, blkCnt;  													 /* Loop counters */

  /* S->pState buffer contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + numTaps - 1U;

  /* Total number of output samples to be computed */
  blkCnt = blockSize / S->M;

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

		for(i=0;i<numTaps;i++)
		{
      /* Read coefficients */
      c0 = pCoeffs[i];

      /* Fetch 1 state variable */
      x0 = pState[i];

      /* Perform the multiply-accumulate */
      sum0 += (q63_t)x0 * c0;
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
}

void ref_fir_decimate_fast_q31(
  const arm_fir_decimate_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t *pState = S->pState;                     /* State pointer */
  q31_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q31_t *pStateCurnt;                            /* Points to the current sample of the state */
  q31_t x0, c0;                                  /* Temporary variables to hold state and coefficient values */
  q31_t sum0;                                    /* Accumulator */
  uint32_t numTaps = S->numTaps;                 /* Number of taps */
  uint32_t i, blkCnt;  													 /* Loop counters */

  /* S->pState buffer contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + numTaps - 1U;

  /* Total number of output samples to be computed */
  blkCnt = blockSize / S->M;

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

		for(i=0;i<numTaps;i++)
		{
      /* Read coefficients */
      c0 = pCoeffs[i];

      /* Fetch 1 state variable */
      x0 = pState[i];

      /* Perform the multiply-accumulate */
			sum0 = (q31_t)((((q63_t) sum0 << 32) + ((q63_t) x0 * c0)) >> 32);
		}

    /* Advance the state pointer by the decimation factor           
     * to process the next group of decimation factor number samples */
    pState = pState + S->M;

    /* The result is in the accumulator, store in the destination buffer. */
    *pDst++ = (q31_t) (sum0 << 1);

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

void ref_fir_decimate_q15(
  const arm_fir_decimate_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state */
  q31_t x0, c0;                                  /* Temporary variables to hold state and coefficient values */
  q63_t sum0;                                    /* Accumulator */
  uint32_t numTaps = S->numTaps;                 /* Number of taps */
  uint32_t i, blkCnt;  													 /* Loop counters */

  /* S->pState buffer contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + numTaps - 1U;

  /* Total number of output samples to be computed */
  blkCnt = blockSize / S->M;

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

		for(i=0;i<numTaps;i++)
		{
      /* Read coefficients */
      c0 = pCoeffs[i];

      /* Fetch 1 state variable */
      x0 = pState[i];

      /* Perform the multiply-accumulate */
      sum0 += (q31_t)x0 * c0;
		}

    /* Advance the state pointer by the decimation factor           
     * to process the next group of decimation factor number samples */
    pState = pState + S->M;

    /* The result is in the accumulator, store in the destination buffer. */
    *pDst++ = ref_sat_q15(sum0 >> 15);

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

void ref_fir_decimate_fast_q15(
  const arm_fir_decimate_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state */
  q15_t x0, c0;                                  /* Temporary variables to hold state and coefficient values */
  q31_t sum0;                                    /* Accumulator */
  uint32_t numTaps = S->numTaps;                 /* Number of taps */
  uint32_t i, blkCnt;  													 /* Loop counters */

  /* S->pState buffer contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + numTaps - 1U;

  /* Total number of output samples to be computed */
  blkCnt = blockSize / S->M;

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

		for(i=0;i<numTaps;i++)
		{
      /* Read coefficients */
      c0 = pCoeffs[i];

      /* Fetch 1 state variable */
      x0 = pState[i];

      /* Perform the multiply-accumulate */
      sum0 += x0 * c0;
		}

    /* Advance the state pointer by the decimation factor           
     * to process the next group of decimation factor number samples */
    pState = pState + S->M;

    /* The result is in the accumulator, store in the destination buffer. */
    *pDst++ = ref_sat_q15(sum0 >> 15);

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

