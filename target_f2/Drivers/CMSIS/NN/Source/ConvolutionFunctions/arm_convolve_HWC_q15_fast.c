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
 * Title:        arm_convolve_HWC_q15_fast.c
 * Description:  Fast Q15 version of convolution
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
   * @brief Fast Q15 convolution function
   * @param[in]       Im_in       pointer to input tensor
   * @param[in]       dim_im_in   input tensor dimention
   * @param[in]       ch_im_in    number of input tensor channels
   * @param[in]       wt          pointer to kernel weights
   * @param[in]       ch_im_out   number of filters, i.e., output tensor channels
   * @param[in]       dim_kernel  filter kernel size
   * @param[in]       padding     padding sizes
   * @param[in]       stride      convolution stride
   * @param[in]       bias        pointer to bias
   * @param[in]       bias_shift  amount of left-shift for bias
   * @param[in]       out_shift   amount of right-shift for output
   * @param[in,out]   Im_out      pointer to output tensor
   * @param[in]       dim_im_out  output tensor dimension
   * @param[in,out]   bufferA     pointer to buffer space for input 
   * @param[in,out]   bufferB     pointer to buffer space for output
   * @return     The function returns either
   * <code>ARM_MATH_SIZE_MISMATCH</code> or <code>ARM_MATH_SUCCESS</code> based on the outcome of size checking.
   *
   * @details
   *
   * <b>Buffer size:</b>
   *
   * bufferA size: 2*ch_im_in*dim_kernel*dim_kernel
   *
   * bufferB size: 0
   *
   * <b>Input dimension constraints:</b>
   *
   * ch_im_in is multiple of 2 
   *
   * ch_im_out is multipe of 2
   *
   */

