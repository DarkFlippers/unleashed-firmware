#include "ref.h"

void ref_conv_f32(
  float32_t * pSrcA,
  uint32_t 		srcALen,
  float32_t * pSrcB,
  uint32_t 		srcBLen,
  float32_t * pDst)
{
  float32_t sum;                                 /* Accumulator */
  uint32_t i, j;                                 /* loop counters */

  /* Loop to calculate convolution for output length number of times */
  for (i = 0; i < srcALen + srcBLen - 1; i++)
  {
    /* Initialize sum with zero to carry out MAC operations */
    sum = 0.0f;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0; j <= i; j++)
    {
      /* Check the array limitations */
      if ((i - j < srcBLen) && (j < srcALen))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += pSrcB[i - j] * pSrcA[j];
      }
    }
    /* Store the output in the destination buffer */
    pDst[i] = sum;
  }
}

arm_status ref_conv_partial_f32(
  float32_t * pSrcA,
  uint32_t srcALen,
  float32_t * pSrcB,
  uint32_t srcBLen,
  float32_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints)
{
	ref_conv_f32(pSrcA,srcALen,pSrcB,srcBLen,pDst);
	
	return ARM_MATH_SUCCESS;
}

void ref_conv_q31(
  q31_t * pSrcA,
  uint32_t srcALen,
  q31_t * pSrcB,
  uint32_t srcBLen,
  q31_t * pDst)
{
  q63_t sum;                                     /* Accumulator */
  uint32_t i, j;                                 /* loop counter */

  /* Loop to calculate output of convolution for output length number of times */
  for (i = 0; i < srcALen + srcBLen - 1; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0; j <= i; j++)
    {
      /* Check the array limitations */
      if ((i - j < srcBLen) && (j < srcALen))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += (q63_t) pSrcA[j] * (pSrcB[i - j]);
      }
    }

    /* Store the output in the destination buffer */
    pDst[i] = (q31_t)(sum >> 31U);
  }
}

void ref_conv_fast_q31(
  q31_t * pSrcA,
  uint32_t srcALen,
  q31_t * pSrcB,
  uint32_t srcBLen,
  q31_t * pDst)
{
  q31_t sum;                                     /* Accumulator */
  uint32_t i, j;                                 /* loop counter */

  /* Loop to calculate output of convolution for output length number of times */
  for (i = 0; i < srcALen + srcBLen - 1; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0; j <= i; j++)
    {
      /* Check the array limitations */
      if ((i - j < srcBLen) && (j < srcALen))
      {
        /* z[i] += x[i-j] * y[j] */
				sum = (q31_t) ((((q63_t)sum << 32) +
                      ((q63_t)pSrcA[j] * pSrcB[i - j])) >> 32);
      }
    }

    /* Store the output in the destination buffer */
    pDst[i] = (q31_t)(sum << 1U);
  }
}

arm_status ref_conv_partial_q31(
  q31_t * pSrcA,
  uint32_t srcALen,
  q31_t * pSrcB,
  uint32_t srcBLen,
  q31_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints)
{
	ref_conv_q31(pSrcA,srcALen,pSrcB,srcBLen,pDst);
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_conv_partial_fast_q31(
  q31_t * pSrcA,
  uint32_t srcALen,
  q31_t * pSrcB,
  uint32_t srcBLen,
  q31_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints)
{
	ref_conv_fast_q31(pSrcA,srcALen,pSrcB,srcBLen,pDst);
   
	return ARM_MATH_SUCCESS;
}

void ref_conv_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst)
{
  q63_t sum;                                     /* Accumulator */
  uint32_t i, j;                                 /* loop counter */

  /* Loop to calculate output of convolution for output length number of times */
  for (i = 0; i < srcALen + srcBLen - 1; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0; j <= i; j++)
    {
      /* Check the array limitations */
      if ((i - j < srcBLen) && (j < srcALen))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += (q31_t)pSrcA[j] * pSrcB[i - j];
      }
    }

    /* Store the output in the destination buffer */
    pDst[i] = ref_sat_q15(sum >> 15U);
  }
}

arm_status ref_conv_partial_fast_opt_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints,
  q15_t * pScratch1,
  q15_t * pScratch2)
{
  q31_t sum;                                     /* Accumulator */
  uint32_t i, j;                                 /* loop counter */

  /* Loop to calculate output of convolution for output length number of times */
  for (i = 0; i < srcALen + srcBLen - 1; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0; j <= i; j++)
    {
      /* Check the array limitations */
      if ((i - j < srcBLen) && (j < srcALen))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += (q31_t)pSrcA[j] * pSrcB[i - j];
      }
    }

    /* Store the output in the destination buffer */
    pDst[i] = ref_sat_q15(sum >> 15U);
  }
	
  return ARM_MATH_SUCCESS;
}

void ref_conv_fast_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst)
{
  q31_t sum;                                     /* Accumulator */
  uint32_t i, j;                                 /* loop counter */

  /* Loop to calculate output of convolution for output length number of times */
  for (i = 0; i < srcALen + srcBLen - 1; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0; j <= i; j++)
    {
      /* Check the array limitations */
      if ((i - j < srcBLen) && (j < srcALen))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += (q31_t)pSrcA[j] * pSrcB[i - j];
      }
    }

    /* Store the output in the destination buffer */
    pDst[i] = sum >> 15U;
  }
}

void ref_conv_fast_opt_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst,
  q15_t * pScratch1,
  q15_t * pScratch2)
{
  q31_t sum;                                     /* Accumulator */
  uint32_t i, j;                                 /* loop counter */

  /* Loop to calculate output of convolution for output length number of times */
  for (i = 0; i < srcALen + srcBLen - 1; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0; j <= i; j++)
    {
      /* Check the array limitations */
      if ((i - j < srcBLen) && (j < srcALen))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += (q31_t)pSrcA[j] * pSrcB[i - j];
      }
    }

    /* Store the output in the destination buffer */
    pDst[i] = ref_sat_q15(sum >> 15U);
  }
}

arm_status ref_conv_partial_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints)
{
	ref_conv_q15(pSrcA,srcALen,pSrcB,srcBLen,pDst);
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_conv_partial_fast_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints)
{
	ref_conv_fast_q15(pSrcA,srcALen,pSrcB,srcBLen,pDst);
	
	return ARM_MATH_SUCCESS;
}


void ref_conv_q7(
  q7_t * pSrcA,
  uint32_t srcALen,
  q7_t * pSrcB,
  uint32_t srcBLen,
  q7_t * pDst)
{
  q31_t sum;                                     /* Accumulator */
  uint32_t i, j;                                 /* loop counter */

  /* Loop to calculate output of convolution for output length number of times */
  for (i = 0; i < srcALen + srcBLen - 1; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0; j <= i; j++)
    {
      /* Check the array limitations */
      if ((i - j < srcBLen) && (j < srcALen))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += (q15_t)pSrcA[j] * pSrcB[i - j];
      }
    }

    /* Store the output in the destination buffer */
    pDst[i] = (q7_t)ref_sat_q7(sum >> 7);
  }
}

arm_status ref_conv_partial_q7(
  q7_t * pSrcA,
  uint32_t srcALen,
  q7_t * pSrcB,
  uint32_t srcBLen,
  q7_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints)
{	
	ref_conv_q7(pSrcA,srcALen,pSrcB,srcBLen,pDst);
	
	return ARM_MATH_SUCCESS;
}
