/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cfft_radix2_q15.c
 * Description:  Radix-2 Decimation in Frequency CFFT & CIFFT Fixed point processing function
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

void arm_radix2_butterfly_q15(
  q15_t * pSrc,
  uint32_t fftLen,
  q15_t * pCoef,
  uint16_t twidCoefModifier);

void arm_radix2_butterfly_inverse_q15(
  q15_t * pSrc,
  uint32_t fftLen,
  q15_t * pCoef,
  uint16_t twidCoefModifier);

void arm_bitreversal_q15(
  q15_t * pSrc,
  uint32_t fftLen,
  uint16_t bitRevFactor,
  uint16_t * pBitRevTab);

/**
 * @ingroup groupTransforms
 */

/**
 * @addtogroup ComplexFFT
 * @{
 */

/**
 * @details
 * @brief Processing function for the fixed-point CFFT/CIFFT.
 * @deprecated Do not use this function.  It has been superseded by \ref arm_cfft_q15 and will be removed
 * @param[in]      *S    points to an instance of the fixed-point CFFT/CIFFT structure.
 * @param[in, out] *pSrc points to the complex data buffer of size <code>2*fftLen</code>. Processing occurs in-place.
 * @return none.
 */

void arm_cfft_radix2_q15(
  const arm_cfft_radix2_instance_q15 * S,
  q15_t * pSrc)
{

  if (S->ifftFlag == 1U)
  {
    arm_radix2_butterfly_inverse_q15(pSrc, S->fftLen,
                                     S->pTwiddle, S->twidCoefModifier);
  }
  else
  {
    arm_radix2_butterfly_q15(pSrc, S->fftLen,
                             S->pTwiddle, S->twidCoefModifier);
  }

  arm_bitreversal_q15(pSrc, S->fftLen, S->bitRevFactor, S->pBitRevTable);
}

/**
 * @} end of ComplexFFT group
 */

