/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_bitreversal.c
 * Description:  Bitreversal functions
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
#include "arm_common_tables.h"

/*
* @brief  In-place bit reversal function.
* @param[in, out] *pSrc        points to the in-place buffer of floating-point data type.
* @param[in]      fftSize      length of the FFT.
* @param[in]      bitRevFactor bit reversal modifier that supports different size FFTs with the same bit reversal table.
* @param[in]      *pBitRevTab  points to the bit reversal table.
* @return none.
*/

void arm_bitreversal_f32(
float32_t * pSrc,
uint16_t fftSize,
uint16_t bitRevFactor,
uint16_t * pBitRevTab)
{
   uint16_t fftLenBy2, fftLenBy2p1;
   uint16_t i, j;
   float32_t in;

   /*  Initializations */
   j = 0U;
   fftLenBy2 = fftSize >> 1U;
   fftLenBy2p1 = (fftSize >> 1U) + 1U;

   /* Bit Reversal Implementation */
   for (i = 0U; i <= (fftLenBy2 - 2U); i += 2U)
   {
      if (i < j)
      {
         /*  pSrc[i] <-> pSrc[j]; */
         in = pSrc[2U * i];
         pSrc[2U * i] = pSrc[2U * j];
         pSrc[2U * j] = in;

         /*  pSrc[i+1U] <-> pSrc[j+1U] */
         in = pSrc[(2U * i) + 1U];
         pSrc[(2U * i) + 1U] = pSrc[(2U * j) + 1U];
         pSrc[(2U * j) + 1U] = in;

         /*  pSrc[i+fftLenBy2p1] <-> pSrc[j+fftLenBy2p1] */
         in = pSrc[2U * (i + fftLenBy2p1)];
         pSrc[2U * (i + fftLenBy2p1)] = pSrc[2U * (j + fftLenBy2p1)];
         pSrc[2U * (j + fftLenBy2p1)] = in;

         /*  pSrc[i+fftLenBy2p1+1U] <-> pSrc[j+fftLenBy2p1+1U] */
         in = pSrc[(2U * (i + fftLenBy2p1)) + 1U];
         pSrc[(2U * (i + fftLenBy2p1)) + 1U] =
         pSrc[(2U * (j + fftLenBy2p1)) + 1U];
         pSrc[(2U * (j + fftLenBy2p1)) + 1U] = in;

      }

      /*  pSrc[i+1U] <-> pSrc[j+1U] */
      in = pSrc[2U * (i + 1U)];
      pSrc[2U * (i + 1U)] = pSrc[2U * (j + fftLenBy2)];
      pSrc[2U * (j + fftLenBy2)] = in;

      /*  pSrc[i+2U] <-> pSrc[j+2U] */
      in = pSrc[(2U * (i + 1U)) + 1U];
      pSrc[(2U * (i + 1U)) + 1U] = pSrc[(2U * (j + fftLenBy2)) + 1U];
      pSrc[(2U * (j + fftLenBy2)) + 1U] = in;

      /*  Reading the index for the bit reversal */
      j = *pBitRevTab;

      /*  Updating the bit reversal index depending on the fft length  */
      pBitRevTab += bitRevFactor;
   }
}



/*
* @brief  In-place bit reversal function.
* @param[in, out] *pSrc        points to the in-place buffer of Q31 data type.
* @param[in]      fftLen       length of the FFT.
* @param[in]      bitRevFactor bit reversal modifier that supports different size FFTs with the same bit reversal table
* @param[in]      *pBitRevTab  points to bit reversal table.
* @return none.
*/

