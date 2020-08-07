#include "ref.h"

void ref_correlate_f32(
  float32_t * pSrcA,
  uint32_t srcALen,
  float32_t * pSrcB,
  uint32_t srcBLen,
  float32_t * pDst)
{
  float32_t *pIn1 = pSrcA;                       /* inputA pointer 			*/
  float32_t *pIn2 = pSrcB + (srcBLen - 1U);      /* inputB pointer 			*/
  float32_t sum;                                 /* Accumulator 				*/
  uint32_t i = 0U, j;                            /* loop counters 			*/
  uint32_t inv = 0U;                             /* Reverse order flag 	*/
  uint32_t tot = 0U;                             /* Length 							*/

  /* The algorithm implementation is based on the lengths of the inputs. 
   * srcB is always made to slide across srcA. 
   * So srcBLen is always considered as shorter or equal to srcALen 
   * But CORR(x, y) is reverse of CORR(y, x) 
   * So, when srcBLen > srcALen, output pointer is made to point to the end of the output buffer 
   * and a variable, inv is set to 1 
   * If lengths are not equal then zero pad has to be done to make the two    
   * inputs of same length. But to improve the performance, we include zeroes    
   * in the output instead of zero padding either of the the inputs
   * If srcALen > srcBLen, (srcALen - srcBLen) zeroes has to included in the    
   * starting of the output buffer 
   * If srcALen < srcBLen, (srcALen - srcBLen) zeroes has to included in the   
   * ending of the output buffer 
   * Once the zero padding is done the remaining of the output is calcualted   
   * using convolution but with the shorter signal time shifted. 
	 */

  /* Calculate the length of the remaining sequence */
  tot = srcALen + srcBLen - 2U;

  if (srcALen > srcBLen)
  {
    /* Calculating the number of zeros to be padded to the output */
    /* Initialise the pointer after zero padding */
    pDst += srcALen - srcBLen;
  }
  else if (srcALen < srcBLen)
  {
    /* Initialization to inputB pointer */
    pIn1 = pSrcB;

    /* Initialization to the end of inputA pointer */
    pIn2 = pSrcA + srcALen - 1U;

    /* Initialisation of the pointer after zero padding */
    pDst += tot;

    /* Swapping the lengths */
    j = srcALen;
    srcALen = srcBLen;
    srcBLen = j;

    /* Setting the reverse flag */
    inv = 1;
  }

  /* Loop to calculate convolution for output length number of times */
  for (i = 0U; i <= tot; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0.0f;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0U; j <= i; j++)
    {
      /* Check the array limitations */
      if ((i - j < srcBLen) && (j < srcALen))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += pIn1[j] * pIn2[-((int32_t)i - j)];
      }
    }
    /* Store the output in the destination buffer */
    if (inv == 1)
      *pDst-- = sum;
    else
      *pDst++ = sum;
  }
}

void ref_correlate_q31(
  q31_t * pSrcA,
  uint32_t srcALen,
  q31_t * pSrcB,
  uint32_t srcBLen,
  q31_t * pDst)
{
  q31_t *pIn1 = pSrcA;                           /* inputA pointer               */
  q31_t *pIn2 = pSrcB + (srcBLen - 1U);          /* inputB pointer               */
  q63_t sum;                                     /* Accumulators                  */
  uint32_t i = 0U, j;                            /* loop counters */
  uint32_t inv = 0U;                             /* Reverse order flag */
  uint32_t tot = 0U;                             /* Length */

  /* Calculate the length of the remaining sequence */
  tot = ((srcALen + srcBLen) - 2U);

  if (srcALen > srcBLen)
  {
    /* Calculating the number of zeros to be padded to the output */
    j = srcALen - srcBLen;

    /* Initialise the pointer after zero padding */
    pDst += j;
  }

  else if (srcALen < srcBLen)
  {
    /* Initialization to inputB pointer */
    pIn1 = pSrcB;

    /* Initialization to the end of inputA pointer */
    pIn2 = pSrcA + (srcALen - 1U);

    /* Initialisation of the pointer after zero padding */
    pDst = pDst + tot;

    /* Swapping the lengths */
    j = srcALen;
    srcALen = srcBLen;
    srcBLen = j;

    /* Setting the reverse flag */
    inv = 1;

  }

  /* Loop to calculate correlation for output length number of times */
  for (i = 0U; i <= tot; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to correlation equation */
    for (j = 0U; j <= i; j++)
    {
      /* Check the array limitations */
      if ((((i - j) < srcBLen) && (j < srcALen)))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += ((q63_t) pIn1[j] * pIn2[-((int32_t) i - j)]);
      }
    }
    /* Store the output in the destination buffer */
    if (inv == 1)
      *pDst-- = (q31_t)(sum >> 31U);
    else
      *pDst++ = (q31_t)(sum >> 31U);
  }
}

