#include "ref.h"

void ref_biquad_cascade_df2T_f32(
	const arm_biquad_cascade_df2T_instance_f32 * S,
	float32_t * pSrc,
	float32_t * pDst,
	uint32_t blockSize)
{
   float32_t *pIn = pSrc;                         /*  source pointer            */
   float32_t *pOut = pDst;                        /*  destination pointer       */
   float32_t *pState = S->pState;                 /*  State pointer             */
   float32_t *pCoeffs = S->pCoeffs;               /*  coefficient pointer       */
   float32_t acc;                                 /*  accumulator               */
   float32_t b0, b1, b2, a1, a2;                  /*  Filter coefficients       */
   float32_t Xn;                                  /*  temporary input           */
   float32_t d1, d2;                              /*  state variables           */
   uint32_t sample, stage = S->numStages;         /*  loop counters             */

   do
   {
      /* Reading the coefficients */
      b0 = *pCoeffs++;
      b1 = *pCoeffs++;
      b2 = *pCoeffs++;
      a1 = *pCoeffs++;
      a2 = *pCoeffs++;

      /*Reading the state values */
      d1 = pState[0];
      d2 = pState[1];

      sample = blockSize;

      while (sample > 0U)
      {
         /* Read the input */
         Xn = *pIn++;

         /* y[n] = b0 * x[n] + d1 */
         acc = (b0 * Xn) + d1;

         /* Store the result in the accumulator in the destination buffer. */
         *pOut++ = acc;

         /* Every time after the output is computed state should be updated. */
         /* d1 = b1 * x[n] + a1 * y[n] + d2 */
         d1 = (b1 * Xn + a1 * acc) + d2;

         /* d2 = b2 * x[n] + a2 * y[n] */
         d2 = (b2 * Xn) + (a2 * acc);

         /* decrement the loop counter */
         sample--;
      }

      /* Store the updated state variables back into the state array */
      *pState++ = d1;
      *pState++ = d2;

      /* The current stage input is given as the output to the next stage */
      pIn = pDst;

      /*Reset the output working pointer */
      pOut = pDst;

      /* decrement the loop counter */
      stage--;

   } while (stage > 0U);
}


void ref_biquad_cascade_stereo_df2T_f32(
	const arm_biquad_cascade_stereo_df2T_instance_f32 * S,
	float32_t * pSrc,
	float32_t * pDst,
	uint32_t blockSize)
{
    float32_t *pIn = pSrc;                         /*  source pointer            */
    float32_t *pOut = pDst;                        /*  destination pointer       */
    float32_t *pState = S->pState;                 /*  State pointer             */
    float32_t *pCoeffs = S->pCoeffs;               /*  coefficient pointer       */
    float32_t acc1a, acc1b;                        /*  accumulator               */
    float32_t b0, b1, b2, a1, a2;                  /*  Filter coefficients       */
    float32_t Xn1a, Xn1b;                          /*  temporary input           */
    float32_t d1a, d2a, d1b, d2b;                  /*  state variables           */
    uint32_t sample, stage = S->numStages;         /*  loop counters             */

    do
    {
        /* Reading the coefficients */
        b0 = *pCoeffs++;
        b1 = *pCoeffs++;
        b2 = *pCoeffs++;
        a1 = *pCoeffs++;
        a2 = *pCoeffs++;

        /*Reading the state values */
        d1a = pState[0];
        d2a = pState[1];
        d1b = pState[2];
        d2b = pState[3];

        sample = blockSize;

        while (sample > 0U)
        {
            /* Read the input */
            Xn1a = *pIn++; //Channel a
            Xn1b = *pIn++; //Channel b

            /* y[n] = b0 * x[n] + d1 */
            acc1a = (b0 * Xn1a) + d1a;
            acc1b = (b0 * Xn1b) + d1b;

            /* Store the result in the accumulator in the destination buffer. */
            *pOut++ = acc1a;
            *pOut++ = acc1b;

            /* Every time after the output is computed state should be updated. */
            /* d1 = b1 * x[n] + a1 * y[n] + d2 */
            d1a = ((b1 * Xn1a) + (a1 * acc1a)) + d2a;
            d1b = ((b1 * Xn1b) + (a1 * acc1b)) + d2b;

            /* d2 = b2 * x[n] + a2 * y[n] */
            d2a = (b2 * Xn1a) + (a2 * acc1a);
            d2b = (b2 * Xn1b) + (a2 * acc1b);

            /* decrement the loop counter */
            sample--;
        }

        /* Store the updated state variables back into the state array */
        *pState++ = d1a;
        *pState++ = d2a;
        *pState++ = d1b;
        *pState++ = d2b;

        /* The current stage input is given as the output to the next stage */
        pIn = pDst;

        /*Reset the output working pointer */
        pOut = pDst;

        /* decrement the loop counter */
        stage--;

    } while (stage > 0U);
	
}

