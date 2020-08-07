/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_biquad_cascade_df1_32x64_q31.c
 * Description:  High precision Q31 Biquad cascade filter processing function
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
 * @defgroup BiquadCascadeDF1_32x64 High Precision Q31 Biquad Cascade Filter
 *
 * This function implements a high precision Biquad cascade filter which operates on
 * Q31 data values.  The filter coefficients are in 1.31 format and the state variables
 * are in 1.63 format.  The double precision state variables reduce quantization noise
 * in the filter and provide a cleaner output.
 * These filters are particularly useful when implementing filters in which the
 * singularities are close to the unit circle.  This is common for low pass or high
 * pass filters with very low cutoff frequencies.
 *
 * The function operates on blocks of input and output data
 * and each call to the function processes <code>blockSize</code> samples through
 * the filter. <code>pSrc</code> and <code>pDst</code> points to input and output arrays
 * containing <code>blockSize</code> Q31 values.
 *
 * \par Algorithm
 * Each Biquad stage implements a second order filter using the difference equation:
 * <pre>
 *     y[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2]
 * </pre>
 * A Direct Form I algorithm is used with 5 coefficients and 4 state variables per stage.
 * \image html Biquad.gif "Single Biquad filter stage"
 * Coefficients <code>b0, b1, and b2 </code> multiply the input signal <code>x[n]</code> and are referred to as the feedforward coefficients.
 * Coefficients <code>a1</code> and <code>a2</code> multiply the output signal <code>y[n]</code> and are referred to as the feedback coefficients.
 * Pay careful attention to the sign of the feedback coefficients.
 * Some design tools use the difference equation
 * <pre>
 *     y[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] - a1 * y[n-1] - a2 * y[n-2]
 * </pre>
 * In this case the feedback coefficients <code>a1</code> and <code>a2</code> must be negated when used with the CMSIS DSP Library.
 *
 * \par
 * Higher order filters are realized as a cascade of second order sections.
 * <code>numStages</code> refers to the number of second order stages used.
 * For example, an 8th order filter would be realized with <code>numStages=4</code> second order stages.
 * \image html BiquadCascade.gif "8th order filter using a cascade of Biquad stages"
 * A 9th order filter would be realized with <code>numStages=5</code> second order stages with the coefficients for one of the stages configured as a first order filter (<code>b2=0</code> and <code>a2=0</code>).
 *
 * \par
 * The <code>pState</code> points to state variables array .
 * Each Biquad stage has 4 state variables <code>x[n-1], x[n-2], y[n-1],</code> and <code>y[n-2]</code> and each state variable in 1.63 format to improve precision.
 * The state variables are arranged in the array as:
 * <pre>
 *     {x[n-1], x[n-2], y[n-1], y[n-2]}
 * </pre>
 *
 * \par
 * The 4 state variables for stage 1 are first, then the 4 state variables for stage 2, and so on.
 * The state array has a total length of <code>4*numStages</code> values of data in 1.63 format.
 * The state variables are updated after each block of data is processed; the coefficients are untouched.
 *
 * \par Instance Structure
 * The coefficients and state variables for a filter are stored together in an instance data structure.
 * A separate instance structure must be defined for each filter.
 * Coefficient arrays may be shared among several instances while state variable arrays cannot be shared.
 *
 * \par Init Function
 * There is also an associated initialization function which performs the following operations:
 * - Sets the values of the internal structure fields.
 * - Zeros out the values in the state buffer.
 * To do this manually without calling the init function, assign the follow subfields of the instance structure:
 * numStages, pCoeffs, postShift, pState. Also set all of the values in pState to zero.
 *
 * \par
 * Use of the initialization function is optional.
 * However, if the initialization function is used, then the instance structure cannot be placed into a const data section.
 * To place an instance structure into a const data section, the instance structure must be manually initialized.
 * Set the values in the state buffer to zeros before static initialization.
 * For example, to statically initialize the filter instance structure use
 * <pre>
 *     arm_biquad_cas_df1_32x64_ins_q31 S1 = {numStages, pState, pCoeffs, postShift};
 * </pre>
 * where <code>numStages</code> is the number of Biquad stages in the filter; <code>pState</code> is the address of the state buffer;
 * <code>pCoeffs</code> is the address of the coefficient buffer; <code>postShift</code> shift to be applied which is described in detail below.
 * \par Fixed-Point Behavior
 * Care must be taken while using Biquad Cascade 32x64 filter function.
 * Following issues must be considered:
 * - Scaling of coefficients
 * - Filter gain
 * - Overflow and saturation
 *
 * \par
 * Filter coefficients are represented as fractional values and
 * restricted to lie in the range <code>[-1 +1)</code>.
 * The processing function has an additional scaling parameter <code>postShift</code>
 * which allows the filter coefficients to exceed the range <code>[+1 -1)</code>.
 * At the output of the filter's accumulator is a shift register which shifts the result by <code>postShift</code> bits.
 * \image html BiquadPostshift.gif "Fixed-point Biquad with shift by postShift bits after accumulator"
 * This essentially scales the filter coefficients by <code>2^postShift</code>.
 * For example, to realize the coefficients
 * <pre>
 *    {1.5, -0.8, 1.2, 1.6, -0.9}
 * </pre>
 * set the Coefficient array to:
 * <pre>
 *    {0.75, -0.4, 0.6, 0.8, -0.45}
 * </pre>
 * and set <code>postShift=1</code>
 *
 * \par
 * The second thing to keep in mind is the gain through the filter.
 * The frequency response of a Biquad filter is a function of its coefficients.
 * It is possible for the gain through the filter to exceed 1.0 meaning that the filter increases the amplitude of certain frequencies.
 * This means that an input signal with amplitude < 1.0 may result in an output > 1.0 and these are saturated or overflowed based on the implementation of the filter.
 * To avoid this behavior the filter needs to be scaled down such that its peak gain < 1.0 or the input signal must be scaled down so that the combination of input and filter are never overflowed.
 *
 * \par
 * The third item to consider is the overflow and saturation behavior of the fixed-point Q31 version.
 * This is described in the function specific documentation below.
 */

