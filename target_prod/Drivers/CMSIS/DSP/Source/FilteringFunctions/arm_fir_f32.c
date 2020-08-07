/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fir_f32.c
 * Description:  Floating-point FIR filter processing function
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
* @defgroup FIR Finite Impulse Response (FIR) Filters
*
* This set of functions implements Finite Impulse Response (FIR) filters
* for Q7, Q15, Q31, and floating-point data types.  Fast versions of Q15 and Q31 are also provided.
* The functions operate on blocks of input and output data and each call to the function processes
* <code>blockSize</code> samples through the filter.  <code>pSrc</code> and
* <code>pDst</code> points to input and output arrays containing <code>blockSize</code> values.
*
* \par Algorithm:
* The FIR filter algorithm is based upon a sequence of multiply-accumulate (MAC) operations.
* Each filter coefficient <code>b[n]</code> is multiplied by a state variable which equals a previous input sample <code>x[n]</code>.
* <pre>
*    y[n] = b[0] * x[n] + b[1] * x[n-1] + b[2] * x[n-2] + ...+ b[numTaps-1] * x[n-numTaps+1]
* </pre>
* \par
* \image html FIR.gif "Finite Impulse Response filter"
* \par
* <code>pCoeffs</code> points to a coefficient array of size <code>numTaps</code>.
* Coefficients are stored in time reversed order.
* \par
* <pre>
*    {b[numTaps-1], b[numTaps-2], b[N-2], ..., b[1], b[0]}
* </pre>
* \par
* <code>pState</code> points to a state array of size <code>numTaps + blockSize - 1</code>.
* Samples in the state buffer are stored in the following order.
* \par
* <pre>
*    {x[n-numTaps+1], x[n-numTaps], x[n-numTaps-1], x[n-numTaps-2]....x[0], x[1], ..., x[blockSize-1]}
* </pre>
* \par
* Note that the length of the state buffer exceeds the length of the coefficient array by <code>blockSize-1</code>.
* The increased state buffer length allows circular addressing, which is traditionally used in the FIR filters,
* to be avoided and yields a significant speed improvement.
* The state variables are updated after each block of data is processed; the coefficients are untouched.
* \par Instance Structure
* The coefficients and state variables for a filter are stored together in an instance data structure.
* A separate instance structure must be defined for each filter.
* Coefficient arrays may be shared among several instances while state variable arrays cannot be shared.
* There are separate instance structure declarations for each of the 4 supported data types.
*
* \par Initialization Functions
* There is also an associated initialization function for each data type.
* The initialization function performs the following operations:
* - Sets the values of the internal structure fields.
* - Zeros out the values in the state buffer.
* To do this manually without calling the init function, assign the follow subfields of the instance structure:
* numTaps, pCoeffs, pState. Also set all of the values in pState to zero.
*
* \par
* Use of the initialization function is optional.
* However, if the initialization function is used, then the instance structure cannot be placed into a const data section.
* To place an instance structure into a const data section, the instance structure must be manually initialized.
* Set the values in the state buffer to zeros before static initialization.
* The code below statically initializes each of the 4 different data type filter instance structures
* <pre>
*arm_fir_instance_f32 S = {numTaps, pState, pCoeffs};
*arm_fir_instance_q31 S = {numTaps, pState, pCoeffs};
*arm_fir_instance_q15 S = {numTaps, pState, pCoeffs};
*arm_fir_instance_q7 S =  {numTaps, pState, pCoeffs};
* </pre>
*
* where <code>numTaps</code> is the number of filter coefficients in the filter; <code>pState</code> is the address of the state buffer;
* <code>pCoeffs</code> is the address of the coefficient buffer.
*
* \par Fixed-Point Behavior
* Care must be taken when using the fixed-point versions of the FIR filter functions.
* In particular, the overflow and saturation behavior of the accumulator used in each function must be considered.
* Refer to the function specific documentation below for usage guidelines.
*/

/**
* @addtogroup FIR
* @{
*/

/**
*
* @param[in]  *S points to an instance of the floating-point FIR filter structure.
* @param[in]  *pSrc points to the block of input data.
* @param[out] *pDst points to the block of output data.
* @param[in]  blockSize number of samples to process per call.
* @return     none.
*
*/

#if defined(ARM_MATH_CM7)

