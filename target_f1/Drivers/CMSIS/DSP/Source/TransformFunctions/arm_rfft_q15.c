/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_rfft_q15.c
 * Description:  RFFT & RIFFT Q15 process function
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

/* ----------------------------------------------------------------------
 * Internal functions prototypes
 * -------------------------------------------------------------------- */

void arm_split_rfft_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    q15_t * pATable,
    q15_t * pBTable,
    q15_t * pDst,
    uint32_t modifier);

void arm_split_rifft_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    q15_t * pATable,
    q15_t * pBTable,
    q15_t * pDst,
    uint32_t modifier);

/**
* @addtogroup RealFFT
* @{
*/

/**
* @brief Processing function for the Q15 RFFT/RIFFT.
* @param[in]  *S    points to an instance of the Q15 RFFT/RIFFT structure.
* @param[in]  *pSrc points to the input buffer.
* @param[out] *pDst points to the output buffer.
* @return none.
*
* \par Input an output formats:
* \par
* Internally input is downscaled by 2 for every stage to avoid saturations inside CFFT/CIFFT process.
* Hence the output format is different for different RFFT sizes.
* The input and output formats for different RFFT sizes and number of bits to upscale are mentioned in the tables below for RFFT and RIFFT:
* \par
* \image html RFFTQ15.gif "Input and Output Formats for Q15 RFFT"
* \par
* \image html RIFFTQ15.gif "Input and Output Formats for Q15 RIFFT"
*/

void arm_rfft_q15(
    const arm_rfft_instance_q15 * S,
    q15_t * pSrc,
    q15_t * pDst)
{
    const arm_cfft_instance_q15 *S_CFFT = S->pCfft;
    uint32_t i;
    uint32_t L2 = S->fftLenReal >> 1;

    /* Calculation of RIFFT of input */
    if (S->ifftFlagR == 1U)
    {
        /*  Real IFFT core process */
        arm_split_rifft_q15(pSrc, L2, S->pTwiddleAReal,
                            S->pTwiddleBReal, pDst, S->twidCoefRModifier);

        /* Complex IFFT process */
        arm_cfft_q15(S_CFFT, pDst, S->ifftFlagR, S->bitReverseFlagR);

        for(i=0;i<S->fftLenReal;i++)
        {
            pDst[i] = pDst[i] << 1;
        }
    }
    else
    {
        /* Calculation of RFFT of input */

        /* Complex FFT process */
        arm_cfft_q15(S_CFFT, pSrc, S->ifftFlagR, S->bitReverseFlagR);

        /*  Real FFT core process */
        arm_split_rfft_q15(pSrc, L2, S->pTwiddleAReal,
                            S->pTwiddleBReal, pDst, S->twidCoefRModifier);
    }
}

/**
* @} end of RealFFT group
*/

/**
* @brief  Core Real FFT process
* @param  *pSrc 				points to the input buffer.
* @param  fftLen  				length of FFT.
* @param  *pATable 			points to the A twiddle Coef buffer.
* @param  *pBTable 			points to the B twiddle Coef buffer.
* @param  *pDst 				points to the output buffer.
* @param  modifier 	        twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
* @return none.
* The function implements a Real FFT
*/