void ref_biquad_cascade_df2T_f64(
	const arm_biquad_cascade_df2T_instance_f64 * S,
	float64_t * pSrc,
	float64_t * pDst,
	uint32_t blockSize)
{
   float64_t *pIn = pSrc;                         /*  source pointer            */
   float64_t *pOut = pDst;                        /*  destination pointer       */
   float64_t *pState = S->pState;                 /*  State pointer             */
   float64_t *pCoeffs = S->pCoeffs;               /*  coefficient pointer       */
   float64_t acc;                                 /*  accumulator               */
   float64_t b0, b1, b2, a1, a2;                  /*  Filter coefficients       */
   float64_t Xn;                                  /*  temporary input           */
   float64_t d1, d2;                              /*  state variables           */
   uint32_t sample, stage = S->numStages;         /*  loop counters             */

   do
   {
      /* Reading the coefficients */
      b0 = *pCoeffs++;
      b1 = *pCoeffs++;
      b2 = *pCoeffs++;
      a1 = *pCoeffs++;
      a2 = *pCoeffs++;

      /*Reading the state values */
      d1 = pState[0];
      d2 = pState[1];

      sample = blockSize;

      while (sample > 0U)
      {
         /* Read the input */
         Xn = *pIn++;

         /* y[n] = b0 * x[n] + d1 */
         acc = (b0 * Xn) + d1;

         /* Store the result in the accumulator in the destination buffer. */
         *pOut++ = acc;

         /* Every time after the output is computed state should be updated. */
         /* d1 = b1 * x[n] + a1 * y[n] + d2 */
         d1 = (b1 * Xn + a1 * acc) + d2;

         /* d2 = b2 * x[n] + a2 * y[n] */
         d2 = (b2 * Xn) + (a2 * acc);

         /* decrement the loop counter */
         sample--;
      }

      /* Store the updated state variables back into the state array */
      *pState++ = d1;
      *pState++ = d2;

      /* The current stage input is given as the output to the next stage */
      pIn = pDst;

      /*Reset the output working pointer */
      pOut = pDst;

      /* decrement the loop counter */
      stage--;

   } while (stage > 0U);
}