void arm_fir_f32(
const arm_fir_instance_f32 * S,
float32_t * pSrc,
float32_t * pDst,
uint32_t blockSize)
{
   float32_t *pState = S->pState;                 /* State pointer */
   float32_t *pCoeffs = S->pCoeffs;               /* Coefficient pointer */
   float32_t *pStateCurnt;                        /* Points to the current sample of the state */
   float32_t *px, *pb;                            /* Temporary pointers for state and coefficient buffers */
   float32_t acc0, acc1, acc2, acc3, acc4, acc5, acc6, acc7;     /* Accumulators */
   float32_t x0, x1, x2, x3, x4, x5, x6, x7, c0;  /* Temporary variables to hold state and coefficient values */
   uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
   uint32_t i, tapCnt, blkCnt;                    /* Loop counters */

   /* S->pState points to state array which contains previous frame (numTaps - 1) samples */
   /* pStateCurnt points to the location where the new input data should be written */
   pStateCurnt = &(S->pState[(numTaps - 1U)]);

   /* Apply loop unrolling and compute 8 output values simultaneously.
    * The variables acc0 ... acc7 hold output values that are being computed:
    *
    *    acc0 =  b[numTaps-1] * x[n-numTaps-1] + b[numTaps-2] * x[n-numTaps-2] + b[numTaps-3] * x[n-numTaps-3] +...+ b[0] * x[0]
    *    acc1 =  b[numTaps-1] * x[n-numTaps] +   b[numTaps-2] * x[n-numTaps-1] + b[numTaps-3] * x[n-numTaps-2] +...+ b[0] * x[1]
    *    acc2 =  b[numTaps-1] * x[n-numTaps+1] + b[numTaps-2] * x[n-numTaps] +   b[numTaps-3] * x[n-numTaps-1] +...+ b[0] * x[2]
    *    acc3 =  b[numTaps-1] * x[n-numTaps+2] + b[numTaps-2] * x[n-numTaps+1] + b[numTaps-3] * x[n-numTaps]   +...+ b[0] * x[3]
    */
   blkCnt = blockSize >> 3;

   /* First part of the processing with loop unrolling.  Compute 8 outputs at a time.
   ** a second loop below computes the remaining 1 to 7 samples. */
   while (blkCnt > 0U)
   {
      /* Copy four new input samples into the state buffer */
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;

      /* Set all accumulators to zero */
      acc0 = 0.0f;
      acc1 = 0.0f;
      acc2 = 0.0f;
      acc3 = 0.0f;
      acc4 = 0.0f;
      acc5 = 0.0f;
      acc6 = 0.0f;
      acc7 = 0.0f;

      /* Initialize state pointer */
      px = pState;

      /* Initialize coeff pointer */
      pb = (pCoeffs);

      /* This is separated from the others to avoid
       * a call to __aeabi_memmove which would be slower
       */
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;

      /* Read the first seven samples from the state buffer:  x[n-numTaps], x[n-numTaps-1], x[n-numTaps-2] */
      x0 = *px++;
      x1 = *px++;
      x2 = *px++;
      x3 = *px++;
      x4 = *px++;
      x5 = *px++;
      x6 = *px++;

      /* Loop unrolling.  Process 8 taps at a time. */
      tapCnt = numTaps >> 3U;

      /* Loop over the number of taps.  Unroll by a factor of 8.
       ** Repeat until we've computed numTaps-8 coefficients. */
      while (tapCnt > 0U)
      {
         /* Read the b[numTaps-1] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-3] sample */
         x7 = *(px++);

         /* acc0 +=  b[numTaps-1] * x[n-numTaps] */
         acc0 += x0 * c0;

         /* acc1 +=  b[numTaps-1] * x[n-numTaps-1] */
         acc1 += x1 * c0;

         /* acc2 +=  b[numTaps-1] * x[n-numTaps-2] */
         acc2 += x2 * c0;

         /* acc3 +=  b[numTaps-1] * x[n-numTaps-3] */
         acc3 += x3 * c0;

         /* acc4 +=  b[numTaps-1] * x[n-numTaps-4] */
         acc4 += x4 * c0;

         /* acc1 +=  b[numTaps-1] * x[n-numTaps-5] */
         acc5 += x5 * c0;

         /* acc2 +=  b[numTaps-1] * x[n-numTaps-6] */
         acc6 += x6 * c0;

         /* acc3 +=  b[numTaps-1] * x[n-numTaps-7] */
         acc7 += x7 * c0;

         /* Read the b[numTaps-2] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-4] sample */
         x0 = *(px++);

         /* Perform the multiply-accumulate */
         acc0 += x1 * c0;
         acc1 += x2 * c0;
         acc2 += x3 * c0;
         acc3 += x4 * c0;
         acc4 += x5 * c0;
         acc5 += x6 * c0;
         acc6 += x7 * c0;
         acc7 += x0 * c0;

         /* Read the b[numTaps-3] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-5] sample */
         x1 = *(px++);

         /* Perform the multiply-accumulates */
         acc0 += x2 * c0;
         acc1 += x3 * c0;
         acc2 += x4 * c0;
         acc3 += x5 * c0;
         acc4 += x6 * c0;
         acc5 += x7 * c0;
         acc6 += x0 * c0;
         acc7 += x1 * c0;

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x2 = *(px++);

         /* Perform the multiply-accumulates */
         acc0 += x3 * c0;
         acc1 += x4 * c0;
         acc2 += x5 * c0;
         acc3 += x6 * c0;
         acc4 += x7 * c0;
         acc5 += x0 * c0;
         acc6 += x1 * c0;
         acc7 += x2 * c0;

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x3 = *(px++);
         /* Perform the multiply-accumulates */
         acc0 += x4 * c0;
         acc1 += x5 * c0;
         acc2 += x6 * c0;
         acc3 += x7 * c0;
         acc4 += x0 * c0;
         acc5 += x1 * c0;
         acc6 += x2 * c0;
         acc7 += x3 * c0;

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x4 = *(px++);

         /* Perform the multiply-accumulates */
         acc0 += x5 * c0;
         acc1 += x6 * c0;
         acc2 += x7 * c0;
         acc3 += x0 * c0;
         acc4 += x1 * c0;
         acc5 += x2 * c0;
         acc6 += x3 * c0;
         acc7 += x4 * c0;

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x5 = *(px++);

         /* Perform the multiply-accumulates */
         acc0 += x6 * c0;
         acc1 += x7 * c0;
         acc2 += x0 * c0;
         acc3 += x1 * c0;
         acc4 += x2 * c0;
         acc5 += x3 * c0;
         acc6 += x4 * c0;
         acc7 += x5 * c0;

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x6 = *(px++);

         /* Perform the multiply-accumulates */
         acc0 += x7 * c0;
         acc1 += x0 * c0;
         acc2 += x1 * c0;
         acc3 += x2 * c0;
         acc4 += x3 * c0;
         acc5 += x4 * c0;
         acc6 += x5 * c0;
         acc7 += x6 * c0;

         tapCnt--;
      }

      /* If the filter length is not a multiple of 8, compute the remaining filter taps */
      tapCnt = numTaps % 0x8U;

      while (tapCnt > 0U)
      {
         /* Read coefficients */
         c0 = *(pb++);

         /* Fetch 1 state variable */
         x7 = *(px++);

         /* Perform the multiply-accumulates */
         acc0 += x0 * c0;
         acc1 += x1 * c0;
         acc2 += x2 * c0;
         acc3 += x3 * c0;
         acc4 += x4 * c0;
         acc5 += x5 * c0;
         acc6 += x6 * c0;
         acc7 += x7 * c0;

         /* Reuse the present sample states for next sample */
         x0 = x1;
         x1 = x2;
         x2 = x3;
         x3 = x4;
         x4 = x5;
         x5 = x6;
         x6 = x7;

         /* Decrement the loop counter */
         tapCnt--;
      }

      /* Advance the state pointer by 8 to process the next group of 8 samples */
      pState = pState + 8;

      /* The results in the 8 accumulators, store in the destination buffer. */
      *pDst++ = acc0;
      *pDst++ = acc1;
      *pDst++ = acc2;
      *pDst++ = acc3;
      *pDst++ = acc4;
      *pDst++ = acc5;
      *pDst++ = acc6;
      *pDst++ = acc7;

      blkCnt--;
   }

   /* If the blockSize is not a multiple of 8, compute any remaining output samples here.
   ** No loop unrolling is used. */
   blkCnt = blockSize % 0x8U;

   while (blkCnt > 0U)
   {
      /* Copy one sample at a time into state buffer */
      *pStateCurnt++ = *pSrc++;

      /* Set the accumulator to zero */
      acc0 = 0.0f;

      /* Initialize state pointer */
      px = pState;

      /* Initialize Coefficient pointer */
      pb = (pCoeffs);

      i = numTaps;

      /* Perform the multiply-accumulates */
      do
      {
         acc0 += *px++ * *pb++;
         i--;

      } while (i > 0U);

      /* The result is store in the destination buffer. */
      *pDst++ = acc0;

      /* Advance state pointer by 1 for the next sample */
      pState = pState + 1;

      blkCnt--;
   }

   /* Processing is complete.
   ** Now copy the last numTaps - 1 samples to the start of the state buffer.
   ** This prepares the state buffer for the next function call. */

   /* Points to the start of the state buffer */
   pStateCurnt = S->pState;

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
}

