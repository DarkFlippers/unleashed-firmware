/*
 * Copyright (C) 2010-2018 Arm Limited or its affiliates. All rights reserved.
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

/* ----------------------------------------------------------------------
 * Project:      CMSIS NN Library
 * Title:        arm_depthwise_separable_conv_HWC_q7_nonsquare.c
 * Description:  Q7 depthwise separable convolution function (non-square shape)
 *
 * $Date:        17. January 2018
 * $Revision:    V.1.0.0
 *
 * Target Processor:  Cortex-M cores
 *
 * -------------------------------------------------------------------- */

#include "arm_math.h"
#include "arm_nnfunctions.h"

/**
 *  @ingroup groupNN
 */

/**
 * @addtogroup NNConv
 * @{
 */

/**
 * @brief Q7 depthwise separable convolution function (non-square shape)
 * @param[in]       Im_in         pointer to input tensor
 * @param[in]       dim_im_in_x   input tensor dimention x
 * @param[in]       dim_im_in_y   input tensor dimention y
 * @param[in]       ch_im_in      number of input tensor channels
 * @param[in]       wt            pointer to kernel weights
 * @param[in]       ch_im_out     number of filters, i.e., output tensor channels
 * @param[in]       dim_kernel_x  filter kernel size x
 * @param[in]       dim_kernel_y  filter kernel size y
 * @param[in]       padding_x     padding sizes x
 * @param[in]       padding_y     padding sizes y
 * @param[in]       stride_x      convolution stride x
 * @param[in]       stride_y      convolution stride y
 * @param[in]       bias          pointer to bias
 * @param[in]       bias_shift    amount of left-shift for bias
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in,out]   Im_out        pointer to output tensor
 * @param[in]       dim_im_out_x  output tensor dimension x
 * @param[in]       dim_im_out_y  output tensor dimension y
 * @param[in,out]   bufferA       pointer to buffer space for input 
 * @param[in,out]   bufferB       pointer to buffer space for output
 * @return     The function returns either
 * <code>ARM_MATH_SIZE_MISMATCH</code> or <code>ARM_MATH_SUCCESS</code> based on the outcome of size checking.
 *
 * This function is the version with full list of optimization tricks, but with
 * some contraints:
 *   ch_im_in is multiple of 2
 *   ch_im_out is multiple of 2
 */

