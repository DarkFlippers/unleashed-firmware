#include "ref.h"

void ref_fir_interpolate_f32(
  const arm_fir_interpolate_instance_f32 * S,
  float32_t * pSrc,
  float32_t * pDst,
  uint32_t blockSize)
{
  float32_t *pState = S->pState;                 /* State pointer */
  float32_t *pCoeffs = S->pCoeffs;               /* Coefficient pointer */
  float32_t *pStateCurnt;                        /* Points to the current sample of the state */
  float32_t *ptr1, *ptr2;                        /* Temporary pointers for state and coefficient buffers */
  float32_t sum;                                 /* Accumulator */
  uint32_t i, blkCnt;                            /* Loop counters */
  uint16_t phaseLen = S->phaseLength, tapCnt;    /* Length of each polyphase filter component */


  /* S->pState buffer contains previous frame (phaseLen - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + phaseLen - 1;

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
      sum = 0.0f;

      /* Initialize state pointer */
      ptr1 = pState;

      /* Initialize coefficient pointer */
      ptr2 = pCoeffs + i - 1;

      /* Loop over the polyPhase length */
      tapCnt = phaseLen;

      while (tapCnt > 0U)
      {
        /* Perform the multiply-accumulate */
        sum += *ptr1++ * *ptr2;

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Decrement the loop counter */
        tapCnt--;
      }

      /* The result is in the accumulator, store in the destination buffer. */
      *pDst++ = sum;

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
   ** Now copy the last phaseLen - 1 samples to the start of the state buffer.       
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  tapCnt = phaseLen - 1U;

  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }

}

void ref_fir_interpolate_q31(
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
  pStateCurnt = S->pState + (q31_t)phaseLen - 1;

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
      ptr2 = pCoeffs + i - 1;

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
      *pDst++ = (q31_t)(sum >> 31);

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

void ref_fir_interpolate_q15(
  const arm_fir_interpolate_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer                                            */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer                                      */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state                */
  q15_t *ptr1, *ptr2;                            /* Temporary pointers for state and coefficient buffers     */
  q63_t sum;                                     /* Accumulator */
  q15_t x0, c0;                                  /* Temporary variables to hold state and coefficient values */
  uint32_t i, blkCnt, tapCnt;                    /* Loop counters                                            */
  uint16_t phaseLen = S->phaseLength;            /* Length of each polyphase filter component */


  /* S->pState buffer contains previous frame (phaseLen - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + phaseLen - 1;

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
      ptr2 = pCoeffs + i - 1;

      /* Loop over the polyPhase length */
      tapCnt = (uint32_t)phaseLen;

      while (tapCnt > 0U)
      {
        /* Read the coefficient */
        c0 = *ptr2;

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *ptr1++;

        /* Perform the multiply-accumulate */
        sum += (q31_t) x0 * c0;

        /* Decrement the loop counter */
        tapCnt--;
      }

      /* Store the result after converting to 1.15 format in the destination buffer */
      *pDst++ = ref_sat_q15(sum >> 15);

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
   ** Now copy the last phaseLen - 1 samples to the start of the state buffer.       
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  i = (uint32_t) phaseLen - 1U;

  while (i > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    i--;
  }

}
