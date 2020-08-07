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

#include "ref_functions.h"

void arm_convolve_HWC_q15_ref(const q15_t * Im_in,  // input image
                              const uint16_t dim_im_in, // input image dimention
                              const uint16_t ch_im_in,  // number of input image channels
                              const q15_t * wt, // kernel weights 
                              const uint16_t ch_im_out, // number of filters, i.e., output image channels
                              const uint16_t dim_kernel,    // filter kernel size
                              const uint16_t padding,   // padding sizes
                              const uint16_t stride,    // stride
                              const q15_t * bias,   // bias
                              const uint16_t bias_shift, const uint16_t out_shift, q15_t * Im_out,  // output image
                              const uint16_t dim_im_out,    // output image dimension
                              q15_t * bufferA,  //buffer space for input
                              q7_t * bufferB    //buffer space for output
    )
{
    int       i, j, k, l, m, n;
    int       conv_out;
    int       in_row, in_col;

    for (i = 0; i < ch_im_out; i++)
    {
        for (j = 0; j < dim_im_out; j++)
        {
            for (k = 0; k < dim_im_out; k++)
            {
#ifndef ARM_NN_TRUNCATE
                conv_out = (bias[i] << bias_shift) + (0x1 << (out_shift - 1));
#else
                conv_out = bias[i] << bias_shift;
#endif
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
                                conv_out += Im_in[(in_row * dim_im_in + in_col) * ch_im_in + l] *
                                    wt[i * ch_im_in * dim_kernel * dim_kernel + (m * dim_kernel + n) * ch_im_in + l];
                            }
                        }
                    }
                }
                Im_out[i + (j * dim_im_out + k) * ch_im_out] = (q15_t) __SSAT((conv_out >> out_shift), 16);
            }
        }
    }
}
