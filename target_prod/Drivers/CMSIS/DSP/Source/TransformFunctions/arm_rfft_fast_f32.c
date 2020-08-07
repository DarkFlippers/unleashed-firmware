/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_rfft_f32.c
 * Description:  RFFT & RIFFT Floating point process function
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

void stage_rfft_f32(
  arm_rfft_fast_instance_f32 * S,
  float32_t * p, float32_t * pOut)
{
   uint32_t  k;								   /* Loop Counter                     */
   float32_t twR, twI;						   /* RFFT Twiddle coefficients        */
   float32_t * pCoeff = S->pTwiddleRFFT;  /* Points to RFFT Twiddle factors   */
   float32_t *pA = p;						   /* increasing pointer               */
   float32_t *pB = p;						   /* decreasing pointer               */
   float32_t xAR, xAI, xBR, xBI;				/* temporary variables              */
   float32_t t1a, t1b;				         /* temporary variables              */
   float32_t p0, p1, p2, p3;				   /* temporary variables              */


   k = (S->Sint).fftLen - 1;

   /* Pack first and last sample of the frequency domain together */

   xBR = pB[0];
   xBI = pB[1];
   xAR = pA[0];
   xAI = pA[1];

   twR = *pCoeff++ ;
   twI = *pCoeff++ ;

   // U1 = XA(1) + XB(1); % It is real
   t1a = xBR + xAR  ;

   // U2 = XB(1) - XA(1); % It is imaginary
   t1b = xBI + xAI  ;

   // real(tw * (xB - xA)) = twR * (xBR - xAR) - twI * (xBI - xAI);
   // imag(tw * (xB - xA)) = twI * (xBR - xAR) + twR * (xBI - xAI);
   *pOut++ = 0.5f * ( t1a + t1b );
   *pOut++ = 0.5f * ( t1a - t1b );

   // XA(1) = 1/2*( U1 - imag(U2) +  i*( U1 +imag(U2) ));
   pB  = p + 2*k;
   pA += 2;

   do
   {
      /*
         function X = my_split_rfft(X, ifftFlag)
         % X is a series of real numbers
         L  = length(X);
         XC = X(1:2:end) +i*X(2:2:end);
         XA = fft(XC);
         XB = conj(XA([1 end:-1:2]));
         TW = i*exp(-2*pi*i*[0:L/2-1]/L).';
         for l = 2:L/2
            XA(l) = 1/2 * (XA(l) + XB(l) + TW(l) * (XB(l) - XA(l)));
         end
         XA(1) = 1/2* (XA(1) + XB(1) + TW(1) * (XB(1) - XA(1))) + i*( 1/2*( XA(1) + XB(1) + i*( XA(1) - XB(1))));
         X = XA;
      */

      xBI = pB[1];
      xBR = pB[0];
      xAR = pA[0];
      xAI = pA[1];

      twR = *pCoeff++;
      twI = *pCoeff++;

      t1a = xBR - xAR ;
      t1b = xBI + xAI ;

      // real(tw * (xB - xA)) = twR * (xBR - xAR) - twI * (xBI - xAI);
      // imag(tw * (xB - xA)) = twI * (xBR - xAR) + twR * (xBI - xAI);
      p0 = twR * t1a;
      p1 = twI * t1a;
      p2 = twR * t1b;
      p3 = twI * t1b;

      *pOut++ = 0.5f * (xAR + xBR + p0 + p3 ); //xAR
      *pOut++ = 0.5f * (xAI - xBI + p1 - p2 ); //xAI

      pA += 2;
      pB -= 2;
      k--;
   } while (k > 0U);
}