void ref_correlate_fast_q31(
  q31_t * pSrcA,
  uint32_t srcALen,
  q31_t * pSrcB,
  uint32_t srcBLen,
  q31_t * pDst)
{
  q31_t *pIn1 = pSrcA;                           /* inputA pointer               */
  q31_t *pIn2 = pSrcB + (srcBLen - 1U);          /* inputB pointer               */
  q63_t sum;                                     /* Accumulators                  */
  uint32_t i = 0U, j;                            /* loop counters */
  uint32_t inv = 0U;                             /* Reverse order flag */
  uint32_t tot = 0U;                             /* Length */

  /* Calculate the length of the remaining sequence */
  tot = ((srcALen + srcBLen) - 2U);

  if (srcALen > srcBLen)
  {
    /* Calculating the number of zeros to be padded to the output */
    j = srcALen - srcBLen;

    /* Initialise the pointer after zero padding */
    pDst += j;
  }

  else if (srcALen < srcBLen)
  {
    /* Initialization to inputB pointer */
    pIn1 = pSrcB;

    /* Initialization to the end of inputA pointer */
    pIn2 = pSrcA + (srcALen - 1U);

    /* Initialisation of the pointer after zero padding */
    pDst = pDst + tot;

    /* Swapping the lengths */
    j = srcALen;
    srcALen = srcBLen;
    srcBLen = j;

    /* Setting the reverse flag */
    inv = 1;

  }

  /* Loop to calculate correlation for output length number of times */
  for (i = 0U; i <= tot; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to correlation equation */
    for (j = 0U; j <= i; j++)
    {
      /* Check the array limitations */
      if ((((i - j) < srcBLen) && (j < srcALen)))
      {
        /* z[i] += x[i-j] * y[j] */
        sum = (q31_t) ((((q63_t) sum << 32) +
												((q63_t) pIn1[j] * pIn2[-((int32_t) i - j)])) >> 32);
      }
    }
    /* Store the output in the destination buffer */
    if (inv == 1)
      *pDst-- = (q31_t)(sum << 1U);
    else
      *pDst++ = (q31_t)(sum << 1U);
  }          
}

void ref_correlate_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst)
{
  q15_t *pIn1 = pSrcA;                           /* inputA pointer               */
  q15_t *pIn2 = pSrcB + (srcBLen - 1U);          /* inputB pointer               */
  q63_t sum;                                     /* Accumulators                  */
  uint32_t i = 0U, j;                            /* loop counters */
  uint32_t inv = 0U;                             /* Reverse order flag */
  uint32_t tot = 0U;                             /* Length */

  /* Calculate the length of the remaining sequence */
  tot = ((srcALen + srcBLen) - 2U);

  if (srcALen > srcBLen)
  {
    /* Calculating the number of zeros to be padded to the output */
    j = srcALen - srcBLen;

    /* Initialise the pointer after zero padding */
    pDst += j;
  }

  else if (srcALen < srcBLen)
  {
    /* Initialization to inputB pointer */
    pIn1 = pSrcB;

    /* Initialization to the end of inputA pointer */
    pIn2 = pSrcA + (srcALen - 1U);

    /* Initialisation of the pointer after zero padding */
    pDst = pDst + tot;

    /* Swapping the lengths */
    j = srcALen;
    srcALen = srcBLen;
    srcBLen = j;

    /* Setting the reverse flag */
    inv = 1;

  }

  /* Loop to calculate convolution for output length number of times */
  for (i = 0U; i <= tot; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0U; j <= i; j++)
    {
      /* Check the array limitations */
      if ((((i - j) < srcBLen) && (j < srcALen)))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += ((q31_t) pIn1[j] * pIn2[-((int32_t) i - j)]);
      }
    }
    /* Store the output in the destination buffer */
    if (inv == 1)
      *pDst-- = (q15_t) ref_sat_q15(sum >> 15U);
    else
      *pDst++ = (q15_t) ref_sat_q15(sum >> 15U);
  }
}