void arm_radix2_butterfly_q15(
  q15_t * pSrc,
  uint32_t fftLen,
  q15_t * pCoef,
  uint16_t twidCoefModifier)
{
#if defined (ARM_MATH_DSP)

  unsigned i, j, k, l;
  unsigned n1, n2, ia;
  q15_t in;
  q31_t T, S, R;
  q31_t coeff, out1, out2;

  //N = fftLen;
  n2 = fftLen;

  n1 = n2;
  n2 = n2 >> 1;
  ia = 0;

  // loop for groups
  for (i = 0; i < n2; i++)
  {
    coeff = _SIMD32_OFFSET(pCoef + (ia * 2U));

    ia = ia + twidCoefModifier;

    l = i + n2;

    T = _SIMD32_OFFSET(pSrc + (2 * i));
    in = ((int16_t) (T & 0xFFFF)) >> 1;
    T = ((T >> 1) & 0xFFFF0000) | (in & 0xFFFF);

    S = _SIMD32_OFFSET(pSrc + (2 * l));
    in = ((int16_t) (S & 0xFFFF)) >> 1;
    S = ((S >> 1) & 0xFFFF0000) | (in & 0xFFFF);

    R = __QSUB16(T, S);

    _SIMD32_OFFSET(pSrc + (2 * i)) = __SHADD16(T, S);

#ifndef ARM_MATH_BIG_ENDIAN

    out1 = __SMUAD(coeff, R) >> 16;
    out2 = __SMUSDX(coeff, R);

#else

    out1 = __SMUSDX(R, coeff) >> 16U;
    out2 = __SMUAD(coeff, R);

#endif //     #ifndef ARM_MATH_BIG_ENDIAN

    _SIMD32_OFFSET(pSrc + (2U * l)) =
      (q31_t) ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);

    coeff = _SIMD32_OFFSET(pCoef + (ia * 2U));

    ia = ia + twidCoefModifier;

    // loop for butterfly
    i++;
    l++;

    T = _SIMD32_OFFSET(pSrc + (2 * i));
    in = ((int16_t) (T & 0xFFFF)) >> 1;
    T = ((T >> 1) & 0xFFFF0000) | (in & 0xFFFF);

    S = _SIMD32_OFFSET(pSrc + (2 * l));
    in = ((int16_t) (S & 0xFFFF)) >> 1;
    S = ((S >> 1) & 0xFFFF0000) | (in & 0xFFFF);

    R = __QSUB16(T, S);

    _SIMD32_OFFSET(pSrc + (2 * i)) = __SHADD16(T, S);

#ifndef ARM_MATH_BIG_ENDIAN

    out1 = __SMUAD(coeff, R) >> 16;
    out2 = __SMUSDX(coeff, R);

#else

    out1 = __SMUSDX(R, coeff) >> 16U;
    out2 = __SMUAD(coeff, R);

#endif //     #ifndef ARM_MATH_BIG_ENDIAN

    _SIMD32_OFFSET(pSrc + (2U * l)) =
      (q31_t) ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);

  }                             // groups loop end

  twidCoefModifier = twidCoefModifier << 1U;

  // loop for stage
  for (k = fftLen / 2; k > 2; k = k >> 1)
  {
    n1 = n2;
    n2 = n2 >> 1;
    ia = 0;

    // loop for groups
    for (j = 0; j < n2; j++)
    {
      coeff = _SIMD32_OFFSET(pCoef + (ia * 2U));

      ia = ia + twidCoefModifier;

      // loop for butterfly
      for (i = j; i < fftLen; i += n1)
      {
        l = i + n2;

        T = _SIMD32_OFFSET(pSrc + (2 * i));

        S = _SIMD32_OFFSET(pSrc + (2 * l));

        R = __QSUB16(T, S);

        _SIMD32_OFFSET(pSrc + (2 * i)) = __SHADD16(T, S);

#ifndef ARM_MATH_BIG_ENDIAN

        out1 = __SMUAD(coeff, R) >> 16;
        out2 = __SMUSDX(coeff, R);

#else

        out1 = __SMUSDX(R, coeff) >> 16U;
        out2 = __SMUAD(coeff, R);

#endif //     #ifndef ARM_MATH_BIG_ENDIAN

        _SIMD32_OFFSET(pSrc + (2U * l)) =
          (q31_t) ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);

        i += n1;

        l = i + n2;

        T = _SIMD32_OFFSET(pSrc + (2 * i));

        S = _SIMD32_OFFSET(pSrc + (2 * l));

        R = __QSUB16(T, S);

        _SIMD32_OFFSET(pSrc + (2 * i)) = __SHADD16(T, S);

#ifndef ARM_MATH_BIG_ENDIAN

        out1 = __SMUAD(coeff, R) >> 16;
        out2 = __SMUSDX(coeff, R);

#else

        out1 = __SMUSDX(R, coeff) >> 16U;
        out2 = __SMUAD(coeff, R);

#endif //     #ifndef ARM_MATH_BIG_ENDIAN

        _SIMD32_OFFSET(pSrc + (2U * l)) =
          (q31_t) ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);

      }                         // butterfly loop end

    }                           // groups loop end

    twidCoefModifier = twidCoefModifier << 1U;
  }                             // stages loop end

  n1 = n2;
  n2 = n2 >> 1;
  ia = 0;

  coeff = _SIMD32_OFFSET(pCoef + (ia * 2U));

  ia = ia + twidCoefModifier;

  // loop for butterfly
  for (i = 0; i < fftLen; i += n1)
  {
    l = i + n2;

    T = _SIMD32_OFFSET(pSrc + (2 * i));

    S = _SIMD32_OFFSET(pSrc + (2 * l));

    R = __QSUB16(T, S);

    _SIMD32_OFFSET(pSrc + (2 * i)) = __QADD16(T, S);

    _SIMD32_OFFSET(pSrc + (2U * l)) = R;

    i += n1;
    l = i + n2;

    T = _SIMD32_OFFSET(pSrc + (2 * i));

    S = _SIMD32_OFFSET(pSrc + (2 * l));

    R = __QSUB16(T, S);

    _SIMD32_OFFSET(pSrc + (2 * i)) = __QADD16(T, S);

    _SIMD32_OFFSET(pSrc + (2U * l)) = R;

  }                             // groups loop end