void arm_split_rfft_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    q15_t * pATable,
    q15_t * pBTable,
    q15_t * pDst,
    uint32_t modifier)
{
    uint32_t i;                                    /* Loop Counter */
    q31_t outR, outI;                              /* Temporary variables for output */
    q15_t *pCoefA, *pCoefB;                        /* Temporary pointers for twiddle factors */
    q15_t *pSrc1, *pSrc2;
#if defined (ARM_MATH_DSP)
    q15_t *pD1, *pD2;
#endif

    //  pSrc[2U * fftLen] = pSrc[0];
    //  pSrc[(2U * fftLen) + 1U] = pSrc[1];

    pCoefA = &pATable[modifier * 2U];
    pCoefB = &pBTable[modifier * 2U];

    pSrc1 = &pSrc[2];
    pSrc2 = &pSrc[(2U * fftLen) - 2U];

#if defined (ARM_MATH_DSP)

    /* Run the below code for Cortex-M4 and Cortex-M3 */
    i = 1U;
    pD1 = pDst + 2;
    pD2 = pDst + (4U * fftLen) - 2;

    for(i = fftLen - 1; i > 0; i--)
    {
        /*
        outR = (pSrc[2 * i] * pATable[2 * i] - pSrc[2 * i + 1] * pATable[2 * i + 1]
        + pSrc[2 * n - 2 * i] * pBTable[2 * i] +
        pSrc[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);
        */

        /* outI = (pIn[2 * i + 1] * pATable[2 * i] + pIn[2 * i] * pATable[2 * i + 1] +
        pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i]); */


#ifndef ARM_MATH_BIG_ENDIAN

        /* pSrc[2 * i] * pATable[2 * i] - pSrc[2 * i + 1] * pATable[2 * i + 1] */
        outR = __SMUSD(*__SIMD32(pSrc1), *__SIMD32(pCoefA));

#else

        /* -(pSrc[2 * i + 1] * pATable[2 * i + 1] - pSrc[2 * i] * pATable[2 * i]) */
        outR = -(__SMUSD(*__SIMD32(pSrc1), *__SIMD32(pCoefA)));

#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */

        /* pSrc[2 * n - 2 * i] * pBTable[2 * i] +
        pSrc[2 * n - 2 * i + 1] * pBTable[2 * i + 1]) */
        outR = __SMLAD(*__SIMD32(pSrc2), *__SIMD32(pCoefB), outR) >> 16U;

        /* pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i] */

#ifndef ARM_MATH_BIG_ENDIAN

        outI = __SMUSDX(*__SIMD32(pSrc2)--, *__SIMD32(pCoefB));

#else

        outI = __SMUSDX(*__SIMD32(pCoefB), *__SIMD32(pSrc2)--);

#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */

        /* (pIn[2 * i + 1] * pATable[2 * i] + pIn[2 * i] * pATable[2 * i + 1] */
        outI = __SMLADX(*__SIMD32(pSrc1)++, *__SIMD32(pCoefA), outI);

        /* write output */
        *pD1++ = (q15_t) outR;
        *pD1++ = outI >> 16U;

        /* write complex conjugate output */
        pD2[0] = (q15_t) outR;
        pD2[1] = -(outI >> 16U);
        pD2 -= 2;

        /* update coefficient pointer */
        pCoefB = pCoefB + (2U * modifier);
        pCoefA = pCoefA + (2U * modifier);
    }

    pDst[2U * fftLen] = (pSrc[0] - pSrc[1]) >> 1;
    pDst[(2U * fftLen) + 1U] = 0;

    pDst[0] = (pSrc[0] + pSrc[1]) >> 1;
    pDst[1] = 0;

#else

    /* Run the below code for Cortex-M0 */
    i = 1U;

    while (i < fftLen)
    {
        /*
        outR = (pSrc[2 * i] * pATable[2 * i] - pSrc[2 * i + 1] * pATable[2 * i + 1]
        + pSrc[2 * n - 2 * i] * pBTable[2 * i] +
        pSrc[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);
        */

        outR = *pSrc1 * *pCoefA;
        outR = outR - (*(pSrc1 + 1) * *(pCoefA + 1));
        outR = outR + (*pSrc2 * *pCoefB);
        outR = (outR + (*(pSrc2 + 1) * *(pCoefB + 1))) >> 16;


        /* outI = (pIn[2 * i + 1] * pATable[2 * i] + pIn[2 * i] * pATable[2 * i + 1] +
        pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i]);
        */

        outI = *pSrc2 * *(pCoefB + 1);
        outI = outI - (*(pSrc2 + 1) * *pCoefB);
        outI = outI + (*(pSrc1 + 1) * *pCoefA);
        outI = outI + (*pSrc1 * *(pCoefA + 1));

        /* update input pointers */
        pSrc1 += 2U;
        pSrc2 -= 2U;

        /* write output */
        pDst[2U * i] = (q15_t) outR;
        pDst[(2U * i) + 1U] = outI >> 16U;

        /* write complex conjugate output */
        pDst[(4U * fftLen) - (2U * i)] = (q15_t) outR;
        pDst[((4U * fftLen) - (2U * i)) + 1U] = -(outI >> 16U);

        /* update coefficient pointer */
        pCoefB = pCoefB + (2U * modifier);
        pCoefA = pCoefA + (2U * modifier);

        i++;
    }

    pDst[2U * fftLen] = (pSrc[0] - pSrc[1]) >> 1;
    pDst[(2U * fftLen) + 1U] = 0;

    pDst[0] = (pSrc[0] + pSrc[1]) >> 1;
    pDst[1] = 0;

#endif /* #if defined (ARM_MATH_DSP) */
}


