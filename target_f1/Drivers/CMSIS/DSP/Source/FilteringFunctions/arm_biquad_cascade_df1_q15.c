/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_biquad_cascade_df1_q15.c
 * Description:  Processing function for the Q15 Biquad cascade DirectFormI(DF1) filter
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
 * @addtogroup BiquadCascadeDF1
 * @{
 */

/**
 * @brief Processing function for the Q15 Biquad cascade filter.
 * @param[in]  *S points to an instance of the Q15 Biquad cascade structure.
 * @param[in]  *pSrc points to the block of input data.
 * @param[out] *pDst points to the location where the output result is written.
 * @param[in]  blockSize number of samples to process per call.
 * @return none.
 *
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using a 64-bit internal accumulator.
 * Both coefficients and state variables are represented in 1.15 format and multiplications yield a 2.30 result.
 * The 2.30 intermediate results are accumulated in a 64-bit accumulator in 34.30 format.
 * There is no risk of internal overflow with this approach and the full precision of intermediate multiplications is preserved.
 * The accumulator is then shifted by <code>postShift</code> bits to truncate the result to 1.15 format by discarding the low 16 bits.
 * Finally, the result is saturated to 1.15 format.
 *
 * \par
 * Refer to the function <code>arm_biquad_cascade_df1_fast_q15()</code> for a faster but less precise implementation of this filter for Cortex-M3 and Cortex-M4.
 */

void arm_biquad_cascade_df1_q15(
  const arm_biquad_casd_df1_inst_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{


#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  q15_t *pIn = pSrc;                             /*  Source pointer                               */
  q15_t *pOut = pDst;                            /*  Destination pointer                          */
  q31_t in;                                      /*  Temporary variable to hold input value       */
  q31_t out;                                     /*  Temporary variable to hold output value      */
  q31_t b0;                                      /*  Temporary variable to hold bo value          */
  q31_t b1, a1;                                  /*  Filter coefficients                          */
  q31_t state_in, state_out;                     /*  Filter state variables                       */
  q31_t acc_l, acc_h;
  q63_t acc;                                     /*  Accumulator                                  */
  int32_t lShift = (15 - (int32_t) S->postShift);       /*  Post shift                                   */
  q15_t *pState = S->pState;                     /*  State pointer                                */
  q15_t *pCoeffs = S->pCoeffs;                   /*  Coefficient pointer                          */
  uint32_t sample, stage = (uint32_t) S->numStages;     /*  Stage loop counter                           */
  int32_t uShift = (32 - lShift);

  do
  {
    /* Read the b0 and 0 coefficients using SIMD  */
    b0 = *__SIMD32(pCoeffs)++;

    /* Read the b1 and b2 coefficients using SIMD */
    b1 = *__SIMD32(pCoeffs)++;

    /* Read the a1 and a2 coefficients using SIMD */
    a1 = *__SIMD32(pCoeffs)++;

    /* Read the input state values from the state buffer:  x[n-1], x[n-2] */
    state_in = *__SIMD32(pState)++;

    /* Read the output state values from the state buffer:  y[n-1], y[n-2] */
    state_out = *__SIMD32(pState)--;

    /* Apply loop unrolling and compute 2 output values simultaneously. */
    /*      The variable acc hold output values that are being computed:
     *
     *    acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2]
     *    acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2]
     */
    sample = blockSize >> 1U;

    /* First part of the processing with loop unrolling.  Compute 2 outputs at a time.
     ** a second loop below computes the remaining 1 sample. */
    while (sample > 0U)
    {

      /* Read the input */
      in = *__SIMD32(pIn)++;

      /* out =  b0 * x[n] + 0 * 0 */
      out = __SMUAD(b0, in);

      /* acc +=  b1 * x[n-1] +  b2 * x[n-2] + out */
      acc = __SMLALD(b1, state_in, out);
      /* acc +=  a1 * y[n-1] +  a2 * y[n-2] */
      acc = __SMLALD(a1, state_out, acc);

      /* The result is converted from 3.29 to 1.31 if postShift = 1, and then saturation is applied */
      /* Calc lower part of acc */
      acc_l = acc & 0xffffffff;

      /* Calc upper part of acc */
      acc_h = (acc >> 32) & 0xffffffff;

      /* Apply shift for lower part of acc and upper part of acc */
      out = (uint32_t) acc_l >> lShift | acc_h << uShift;

      out = __SSAT(out, 16);

      /* Every time after the output is computed state should be updated. */
      /* The states should be updated as:  */
      /* Xn2 = Xn1    */
      /* Xn1 = Xn     */
      /* Yn2 = Yn1    */
      /* Yn1 = acc   */
      /* x[n-N], x[n-N-1] are packed together to make state_in of type q31 */
      /* y[n-N], y[n-N-1] are packed together to make state_out of type q31 */

#ifndef  ARM_MATH_BIG_ENDIAN

      state_in = __PKHBT(in, state_in, 16);
      state_out = __PKHBT(out, state_out, 16);

#else

      state_in = __PKHBT(state_in >> 16, (in >> 16), 16);
      state_out = __PKHBT(state_out >> 16, (out), 16);

#endif /* #ifndef  ARM_MATH_BIG_ENDIAN */

      /* out =  b0 * x[n] + 0 * 0 */
      out = __SMUADX(b0, in);
      /* acc +=  b1 * x[n-1] +  b2 * x[n-2] + out */
      acc = __SMLALD(b1, state_in, out);
      /* acc +=  a1 * y[n-1] + a2 * y[n-2] */
      acc = __SMLALD(a1, state_out, acc);

      /* The result is converted from 3.29 to 1.31 if postShift = 1, and then saturation is applied */
      /* Calc lower part of acc */
      acc_l = acc & 0xffffffff;

      /* Calc upper part of acc */
      acc_h = (acc >> 32) & 0xffffffff;

      /* Apply shift for lower part of acc and upper part of acc */
      out = (uint32_t) acc_l >> lShift | acc_h << uShift;

      out = __SSAT(out, 16);

      /* Store the output in the destination buffer. */

#ifndef  ARM_MATH_BIG_ENDIAN

      *__SIMD32(pOut)++ = __PKHBT(state_out, out, 16);

#else

      *__SIMD32(pOut)++ = __PKHBT(out, state_out >> 16, 16);

#endif /* #ifndef  ARM_MATH_BIG_ENDIAN */

      /* Every time after the output is computed state should be updated. */
      /* The states should be updated as:  */
      /* Xn2 = Xn1    */
      /* Xn1 = Xn     */
      /* Yn2 = Yn1    */
      /* Yn1 = acc   */
      /* x[n-N], x[n-N-1] are packed together to make state_in of type q31 */
      /* y[n-N], y[n-N-1] are packed together to make state_out of type q31 */
#ifndef  ARM_MATH_BIG_ENDIAN

      state_in = __PKHBT(in >> 16, state_in, 16);
      state_out = __PKHBT(out, state_out, 16);

#else

      state_in = __PKHBT(state_in >> 16, in, 16);
      state_out = __PKHBT(state_out >> 16, out, 16);

#endif /* #ifndef  ARM_MATH_BIG_ENDIAN */


      /* Decrement the loop counter */
      sample--;

    }

    /* If the blockSize is not a multiple of 2, compute any remaining output samples here.
     ** No loop unrolling is used. */

    if ((blockSize & 0x1U) != 0U)
    {
      /* Read the input */
      in = *pIn++;

      /* out =  b0 * x[n] + 0 * 0 */

#ifndef  ARM_MATH_BIG_ENDIAN

      out = __SMUAD(b0, in);

#else

      out = __SMUADX(b0, in);

#endif /* #ifndef  ARM_MATH_BIG_ENDIAN */

      /* acc =  b1 * x[n-1] + b2 * x[n-2] + out */
      acc = __SMLALD(b1, state_in, out);
      /* acc +=  a1 * y[n-1] + a2 * y[n-2] */
      acc = __SMLALD(a1, state_out, acc);

      /* The result is converted from 3.29 to 1.31 if postShift = 1, and then saturation is applied */
      /* Calc lower part of acc */
      acc_l = acc & 0xffffffff;

      /* Calc upper part of acc */
      acc_h = (acc >> 32) & 0xffffffff;

      /* Apply shift for lower part of acc and upper part of acc */
      out = (uint32_t) acc_l >> lShift | acc_h << uShift;

      out = __SSAT(out, 16);

      /* Store the output in the destination buffer. */
      *pOut++ = (q15_t) out;

      /* Every time after the output is computed state should be updated. */
      /* The states should be updated as:  */
      /* Xn2 = Xn1    */
      /* Xn1 = Xn     */
      /* Yn2 = Yn1    */
      /* Yn1 = acc   */
      /* x[n-N], x[n-N-1] are packed together to make state_in of type q31 */
      /* y[n-N], y[n-N-1] are packed together to make state_out of type q31 */

#ifndef  ARM_MATH_BIG_ENDIAN

      state_in = __PKHBT(in, state_in, 16);
      state_out = __PKHBT(out, state_out, 16);

#else

      state_in = __PKHBT(state_in >> 16, in, 16);
      state_out = __PKHBT(state_out >> 16, out, 16);

#endif /* #ifndef  ARM_MATH_BIG_ENDIAN */

    }

    /*  The first stage goes from the input wire to the output wire.  */
    /*  Subsequent numStages occur in-place in the output wire  */
    pIn = pDst;

    /* Reset the output pointer */
    pOut = pDst;

    /*  Store the updated state variables back into the state array */
    *__SIMD32(pState)++ = state_in;
    *__SIMD32(pState)++ = state_out;


    /* Decrement the loop counter */
    stage--;

  } while (stage > 0U);

#else

  /* Run the below code for Cortex-M0 */

  q15_t *pIn = pSrc;                             /*  Source pointer                               */
  q15_t *pOut = pDst;                            /*  Destination pointer                          */
  q15_t b0, b1, b2, a1, a2;                      /*  Filter coefficients           */
  q15_t Xn1, Xn2, Yn1, Yn2;                      /*  Filter state variables        */
  q15_t Xn;                                      /*  temporary input               */
  q63_t acc;                                     /*  Accumulator                                  */
  int32_t shift = (15 - (int32_t) S->postShift); /*  Post shift                                   */
  q15_t *pState = S->pState;                     /*  State pointer                                */
  q15_t *pCoeffs = S->pCoeffs;                   /*  Coefficient pointer                          */
  uint32_t sample, stage = (uint32_t) S->numStages;     /*  Stage loop counter                           */

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
      acc = (q31_t) b0 *Xn;

      /* acc +=  b1 * x[n-1] */
      acc += (q31_t) b1 *Xn1;
      /* acc +=  b[2] * x[n-2] */
      acc += (q31_t) b2 *Xn2;
      /* acc +=  a1 * y[n-1] */
      acc += (q31_t) a1 *Yn1;
      /* acc +=  a2 * y[n-2] */
      acc += (q31_t) a2 *Yn2;

      /* The result is converted to 1.31  */
      acc = __SSAT((acc >> shift), 16);

      /* Every time after the output is computed state should be updated. */
      /* The states should be updated as:  */
      /* Xn2 = Xn1    */
      /* Xn1 = Xn     */
      /* Yn2 = Yn1    */
      /* Yn1 = acc    */
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

#endif /* #if defined (ARM_MATH_DSP) */

}


/**
 * @} end of BiquadCascadeDF1 group
 */
