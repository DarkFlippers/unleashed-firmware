/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_lms_f32.c
 * Description:  Processing function for the floating-point LMS filter
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
 * @defgroup LMS Least Mean Square (LMS) Filters
 *
 * LMS filters are a class of adaptive filters that are able to "learn" an unknown transfer functions.
 * LMS filters use a gradient descent method in which the filter coefficients are updated based on the instantaneous error signal.
 * Adaptive filters are often used in communication systems, equalizers, and noise removal.
 * The CMSIS DSP Library contains LMS filter functions that operate on Q15, Q31, and floating-point data types.
 * The library also contains normalized LMS filters in which the filter coefficient adaptation is indepedent of the level of the input signal.
 *
 * An LMS filter consists of two components as shown below.
 * The first component is a standard transversal or FIR filter.
 * The second component is a coefficient update mechanism.
 * The LMS filter has two input signals.
 * The "input" feeds the FIR filter while the "reference input" corresponds to the desired output of the FIR filter.
 * That is, the FIR filter coefficients are updated so that the output of the FIR filter matches the reference input.
 * The filter coefficient update mechanism is based on the difference between the FIR filter output and the reference input.
 * This "error signal" tends towards zero as the filter adapts.
 * The LMS processing functions accept the input and reference input signals and generate the filter output and error signal.
 * \image html LMS.gif "Internal structure of the Least Mean Square filter"
 *
 * The functions operate on blocks of data and each call to the function processes
 * <code>blockSize</code> samples through the filter.
 * <code>pSrc</code> points to input signal, <code>pRef</code> points to reference signal,
 * <code>pOut</code> points to output signal and <code>pErr</code> points to error signal.
 * All arrays contain <code>blockSize</code> values.
 *
 * The functions operate on a block-by-block basis.
 * Internally, the filter coefficients <code>b[n]</code> are updated on a sample-by-sample basis.
 * The convergence of the LMS filter is slower compared to the normalized LMS algorithm.
 *
 * \par Algorithm:
 * The output signal <code>y[n]</code> is computed by a standard FIR filter:
 * <pre>
 *     y[n] = b[0] * x[n] + b[1] * x[n-1] + b[2] * x[n-2] + ...+ b[numTaps-1] * x[n-numTaps+1]
 * </pre>
 *
 * \par
 * The error signal equals the difference between the reference signal <code>d[n]</code> and the filter output:
 * <pre>
 *     e[n] = d[n] - y[n].
 * </pre>
 *
 * \par
 * After each sample of the error signal is computed, the filter coefficients <code>b[k]</code> are updated on a sample-by-sample basis:
 * <pre>
 *     b[k] = b[k] + e[n] * mu * x[n-k],  for k=0, 1, ..., numTaps-1
 * </pre>
 * where <code>mu</code> is the step size and controls the rate of coefficient convergence.
 *\par
 * In the APIs, <code>pCoeffs</code> points to a coefficient array of size <code>numTaps</code>.
 * Coefficients are stored in time reversed order.
 * \par
 * <pre>
 *    {b[numTaps-1], b[numTaps-2], b[N-2], ..., b[1], b[0]}
 * </pre>
 * \par
 * <code>pState</code> points to a state array of size <code>numTaps + blockSize - 1</code>.
 * Samples in the state buffer are stored in the order:
 * \par
 * <pre>
 *    {x[n-numTaps+1], x[n-numTaps], x[n-numTaps-1], x[n-numTaps-2]....x[0], x[1], ..., x[blockSize-1]}
 * </pre>
 * \par
 * Note that the length of the state buffer exceeds the length of the coefficient array by <code>blockSize-1</code> samples.
 * The increased state buffer length allows circular addressing, which is traditionally used in FIR filters,
 * to be avoided and yields a significant speed improvement.
 * The state variables are updated after each block of data is processed.
 * \par Instance Structure
 * The coefficients and state variables for a filter are stored together in an instance data structure.
 * A separate instance structure must be defined for each filter and
 * coefficient and state arrays cannot be shared among instances.
 * There are separate instance structure declarations for each of the 3 supported data types.
 *
 * \par Initialization Functions
 * There is also an associated initialization function for each data type.
 * The initialization function performs the following operations:
 * - Sets the values of the internal structure fields.
 * - Zeros out the values in the state buffer.
 * To do this manually without calling the init function, assign the follow subfields of the instance structure:
 * numTaps, pCoeffs, mu, postShift (not for f32), pState. Also set all of the values in pState to zero.
 *
 * \par
 * Use of the initialization function is optional.
 * However, if the initialization function is used, then the instance structure cannot be placed into a const data section.
 * To place an instance structure into a const data section, the instance structure must be manually initialized.
 * Set the values in the state buffer to zeros before static initialization.
 * The code below statically initializes each of the 3 different data type filter instance structures
 * <pre>
 *    arm_lms_instance_f32 S = {numTaps, pState, pCoeffs, mu};
 *    arm_lms_instance_q31 S = {numTaps, pState, pCoeffs, mu, postShift};
 *    arm_lms_instance_q15 S = {numTaps, pState, pCoeffs, mu, postShift};
 * </pre>
 * where <code>numTaps</code> is the number of filter coefficients in the filter; <code>pState</code> is the address of the state buffer;
 * <code>pCoeffs</code> is the address of the coefficient buffer; <code>mu</code> is the step size parameter; and <code>postShift</code> is the shift applied to coefficients.
 *
 * \par Fixed-Point Behavior:
 * Care must be taken when using the Q15 and Q31 versions of the LMS filter.
 * The following issues must be considered:
 * - Scaling of coefficients
 * - Overflow and saturation
 *
 * \par Scaling of Coefficients:
 * Filter coefficients are represented as fractional values and
 * coefficients are restricted to lie in the range <code>[-1 +1)</code>.
 * The fixed-point functions have an additional scaling parameter <code>postShift</code>.
 * At the output of the filter's accumulator is a shift register which shifts the result by <code>postShift</code> bits.
 * This essentially scales the filter coefficients by <code>2^postShift</code> and
 * allows the filter coefficients to exceed the range <code>[+1 -1)</code>.
 * The value of <code>postShift</code> is set by the user based on the expected gain through the system being modeled.
 *
 * \par Overflow and Saturation:
 * Overflow and saturation behavior of the fixed-point Q15 and Q31 versions are
 * described separately as part of the function specific documentation below.
 */

