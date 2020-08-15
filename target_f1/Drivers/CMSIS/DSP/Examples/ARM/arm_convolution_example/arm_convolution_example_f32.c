/* ----------------------------------------------------------------------
* Copyright (C) 2010-2012 ARM Limited. All rights reserved.
*
* $Date:         17. January 2013
* $Revision:     V1.4.0
*
* Project:       CMSIS DSP Library
* Title:         arm_convolution_example_f32.c
*
* Description:   Example code demonstrating Convolution of two input signals using fft.
*
* Target Processor: Cortex-M4/Cortex-M3
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*   - Neither the name of ARM LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------- */

/**
 * @ingroup groupExamples
 */

/**
 * @defgroup ConvolutionExample Convolution Example
 *
 * \par Description:
 * \par
 * Demonstrates the convolution theorem with the use of the Complex FFT, Complex-by-Complex
 * Multiplication, and Support Functions.
 *
 * \par Algorithm:
 * \par
 * The convolution theorem states that convolution in the time domain corresponds to
 * multiplication in the frequency domain. Therefore, the Fourier transform of the convoution of
 * two signals is equal to the product of their individual Fourier transforms.
 * The Fourier transform of a signal can be evaluated efficiently using the Fast Fourier Transform (FFT).
 * \par
 * Two input signals, <code>a[n]</code> and <code>b[n]</code>, with lengths \c n1 and \c n2 respectively,
 * are zero padded so that their lengths become \c N, which is greater than or equal to <code>(n1+n2-1)</code>
 * and is a power of 4 as FFT implementation is radix-4.
 * The convolution of <code>a[n]</code> and <code>b[n]</code> is obtained by taking the FFT of the input
 * signals, multiplying the Fourier transforms of the two signals, and taking the inverse FFT of
 * the multiplied result.
 * \par
 * This is denoted by the following equations:
 * <pre> A[k] = FFT(a[n],N)
 * B[k] = FFT(b[n],N)
 * conv(a[n], b[n]) = IFFT(A[k] * B[k], N)</pre>
 * where <code>A[k]</code> and <code>B[k]</code> are the N-point FFTs of the signals <code>a[n]</code>
 * and <code>b[n]</code> respectively.
 * The length of the convolved signal is <code>(n1+n2-1)</code>.
 *
 * \par Block Diagram:
 * \par
 * \image html Convolution.gif
 *
 * \par Variables Description:
 * \par
 * \li \c testInputA_f32 points to the first input sequence
 * \li \c srcALen length of the first input sequence
 * \li \c testInputB_f32 points to the second input sequence
 * \li \c srcBLen length of the second input sequence
 * \li \c outLen length of convolution output sequence, <code>(srcALen + srcBLen - 1)</code>
 * \li \c AxB points to the output array where the product of individual FFTs of inputs is stored.
 *
 * \par CMSIS DSP Software Library Functions Used:
 * \par
 * - arm_fill_f32()
 * - arm_copy_f32()
 * - arm_cfft_radix4_init_f32()
 * - arm_cfft_radix4_f32()
 * - arm_cmplx_mult_cmplx_f32()
 *
 * <b> Refer  </b>
 * \link arm_convolution_example_f32.c \endlink
 *
 */


/** \example arm_convolution_example_f32.c
  */

#include "arm_math.h"
#include "math_helper.h"

/* ----------------------------------------------------------------------
* Defines each of the tests performed
* ------------------------------------------------------------------- */
#define MAX_BLOCKSIZE   128
#define DELTA           (0.000001f)
#define SNR_THRESHOLD   90

/* ----------------------------------------------------------------------
* Declare I/O buffers
* ------------------------------------------------------------------- */
float32_t Ak[MAX_BLOCKSIZE];        /* Input A */
float32_t Bk[MAX_BLOCKSIZE];        /* Input B */
float32_t AxB[MAX_BLOCKSIZE * 2];   /* Output */

/* ----------------------------------------------------------------------
* Test input data for Floating point Convolution example for 32-blockSize
* Generated by the MATLAB randn() function
* ------------------------------------------------------------------- */
float32_t testInputA_f32[64] =
{
  -0.808920,   1.357369,   1.180861,  -0.504544,   1.762637,  -0.703285,
   1.696966,   0.620571,  -0.151093,  -0.100235,  -0.872382,  -0.403579,
  -0.860749,  -0.382648,  -1.052338,   0.128113,  -0.646269,   1.093377,
  -2.209198,   0.471706,   0.408901,   1.266242,   0.598252,   1.176827,
  -0.203421,   0.213596,  -0.851964,  -0.466958,   0.021841,  -0.698938,
  -0.604107,   0.461778,  -0.318219,   0.942520,   0.577585,   0.417619,
   0.614665,   0.563679,  -1.295073,  -0.764437,   0.952194,  -0.859222,
  -0.618554,  -2.268542,  -1.210592,   1.655853,  -2.627219,  -0.994249,
  -1.374704,   0.343799,   0.025619,   1.227481,  -0.708031,   0.069355,
  -1.845228,  -1.570886,   1.010668,  -1.802084,   1.630088,   1.286090,
  -0.161050,  -0.940794,   0.367961,   0.291907

};