#else

  unsigned i, j, k, l;
  unsigned n1, n2, ia;
  q15_t xt, yt, cosVal, sinVal;


  //N = fftLen;
  n2 = fftLen;

  n1 = n2;
  n2 = n2 >> 1;
  ia = 0;

  // loop for groups
  for (j = 0; j < n2; j++)
  {
    cosVal = pCoef[ia * 2];
    sinVal = pCoef[(ia * 2) + 1];
    ia = ia + twidCoefModifier;

    // loop for butterfly
    for (i = j; i < fftLen; i += n1)
    {
      l = i + n2;
      xt = (pSrc[2 * i] >> 1U) - (pSrc[2 * l] >> 1U);
      pSrc[2 * i] = ((pSrc[2 * i] >> 1U) + (pSrc[2 * l] >> 1U)) >> 1U;

      yt = (pSrc[2 * i + 1] >> 1U) - (pSrc[2 * l + 1] >> 1U);
      pSrc[2 * i + 1] =
        ((pSrc[2 * l + 1] >> 1U) + (pSrc[2 * i + 1] >> 1U)) >> 1U;

      pSrc[2U * l] = (((int16_t) (((q31_t) xt * cosVal) >> 16)) +
                      ((int16_t) (((q31_t) yt * sinVal) >> 16)));

      pSrc[2U * l + 1U] = (((int16_t) (((q31_t) yt * cosVal) >> 16)) -
                           ((int16_t) (((q31_t) xt * sinVal) >> 16)));

    }                           // butterfly loop end

  }                             // groups loop end

  twidCoefModifier = twidCoefModifier << 1U;

  // loop for stage
  for (k = fftLen / 2; k > 2; k = k >> 1)
  {
    n1 = n2;
    n2 = n2 >> 1;
    ia = 0;

    // loop for groups
    for (j = 0; j < n2; j++)
    {
      cosVal = pCoef[ia * 2];
      sinVal = pCoef[(ia * 2) + 1];
      ia = ia + twidCoefModifier;

      // loop for butterfly
      for (i = j; i < fftLen; i += n1)
      {
        l = i + n2;
        xt = pSrc[2 * i] - pSrc[2 * l];
        pSrc[2 * i] = (pSrc[2 * i] + pSrc[2 * l]) >> 1U;

        yt = pSrc[2 * i + 1] - pSrc[2 * l + 1];
        pSrc[2 * i + 1] = (pSrc[2 * l + 1] + pSrc[2 * i + 1]) >> 1U;

        pSrc[2U * l] = (((int16_t) (((q31_t) xt * cosVal) >> 16)) +
                        ((int16_t) (((q31_t) yt * sinVal) >> 16)));

        pSrc[2U * l + 1U] = (((int16_t) (((q31_t) yt * cosVal) >> 16)) -
                             ((int16_t) (((q31_t) xt * sinVal) >> 16)));

      }                         // butterfly loop end

    }                           // groups loop end

    twidCoefModifier = twidCoefModifier << 1U;
  }                             // stages loop end

  n1 = n2;
  n2 = n2 >> 1;
  ia = 0;

  // loop for groups
  for (j = 0; j < n2; j++)
  {
    cosVal = pCoef[ia * 2];
    sinVal = pCoef[(ia * 2) + 1];

    ia = ia + twidCoefModifier;

    // loop for butterfly
    for (i = j; i < fftLen; i += n1)
    {
      l = i + n2;
      xt = pSrc[2 * i] - pSrc[2 * l];
      pSrc[2 * i] = (pSrc[2 * i] + pSrc[2 * l]);

      yt = pSrc[2 * i + 1] - pSrc[2 * l + 1];
      pSrc[2 * i + 1] = (pSrc[2 * l + 1] + pSrc[2 * i + 1]);

      pSrc[2U * l] = xt;

      pSrc[2U * l + 1U] = yt;

    }                           // butterfly loop end

  }                             // groups loop end

  twidCoefModifier = twidCoefModifier << 1U;

#endif //             #if defined (ARM_MATH_DSP)

}