void ref_biquad_cascade_df1_f32(
  const arm_biquad_casd_df1_inst_f32 * S,
  float32_t * pSrc,
  float32_t * pDst,
  uint32_t blockSize)
{
  float32_t *pIn = pSrc;                         /*  source pointer            */
  float32_t *pOut = pDst;                        /*  destination pointer       */
  float32_t *pState = S->pState;                 /*  pState pointer            */
  float32_t *pCoeffs = S->pCoeffs;               /*  coefficient pointer       */
  float32_t acc;                                 /*  Simulates the accumulator */
  float32_t b0, b1, b2, a1, a2;                  /*  Filter coefficients       */
  float32_t Xn1, Xn2, Yn1, Yn2;                  /*  Filter pState variables   */
  float32_t Xn;                                  /*  temporary input           */
  uint32_t sample, stage = S->numStages;         /*  loop counters             */

  do
  {
    /* Reading the coefficients */
    b0 = *pCoeffs++;
    b1 = *pCoeffs++;
    b2 = *pCoeffs++;
    a1 = *pCoeffs++;
    a2 = *pCoeffs++;

    /* Reading the pState values */
    Xn1 = pState[0];
    Xn2 = pState[1];
    Yn1 = pState[2];
    Yn2 = pState[3];

    /*      The variables acc holds the output value that is computed:        
     *    acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1]   + a2 * y[n-2]        
     */

    sample = blockSize;

    while (sample > 0U)
    {
      /* Read the input */
      Xn = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
      acc = (b0 * Xn) + (b1 * Xn1) + (b2 * Xn2) + (a1 * Yn1) + (a2 * Yn2);

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = acc;

      /* Every time after the output is computed state should be updated. */
      /* The states should be updated as:    */
      /* Xn2 = Xn1    */
      /* Xn1 = Xn     */
      /* Yn2 = Yn1    */
      /* Yn1 = acc   */
      Xn2 = Xn1;
      Xn1 = Xn;
      Yn2 = Yn1;
      Yn1 = acc;

      /* decrement the loop counter */
      sample--;
    }

    /*  Store the updated state variables back into the pState array */
    *pState++ = Xn1;
    *pState++ = Xn2;
    *pState++ = Yn1;
    *pState++ = Yn2;

    /*  The first stage goes from the input buffer to the output buffer. */
    /*  Subsequent numStages  occur in-place in the output buffer */
    pIn = pDst;

    /* Reset the output pointer */
    pOut = pDst;

    /* decrement the loop counter */
    stage--;

  } while (stage > 0U);
}

void ref_biquad_cas_df1_32x64_q31(
  const arm_biquad_cas_df1_32x64_ins_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t *pIn = pSrc;                             /*  input pointer initialization  			*/
  q31_t *pOut = pDst;                            /*  output pointer initialization 			*/
  q63_t *pState = S->pState;                     /*  state pointer initialization  			*/
  q31_t *pCoeffs = S->pCoeffs;                   /*  coeff pointer initialization  			*/
  q63_t acc;                                     /*  accumulator                   			*/
  q31_t Xn1, Xn2;                                /*  Input Filter state variables  			*/
  q63_t Yn1, Yn2;                                /*  Output Filter state variables 			*/
  q31_t b0, b1, b2, a1, a2;                      /*  Filter coefficients           			*/
  q31_t Xn;                                      /*  temporary input               			*/
  int32_t shift = (int32_t) S->postShift + 1;    /*  Shift to be applied to the output 	*/
  uint32_t sample, stage = S->numStages;         /*  loop counters                     	*/
  q31_t acc_l, acc_h;                            /*  temporary output               		*/
  uint32_t uShift = ((uint32_t) S->postShift + 1U);
  uint32_t lShift = 32U - uShift;                /*  Shift to be applied to the output 	*/

  do
  {
    /* Reading the coefficients */
    b0 = *pCoeffs++;
    b1 = *pCoeffs++;
    b2 = *pCoeffs++;
    a1 = *pCoeffs++;
    a2 = *pCoeffs++;

    /* Reading the state values */
    Xn1 = pState[0];
    Xn2 = pState[1];
    Yn1 = pState[2];
    Yn2 = pState[3];

    sample = blockSize;

    while (sample > 0U)
    {
      /* Read the input */
      Xn = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
      acc = (q63_t)Xn*b0 + (q63_t)Xn1*b1 + (q63_t)Xn2*b2;
      /* acc +=  a1 * y[n-1] */
      acc += mult32x64(Yn1, a1);
      /* acc +=  a2 * y[n-2] */
      acc += mult32x64(Yn2, a2);

      /* Every time after the output is computed state should be updated. */
      Xn2 = Xn1;
      Xn1 = Xn;
      Yn2 = Yn1;

      /* The result is converted to 1.63, Yn1 variable is reused  */
      Yn1 = acc << shift;

      /* Calc lower part of acc */
      acc_l = acc & 0xffffffff;

      /* Calc upper part of acc */
      acc_h = (acc >> 32) & 0xffffffff;

      /* Apply shift for lower part of acc and upper part of acc */
      acc_h = (uint32_t) acc_l >> lShift | acc_h << uShift;

      /* Store the output in the destination buffer in 1.31 format. */
      *pOut++ = acc_h;

      /* decrement the loop counter */
      sample--;
    }

    /*  The first stage output is given as input to the second stage. */
    pIn = pDst;

    /* Reset to destination buffer working pointer */
    pOut = pDst;

    /*  Store the updated state variables back into the pState array */
    *pState++ = (q63_t) Xn1;
    *pState++ = (q63_t) Xn2;
    *pState++ = Yn1;
    *pState++ = Yn2;

  } while (--stage);
}

