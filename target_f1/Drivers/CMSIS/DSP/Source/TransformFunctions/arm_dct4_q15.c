/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_dct4_q15.c
 * Description:  Processing function of DCT4 & IDCT4 Q15
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
 * @addtogroup DCT4_IDCT4
 * @{
 */

/**
 * @brief Processing function for the Q15 DCT4/IDCT4.
 * @param[in]       *S             points to an instance of the Q15 DCT4 structure.
 * @param[in]       *pState        points to state buffer.
 * @param[in,out]   *pInlineBuffer points to the in-place input and output buffer.
 * @return none.
 *
 * \par Input an output formats:
 * Internally inputs are downscaled in the RFFT process function to avoid overflows.
 * Number of bits downscaled, depends on the size of the transform.
 * The input and output formats for different DCT sizes and number of bits to upscale are mentioned in the table below:
 *
 * \image html dct4FormatsQ15Table.gif
 */

void arm_dct4_q15(
  const arm_dct4_instance_q15 * S,
  q15_t * pState,
  q15_t * pInlineBuffer)
{
  uint32_t i;                                    /* Loop counter */
  q15_t *weights = S->pTwiddle;                  /* Pointer to the Weights table */
  q15_t *cosFact = S->pCosFactor;                /* Pointer to the cos factors table */
  q15_t *pS1, *pS2, *pbuff;                      /* Temporary pointers for input buffer and pState buffer */
  q15_t in;                                      /* Temporary variable */


  /* DCT4 computation involves DCT2 (which is calculated using RFFT)
   * along with some pre-processing and post-processing.
   * Computational procedure is explained as follows:
   * (a) Pre-processing involves multiplying input with cos factor,
   *     r(n) = 2 * u(n) * cos(pi*(2*n+1)/(4*n))
   *              where,
   *                 r(n) -- output of preprocessing
   *                 u(n) -- input to preprocessing(actual Source buffer)
   * (b) Calculation of DCT2 using FFT is divided into three steps:
   *                  Step1: Re-ordering of even and odd elements of input.
   *                  Step2: Calculating FFT of the re-ordered input.
   *                  Step3: Taking the real part of the product of FFT output and weights.
   * (c) Post-processing - DCT4 can be obtained from DCT2 output using the following equation:
   *                   Y4(k) = Y2(k) - Y4(k-1) and Y4(-1) = Y4(0)
   *                        where,
   *                           Y4 -- DCT4 output,   Y2 -- DCT2 output
   * (d) Multiplying the output with the normalizing factor sqrt(2/N).
   */

        /*-------- Pre-processing ------------*/
  /* Multiplying input with cos factor i.e. r(n) = 2 * x(n) * cos(pi*(2*n+1)/(4*n)) */
  arm_mult_q15(pInlineBuffer, cosFact, pInlineBuffer, S->N);
  arm_shift_q15(pInlineBuffer, 1, pInlineBuffer, S->N);

  /* ----------------------------------------------------------------
   * Step1: Re-ordering of even and odd elements as
   *             pState[i] =  pInlineBuffer[2*i] and
   *             pState[N-i-1] = pInlineBuffer[2*i+1] where i = 0 to N/2
   ---------------------------------------------------------------------*/

  /* pS1 initialized to pState */
  pS1 = pState;

  /* pS2 initialized to pState+N-1, so that it points to the end of the state buffer */
  pS2 = pState + (S->N - 1U);

  /* pbuff initialized to input buffer */
  pbuff = pInlineBuffer;


#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  /* Initializing the loop counter to N/2 >> 2 for loop unrolling by 4 */
  i = (uint32_t) S->Nby2 >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  do
  {
    /* Re-ordering of even and odd elements */
    /* pState[i] =  pInlineBuffer[2*i] */
    *pS1++ = *pbuff++;
    /* pState[N-i-1] = pInlineBuffer[2*i+1] */
    *pS2-- = *pbuff++;

    *pS1++ = *pbuff++;
    *pS2-- = *pbuff++;

    *pS1++ = *pbuff++;
    *pS2-- = *pbuff++;

    *pS1++ = *pbuff++;
    *pS2-- = *pbuff++;

    /* Decrement the loop counter */
    i--;
  } while (i > 0U);

  /* pbuff initialized to input buffer */
  pbuff = pInlineBuffer;

  /* pS1 initialized to pState */
  pS1 = pState;

  /* Initializing the loop counter to N/4 instead of N for loop unrolling */
  i = (uint32_t) S->N >> 2U;

  /* Processing with loop unrolling 4 times as N is always multiple of 4.
   * Compute 4 outputs at a time */
  do
  {
    /* Writing the re-ordered output back to inplace input buffer */
    *pbuff++ = *pS1++;
    *pbuff++ = *pS1++;
    *pbuff++ = *pS1++;
    *pbuff++ = *pS1++;

    /* Decrement the loop counter */
    i--;
  } while (i > 0U);


  /* ---------------------------------------------------------
   *     Step2: Calculate RFFT for N-point input
   * ---------------------------------------------------------- */
  /* pInlineBuffer is real input of length N , pState is the complex output of length 2N */
  arm_rfft_q15(S->pRfft, pInlineBuffer, pState);

 /*----------------------------------------------------------------------
  *  Step3: Multiply the FFT output with the weights.
  *----------------------------------------------------------------------*/
  arm_cmplx_mult_cmplx_q15(pState, weights, pState, S->N);

  /* The output of complex multiplication is in 3.13 format.
   * Hence changing the format of N (i.e. 2*N elements) complex numbers to 1.15 format by shifting left by 2 bits. */
  arm_shift_q15(pState, 2, pState, S->N * 2);

  /* ----------- Post-processing ---------- */
  /* DCT-IV can be obtained from DCT-II by the equation,
   *       Y4(k) = Y2(k) - Y4(k-1) and Y4(-1) = Y4(0)
   *       Hence, Y4(0) = Y2(0)/2  */
  /* Getting only real part from the output and Converting to DCT-IV */

  /* Initializing the loop counter to N >> 2 for loop unrolling by 4 */
  i = ((uint32_t) S->N - 1U) >> 2U;

  /* pbuff initialized to input buffer. */
  pbuff = pInlineBuffer;

  /* pS1 initialized to pState */
  pS1 = pState;

  /* Calculating Y4(0) from Y2(0) using Y4(0) = Y2(0)/2 */
  in = *pS1++ >> 1U;
  /* input buffer acts as inplace, so output values are stored in the input itself. */
  *pbuff++ = in;

  /* pState pointer is incremented twice as the real values are located alternatively in the array */
  pS1++;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  do
  {
    /* Calculating Y4(1) to Y4(N-1) from Y2 using equation Y4(k) = Y2(k) - Y4(k-1) */
    /* pState pointer (pS1) is incremented twice as the real values are located alternatively in the array */
    in = *pS1++ - in;
    *pbuff++ = in;
    /* points to the next real value */
    pS1++;

    in = *pS1++ - in;
    *pbuff++ = in;
    pS1++;

    in = *pS1++ - in;
    *pbuff++ = in;
    pS1++;

    in = *pS1++ - in;
    *pbuff++ = in;
    pS1++;

    /* Decrement the loop counter */
    i--;
  } while (i > 0U);

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  i = ((uint32_t) S->N - 1U) % 0x4U;

  while (i > 0U)
  {
    /* Calculating Y4(1) to Y4(N-1) from Y2 using equation Y4(k) = Y2(k) - Y4(k-1) */
    /* pState pointer (pS1) is incremented twice as the real values are located alternatively in the array */
    in = *pS1++ - in;
    *pbuff++ = in;
    /* points to the next real value */
    pS1++;

    /* Decrement the loop counter */
    i--;
  }


   /*------------ Normalizing the output by multiplying with the normalizing factor ----------*/

  /* Initializing the loop counter to N/4 instead of N for loop unrolling */
  i = (uint32_t) S->N >> 2U;

  /* pbuff initialized to the pInlineBuffer(now contains the output values) */
  pbuff = pInlineBuffer;

  /* Processing with loop unrolling 4 times as N is always multiple of 4.  Compute 4 outputs at a time */
  do
  {
    /* Multiplying pInlineBuffer with the normalizing factor sqrt(2/N) */
    in = *pbuff;
    *pbuff++ = ((q15_t) (((q31_t) in * S->normalize) >> 15));

    in = *pbuff;
    *pbuff++ = ((q15_t) (((q31_t) in * S->normalize) >> 15));

    in = *pbuff;
    *pbuff++ = ((q15_t) (((q31_t) in * S->normalize) >> 15));

    in = *pbuff;
    *pbuff++ = ((q15_t) (((q31_t) in * S->normalize) >> 15));

    /* Decrement the loop counter */
    i--;
  } while (i > 0U);


#else

  /* Run the below code for Cortex-M0 */

  /* Initializing the loop counter to N/2 */
  i = (uint32_t) S->Nby2;

  do
  {
    /* Re-ordering of even and odd elements */
    /* pState[i] =  pInlineBuffer[2*i] */
    *pS1++ = *pbuff++;
    /* pState[N-i-1] = pInlineBuffer[2*i+1] */
    *pS2-- = *pbuff++;

    /* Decrement the loop counter */
    i--;
  } while (i > 0U);

  /* pbuff initialized to input buffer */
  pbuff = pInlineBuffer;

  /* pS1 initialized to pState */
  pS1 = pState;

  /* Initializing the loop counter */
  i = (uint32_t) S->N;

  do
  {
    /* Writing the re-ordered output back to inplace input buffer */
    *pbuff++ = *pS1++;

    /* Decrement the loop counter */
    i--;
  } while (i > 0U);


  /* ---------------------------------------------------------
   *     Step2: Calculate RFFT for N-point input
   * ---------------------------------------------------------- */
  /* pInlineBuffer is real input of length N , pState is the complex output of length 2N */
  arm_rfft_q15(S->pRfft, pInlineBuffer, pState);

 /*----------------------------------------------------------------------
  *  Step3: Multiply the FFT output with the weights.
  *----------------------------------------------------------------------*/
  arm_cmplx_mult_cmplx_q15(pState, weights, pState, S->N);

  /* The output of complex multiplication is in 3.13 format.
   * Hence changing the format of N (i.e. 2*N elements) complex numbers to 1.15 format by shifting left by 2 bits. */
  arm_shift_q15(pState, 2, pState, S->N * 2);

  /* ----------- Post-processing ---------- */
  /* DCT-IV can be obtained from DCT-II by the equation,
   *       Y4(k) = Y2(k) - Y4(k-1) and Y4(-1) = Y4(0)
   *       Hence, Y4(0) = Y2(0)/2  */
  /* Getting only real part from the output and Converting to DCT-IV */

  /* Initializing the loop counter */
  i = ((uint32_t) S->N - 1U);

  /* pbuff initialized to input buffer. */
  pbuff = pInlineBuffer;

  /* pS1 initialized to pState */
  pS1 = pState;

  /* Calculating Y4(0) from Y2(0) using Y4(0) = Y2(0)/2 */
  in = *pS1++ >> 1U;
  /* input buffer acts as inplace, so output values are stored in the input itself. */
  *pbuff++ = in;

  /* pState pointer is incremented twice as the real values are located alternatively in the array */
  pS1++;

  do
  {
    /* Calculating Y4(1) to Y4(N-1) from Y2 using equation Y4(k) = Y2(k) - Y4(k-1) */
    /* pState pointer (pS1) is incremented twice as the real values are located alternatively in the array */
    in = *pS1++ - in;
    *pbuff++ = in;
    /* points to the next real value */
    pS1++;

    /* Decrement the loop counter */
    i--;
  } while (i > 0U);

   /*------------ Normalizing the output by multiplying with the normalizing factor ----------*/

  /* Initializing the loop counter */
  i = (uint32_t) S->N;

  /* pbuff initialized to the pInlineBuffer(now contains the output values) */
  pbuff = pInlineBuffer;

  do
  {
    /* Multiplying pInlineBuffer with the normalizing factor sqrt(2/N) */
    in = *pbuff;
    *pbuff++ = ((q15_t) (((q31_t) in * S->normalize) >> 15));

    /* Decrement the loop counter */
    i--;
  } while (i > 0U);

#endif /* #if defined (ARM_MATH_DSP) */

}

/**
   * @} end of DCT4_IDCT4 group
   */