void arm_radix2_butterfly_inverse_q15(
  q15_t * pSrc,
  uint32_t fftLen,
  q15_t * pCoef,
  uint16_t twidCoefModifier)
{
#if defined (ARM_MATH_DSP)

  unsigned i, j, k, l;
  unsigned n1, n2, ia;
  q15_t in;
  q31_t T, S, R;
  q31_t coeff, out1, out2;

  //N = fftLen;
  n2 = fftLen;

  n1 = n2;
  n2 = n2 >> 1;
  ia = 0;

  // loop for groups
  for (i = 0; i < n2; i++)
  {
    coeff = _SIMD32_OFFSET(pCoef + (ia * 2U));

    ia = ia + twidCoefModifier;

    l = i + n2;

    T = _SIMD32_OFFSET(pSrc + (2 * i));
    in = ((int16_t) (T & 0xFFFF)) >> 1;
    T = ((T >> 1) & 0xFFFF0000) | (in & 0xFFFF);

    S = _SIMD32_OFFSET(pSrc + (2 * l));
    in = ((int16_t) (S & 0xFFFF)) >> 1;
    S = ((S >> 1) & 0xFFFF0000) | (in & 0xFFFF);

    R = __QSUB16(T, S);

    _SIMD32_OFFSET(pSrc + (2 * i)) = __SHADD16(T, S);

#ifndef ARM_MATH_BIG_ENDIAN

    out1 = __SMUSD(coeff, R) >> 16;
    out2 = __SMUADX(coeff, R);
#else

    out1 = __SMUADX(R, coeff) >> 16U;
    out2 = __SMUSD(__QSUB(0, coeff), R);

#endif //     #ifndef ARM_MATH_BIG_ENDIAN

    _SIMD32_OFFSET(pSrc + (2U * l)) =
      (q31_t) ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);

    coeff = _SIMD32_OFFSET(pCoef + (ia * 2U));

    ia = ia + twidCoefModifier;

    // loop for butterfly
    i++;
    l++;

    T = _SIMD32_OFFSET(pSrc + (2 * i));
    in = ((int16_t) (T & 0xFFFF)) >> 1;
    T = ((T >> 1) & 0xFFFF0000) | (in & 0xFFFF);

    S = _SIMD32_OFFSET(pSrc + (2 * l));
    in = ((int16_t) (S & 0xFFFF)) >> 1;
    S = ((S >> 1) & 0xFFFF0000) | (in & 0xFFFF);

    R = __QSUB16(T, S);

    _SIMD32_OFFSET(pSrc + (2 * i)) = __SHADD16(T, S);

#ifndef ARM_MATH_BIG_ENDIAN

    out1 = __SMUSD(coeff, R) >> 16;
    out2 = __SMUADX(coeff, R);
#else

    out1 = __SMUADX(R, coeff) >> 16U;
    out2 = __SMUSD(__QSUB(0, coeff), R);

#endif //     #ifndef ARM_MATH_BIG_ENDIAN

    _SIMD32_OFFSET(pSrc + (2U * l)) =
      (q31_t) ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);

  }                             // groups loop end

  twidCoefModifier = twidCoefModifier << 1U;

  // loop for stage
  for (k = fftLen / 2; k > 2; k = k >> 1)
  {
    n1 = n2;
    n2 = n2 >> 1;
    ia = 0;

    // loop for groups
    for (j = 0; j < n2; j++)
    {
      coeff = _SIMD32_OFFSET(pCoef + (ia * 2U));

      ia = ia + twidCoefModifier;

      // loop for butterfly
      for (i = j; i < fftLen; i += n1)
      {
        l = i + n2;

        T = _SIMD32_OFFSET(pSrc + (2 * i));

        S = _SIMD32_OFFSET(pSrc + (2 * l));

        R = __QSUB16(T, S);

        _SIMD32_OFFSET(pSrc + (2 * i)) = __SHADD16(T, S);

#ifndef ARM_MATH_BIG_ENDIAN

        out1 = __SMUSD(coeff, R) >> 16;
        out2 = __SMUADX(coeff, R);

#else

        out1 = __SMUADX(R, coeff) >> 16U;
        out2 = __SMUSD(__QSUB(0, coeff), R);

#endif //     #ifndef ARM_MATH_BIG_ENDIAN

        _SIMD32_OFFSET(pSrc + (2U * l)) =
          (q31_t) ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);

        i += n1;

        l = i + n2;

        T = _SIMD32_OFFSET(pSrc + (2 * i));

        S = _SIMD32_OFFSET(pSrc + (2 * l));

        R = __QSUB16(T, S);

        _SIMD32_OFFSET(pSrc + (2 * i)) = __SHADD16(T, S);

#ifndef ARM_MATH_BIG_ENDIAN

        out1 = __SMUSD(coeff, R) >> 16;
        out2 = __SMUADX(coeff, R);
#else

        out1 = __SMUADX(R, coeff) >> 16U;
        out2 = __SMUSD(__QSUB(0, coeff), R);

#endif //     #ifndef ARM_MATH_BIG_ENDIAN

        _SIMD32_OFFSET(pSrc + (2U * l)) =
          (q31_t) ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);

      }                         // butterfly loop end

    }                           // groups loop end

    twidCoefModifier = twidCoefModifier << 1U;
  }                             // stages loop end

  n1 = n2;
  n2 = n2 >> 1;
  ia = 0;

  // loop for groups
  for (j = 0; j < n2; j++)
  {
    coeff = _SIMD32_OFFSET(pCoef + (ia * 2U));

    ia = ia + twidCoefModifier;

    // loop for butterfly
    for (i = j; i < fftLen; i += n1)
    {
      l = i + n2;

      T = _SIMD32_OFFSET(pSrc + (2 * i));

      S = _SIMD32_OFFSET(pSrc + (2 * l));

      R = __QSUB16(T, S);

      _SIMD32_OFFSET(pSrc + (2 * i)) = __QADD16(T, S);

      _SIMD32_OFFSET(pSrc + (2U * l)) = R;

    }                           // butterfly loop end

  }                             // groups loop end

  twidCoefModifier = twidCoefModifier << 1U;