/* Prepares data for inverse cfft */
void merge_rfft_f32(
arm_rfft_fast_instance_f32 * S,
float32_t * p, float32_t * pOut)
{
   uint32_t  k;								/* Loop Counter                     */
   float32_t twR, twI;						/* RFFT Twiddle coefficients        */
   float32_t *pCoeff = S->pTwiddleRFFT;		/* Points to RFFT Twiddle factors   */
   float32_t *pA = p;						/* increasing pointer               */
   float32_t *pB = p;						/* decreasing pointer               */
   float32_t xAR, xAI, xBR, xBI;			/* temporary variables              */
   float32_t t1a, t1b, r, s, t, u;			/* temporary variables              */

   k = (S->Sint).fftLen - 1;

   xAR = pA[0];
   xAI = pA[1];

   pCoeff += 2 ;

   *pOut++ = 0.5f * ( xAR + xAI );
   *pOut++ = 0.5f * ( xAR - xAI );

   pB  =  p + 2*k ;
   pA +=  2	   ;

   while (k > 0U)
   {
      /* G is half of the frequency complex spectrum */
      //for k = 2:N
      //    Xk(k) = 1/2 * (G(k) + conj(G(N-k+2)) + Tw(k)*( G(k) - conj(G(N-k+2))));
      xBI =   pB[1]    ;
      xBR =   pB[0]    ;
      xAR =  pA[0];
      xAI =  pA[1];

      twR = *pCoeff++;
      twI = *pCoeff++;

      t1a = xAR - xBR ;
      t1b = xAI + xBI ;

      r = twR * t1a;
      s = twI * t1b;
      t = twI * t1a;
      u = twR * t1b;

      // real(tw * (xA - xB)) = twR * (xAR - xBR) - twI * (xAI - xBI);
      // imag(tw * (xA - xB)) = twI * (xAR - xBR) + twR * (xAI - xBI);
      *pOut++ = 0.5f * (xAR + xBR - r - s ); //xAR
      *pOut++ = 0.5f * (xAI - xBI + t - u ); //xAI

      pA += 2;
      pB -= 2;
      k--;
   }

}

/**
* @ingroup groupTransforms
*/