/**
* @brief  Core Real IFFT process
* @param[in]   *pSrc 				points to the input buffer.
* @param[in]   fftLen  		    length of FFT.
* @param[in]   *pATable 			points to the twiddle Coef A buffer.
* @param[in]   *pBTable 			points to the twiddle Coef B buffer.
* @param[out]  *pDst 				points to the output buffer.
* @param[in]   modifier 	        twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
* @return none.
* The function implements a Real IFFT
*/
void arm_split_rifft_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    q15_t * pATable,
    q15_t * pBTable,
    q15_t * pDst,
    uint32_t modifier)
{
    uint32_t i;                                    /* Loop Counter */
    q31_t outR, outI;                              /* Temporary variables for output */
    q15_t *pCoefA, *pCoefB;                        /* Temporary pointers for twiddle factors */
    q15_t *pSrc1, *pSrc2;
    q15_t *pDst1 = &pDst[0];

    pCoefA = &pATable[0];
    pCoefB = &pBTable[0];

    pSrc1 = &pSrc[0];
    pSrc2 = &pSrc[2U * fftLen];

#if defined (ARM_MATH_DSP)

    /* Run the below code for Cortex-M4 and Cortex-M3 */
    i = fftLen;

    while (i > 0U)
    {
        /*
        outR = (pIn[2 * i] * pATable[2 * i] + pIn[2 * i + 1] * pATable[2 * i + 1] +
        pIn[2 * n - 2 * i] * pBTable[2 * i] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);

        outI = (pIn[2 * i + 1] * pATable[2 * i] - pIn[2 * i] * pATable[2 * i + 1] -
        pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i]);
        */


#ifndef ARM_MATH_BIG_ENDIAN

        /* pIn[2 * n - 2 * i] * pBTable[2 * i] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i + 1]) */
        outR = __SMUSD(*__SIMD32(pSrc2), *__SIMD32(pCoefB));

#else

        /* -(-pIn[2 * n - 2 * i] * pBTable[2 * i] +
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i + 1])) */
        outR = -(__SMUSD(*__SIMD32(pSrc2), *__SIMD32(pCoefB)));

#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */

        /* pIn[2 * i] * pATable[2 * i] + pIn[2 * i + 1] * pATable[2 * i + 1] +
        pIn[2 * n - 2 * i] * pBTable[2 * i] */
        outR = __SMLAD(*__SIMD32(pSrc1), *__SIMD32(pCoefA), outR) >> 16U;

        /*
        -pIn[2 * n - 2 * i] * pBTable[2 * i + 1] +
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i] */
        outI = __SMUADX(*__SIMD32(pSrc2)--, *__SIMD32(pCoefB));

        /* pIn[2 * i + 1] * pATable[2 * i] - pIn[2 * i] * pATable[2 * i + 1] */

#ifndef ARM_MATH_BIG_ENDIAN

        outI = __SMLSDX(*__SIMD32(pCoefA), *__SIMD32(pSrc1)++, -outI);

#else

        outI = __SMLSDX(*__SIMD32(pSrc1)++, *__SIMD32(pCoefA), -outI);

#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
        /* write output */

#ifndef ARM_MATH_BIG_ENDIAN

        *__SIMD32(pDst1)++ = __PKHBT(outR, (outI >> 16U), 16);

#else

        *__SIMD32(pDst1)++ = __PKHBT((outI >> 16U), outR, 16);

#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */

        /* update coefficient pointer */
        pCoefB = pCoefB + (2U * modifier);
        pCoefA = pCoefA + (2U * modifier);

        i--;
    }
#else
    /* Run the below code for Cortex-M0 */
    i = fftLen;

    while (i > 0U)
    {
        /*
        outR = (pIn[2 * i] * pATable[2 * i] + pIn[2 * i + 1] * pATable[2 * i + 1] +
        pIn[2 * n - 2 * i] * pBTable[2 * i] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);
        */

        outR = *pSrc2 * *pCoefB;
        outR = outR - (*(pSrc2 + 1) * *(pCoefB + 1));
        outR = outR + (*pSrc1 * *pCoefA);
        outR = (outR + (*(pSrc1 + 1) * *(pCoefA + 1))) >> 16;

        /*
        outI = (pIn[2 * i + 1] * pATable[2 * i] - pIn[2 * i] * pATable[2 * i + 1] -
        pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i]);
        */

        outI = *(pSrc1 + 1) * *pCoefA;
        outI = outI - (*pSrc1 * *(pCoefA + 1));
        outI = outI - (*pSrc2 * *(pCoefB + 1));
        outI = outI - (*(pSrc2 + 1) * *(pCoefB));

        /* update input pointers */
        pSrc1 += 2U;
        pSrc2 -= 2U;

        /* write output */
        *pDst1++ = (q15_t) outR;
        *pDst1++ = (q15_t) (outI >> 16);

        /* update coefficient pointer */
        pCoefB = pCoefB + (2U * modifier);
        pCoefA = pCoefA + (2U * modifier);

        i--;
    }
#endif /* #if defined (ARM_MATH_DSP) */
}