#else


  unsigned i, j, k, l;
  unsigned n1, n2, ia;
  q15_t xt, yt, cosVal, sinVal;

  //N = fftLen;
  n2 = fftLen;

  n1 = n2;
  n2 = n2 >> 1;
  ia = 0;

  // loop for groups
  for (j = 0; j < n2; j++)
  {
    cosVal = pCoef[ia * 2];
    sinVal = pCoef[(ia * 2) + 1];
    ia = ia + twidCoefModifier;

    // loop for butterfly
    for (i = j; i < fftLen; i += n1)
    {
      l = i + n2;
      xt = (pSrc[2 * i] >> 1U) - (pSrc[2 * l] >> 1U);
      pSrc[2 * i] = ((pSrc[2 * i] >> 1U) + (pSrc[2 * l] >> 1U)) >> 1U;

      yt = (pSrc[2 * i + 1] >> 1U) - (pSrc[2 * l + 1] >> 1U);
      pSrc[2 * i + 1] =
        ((pSrc[2 * l + 1] >> 1U) + (pSrc[2 * i + 1] >> 1U)) >> 1U;

      pSrc[2U * l] = (((int16_t) (((q31_t) xt * cosVal) >> 16)) -
                      ((int16_t) (((q31_t) yt * sinVal) >> 16)));

      pSrc[2U * l + 1U] = (((int16_t) (((q31_t) yt * cosVal) >> 16)) +
                           ((int16_t) (((q31_t) xt * sinVal) >> 16)));

    }                           // butterfly loop end

  }                             // groups loop end

  twidCoefModifier = twidCoefModifier << 1U;

  // loop for stage
  for (k = fftLen / 2; k > 2; k = k >> 1)
  {
    n1 = n2;
    n2 = n2 >> 1;
    ia = 0;

    // loop for groups
    for (j = 0; j < n2; j++)
    {
      cosVal = pCoef[ia * 2];
      sinVal = pCoef[(ia * 2) + 1];
      ia = ia + twidCoefModifier;

      // loop for butterfly
      for (i = j; i < fftLen; i += n1)
      {
        l = i + n2;
        xt = pSrc[2 * i] - pSrc[2 * l];
        pSrc[2 * i] = (pSrc[2 * i] + pSrc[2 * l]) >> 1U;

        yt = pSrc[2 * i + 1] - pSrc[2 * l + 1];
        pSrc[2 * i + 1] = (pSrc[2 * l + 1] + pSrc[2 * i + 1]) >> 1U;

        pSrc[2U * l] = (((int16_t) (((q31_t) xt * cosVal) >> 16)) -
                        ((int16_t) (((q31_t) yt * sinVal) >> 16)));

        pSrc[2U * l + 1U] = (((int16_t) (((q31_t) yt * cosVal) >> 16)) +
                             ((int16_t) (((q31_t) xt * sinVal) >> 16)));

      }                         // butterfly loop end

    }                           // groups loop end

    twidCoefModifier = twidCoefModifier << 1U;
  }                             // stages loop end

  n1 = n2;
  n2 = n2 >> 1;
  ia = 0;

  cosVal = pCoef[ia * 2];
  sinVal = pCoef[(ia * 2) + 1];

  ia = ia + twidCoefModifier;

  // loop for butterfly
  for (i = 0; i < fftLen; i += n1)
  {
    l = i + n2;
    xt = pSrc[2 * i] - pSrc[2 * l];
    pSrc[2 * i] = (pSrc[2 * i] + pSrc[2 * l]);

    yt = pSrc[2 * i + 1] - pSrc[2 * l + 1];
    pSrc[2 * i + 1] = (pSrc[2 * l + 1] + pSrc[2 * i + 1]);

    pSrc[2U * l] = xt;

    pSrc[2U * l + 1U] = yt;

  }                             // groups loop end


#endif //             #if defined (ARM_MATH_DSP)

}