#elif defined(ARM_MATH_CM0_FAMILY)

void arm_fir_f32(
const arm_fir_instance_f32 * S,
float32_t * pSrc,
float32_t * pDst,
uint32_t blockSize)
{
   float32_t *pState = S->pState;                 /* State pointer */
   float32_t *pCoeffs = S->pCoeffs;               /* Coefficient pointer */
   float32_t *pStateCurnt;                        /* Points to the current sample of the state */
   float32_t *px, *pb;                            /* Temporary pointers for state and coefficient buffers */
   uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
   uint32_t i, tapCnt, blkCnt;                    /* Loop counters */

   /* Run the below code for Cortex-M0 */

   float32_t acc;

   /* S->pState points to state array which contains previous frame (numTaps - 1) samples */
   /* pStateCurnt points to the location where the new input data should be written */
   pStateCurnt = &(S->pState[(numTaps - 1U)]);

   /* Initialize blkCnt with blockSize */
   blkCnt = blockSize;

   while (blkCnt > 0U)
   {
      /* Copy one sample at a time into state buffer */
      *pStateCurnt++ = *pSrc++;

      /* Set the accumulator to zero */
      acc = 0.0f;

      /* Initialize state pointer */
      px = pState;

      /* Initialize Coefficient pointer */
      pb = pCoeffs;

      i = numTaps;

      /* Perform the multiply-accumulates */
      do
      {
         /* acc =  b[numTaps-1] * x[n-numTaps-1] + b[numTaps-2] * x[n-numTaps-2] + b[numTaps-3] * x[n-numTaps-3] +...+ b[0] * x[0] */
         acc += *px++ * *pb++;
         i--;

      } while (i > 0U);

      /* The result is store in the destination buffer. */
      *pDst++ = acc;

      /* Advance state pointer by 1 for the next sample */
      pState = pState + 1;

      blkCnt--;
   }

   /* Processing is complete.
   ** Now copy the last numTaps - 1 samples to the starting of the state buffer.
   ** This prepares the state buffer for the next function call. */

   /* Points to the start of the state buffer */
   pStateCurnt = S->pState;

   /* Copy numTaps number of values */
   tapCnt = numTaps - 1U;

   /* Copy data */
   while (tapCnt > 0U)
   {
      *pStateCurnt++ = *pState++;

      /* Decrement the loop counter */
      tapCnt--;
   }

}