/**
 * @addtogroup LMS
 * @{
 */

/**
 * @details
 * This function operates on floating-point data types.
 *
 * @brief Processing function for floating-point LMS filter.
 * @param[in]  *S points to an instance of the floating-point LMS filter structure.
 * @param[in]  *pSrc points to the block of input data.
 * @param[in]  *pRef points to the block of reference data.
 * @param[out] *pOut points to the block of output data.
 * @param[out] *pErr points to the block of error data.
 * @param[in]  blockSize number of samples to process.
 * @return     none.
 */

void arm_lms_f32(
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
  float32_t *px, *pb;                            /* Temporary pointers for state and coefficient buffers */
  float32_t mu = S->mu;                          /* Adaptive factor */
  uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
  uint32_t tapCnt, blkCnt;                       /* Loop counters */
  float32_t sum, e, d;                           /* accumulator, error, reference data sample */
  float32_t w = 0.0f;                            /* weight factor */

  e = 0.0f;
  d = 0.0f;

  /* S->pState points to state array which contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = &(S->pState[(numTaps - 1U)]);

  blkCnt = blockSize;


#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  while (blkCnt > 0U)
  {
    /* Copy the new input sample into the state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Initialize pState pointer */
    px = pState;

    /* Initialize coeff pointer */
    pb = (pCoeffs);

    /* Set the accumulator to zero */
    sum = 0.0f;

    /* Loop unrolling.  Process 4 taps at a time. */
    tapCnt = numTaps >> 2;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      sum += (*px++) * (*pb++);
      sum += (*px++) * (*pb++);
      sum += (*px++) * (*pb++);
      sum += (*px++) * (*pb++);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* If the filter length is not a multiple of 4, compute the remaining filter taps */
    tapCnt = numTaps % 0x4U;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      sum += (*px++) * (*pb++);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* The result in the accumulator, store in the destination buffer. */
    *pOut++ = sum;

    /* Compute and store error */
    d = (float32_t) (*pRef++);
    e = d - sum;
    *pErr++ = e;

    /* Calculation of Weighting factor for the updating filter coefficients */
    w = e * mu;

    /* Initialize pState pointer */
    px = pState;

    /* Initialize coeff pointer */
    pb = (pCoeffs);

    /* Loop unrolling.  Process 4 taps at a time. */
    tapCnt = numTaps >> 2;

    /* Update filter coefficients */
    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      *pb = *pb + (w * (*px++));
      pb++;

      *pb = *pb + (w * (*px++));
      pb++;

      *pb = *pb + (w * (*px++));
      pb++;

      *pb = *pb + (w * (*px++));
      pb++;

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* If the filter length is not a multiple of 4, compute the remaining filter taps */
    tapCnt = numTaps % 0x4U;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      *pb = *pb + (w * (*px++));
      pb++;

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Advance state pointer by 1 for the next sample */
    pState = pState + 1;

    /* Decrement the loop counter */
    blkCnt--;
  }


  /* Processing is complete. Now copy the last numTaps - 1 samples to the
     satrt of the state buffer. This prepares the state buffer for the
     next function call. */

  /* Points to the start of the pState buffer */
  pStateCurnt = S->pState;

  /* Loop unrolling for (numTaps - 1U) samples copy */
  tapCnt = (numTaps - 1U) >> 2U;

  /* copy data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }

  /* Calculate remaining number of copies */
  tapCnt = (numTaps - 1U) % 0x4U;

  /* Copy the remaining q31_t data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }

#else

  /* Run the below code for Cortex-M0 */

  while (blkCnt > 0U)
  {
    /* Copy the new input sample into the state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Initialize pState pointer */
    px = pState;

    /* Initialize pCoeffs pointer */
    pb = pCoeffs;

    /* Set the accumulator to zero */
    sum = 0.0f;

    /* Loop over numTaps number of values */
    tapCnt = numTaps;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      sum += (*px++) * (*pb++);

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* The result is stored in the destination buffer. */
    *pOut++ = sum;

    /* Compute and store error */
    d = (float32_t) (*pRef++);
    e = d - sum;
    *pErr++ = e;

    /* Weighting factor for the LMS version */
    w = e * mu;

    /* Initialize pState pointer */
    px = pState;

    /* Initialize pCoeffs pointer */
    pb = pCoeffs;

    /* Loop over numTaps number of values */
    tapCnt = numTaps;

    while (tapCnt > 0U)
    {
      /* Perform the multiply-accumulate */
      *pb = *pb + (w * (*px++));
      pb++;

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Advance state pointer by 1 for the next sample */
    pState = pState + 1;

    /* Decrement the loop counter */
    blkCnt--;
  }


  /* Processing is complete. Now copy the last numTaps - 1 samples to the
   * start of the state buffer. This prepares the state buffer for the
   * next function call. */

  /* Points to the start of the pState buffer */
  pStateCurnt = S->pState;

  /*  Copy (numTaps - 1U) samples  */
  tapCnt = (numTaps - 1U);

  /* Copy the data */
  while (tapCnt > 0U)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }

#endif /*   #if defined (ARM_MATH_DSP) */

}

/**
   * @} end of LMS group
   */
