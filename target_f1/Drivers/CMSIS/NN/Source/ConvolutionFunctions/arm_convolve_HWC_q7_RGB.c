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
 * Title:        arm_convolve_HWC_q7_RGB.c
 * Description:  Q7 version of convolution for RGB image
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
   * @brief Q7 convolution function for RGB image
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
   * ch_im_in equals 3
   *
   * This kernel is written exclusively for convolution with ch_im_in
   * equals 3. This applies on the first layer of CNNs which has input
   * image with RGB format.
   */

arm_status
arm_convolve_HWC_q7_RGB(const q7_t * Im_in,
                        const uint16_t dim_im_in,
                        const uint16_t ch_im_in,
                        const q7_t * wt,
                        const uint16_t ch_im_out,
                        const uint16_t dim_kernel,
                        const uint16_t padding,
                        const uint16_t stride,
                        const q7_t * bias,
                        const uint16_t bias_shift,
                        const uint16_t out_shift,
                        q7_t * Im_out, const uint16_t dim_im_out, q15_t * bufferA, q7_t * bufferB)
{

#if defined (ARM_MATH_DSP)
    /* Run the following code for Cortex-M4 and Cortex-M7 */
    int16_t   i_out_y, i_out_x, i_ker_y, i_ker_x;

    /*
     *  Here we use bufferA as q15_t internally as computation are done with q15_t level
     *  im2col are done to output in q15_t format from q7_t input
     */
    q15_t    *pBuffer = bufferA;
    q7_t     *pOut = Im_out;

    // check if number of input channels is 3
    if (ch_im_in != 3)
    {
        return ARM_MATH_SIZE_MISMATCH;
    }
    // This part implements the im2col function
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
                        /* Equivalent to arm_fill_q15(0, pBuffer, ch_im_in) with assumption: ch_im_in = 3 */
                        *__SIMD32(pBuffer) = 0x0;
                        *(pBuffer + 2) = 0;
                        pBuffer += 3;
                    } else
                    {
                        /* 
                         * Equivalent to:
                         *  arm_q7_to_q15_no_shift( (q7_t*)Im_in+(i_ker_y*dim_im_in+i_ker_x)*3, pBuffer, 3);
                         */

                        const q7_t *pPixel = Im_in + (i_ker_y * dim_im_in + i_ker_x) * 3;
                        q31_t     buf = *__SIMD32(pPixel);

                        union arm_nnword top;
                        union arm_nnword bottom;

                        top.word = __SXTB16(buf);
                        bottom.word = __SXTB16(__ROR(buf, 8));

#ifndef ARM_MATH_BIG_ENDIAN
                        /*
                         *  little-endian, | omit | 3rd  | 2nd  | 1st  |
                         *                MSB                         LSB
                         *   top | 3rd | 1st |; bottom | omit | 2nd |
                         *
                         *  version 1, need to swap 2nd and 3rd weight
                         * *__SIMD32(pBuffer) = top.word;
                         * *(pBuffer+2) = bottom.half_words[0];
                         *
                         *  version 2, no weight shuffling required
                         */
                        *pBuffer++ = top.half_words[0];
                        *__SIMD32(pBuffer) = __PKHBT(bottom.word, top.word, 0);
#else
                        /*
                         *  big-endian,    | 1st  | 2nd  | 3rd  | omit | 
                         *                MSB                         LSB
                         *  top | 2nd | omit |; bottom | 1st | 3rd |
                         * 
                         *  version 1, need to swap 2nd and 3rd weight
                         * *__SIMD32(pBuffer) = bottom.word;
                         * *(pBuffer+2) = top.half_words[1];
                         * 
                         *  version 2, no weight shuffling required
                         */
                        *pBuffer++ = bottom.half_words[0];
                        *__SIMD32(pBuffer) = __PKHTB(top.word, bottom.word, 0);
#endif
                        pBuffer += 2;
                    }
                }
            }

            if (pBuffer == bufferA + 2 * 3 * dim_kernel * dim_kernel)
            {
                pOut =
                    arm_nn_mat_mult_kernel_q7_q15(wt, bufferA,
                                                  ch_im_out,
                                                  3 * dim_kernel * dim_kernel, bias_shift, out_shift, bias, pOut);

                /* counter reset */
                pBuffer = bufferA;
            }
        }
    }

    /* left-over because odd number of output pixels */
    if (pBuffer != bufferA)
    {
        const q7_t *pA = wt;
        int       i;

        for (i = 0; i < ch_im_out; i++)
        {
            q31_t     sum = ((q31_t)bias[i] << bias_shift) + NN_ROUND(out_shift);
            q15_t    *pB = bufferA;
            /* basically each time it process 4 entries */
            uint16_t  colCnt = 3 * dim_kernel * dim_kernel >> 2;

            while (colCnt)
            {

                q31_t     inA1, inA2;
                q31_t     inB1, inB2;

                pA = (q7_t *) read_and_pad((void *)pA, &inA1, &inA2);

                inB1 = *__SIMD32(pB)++;
                sum = __SMLAD(inA1, inB1, sum);
                inB2 = *__SIMD32(pB)++;
                sum = __SMLAD(inA2, inB2, sum);

                colCnt--;
            }
            colCnt = 3 * dim_kernel * dim_kernel & 0x3;
            while (colCnt)
            {
                q7_t      inA1 = *pA++;
                q15_t     inB1 = *pB++;
                sum += inA1 * inB1;
                colCnt--;
            }
            *pOut++ = (q7_t) __SSAT((sum >> out_shift), 8);
        }
    }
#else
    /* Run the following code as reference implementation for Cortex-M0 and Cortex-M3 */

    uint16_t  i, j, k, l, m, n;
    int       conv_out;
    signed char in_row, in_col;

    // check if number of input channels is 3
    if (ch_im_in != 3)
    {
        return ARM_MATH_SIZE_MISMATCH;
    }

    for (i = 0; i < ch_im_out; i++)
    {
        for (j = 0; j < dim_im_out; j++)
        {
            for (k = 0; k < dim_im_out; k++)
            {
                conv_out = (bias[i] << bias_shift) + NN_ROUND(out_shift);
                for (m = 0; m < dim_kernel; m++)
                {
                    for (n = 0; n < dim_kernel; n++)
                    {
                        /* if-for implementation */
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
                Im_out[i + (j * dim_im_out + k) * ch_im_out] = (q7_t) __SSAT((conv_out >> out_shift), 8);
            }
        }
    }

#endif                          /* ARM_MATH_DSP */

    /* Return to application */
    return (ARM_MATH_SUCCESS);
}

/**
 * @} end of NNConv group
 */