arm_status arm_depthwise_separable_conv_HWC_q7_nonsquare(const q7_t * Im_in,
                                                         const uint16_t dim_im_in_x,
                                                         const uint16_t dim_im_in_y,
                                                         const uint16_t ch_im_in,
                                                         const q7_t * wt,
                                                         const uint16_t ch_im_out,
                                                         const uint16_t dim_kernel_x,
                                                         const uint16_t dim_kernel_y,
                                                         const uint16_t padding_x,
                                                         const uint16_t padding_y,
                                                         const uint16_t stride_x,
                                                         const uint16_t stride_y,
                                                         const q7_t * bias,
                                                         const uint16_t bias_shift,
                                                         const uint16_t out_shift,
                                                         q7_t * Im_out,
                                                         const uint16_t dim_im_out_x,
                                                         const uint16_t dim_im_out_y, 
                                                         q15_t * bufferA, 
                                                         q7_t * bufferB)
{

#if defined (ARM_MATH_DSP)
    /* Run the following code for Cortex-M4 and Cortex-M7 */

/*
 * Implementation:
 * There are 3 nested loop here:
 * Inner loop: calculate each output value with MAC instruction over an accumulator
 * Mid   loop: loop over different output channel
 * Outer loop: loop over different output (x, y)
 *
 */

    int16_t   i_out_y, i_out_x;
    int16_t   i_ker_y, i_ker_x;
    q7_t     *colBuffer = (q7_t *) bufferA;
    q7_t     *pBuffer = colBuffer;
    const q7_t *pBias = bias;
    q7_t     *pOut = Im_out;
    uint16_t  rowCnt;
    uint16_t  row_shift;

    /* do some checking here, basically ch_im_in == ch_im_out */
    if (ch_im_in != ch_im_out)
    {
        return ARM_MATH_SIZE_MISMATCH;
    }

    for (i_out_y = 0; i_out_y < dim_im_out_y; i_out_y++)
    {
        for (i_out_x = 0; i_out_x < dim_im_out_x; i_out_x++)
        {
            /* we first do im2col here */
            for (i_ker_y = i_out_y * stride_y - padding_y; i_ker_y < i_out_y * stride_y - padding_y + dim_kernel_y;
                 i_ker_y++)
            {
                for (i_ker_x = i_out_x * stride_x - padding_x; i_ker_x < i_out_x * stride_x - padding_x + dim_kernel_x;
                     i_ker_x++)
                {
                    if (i_ker_y < 0 || i_ker_y >= dim_im_in_y || i_ker_x < 0 || i_ker_x >= dim_im_in_x)
                    {
                        /* arm_fill_q7(0, pBuffer, ch_im_in); */
                        memset(pBuffer, 0, ch_im_in);
                    } else
                    {
                        /* arm_copy_q7((q7_t *) Im_in + (i_ker_y * dim_im_in_x + i_ker_x) * ch_im_in, pBuffer, ch_im_in); */
                        memcpy(pBuffer, (q7_t *) Im_in + (i_ker_y * dim_im_in_x + i_ker_x) * ch_im_in, ch_im_in);
                    }
                    pBuffer += ch_im_in;
                }
            }

            /* we will do the computation here for each channel */
            rowCnt = ch_im_out >> 2;
            row_shift = 0;
            pBias = bias;

            while (rowCnt)
            {
                q31_t     sum =  ((q31_t)(*pBias++) << bias_shift) + NN_ROUND(out_shift);
                q31_t     sum2 = ((q31_t)(*pBias++) << bias_shift) + NN_ROUND(out_shift);
                q31_t     sum3 = ((q31_t)(*pBias++) << bias_shift) + NN_ROUND(out_shift);
                q31_t     sum4 = ((q31_t)(*pBias++) << bias_shift) + NN_ROUND(out_shift);

                uint16_t  colCnt = (dim_kernel_x * dim_kernel_y) >> 1;
                q7_t     *pB = colBuffer + row_shift;
                const q7_t *pA = wt + row_shift;
                row_shift += 4;

#ifdef USE_INTRINSIC

#ifndef ARM_MATH_BIG_ENDIAN

                while (colCnt)
                {
                    q31_t     inA1, inA2, inB1, inB2, opA, opB;

                    inB1 = *__SIMD32(pB);
                    pB += ch_im_in;
                    opB = *__SIMD32(pB);
                    pB += ch_im_in;
                    inB2 = __PKHTB(opB, inB1, 16);
                    inB1 = __PKHBT(inB1, opB, 16);
                    inA1 = *__SIMD32(pA);
                    pA += ch_im_in;
                    opB = *__SIMD32(pA);
                    pA += ch_im_in;
                    inA2 = __PKHTB(opB, inA1, 16);
                    inA1 = __PKHBT(inA1, opB, 16);
                    opA = __SXTB16(inA1);
                    opB = __SXTB16(inB1);
                    sum = __SMLAD(opA, opB, sum);
                    opA = __SXTB16(__ROR(inA1, 8));
                    opB = __SXTB16(__ROR(inB1, 8));
                    sum2 = __SMLAD(opA, opB, sum2);
                    opA = __SXTB16(inA2);
                    opB = __SXTB16(inB2);
                    sum3 = __SMLAD(opA, opB, sum3);
                    opA = __SXTB16(__ROR(inA2, 8));
                    opB = __SXTB16(__ROR(inB2, 8));
                    sum4 = __SMLAD(opA, opB, sum4);
                    colCnt--;
                }
#else

                while (colCnt)
                {
                    q31_t     inA1, inA2, inB1, inB2, opA, opB;

                    inB1 = *__SIMD32(pB);
                    pB += ch_im_in;
                    opB = *__SIMD32(pB);
                    pB += ch_im_in;
                    inB2 = __PKHBT(opB, inB1, 16);
                    inB1 = __PKHTB(inB1, opB, 16);
                    inA1 = *__SIMD32(pA);
                    pA += ch_im_in;
                    opB = *__SIMD32(pA);
                    pA += ch_im_in;
                    inA2 = __PKHBT(opB, inA1, 16);
                    inA1 = __PKHTB(inA1, opB, 16);
                    opA = __SXTB16(inA1);
                    opB = __SXTB16(inB1);
                    sum2 = __SMLAD(opA, opB, sum2);
                    opA = __SXTB16(__ROR(inA1, 8));
                    opB = __SXTB16(__ROR(inB1, 8));
                    sum = __SMLAD(opA, opB, sum);
                    opA = __SXTB16(inA2);
                    opB = __SXTB16(inB2);
                    sum4 = __SMLAD(opA, opB, sum4);
                    opA = __SXTB16(__ROR(inA2, 8));
                    opB = __SXTB16(__ROR(inB2, 8));
                    sum3 = __SMLAD(opA, opB, sum3);
                    colCnt--;
                }

#endif                          /* ARM_MATH_BIG_ENDIAN */

#else

#ifndef ARM_MATH_BIG_ENDIAN
                //  r0    r1    r2    r3    r4   r5
                // inA1, inA2, inB1, inB2, opA, opB
                asm volatile ("COL_LOOP:\n"
                              "ldr.w r2, [%[pB], #0]\n"
                              "add.w %[pB], %[pB], %[ch_im_in]\n"
                              "ldr.w r5, [%[pB], #0]\n"
                              "add.w %[pB], %[pB], %[ch_im_in]\n"
                              "pkhtb r3, r5, r2, ASR #16\n"
                              "pkhbt r2, r2, r5, LSL #16\n"
                              "ldr.w r0, [%[pA], #0]\n"
                              "add.w %[pA], %[pA], %[ch_im_in]\n"
                              "ldr.w r5, [%[pA], #0]\n"
                              "add.w %[pA], %[pA], %[ch_im_in]\n"
                              "pkhtb r1, r5, r0, ASR #16\n"
                              "pkhbt r0, r0, r5, LSL #16\n"
                              "sxtb16 r4, r0\n"
                              "sxtb16 r5, r2\n"
                              "smlad %[sum], r4, r5, %[sum]\n"
                              "mov.w r4, r0, ror #8\n"
                              "mov.w r5, r2, ror #8\n"
                              "sxtb16 r4, r4\n"
                              "sxtb16 r5, r5\n"
                              "smlad %[sum2], r4, r5, %[sum2]\n"
                              "sxtb16 r4, r1\n"
                              "sxtb16 r5, r3\n"
                              "smlad %[sum3], r4, r5, %[sum3]\n"
                              "mov.w r4, r1, ror #8\n"
                              "mov.w r5, r3, ror #8\n"
                              "sxtb16 r4, r4\n"
                              "sxtb16 r5, r5\n"
                              "smlad %[sum4], r4, r5, %[sum4]\n"
                              "subs %[colCnt], #1\n"
                              "bne COL_LOOP\n":[sum] "+r"(sum),[sum2] "+r"(sum2),[sum3] "+r"(sum3),
                              [sum4] "+r"(sum4),[pB] "+r"(pB),[pA] "+r"(pA):[colCnt] "r"(colCnt),
                              [ch_im_in] "r"(ch_im_in):"r0", "r1", "r2", "r3", "r4", "r5");
#else
                //  r0    r1    r2    r3    r4   r5
                // inA1, inA2, inB1, inB2, opA, opB
                asm volatile ("COL_LOOP:\n"
                              "ldr.w r2, [%[pB], #0]\n"
                              "add.w %[pB], %[pB], %[ch_im_in]\n"
                              "ldr.w r5, [%[pB], #0]\n"
                              "add.w %[pB], %[pB], %[ch_im_in]\n"
                              "pkhbt r3, r5, r2, LSL #16\n"
                              "pkhtb r2, r2, r5, ASR #16\n"
                              "ldr.w r0, [%[pA], #0]\n"
                              "add.w %[pA], %[pA], %[ch_im_in]\n"
                              "ldr.w r5, [%[pA], #0]\n"
                              "add.w %[pA], %[pA], %[ch_im_in]\n"
                              "pkhbt r1, r5, r0, LSL #16\n"
                              "pkhtb r0, r0, r5, ASR #16\n"
                              "sxtb16 r4, r0\n"
                              "sxtb16 r5, r2\n"
                              "smlad %[sum2], r4, r5, %[sum2]\n"
                              "mov.w r4, r0, ror #8\n"
                              "mov.w r5, r2, ror #8\n"
                              "sxtb16 r4, r4\n"
                              "sxtb16 r5, r5\n"
                              "smlad %[sum], r4, r5, %[sum]\n"
                              "sxtb16 r4, r1\n"
                              "sxtb16 r5, r3\n"
                              "smlad %[sum4], r4, r5, %[sum4]\n"
                              "mov.w r4, r1, ror #8\n"
                              "mov.w r5, r3, ror #8\n"
                              "sxtb16 r4, r4\n"
                              "sxtb16 r5, r5\n"
                              "smlad %[sum3], r4, r5, %[sum3]\n"
                              "subs %[colCnt], #1\n"
                              "bne COL_LOOP\n":[sum] "+r"(sum),[sum2] "+r"(sum2),[sum3] "+r"(sum3),
                              [sum4] "+r"(sum4),[pB] "+r"(pB),[pA] "+r"(pA):[colCnt] "r"(colCnt),
                              [ch_im_in] "r"(ch_im_in):"r0", "r1", "r2", "r3", "r4", "r5");
#endif                          /*ARM_MATH_BIG_ENDIAN */

#endif                          /* USE_INTRINSIC */

                colCnt = (dim_kernel_x * dim_kernel_y) & 0x1;
                while (colCnt)
                {
                    union arm_nnword inA, inB;
                    inA.word = *__SIMD32(pA);
                    pA += ch_im_in;
                    inB.word = *__SIMD32(pB);
                    pB += ch_im_in;
                    sum += inA.bytes[0] * inB.bytes[0];
                    sum2 += inA.bytes[1] * inB.bytes[1];
                    sum3 += inA.bytes[2] * inB.bytes[2];
                    sum4 += inA.bytes[3] * inB.bytes[3];
                    colCnt--;
                }

                *pOut++ = (q7_t) __SSAT((sum >> out_shift), 8);
                *pOut++ = (q7_t) __SSAT((sum2 >> out_shift), 8);
                *pOut++ = (q7_t) __SSAT((sum3 >> out_shift), 8);
                *pOut++ = (q7_t) __SSAT((sum4 >> out_shift), 8);

                rowCnt--;
            }

            rowCnt = ch_im_out & 0x3;
            while (rowCnt)
            {
                q7_t     *pB = colBuffer + row_shift;
                const q7_t *pA = wt + row_shift;
                q31_t     sum = ((q31_t)(*pBias++) << bias_shift) + NN_ROUND(out_shift);
                uint16_t  colCnt = (dim_kernel_x * dim_kernel_y);

                row_shift += 1;

                while (colCnt)
                {
                    q7_t      A1 = *pA;
                    q7_t      B1 = *pB;
                    pA += ch_im_in;
                    pB += ch_im_in;
                    sum += A1 * B1;

                    colCnt--;
                }
                *pOut++ = (q7_t) __SSAT((sum >> out_shift), 8);
                rowCnt--;
            }

            // clear counter and pointers
            pBuffer = colBuffer;
        }
    }

#else
    /* Run the following code as reference implementation for Cortex-M0 and Cortex-M3 */
    int       i_out_y, i_out_x, i_ch_out;
    int       i_ker_y, i_ker_x; 

    /* do some checking here, basically ch_im_in == ch_im_out */
    if (ch_im_in != ch_im_out)
    {
        return ARM_MATH_SIZE_MISMATCH;
    }

    for (i_out_y = 0; i_out_y < dim_im_out_y; i_out_y++)
    {
        for (i_out_x = 0; i_out_x < dim_im_out_x; i_out_x++)
        {
            for (i_ch_out = 0; i_ch_out < ch_im_out; i_ch_out++)
            {
                // for each output 
                int       conv_out = ((q31_t)(bias[i_ch_out]) << bias_shift) + NN_ROUND(out_shift);
                for (i_ker_y = 0; i_ker_y < dim_kernel_y; i_ker_y++)
                {
                    for (i_ker_x = 0; i_ker_x < dim_kernel_x; i_ker_x++)
                    {
                        int       in_row = stride_y * i_out_y + i_ker_y - padding_y;
                        int       in_col = stride_x * i_out_x + i_ker_x - padding_x;
                        if (in_row >= 0 && in_col >= 0 && in_row < dim_im_in_y && in_col < dim_im_in_x)
                        {
                            conv_out += Im_in[(in_row * dim_im_in_x + in_col) * ch_im_in + i_ch_out] *                        
                                wt[(i_ker_y * dim_kernel_x + i_ker_x) * ch_im_out + i_ch_out];
                        }
                    }
                }
                Im_out[(i_out_y * dim_im_out_x + i_out_x) * ch_im_out + i_ch_out] =
                    (q7_t) __SSAT((conv_out >> out_shift), 8);
            }
        }
    }

#endif                          /* ARM_MATH_DSP */


    /* Return to application */
    return ARM_MATH_SUCCESS;

}

/**
 * @} end of NNConv group
 */