/**
 * @addtogroup BiquadCascadeDF1_32x64
 * @{
 */

/**
 * @details

 * @param[in]  *S points to an instance of the high precision Q31 Biquad cascade filter.
 * @param[in]  *pSrc points to the block of input data.
 * @param[out] *pDst points to the block of output data.
 * @param[in]  blockSize number of samples to process.
 * @return none.
 *
 * \par
 * The function is implemented using an internal 64-bit accumulator.
 * The accumulator has a 2.62 format and maintains full precision of the intermediate multiplication results but provides only a single guard bit.
 * Thus, if the accumulator result overflows it wraps around rather than clip.
 * In order to avoid overflows completely the input signal must be scaled down by 2 bits and lie in the range [-0.25 +0.25).
 * After all 5 multiply-accumulates are performed, the 2.62 accumulator is shifted by <code>postShift</code> bits and the result truncated to
 * 1.31 format by discarding the low 32 bits.
 *
 * \par
 * Two related functions are provided in the CMSIS DSP library.
 * <code>arm_biquad_cascade_df1_q31()</code> implements a Biquad cascade with 32-bit coefficients and state variables with a Q63 accumulator.
 * <code>arm_biquad_cascade_df1_fast_q31()</code> implements a Biquad cascade with 32-bit coefficients and state variables with a Q31 accumulator.
 */