#else

/* Run the below code for Cortex-M4 and Cortex-M3 */

void arm_fir_f32(
const arm_fir_instance_f32 * S,
float32_t * pSrc,
float32_t * pDst,
uint32_t blockSize)
{
   float32_t *pState = S->pState;                 /* State pointer */
   float32_t *pCoeffs = S->pCoeffs;               /* Coefficient pointer */
   float32_t *pStateCurnt;                        /* Points to the current sample of the state */
   float32_t *px, *pb;                            /* Temporary pointers for state and coefficient buffers */
   float32_t acc0, acc1, acc2, acc3, acc4, acc5, acc6, acc7;     /* Accumulators */
   float32_t x0, x1, x2, x3, x4, x5, x6, x7, c0;  /* Temporary variables to hold state and coefficient values */
   uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
   uint32_t i, tapCnt, blkCnt;                    /* Loop counters */
   float32_t p0,p1,p2,p3,p4,p5,p6,p7;             /* Temporary product values */

   /* S->pState points to state array which contains previous frame (numTaps - 1) samples */
   /* pStateCurnt points to the location where the new input data should be written */
   pStateCurnt = &(S->pState[(numTaps - 1U)]);

   /* Apply loop unrolling and compute 8 output values simultaneously.
    * The variables acc0 ... acc7 hold output values that are being computed:
    *
    *    acc0 =  b[numTaps-1] * x[n-numTaps-1] + b[numTaps-2] * x[n-numTaps-2] + b[numTaps-3] * x[n-numTaps-3] +...+ b[0] * x[0]
    *    acc1 =  b[numTaps-1] * x[n-numTaps] +   b[numTaps-2] * x[n-numTaps-1] + b[numTaps-3] * x[n-numTaps-2] +...+ b[0] * x[1]
    *    acc2 =  b[numTaps-1] * x[n-numTaps+1] + b[numTaps-2] * x[n-numTaps] +   b[numTaps-3] * x[n-numTaps-1] +...+ b[0] * x[2]
    *    acc3 =  b[numTaps-1] * x[n-numTaps+2] + b[numTaps-2] * x[n-numTaps+1] + b[numTaps-3] * x[n-numTaps]   +...+ b[0] * x[3]
    */
   blkCnt = blockSize >> 3;

   /* First part of the processing with loop unrolling.  Compute 8 outputs at a time.
   ** a second loop below computes the remaining 1 to 7 samples. */
   while (blkCnt > 0U)
   {
      /* Copy four new input samples into the state buffer */
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;

      /* Set all accumulators to zero */
      acc0 = 0.0f;
      acc1 = 0.0f;
      acc2 = 0.0f;
      acc3 = 0.0f;
      acc4 = 0.0f;
      acc5 = 0.0f;
      acc6 = 0.0f;
      acc7 = 0.0f;

      /* Initialize state pointer */
      px = pState;

      /* Initialize coeff pointer */
      pb = (pCoeffs);

      /* This is separated from the others to avoid
       * a call to __aeabi_memmove which would be slower
       */
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;

      /* Read the first seven samples from the state buffer:  x[n-numTaps], x[n-numTaps-1], x[n-numTaps-2] */
      x0 = *px++;
      x1 = *px++;
      x2 = *px++;
      x3 = *px++;
      x4 = *px++;
      x5 = *px++;
      x6 = *px++;

      /* Loop unrolling.  Process 8 taps at a time. */
      tapCnt = numTaps >> 3U;

      /* Loop over the number of taps.  Unroll by a factor of 8.
       ** Repeat until we've computed numTaps-8 coefficients. */
      while (tapCnt > 0U)
      {
         /* Read the b[numTaps-1] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-3] sample */
         x7 = *(px++);

         /* acc0 +=  b[numTaps-1] * x[n-numTaps] */
         p0 = x0 * c0;

         /* acc1 +=  b[numTaps-1] * x[n-numTaps-1] */
         p1 = x1 * c0;

         /* acc2 +=  b[numTaps-1] * x[n-numTaps-2] */
         p2 = x2 * c0;

         /* acc3 +=  b[numTaps-1] * x[n-numTaps-3] */
         p3 = x3 * c0;

         /* acc4 +=  b[numTaps-1] * x[n-numTaps-4] */
         p4 = x4 * c0;

         /* acc1 +=  b[numTaps-1] * x[n-numTaps-5] */
         p5 = x5 * c0;

         /* acc2 +=  b[numTaps-1] * x[n-numTaps-6] */
         p6 = x6 * c0;

         /* acc3 +=  b[numTaps-1] * x[n-numTaps-7] */
         p7 = x7 * c0;

         /* Read the b[numTaps-2] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-4] sample */
         x0 = *(px++);

         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;


         /* Perform the multiply-accumulate */
         p0 = x1 * c0;
         p1 = x2 * c0;
         p2 = x3 * c0;
         p3 = x4 * c0;
         p4 = x5 * c0;
         p5 = x6 * c0;
         p6 = x7 * c0;
         p7 = x0 * c0;

         /* Read the b[numTaps-3] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-5] sample */
         x1 = *(px++);

         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Perform the multiply-accumulates */
         p0 = x2 * c0;
         p1 = x3 * c0;
         p2 = x4 * c0;
         p3 = x5 * c0;
         p4 = x6 * c0;
         p5 = x7 * c0;
         p6 = x0 * c0;
         p7 = x1 * c0;

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x2 = *(px++);

         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Perform the multiply-accumulates */
         p0 = x3 * c0;
         p1 = x4 * c0;
         p2 = x5 * c0;
         p3 = x6 * c0;
         p4 = x7 * c0;
         p5 = x0 * c0;
         p6 = x1 * c0;
         p7 = x2 * c0;

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x3 = *(px++);

         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Perform the multiply-accumulates */
         p0 = x4 * c0;
         p1 = x5 * c0;
         p2 = x6 * c0;
         p3 = x7 * c0;
         p4 = x0 * c0;
         p5 = x1 * c0;
         p6 = x2 * c0;
         p7 = x3 * c0;

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x4 = *(px++);

         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Perform the multiply-accumulates */
         p0 = x5 * c0;
         p1 = x6 * c0;
         p2 = x7 * c0;
         p3 = x0 * c0;
         p4 = x1 * c0;
         p5 = x2 * c0;
         p6 = x3 * c0;
         p7 = x4 * c0;

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x5 = *(px++);

         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Perform the multiply-accumulates */
         p0 = x6 * c0;
         p1 = x7 * c0;
         p2 = x0 * c0;
         p3 = x1 * c0;
         p4 = x2 * c0;
         p5 = x3 * c0;
         p6 = x4 * c0;
         p7 = x5 * c0;

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x6 = *(px++);

         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Perform the multiply-accumulates */
         p0 = x7 * c0;
         p1 = x0 * c0;
         p2 = x1 * c0;
         p3 = x2 * c0;
         p4 = x3 * c0;
         p5 = x4 * c0;
         p6 = x5 * c0;
         p7 = x6 * c0;

         tapCnt--;

         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;
      }

      /* If the filter length is not a multiple of 8, compute the remaining filter taps */
      tapCnt = numTaps % 0x8U;

      while (tapCnt > 0U)
      {
         /* Read coefficients */
         c0 = *(pb++);

         /* Fetch 1 state variable */
         x7 = *(px++);

         /* Perform the multiply-accumulates */
         p0 = x0 * c0;
         p1 = x1 * c0;
         p2 = x2 * c0;
         p3 = x3 * c0;
         p4 = x4 * c0;
         p5 = x5 * c0;
         p6 = x6 * c0;
         p7 = x7 * c0;

         /* Reuse the present sample states for next sample */
         x0 = x1;
         x1 = x2;
         x2 = x3;
         x3 = x4;
         x4 = x5;
         x5 = x6;
         x6 = x7;

         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Decrement the loop counter */
         tapCnt--;
      }

      /* Advance the state pointer by 8 to process the next group of 8 samples */
      pState = pState + 8;

      /* The results in the 8 accumulators, store in the destination buffer. */
      *pDst++ = acc0;
      *pDst++ = acc1;
      *pDst++ = acc2;
      *pDst++ = acc3;
      *pDst++ = acc4;
      *pDst++ = acc5;
      *pDst++ = acc6;
      *pDst++ = acc7;

      blkCnt--;
   }

   /* If the blockSize is not a multiple of 8, compute any remaining output samples here.
   ** No loop unrolling is used. */
   blkCnt = blockSize % 0x8U;

   while (blkCnt > 0U)
   {
      /* Copy one sample at a time into state buffer */
      *pStateCurnt++ = *pSrc++;

      /* Set the accumulator to zero */
      acc0 = 0.0f;

      /* Initialize state pointer */
      px = pState;

      /* Initialize Coefficient pointer */
      pb = (pCoeffs);

      i = numTaps;

      /* Perform the multiply-accumulates */
      do
      {
         acc0 += *px++ * *pb++;
         i--;

      } while (i > 0U);

      /* The result is store in the destination buffer. */
      *pDst++ = acc0;

      /* Advance state pointer by 1 for the next sample */
      pState = pState + 1;

      blkCnt--;
   }

   /* Processing is complete.
   ** Now copy the last numTaps - 1 samples to the start of the state buffer.
   ** This prepares the state buffer for the next function call. */

   /* Points to the start of the state buffer */
   pStateCurnt = S->pState;

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
}

#endif

/**
* @} end of FIR group
*/