void arm_bitreversal_q31(
q31_t * pSrc,
uint32_t fftLen,
uint16_t bitRevFactor,
uint16_t * pBitRevTable)
{
   uint32_t fftLenBy2, fftLenBy2p1, i, j;
   q31_t in;

   /*  Initializations      */
   j = 0U;
   fftLenBy2 = fftLen / 2U;
   fftLenBy2p1 = (fftLen / 2U) + 1U;

   /* Bit Reversal Implementation */
   for (i = 0U; i <= (fftLenBy2 - 2U); i += 2U)
   {
      if (i < j)
      {
         /*  pSrc[i] <-> pSrc[j]; */
         in = pSrc[2U * i];
         pSrc[2U * i] = pSrc[2U * j];
         pSrc[2U * j] = in;

         /*  pSrc[i+1U] <-> pSrc[j+1U] */
         in = pSrc[(2U * i) + 1U];
         pSrc[(2U * i) + 1U] = pSrc[(2U * j) + 1U];
         pSrc[(2U * j) + 1U] = in;

         /*  pSrc[i+fftLenBy2p1] <-> pSrc[j+fftLenBy2p1] */
         in = pSrc[2U * (i + fftLenBy2p1)];
         pSrc[2U * (i + fftLenBy2p1)] = pSrc[2U * (j + fftLenBy2p1)];
         pSrc[2U * (j + fftLenBy2p1)] = in;

         /*  pSrc[i+fftLenBy2p1+1U] <-> pSrc[j+fftLenBy2p1+1U] */
         in = pSrc[(2U * (i + fftLenBy2p1)) + 1U];
         pSrc[(2U * (i + fftLenBy2p1)) + 1U] =
         pSrc[(2U * (j + fftLenBy2p1)) + 1U];
         pSrc[(2U * (j + fftLenBy2p1)) + 1U] = in;

      }

      /*  pSrc[i+1U] <-> pSrc[j+1U] */
      in = pSrc[2U * (i + 1U)];
      pSrc[2U * (i + 1U)] = pSrc[2U * (j + fftLenBy2)];
      pSrc[2U * (j + fftLenBy2)] = in;

      /*  pSrc[i+2U] <-> pSrc[j+2U] */
      in = pSrc[(2U * (i + 1U)) + 1U];
      pSrc[(2U * (i + 1U)) + 1U] = pSrc[(2U * (j + fftLenBy2)) + 1U];
      pSrc[(2U * (j + fftLenBy2)) + 1U] = in;

      /*  Reading the index for the bit reversal */
      j = *pBitRevTable;

      /*  Updating the bit reversal index depending on the fft length */
      pBitRevTable += bitRevFactor;
   }
}



/*
   * @brief  In-place bit reversal function.
   * @param[in, out] *pSrc        points to the in-place buffer of Q15 data type.
   * @param[in]      fftLen       length of the FFT.
   * @param[in]      bitRevFactor bit reversal modifier that supports different size FFTs with the same bit reversal table
   * @param[in]      *pBitRevTab  points to bit reversal table.
   * @return none.
*/

void arm_bitreversal_q15(
q15_t * pSrc16,
uint32_t fftLen,
uint16_t bitRevFactor,
uint16_t * pBitRevTab)
{
   q31_t *pSrc = (q31_t *) pSrc16;
   q31_t in;
   uint32_t fftLenBy2, fftLenBy2p1;
   uint32_t i, j;

   /*  Initializations */
   j = 0U;
   fftLenBy2 = fftLen / 2U;
   fftLenBy2p1 = (fftLen / 2U) + 1U;

   /* Bit Reversal Implementation */
   for (i = 0U; i <= (fftLenBy2 - 2U); i += 2U)
   {
      if (i < j)
      {
         /*  pSrc[i] <-> pSrc[j]; */
         /*  pSrc[i+1U] <-> pSrc[j+1U] */
         in = pSrc[i];
         pSrc[i] = pSrc[j];
         pSrc[j] = in;

         /*  pSrc[i + fftLenBy2p1] <-> pSrc[j + fftLenBy2p1];  */
         /*  pSrc[i + fftLenBy2p1+1U] <-> pSrc[j + fftLenBy2p1+1U] */
         in = pSrc[i + fftLenBy2p1];
         pSrc[i + fftLenBy2p1] = pSrc[j + fftLenBy2p1];
         pSrc[j + fftLenBy2p1] = in;
      }

      /*  pSrc[i+1U] <-> pSrc[j+fftLenBy2];         */
      /*  pSrc[i+2] <-> pSrc[j+fftLenBy2+1U]  */
      in = pSrc[i + 1U];
      pSrc[i + 1U] = pSrc[j + fftLenBy2];
      pSrc[j + fftLenBy2] = in;

      /*  Reading the index for the bit reversal */
      j = *pBitRevTab;

      /*  Updating the bit reversal index depending on the fft length  */
      pBitRevTab += bitRevFactor;
   }
}