void arm_biquad_cas_df1_32x64_q31(
  const arm_biquad_cas_df1_32x64_ins_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t *pIn = pSrc;                             /*  input pointer initialization  */
  q31_t *pOut = pDst;                            /*  output pointer initialization */
  q63_t *pState = S->pState;                     /*  state pointer initialization  */
  q31_t *pCoeffs = S->pCoeffs;                   /*  coeff pointer initialization  */
  q63_t acc;                                     /*  accumulator                   */
  q31_t Xn1, Xn2;                                /*  Input Filter state variables        */
  q63_t Yn1, Yn2;                                /*  Output Filter state variables        */
  q31_t b0, b1, b2, a1, a2;                      /*  Filter coefficients           */
  q31_t Xn;                                      /*  temporary input               */
  int32_t shift = (int32_t) S->postShift + 1;    /*  Shift to be applied to the output */
  uint32_t sample, stage = S->numStages;         /*  loop counters                     */
  q31_t acc_l, acc_h;                            /*  temporary output               */
  uint32_t uShift = ((uint32_t) S->postShift + 1U);
  uint32_t lShift = 32U - uShift;                /*  Shift to be applied to the output */


#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  do
  {
    /* Reading the coefficients */
    b0 = *pCoeffs++;
    b1 = *pCoeffs++;
    b2 = *pCoeffs++;
    a1 = *pCoeffs++;
    a2 = *pCoeffs++;

    /* Reading the state values */
    Xn1 = (q31_t) (pState[0]);
    Xn2 = (q31_t) (pState[1]);
    Yn1 = pState[2];
    Yn2 = pState[3];

    /* Apply loop unrolling and compute 4 output values simultaneously. */
    /* The variable acc hold output value that is being computed and
     * stored in the destination buffer
     * acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2]
     */

    sample = blockSize >> 2U;

    /* First part of the processing with loop unrolling. Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (sample > 0U)
    {
      /* Read the input */
      Xn = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */

      /* acc =  b0 * x[n] */
      acc = (q63_t) Xn *b0;

      /* acc +=  b1 * x[n-1] */
      acc += (q63_t) Xn1 *b1;

      /* acc +=  b[2] * x[n-2] */
      acc += (q63_t) Xn2 *b2;

      /* acc +=  a1 * y[n-1] */
      acc += mult32x64(Yn1, a1);

      /* acc +=  a2 * y[n-2] */
      acc += mult32x64(Yn2, a2);

      /* The result is converted to 1.63 , Yn2 variable is reused */
      Yn2 = acc << shift;

      /* Calc lower part of acc */
      acc_l = acc & 0xffffffff;

      /* Calc upper part of acc */
      acc_h = (acc >> 32) & 0xffffffff;

      /* Apply shift for lower part of acc and upper part of acc */
      acc_h = (uint32_t) acc_l >> lShift | acc_h << uShift;

      /* Store the output in the destination buffer in 1.31 format. */
      *pOut = acc_h;

      /* Read the second input into Xn2, to reuse the value */
      Xn2 = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */

      /* acc +=  b1 * x[n-1] */
      acc = (q63_t) Xn *b1;

      /* acc =  b0 * x[n] */
      acc += (q63_t) Xn2 *b0;

      /* acc +=  b[2] * x[n-2] */
      acc += (q63_t) Xn1 *b2;

      /* acc +=  a1 * y[n-1] */
      acc += mult32x64(Yn2, a1);

      /* acc +=  a2 * y[n-2] */
      acc += mult32x64(Yn1, a2);

      /* The result is converted to 1.63, Yn1 variable is reused */
      Yn1 = acc << shift;

      /* Calc lower part of acc */
      acc_l = acc & 0xffffffff;

      /* Calc upper part of acc */
      acc_h = (acc >> 32) & 0xffffffff;

      /* Apply shift for lower part of acc and upper part of acc */
      acc_h = (uint32_t) acc_l >> lShift | acc_h << uShift;

      /* Read the third input into Xn1, to reuse the value */
      Xn1 = *pIn++;

      /* The result is converted to 1.31 */
      /* Store the output in the destination buffer. */
      *(pOut + 1U) = acc_h;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */

      /* acc =  b0 * x[n] */
      acc = (q63_t) Xn1 *b0;

      /* acc +=  b1 * x[n-1] */
      acc += (q63_t) Xn2 *b1;

      /* acc +=  b[2] * x[n-2] */
      acc += (q63_t) Xn *b2;

      /* acc +=  a1 * y[n-1] */
      acc += mult32x64(Yn1, a1);

      /* acc +=  a2 * y[n-2] */
      acc += mult32x64(Yn2, a2);

      /* The result is converted to 1.63, Yn2 variable is reused  */
      Yn2 = acc << shift;

      /* Calc lower part of acc */
      acc_l = acc & 0xffffffff;

      /* Calc upper part of acc */
      acc_h = (acc >> 32) & 0xffffffff;

      /* Apply shift for lower part of acc and upper part of acc */
      acc_h = (uint32_t) acc_l >> lShift | acc_h << uShift;

      /* Store the output in the destination buffer in 1.31 format. */
      *(pOut + 2U) = acc_h;

      /* Read the fourth input into Xn, to reuse the value */
      Xn = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
      /* acc =  b0 * x[n] */
      acc = (q63_t) Xn *b0;

      /* acc +=  b1 * x[n-1] */
      acc += (q63_t) Xn1 *b1;

      /* acc +=  b[2] * x[n-2] */
      acc += (q63_t) Xn2 *b2;

      /* acc +=  a1 * y[n-1] */
      acc += mult32x64(Yn2, a1);

      /* acc +=  a2 * y[n-2] */
      acc += mult32x64(Yn1, a2);

      /* The result is converted to 1.63, Yn1 variable is reused  */
      Yn1 = acc << shift;

      /* Calc lower part of acc */
      acc_l = acc & 0xffffffff;

      /* Calc upper part of acc */
      acc_h = (acc >> 32) & 0xffffffff;

      /* Apply shift for lower part of acc and upper part of acc */
      acc_h = (uint32_t) acc_l >> lShift | acc_h << uShift;

      /* Store the output in the destination buffer in 1.31 format. */
      *(pOut + 3U) = acc_h;

      /* Every time after the output is computed state should be updated. */
      /* The states should be updated as:  */
      /* Xn2 = Xn1    */
      /* Xn1 = Xn     */
      /* Yn2 = Yn1    */
      /* Yn1 = acc    */
      Xn2 = Xn1;
      Xn1 = Xn;

      /* update output pointer */
      pOut += 4U;

      /* decrement the loop counter */
      sample--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    sample = (blockSize & 0x3U);

    while (sample > 0U)
    {
      /* Read the input */
      Xn = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */

      /* acc =  b0 * x[n] */
      acc = (q63_t) Xn *b0;
      /* acc +=  b1 * x[n-1] */
      acc += (q63_t) Xn1 *b1;
      /* acc +=  b[2] * x[n-2] */
      acc += (q63_t) Xn2 *b2;
      /* acc +=  a1 * y[n-1] */
      acc += mult32x64(Yn1, a1);
      /* acc +=  a2 * y[n-2] */
      acc += mult32x64(Yn2, a2);

      /* Every time after the output is computed state should be updated. */
      /* The states should be updated as:  */
      /* Xn2 = Xn1    */
      /* Xn1 = Xn     */
      /* Yn2 = Yn1    */
      /* Yn1 = acc    */
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
      /* Yn1 = acc << shift; */

      /* Store the output in the destination buffer in 1.31 format. */
/*      *pOut++ = (q31_t) (acc >> (32 - shift));  */

      /* decrement the loop counter */
      sample--;
    }

    /*  The first stage output is given as input to the second stage. */
    pIn = pDst;

    /* Reset to destination buffer working pointer */
    pOut = pDst;

    /*  Store the updated state variables back into the pState array */
    /*  Store the updated state variables back into the pState array */
    *pState++ = (q63_t) Xn1;
    *pState++ = (q63_t) Xn2;
    *pState++ = Yn1;
    *pState++ = Yn2;

  } while (--stage);

#else

  /* Run the below code for Cortex-M0 */

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

    /* The variable acc hold output value that is being computed and
     * stored in the destination buffer
     * acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2]
     */

    sample = blockSize;

    while (sample > 0U)
    {
      /* Read the input */
      Xn = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
      /* acc =  b0 * x[n] */
      acc = (q63_t) Xn *b0;
      /* acc +=  b1 * x[n-1] */
      acc += (q63_t) Xn1 *b1;
      /* acc +=  b[2] * x[n-2] */
      acc += (q63_t) Xn2 *b2;
      /* acc +=  a1 * y[n-1] */
      acc += mult32x64(Yn1, a1);
      /* acc +=  a2 * y[n-2] */
      acc += mult32x64(Yn2, a2);

      /* Every time after the output is computed state should be updated. */
      /* The states should be updated as:  */
      /* Xn2 = Xn1    */
      /* Xn1 = Xn     */
      /* Yn2 = Yn1    */
      /* Yn1 = acc    */
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

      /* Yn1 = acc << shift; */

      /* Store the output in the destination buffer in 1.31 format. */
      /* *pOut++ = (q31_t) (acc >> (32 - shift)); */

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

#endif /*    #if defined (ARM_MATH_DSP)     */
}

  /**
   * @} end of BiquadCascadeDF1_32x64 group
   */