void ref_biquad_cascade_df1_q31(
  const arm_biquad_casd_df1_inst_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{	
  q63_t acc;                                     /*  accumulator                   */
  uint32_t uShift = ((uint32_t) S->postShift + 1U);
  uint32_t lShift = 32U - uShift;                /*  Shift to be applied to the output */
  q31_t *pIn = pSrc;                             /*  input pointer initialization  */
  q31_t *pOut = pDst;                            /*  output pointer initialization */
  q31_t *pState = S->pState;                     /*  pState pointer initialization */
  q31_t *pCoeffs = S->pCoeffs;                   /*  coeff pointer initialization  */
  q31_t Xn1, Xn2, Yn1, Yn2;                      /*  Filter state variables        */
  q31_t b0, b1, b2, a1, a2;                      /*  Filter coefficients           */
  q31_t Xn;                                      /*  temporary input               */
  uint32_t sample, stage = S->numStages;         /*  loop counters                 */

  do
  {
    /* Reading the coefficients */
    b0 = *pCoeffs++;
    b1 = *pCoeffs++;
    b2 = *pCoeffs++;
    a1 = *pCoeffs++;
    a2 = *pCoeffs++;

    /* Reading the state values */
    Xn1 = pState[0];
    Xn2 = pState[1];
    Yn1 = pState[2];
    Yn2 = pState[3];

    /*      The variables acc holds the output value that is computed:         
     *    acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2]         
     */

    sample = blockSize;

    while (sample > 0U)
    {
      /* Read the input */
      Xn = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
      /* acc =  b0 * x[n] */
      acc = (q63_t) b0 *Xn;

      /* acc +=  b1 * x[n-1] */
      acc += (q63_t) b1 *Xn1;
      /* acc +=  b[2] * x[n-2] */
      acc += (q63_t) b2 *Xn2;
      /* acc +=  a1 * y[n-1] */
      acc += (q63_t) a1 *Yn1;
      /* acc +=  a2 * y[n-2] */
      acc += (q63_t) a2 *Yn2;

      /* The result is converted to 1.31  */
      acc = acc >> lShift;

      /* Every time after the output is computed state should be updated. */
      /* The states should be updated as:  */
      /* Xn2 = Xn1    */
      /* Xn1 = Xn     */
      /* Yn2 = Yn1    */
      /* Yn1 = acc    */
      Xn2 = Xn1;
      Xn1 = Xn;
      Yn2 = Yn1;
      Yn1 = (q31_t) acc;

      /* Store the output in the destination buffer. */
      *pOut++ = (q31_t) acc;

      /* decrement the loop counter */
      sample--;
    }

    /*  The first stage goes from the input buffer to the output buffer. */
    /*  Subsequent stages occur in-place in the output buffer */
    pIn = pDst;

    /* Reset to destination pointer */
    pOut = pDst;

    /*  Store the updated state variables back into the pState array */
    *pState++ = Xn1;
    *pState++ = Xn2;
    *pState++ = Yn1;
    *pState++ = Yn2;

  } while (--stage);
}


void ref_biquad_cascade_df1_fast_q31(
  const arm_biquad_casd_df1_inst_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t acc = 0;                                 /*  accumulator                   */
  q31_t Xn1, Xn2, Yn1, Yn2;                      /*  Filter state variables        */
  q31_t b0, b1, b2, a1, a2;                      /*  Filter coefficients           */
  q31_t *pIn = pSrc;                             /*  input pointer initialization  */
  q31_t *pOut = pDst;                            /*  output pointer initialization */
  q31_t *pState = S->pState;                     /*  pState pointer initialization */
  q31_t *pCoeffs = S->pCoeffs;                   /*  coeff pointer initialization  */
  q31_t Xn;                                      /*  temporary input               */
  int32_t shift = (int32_t) S->postShift + 1;    /*  Shift to be applied to the output */
  uint32_t sample, stage = S->numStages;         /*  loop counters                     */

  do
  {
    /* Reading the coefficients */
    b0 = *pCoeffs++;
    b1 = *pCoeffs++;
    b2 = *pCoeffs++;
    a1 = *pCoeffs++;
    a2 = *pCoeffs++;

    /* Reading the state values */
    Xn1 = pState[0];
    Xn2 = pState[1];
    Yn1 = pState[2];
    Yn2 = pState[3];
		
    sample = blockSize;

   while (sample > 0U)
   {
      /* Read the input */
      Xn = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */    
      mult_32x32_keep32_R(acc, b0, Xn);
      multAcc_32x32_keep32_R(acc, b1, Xn1);
      multAcc_32x32_keep32_R(acc, b2, Xn2);
      multAcc_32x32_keep32_R(acc, a1, Yn1);
      multAcc_32x32_keep32_R(acc, a2, Yn2);

      /* The result is converted to 1.31  */
      acc <<= shift;

      /* Every time after the output is computed state should be updated. */
      Xn2 = Xn1;
      Xn1 = Xn;
      Yn2 = Yn1;
      Yn1 = acc;

      /* Store the output in the destination buffer. */
      *pOut++ = acc;

      /* decrement the loop counter */
      sample--;
   }

    /*  The first stage goes from the input buffer to the output buffer. */
    /*  Subsequent stages occur in-place in the output buffer */
    pIn = pDst;

    /* Reset to destination pointer */
    pOut = pDst;

    /*  Store the updated state variables back into the pState array */
    *pState++ = Xn1;
    *pState++ = Xn2;
    *pState++ = Yn1;
    *pState++ = Yn2;

  } while (--stage);
}

void ref_biquad_cascade_df1_fast_q15(
  const arm_biquad_casd_df1_inst_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
	q15_t *pIn = pSrc;                             			/*  Source pointer                           */
	q15_t *pOut = pDst;                            			/*  Destination pointer                      */
	q15_t b0, b1, b2, a1, a2;                      			/*  Filter coefficients           				*/
	q15_t Xn1, Xn2, Yn1, Yn2;                      			/*  Filter state variables        				*/
	q15_t Xn;                                      			/*  temporary input               				*/
	q31_t acc;                                     			/*  Accumulator                              */
	int32_t shift = (15 - (int32_t) S->postShift); 			/*  Post shift                               */
	q15_t *pState = S->pState;                     			/*  State pointer                            */
	q15_t *pCoeffs = S->pCoeffs;                   			/*  Coefficient pointer                      */
  uint32_t sample, stage = (uint32_t) S->numStages;   /*  Stage loop counter                          */

  do
  {
    /* Reading the coefficients */
    b0 = *pCoeffs++;
    pCoeffs++;  // skip the 0 coefficient
    b1 = *pCoeffs++;
    b2 = *pCoeffs++;
    a1 = *pCoeffs++;
    a2 = *pCoeffs++;

    /* Reading the state values */
    Xn1 = pState[0];
    Xn2 = pState[1];
    Yn1 = pState[2];
    Yn2 = pState[3];

    sample = blockSize;

    while (sample > 0U)
    {
      /* Read the input */
      Xn = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
      acc = (q31_t)b0*Xn + (q31_t)b1*Xn1 + (q31_t)b2*Xn2 + (q31_t)a1*Yn1 + (q31_t)a2*Yn2;

      /* The result is converted to 1.15  */
      acc = ref_sat_q15(acc >> shift);

      /* Every time after the output is computed state should be updated. */
      Xn2 = Xn1;
      Xn1 = Xn;
      Yn2 = Yn1;
      Yn1 = (q15_t) acc;

      /* Store the output in the destination buffer. */
      *pOut++ = (q15_t) acc;

      /* decrement the loop counter */
      sample--;
    }

    /*  The first stage goes from the input buffer to the output buffer. */
    /*  Subsequent stages occur in-place in the output buffer */
    pIn = pDst;

    /* Reset to destination pointer */
    pOut = pDst;

    /*  Store the updated state variables back into the pState array */
    *pState++ = Xn1;
    *pState++ = Xn2;
    *pState++ = Yn1;
    *pState++ = Yn2;

  } while (--stage);
}

void ref_biquad_cascade_df1_q15(
  const arm_biquad_casd_df1_inst_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
	q15_t *pIn = pSrc;                             			/*  Source pointer                           */
	q15_t *pOut = pDst;                            			/*  Destination pointer                      */
	q15_t b0, b1, b2, a1, a2;                      			/*  Filter coefficients           				*/
	q15_t Xn1, Xn2, Yn1, Yn2;                      			/*  Filter state variables        				*/
	q15_t Xn;                                      			/*  temporary input               				*/
	q63_t acc;                                     			/*  Accumulator                              */
	int32_t shift = (15 - (int32_t) S->postShift); 			/*  Post shift                               */
	q15_t *pState = S->pState;                     			/*  State pointer                            */
	q15_t *pCoeffs = S->pCoeffs;                   			/*  Coefficient pointer                      */
  uint32_t sample, stage = (uint32_t) S->numStages;   /*  Stage loop counter                          */

  do
  {
    /* Reading the coefficients */
    b0 = *pCoeffs++;
    pCoeffs++;  // skip the 0 coefficient
    b1 = *pCoeffs++;
    b2 = *pCoeffs++;
    a1 = *pCoeffs++;
    a2 = *pCoeffs++;

    /* Reading the state values */
    Xn1 = pState[0];
    Xn2 = pState[1];
    Yn1 = pState[2];
    Yn2 = pState[3];

    sample = blockSize;

    while (sample > 0U)
    {
      /* Read the input */
      Xn = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
      acc = (q31_t)b0*Xn + (q31_t)b1*Xn1 + (q31_t)b2*Xn2 + (q31_t)a1*Yn1 + (q31_t)a2*Yn2;

      /* The result is converted to 1.15  */
      acc = ref_sat_q15(acc >> shift);

      /* Every time after the output is computed state should be updated. */
      Xn2 = Xn1;
      Xn1 = Xn;
      Yn2 = Yn1;
      Yn1 = (q15_t) acc;

      /* Store the output in the destination buffer. */
      *pOut++ = (q15_t) acc;

      /* decrement the loop counter */
      sample--;
    }

    /*  The first stage goes from the input buffer to the output buffer. */
    /*  Subsequent stages occur in-place in the output buffer */
    pIn = pDst;

    /* Reset to destination pointer */
    pOut = pDst;

    /*  Store the updated state variables back into the pState array */
    *pState++ = Xn1;
    *pState++ = Xn2;
    *pState++ = Yn1;
    *pState++ = Yn2;

  } while (--stage);
}