/**
 * @defgroup RealFFT Real FFT Functions
 *
 * \par
 * The CMSIS DSP library includes specialized algorithms for computing the
 * FFT of real data sequences.  The FFT is defined over complex data but
 * in many applications the input is real.  Real FFT algorithms take advantage
 * of the symmetry properties of the FFT and have a speed advantage over complex
 * algorithms of the same length.
 * \par
 * The Fast RFFT algorith relays on the mixed radix CFFT that save processor usage.
 * \par
 * The real length N forward FFT of a sequence is computed using the steps shown below.
 * \par
 * \image html RFFT.gif "Real Fast Fourier Transform"
 * \par
 * The real sequence is initially treated as if it were complex to perform a CFFT.
 * Later, a processing stage reshapes the data to obtain half of the frequency spectrum
 * in complex format. Except the first complex number that contains the two real numbers
 * X[0] and X[N/2] all the data is complex. In other words, the first complex sample
 * contains two real values packed.
 * \par
 * The input for the inverse RFFT should keep the same format as the output of the
 * forward RFFT. A first processing stage pre-process the data to later perform an
 * inverse CFFT.
 * \par
 * \image html RIFFT.gif "Real Inverse Fast Fourier Transform"
 * \par
 * The algorithms for floating-point, Q15, and Q31 data are slightly different
 * and we describe each algorithm in turn.
 * \par Floating-point
 * The main functions are arm_rfft_fast_f32() and arm_rfft_fast_init_f32().
 * The older functions arm_rfft_f32() and arm_rfft_init_f32() have been
 * deprecated but are still documented.
 * \par
 * The FFT of a real N-point sequence has even symmetry in the frequency
 * domain. The second half of the data equals the conjugate of the first
 * half flipped in frequency. Looking at the data, we see that we can
 * uniquely represent the FFT using only N/2 complex numbers. These are
 * packed into the output array in alternating real and imaginary
 * components:
 * \par
 * X = { real[0], imag[0], real[1], imag[1], real[2], imag[2] ...
 * real[(N/2)-1], imag[(N/2)-1 }
 * \par
 * It happens that the first complex number (real[0], imag[0]) is actually
 * all real. real[0] represents the DC offset, and imag[0] should be 0.
 * (real[1], imag[1]) is the fundamental frequency, (real[2], imag[2]) is
 * the first harmonic and so on.
 * \par
 * The real FFT functions pack the frequency domain data in this fashion.
 * The forward transform outputs the data in this form and the inverse
 * transform expects input data in this form. The function always performs
 * the needed bitreversal so that the input and output data is always in
 * normal order. The functions support lengths of [32, 64, 128, ..., 4096]
 * samples.
 * \par Q15 and Q31
 * The real algorithms are defined in a similar manner and utilize N/2 complex
 * transforms behind the scenes.
 * \par
 * The complex transforms used internally include scaling to prevent fixed-point
 * overflows.  The overall scaling equals 1/(fftLen/2).
 * \par
 * A separate instance structure must be defined for each transform used but
 * twiddle factor and bit reversal tables can be reused.
 * \par
 * There is also an associated initialization function for each data type.
 * The initialization function performs the following operations:
 * - Sets the values of the internal structure fields.
 * - Initializes twiddle factor table and bit reversal table pointers.
 * - Initializes the internal complex FFT data structure.
 * \par
 * Use of the initialization function is optional.
 * However, if the initialization function is used, then the instance structure
 * cannot be placed into a const data section. To place an instance structure
 * into a const data section, the instance structure should be manually
 * initialized as follows:
 * <pre>
 *arm_rfft_instance_q31 S = {fftLenReal, fftLenBy2, ifftFlagR, bitReverseFlagR, twidCoefRModifier, pTwiddleAReal, pTwiddleBReal, pCfft};
 *arm_rfft_instance_q15 S = {fftLenReal, fftLenBy2, ifftFlagR, bitReverseFlagR, twidCoefRModifier, pTwiddleAReal, pTwiddleBReal, pCfft};
 * </pre>
 * where <code>fftLenReal</code> is the length of the real transform;
 * <code>fftLenBy2</code> length of  the internal complex transform.
 * <code>ifftFlagR</code> Selects forward (=0) or inverse (=1) transform.
 * <code>bitReverseFlagR</code> Selects bit reversed output (=0) or normal order
 * output (=1).
 * <code>twidCoefRModifier</code> stride modifier for the twiddle factor table.
 * The value is based on the FFT length;
 * <code>pTwiddleAReal</code>points to the A array of twiddle coefficients;
 * <code>pTwiddleBReal</code>points to the B array of twiddle coefficients;
 * <code>pCfft</code> points to the CFFT Instance structure. The CFFT structure
 * must also be initialized.  Refer to arm_cfft_radix4_f32() for details regarding
 * static initialization of the complex FFT instance structure.
 */

/**
* @addtogroup RealFFT
* @{
*/

/**
* @brief Processing function for the floating-point real FFT.
* @param[in]  *S              points to an arm_rfft_fast_instance_f32 structure.
* @param[in]  *p              points to the input buffer.
* @param[in]  *pOut           points to the output buffer.
* @param[in]  ifftFlag        RFFT if flag is 0, RIFFT if flag is 1
* @return none.
*/

void arm_rfft_fast_f32(
arm_rfft_fast_instance_f32 * S,
float32_t * p, float32_t * pOut,
uint8_t ifftFlag)
{
   arm_cfft_instance_f32 * Sint = &(S->Sint);
   Sint->fftLen = S->fftLenRFFT / 2;

   /* Calculation of Real FFT */
   if (ifftFlag)
   {
      /*  Real FFT compression */
      merge_rfft_f32(S, p, pOut);

      /* Complex radix-4 IFFT process */
      arm_cfft_f32( Sint, pOut, ifftFlag, 1);
   }
   else
   {
      /* Calculation of RFFT of input */
      arm_cfft_f32( Sint, p, ifftFlag, 1);

      /*  Real FFT extraction */
      stage_rfft_f32(S, p, pOut);
   }
}

/**
* @} end of RealFFT group
*/