float32_t testInputB_f32[64] =
{
   0.933724,   0.046881,   1.316470,   0.438345,   0.332682,   2.094885,
   0.512081,   0.035546,   0.050894,  -2.320371,   0.168711,  -1.830493,
  -0.444834,  -1.003242,  -0.531494,  -1.365600,  -0.155420,  -0.757692,
  -0.431880,  -0.380021,   0.096243,  -0.695835,   0.558850,  -1.648962,
   0.020369,  -0.363630,   0.887146,   0.845503,  -0.252864,  -0.330397,
   1.269131,  -1.109295,  -1.027876,   0.135940,   0.116721,  -0.293399,
  -1.349799,   0.166078,  -0.802201,   0.369367,  -0.964568,  -2.266011,
   0.465178,   0.651222,  -0.325426,   0.320245,  -0.784178,  -0.579456,
   0.093374,   0.604778,  -0.048225,   0.376297,  -0.394412,   0.578182,
  -1.218141,  -1.387326,   0.692462,  -0.631297,   0.153137,  -0.638952,
  0.635474,   -0.970468,   1.334057,  -0.111370
};

const float testRefOutput_f32[127] =
{
   -0.818943,    1.229484,  -0.533664,    1.016604,   0.341875,  -1.963656,
    5.171476,    3.478033,   7.616361,    6.648384,   0.479069,   1.792012,
   -1.295591,   -7.447818,   0.315830,  -10.657445,  -2.483469,  -6.524236,
   -7.380591,   -3.739005,  -8.388957,    0.184147,  -1.554888,   3.786508,
   -1.684421,    5.400610,  -1.578126,    7.403361,   8.315999,   2.080267,
   11.077776,    2.749673,   7.138962,    2.748762,   0.660363,   0.981552,
    1.442275,    0.552721,  -2.576892,    4.703989,   0.989156,   8.759344,
   -0.564825,   -3.994680,   0.954710,   -5.014144,   6.592329,   1.599488,
  -13.979146,   -0.391891,  -4.453369,   -2.311242,  -2.948764,   1.761415,
   -0.138322,   10.433007,  -2.309103,    4.297153,   8.535523,   3.209462,
    8.695819,    5.569919,   2.514304,    5.582029,   2.060199,   0.642280,
    7.024616,    1.686615,  -6.481756,    1.343084,  -3.526451,   1.099073,
   -2.965764,   -0.173723,  -4.111484,    6.528384,  -6.965658,   1.726291,
    1.535172,   11.023435,   2.338401,   -4.690188,   1.298210,   3.943885,
    8.407885,    5.168365,   0.684131,    1.559181,   1.859998,   2.852417,
    8.574070,   -6.369078,   6.023458,   11.837963,  -6.027632,   4.469678,
   -6.799093,   -2.674048,   6.250367,   -6.809971,  -3.459360,   9.112410,
   -2.711621,   -1.336678,   1.564249,   -1.564297,  -1.296760,   8.904013,
   -3.230109,    6.878013,  -7.819823,    3.369909,  -1.657410,  -2.007358,
   -4.112825,    1.370685,  -3.420525,   -6.276605,   3.244873,  -3.352638,
    1.545372,    0.902211,   0.197489,   -1.408732,   0.523390,   0.348440, 0
};


/* ----------------------------------------------------------------------
* Declare Global variables
* ------------------------------------------------------------------- */
uint32_t srcALen = 64;   /* Length of Input A */
uint32_t srcBLen = 64;   /* Length of Input B */
uint32_t outLen;         /* Length of convolution output */
float32_t snr;           /* output SNR */

int32_t main(void)
{
  arm_status status;                           /* Status of the example */
  arm_cfft_radix4_instance_f32 cfft_instance;  /* CFFT Structure instance */

  /* CFFT Structure instance pointer */
  arm_cfft_radix4_instance_f32 *cfft_instance_ptr =
      (arm_cfft_radix4_instance_f32*) &cfft_instance;

  /* output length of convolution */
  outLen = srcALen + srcBLen - 1;

  /* Initialise the fft input buffers with all zeros */
  arm_fill_f32(0.0,  Ak, MAX_BLOCKSIZE);
  arm_fill_f32(0.0,  Bk, MAX_BLOCKSIZE);

  /* Copy the input values to the fft input buffers */
  arm_copy_f32(testInputA_f32,  Ak, MAX_BLOCKSIZE/2);
  arm_copy_f32(testInputB_f32,  Bk, MAX_BLOCKSIZE/2);

  /* Initialize the CFFT function to compute 64 point fft */
  status = arm_cfft_radix4_init_f32(cfft_instance_ptr, 64, 0, 1);

  /* Transform input a[n] from time domain to frequency domain A[k] */
  arm_cfft_radix4_f32(cfft_instance_ptr, Ak);
  /* Transform input b[n] from time domain to frequency domain B[k] */
  arm_cfft_radix4_f32(cfft_instance_ptr, Bk);

  /* Complex Multiplication of the two input buffers in frequency domain */
  arm_cmplx_mult_cmplx_f32(Ak, Bk, AxB, MAX_BLOCKSIZE/2);

  /* Initialize the CIFFT function to compute 64 point ifft */
  status = arm_cfft_radix4_init_f32(cfft_instance_ptr, 64, 1, 1);

  /* Transform the multiplication output from frequency domain to time domain,
     that gives the convolved output  */
  arm_cfft_radix4_f32(cfft_instance_ptr, AxB);

  /* SNR Calculation */
  snr = arm_snr_f32((float32_t *)testRefOutput_f32, AxB, srcALen + srcBLen - 1);

  /* Compare the SNR with threshold to test whether the
     computed output is matched with the reference output values. */
  if ( snr > SNR_THRESHOLD)
  {
    status = ARM_MATH_SUCCESS;
  }

  if ( status != ARM_MATH_SUCCESS)
  {
    while (1);
  }

  while (1);                             /* main function does not return */
}

 /** \endlink */