arm_status
arm_convolve_HWC_q15_fast(const q15_t * Im_in,
                          const uint16_t dim_im_in,
                          const uint16_t ch_im_in,
                          const q15_t * wt,
                          const uint16_t ch_im_out,
                          const uint16_t dim_kernel,
                          const uint16_t padding,
                          const uint16_t stride,
                          const q15_t * bias,
                          const uint16_t bias_shift,
                          const uint16_t out_shift,
                          q15_t * Im_out, 
                          const uint16_t dim_im_out, 
                          q15_t * bufferA, 
                          q7_t * bufferB)
{

#if defined (ARM_MATH_DSP)
    int16_t   i_out_y, i_out_x, i_ker_y, i_ker_x;

    q15_t    *pBuffer = bufferA;
    q15_t    *im_buffer = bufferA;
    q15_t    *pOut = Im_out;

    if (ch_im_in % 2 != 0 || ch_im_out % 2 != 0)
    {
        /* check if the input dimension meets the constraints */
        return ARM_MATH_SIZE_MISMATCH;
    }

    /* Run the following code for Cortex-M4 and Cortex-M7 */

    /* This part implements the im2col function */
    for (i_out_y = 0; i_out_y < dim_im_out; i_out_y++)
    {
        for (i_out_x = 0; i_out_x < dim_im_out; i_out_x++)
        {
            for (i_ker_y = i_out_y * stride - padding; i_ker_y < i_out_y * stride - padding + dim_kernel; i_ker_y++)
            {
                for (i_ker_x = i_out_x * stride - padding; i_ker_x < i_out_x * stride - padding + dim_kernel; i_ker_x++)
                {
                    if (i_ker_y < 0 || i_ker_y >= dim_im_in || i_ker_x < 0 || i_ker_x >= dim_im_in)
                    {
                        /* arm_fill_q15(0, pBuffer, ch_im_in); */
                        memset(pBuffer, 0, sizeof(q15_t)*ch_im_in);
                    } else
                    {
                        /* arm_copy_q15((q15_t *) Im_in + (i_ker_y * dim_im_in + i_ker_x) * ch_im_in, pBuffer, ch_im_in); */
                        memcpy(pBuffer, (q15_t *) Im_in + (i_ker_y * dim_im_in + i_ker_x) * ch_im_in, sizeof(q15_t)*ch_im_in);
                    }
                    pBuffer += ch_im_in;
                }
            }

            if (i_out_x & 0x1)
            {
                int       i;
                /* initialize the matrix pointers for A */
                const q15_t *pA = wt;

                /* set up the second output pointers */
                q15_t    *pOut2 = pOut + ch_im_out;

                /* this loop over rows in A */
                for (i = 0; i < ch_im_out; i += 2)
                {
                    /* setup pointers for B */
                    q15_t    *pB = im_buffer;
                    const q15_t *pB2 = pB + ch_im_in * dim_kernel * dim_kernel;

                    /* aling the second pointer for A */
                    const q15_t *pA2 = pA + ch_im_in * dim_kernel * dim_kernel;

                    /* init the sum with bias */
                    q31_t     sum =  ((q31_t)bias[i] << bias_shift) + NN_ROUND(out_shift);
                    q31_t     sum2 = ((q31_t)bias[i] << bias_shift) + NN_ROUND(out_shift);
                    q31_t     sum3 = ((q31_t)bias[i + 1] << bias_shift) + NN_ROUND(out_shift);
                    q31_t     sum4 = ((q31_t)bias[i + 1] << bias_shift) + NN_ROUND(out_shift);

                    uint16_t  colCnt = ch_im_in * dim_kernel * dim_kernel >> 1;
                    /* accumulate over the vector */
                    while (colCnt)
                    {
                        q31_t     inA1 = *__SIMD32(pA)++;
                        q31_t     inB1 = *__SIMD32(pB)++;
                        q31_t     inA2 = *__SIMD32(pA2)++;
                        q31_t     inB2 = *__SIMD32(pB2)++;

                        sum = __SMLAD(inA1, inB1, sum);
                        sum2 = __SMLAD(inA1, inB2, sum2);
                        sum3 = __SMLAD(inA2, inB1, sum3);
                        sum4 = __SMLAD(inA2, inB2, sum4);

                        colCnt--;
                    }           /* while over colCnt */
                    colCnt = ch_im_in * dim_kernel * dim_kernel & 0x1;
                    while (colCnt)
                    {
                        q15_t     inA1 = *pA++;
                        q15_t     inB1 = *pB++;
                        q15_t     inA2 = *pA2++;
                        q15_t     inB2 = *pB2++;

                        sum += inA1 * inB1;
                        sum2 += inA1 * inB2;
                        sum3 += inA2 * inB1;
                        sum4 += inA2 * inB2;
                        colCnt--;
                    }           /* while over colCnt */
                    *pOut++ = (q15_t) __SSAT(sum >> out_shift, 16);
                    *pOut++ = (q15_t) __SSAT(sum3 >> out_shift, 16);
                    *pOut2++ = (q15_t) __SSAT(sum2 >> out_shift, 16);
                    *pOut2++ = (q15_t) __SSAT(sum4 >> out_shift, 16);

                    /* skip the row computed with A2 */
                    pA += ch_im_in * dim_kernel * dim_kernel;
                }               /* for over ch_im_out */

                pOut += ch_im_out;
                /* counter reset */
                pBuffer = im_buffer;
            }
        }
    }

#else
    /* Run the following code as reference implementation for Cortex-M0 and Cortex-M3 */
    uint16_t  i, j, k, l, m, n;
    int       conv_out;
    signed char in_row, in_col;

    if (ch_im_in % 2 != 0 || ch_im_out % 2 != 0)
    {
        /* check if the input dimension meets the constraints */
        return ARM_MATH_SIZE_MISMATCH;
    }

    for (i = 0; i < ch_im_out; i++)
    {
        for (j = 0; j < dim_im_out; j++)
        {
            for (k = 0; k < dim_im_out; k++)
            {
                conv_out = ((q31_t)bias[i] << bias_shift) + NN_ROUND(out_shift);
                for (m = 0; m < dim_kernel; m++)
                {
                    for (n = 0; n < dim_kernel; n++)
                    {
                        in_row = stride * j + m - padding;
                        in_col = stride * k + n - padding;
                        if (in_row >= 0 && in_col >= 0 && in_row < dim_im_in && in_col < dim_im_in)
                        {
                            for (l = 0; l < ch_im_in; l++)
                            {
                                conv_out +=
                                    Im_in[(in_row * dim_im_in + in_col) * ch_im_in +
                                          l] * wt[i * ch_im_in * dim_kernel * dim_kernel + (m * dim_kernel +
                                                                                            n) * ch_im_in + l];
                            }
                        }
                    }
                }
                Im_out[i + (j * dim_im_out + k) * ch_im_out] = (q15_t) __SSAT((conv_out >> out_shift), 16);
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