void ref_correlate_fast_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst)
{
  q15_t *pIn1 = pSrcA;                           /* inputA pointer               */
  q15_t *pIn2 = pSrcB + (srcBLen - 1U);          /* inputB pointer               */
  q63_t sum;                                     /* Accumulators                  */
  uint32_t i = 0U, j;                            /* loop counters */
  uint32_t inv = 0U;                             /* Reverse order flag */
  uint32_t tot = 0U;                             /* Length */

  /* Calculate the length of the remaining sequence */
  tot = ((srcALen + srcBLen) - 2U);

  if (srcALen > srcBLen)
  {
    /* Calculating the number of zeros to be padded to the output */
    j = srcALen - srcBLen;

    /* Initialise the pointer after zero padding */
    pDst += j;
  }

  else if (srcALen < srcBLen)
  {
    /* Initialization to inputB pointer */
    pIn1 = pSrcB;

    /* Initialization to the end of inputA pointer */
    pIn2 = pSrcA + (srcALen - 1U);

    /* Initialisation of the pointer after zero padding */
    pDst = pDst + tot;

    /* Swapping the lengths */
    j = srcALen;
    srcALen = srcBLen;
    srcBLen = j;

    /* Setting the reverse flag */
    inv = 1;

  }

  /* Loop to calculate convolution for output length number of times */
  for (i = 0U; i <= tot; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0U; j <= i; j++)
    {
      /* Check the array limitations */
      if ((((i - j) < srcBLen) && (j < srcALen)))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += ((q31_t) pIn1[j] * pIn2[-((int32_t) i - j)]);
      }
    }
    /* Store the output in the destination buffer */
    if (inv == 1)
      *pDst-- = (q15_t)(sum >> 15U);
    else
      *pDst++ = (q15_t)(sum >> 15U);
  }
}

void ref_correlate_fast_opt_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst,
  q15_t * pScratch)
{
  q15_t *pIn1 = pSrcA;                           /* inputA pointer               */
  q15_t *pIn2 = pSrcB + (srcBLen - 1U);          /* inputB pointer               */
  q31_t sum;                                     /* Accumulators                  */
  uint32_t i = 0U, j;                            /* loop counters */
  uint32_t inv = 0U;                             /* Reverse order flag */
  uint32_t tot = 0U;                             /* Length */

  /* Calculate the length of the remaining sequence */
  tot = ((srcALen + srcBLen) - 2U);

  if (srcALen > srcBLen)
  {
    /* Calculating the number of zeros to be padded to the output */
    j = srcALen - srcBLen;

    /* Initialise the pointer after zero padding */
    pDst += j;
  }

  else if (srcALen < srcBLen)
  {
    /* Initialization to inputB pointer */
    pIn1 = pSrcB;

    /* Initialization to the end of inputA pointer */
    pIn2 = pSrcA + (srcALen - 1U);

    /* Initialisation of the pointer after zero padding */
    pDst = pDst + tot;

    /* Swapping the lengths */
    j = srcALen;
    srcALen = srcBLen;
    srcBLen = j;

    /* Setting the reverse flag */
    inv = 1;

  }

  /* Loop to calculate convolution for output length number of times */
  for (i = 0U; i <= tot; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0U; j <= i; j++)
    {
      /* Check the array limitations */
      if ((((i - j) < srcBLen) && (j < srcALen)))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += ((q31_t) pIn1[j] * pIn2[-((int32_t) i - j)]);
      }
    }
    /* Store the output in the destination buffer */
    if (inv == 1)
      *pDst-- = (q15_t) ref_sat_q15(sum >> 15U);
    else
      *pDst++ = (q15_t) ref_sat_q15(sum >> 15U);
  }
}

void ref_correlate_q7(
  q7_t * pSrcA,
  uint32_t srcALen,
  q7_t * pSrcB,
  uint32_t srcBLen,
  q7_t * pDst)
{
  q7_t *pIn1 = pSrcA;                            /* inputA pointer */
  q7_t *pIn2 = pSrcB + (srcBLen - 1U);           /* inputB pointer */
  q31_t sum;                                     /* Accumulator */
  uint32_t i = 0U, j;                            /* loop counters */
  uint32_t inv = 0U;                             /* Reverse order flag */
  uint32_t tot = 0U;                             /* Length */

  /* Calculate the length of the remaining sequence */
  tot = ((srcALen + srcBLen) - 2U);

  if (srcALen > srcBLen)
  {
    /* Calculating the number of zeros to be padded to the output */
    j = srcALen - srcBLen;

    /* Initialise the pointer after zero padding */
    pDst += j;
  }

  else if (srcALen < srcBLen)
  {
    /* Initialization to inputB pointer */
    pIn1 = pSrcB;

    /* Initialization to the end of inputA pointer */
    pIn2 = pSrcA + (srcALen - 1U);

    /* Initialisation of the pointer after zero padding */
    pDst = pDst + tot;

    /* Swapping the lengths */
    j = srcALen;
    srcALen = srcBLen;
    srcBLen = j;

    /* Setting the reverse flag */
    inv = 1;

  }

  /* Loop to calculate convolution for output length number of times */
  for (i = 0U; i <= tot; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0U; j <= i; j++)
    {
      /* Check the array limitations */
      if ((((i - j) < srcBLen) && (j < srcALen)))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += ((q15_t) pIn1[j] * pIn2[-((int32_t) i - j)]);
      }
    }
    /* Store the output in the destination buffer */
    if (inv == 1)
      *pDst-- = (q7_t) __SSAT((sum >> 7U), 8U);
    else
      *pDst++ = (q7_t) __SSAT((sum >> 7U), 8U);
  }
}
