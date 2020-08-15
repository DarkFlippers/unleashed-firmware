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

void arm_depthwise_separable_conv_HWC_q7_ref(const q7_t * Im_in,    // input image
                                             const uint16_t dim_im_in,  // input image dimention
                                             const uint16_t ch_im_in,   // number of input image channels
                                             const q7_t * wt,   // kernel weights 
                                             const uint16_t ch_im_out,  // number of filters, i.e., output image channels
                                             const uint16_t dim_kernel, // filter kernel size
                                             const uint16_t padding,    // padding sizes
                                             const uint16_t stride, // stride
                                             const q7_t * bias, // bias
                                             const uint16_t bias_shift, // amount of left-shift for bias
                                             const uint16_t out_shift,  // amount of right-shift for output
                                             q7_t * Im_out, // output image
                                             const uint16_t dim_im_out, // output image dimension
                                             q15_t * bufferA,   //buffer space for input
                                             q7_t * bufferB //buffer space for output
    )
{
    int       i_out_y, i_out_x, i_ch_out;
    int       i_ker_y, i_ker_x;
    for (i_out_y = 0; i_out_y < dim_im_out; i_out_y++)
    {
        for (i_out_x = 0; i_out_x < dim_im_out; i_out_x++)
        {
            for (i_ch_out = 0; i_ch_out < ch_im_out; i_ch_out++)
            {
                // for each output
#ifndef ARM_NN_TRUNCATE
                int       conv_out = (bias[i_ch_out] << bias_shift) + (0x1 << (out_shift - 1));
#else
                int       conv_out = bias[i_ch_out] << bias_shift;
#endif
                for (i_ker_y = 0; i_ker_y < dim_kernel; i_ker_y++)
                {
                    for (i_ker_x = 0; i_ker_x < dim_kernel; i_ker_x++)
                    {
                        int       in_row = stride * i_out_y + i_ker_y - padding;
                        int       in_col = stride * i_out_x + i_ker_x - padding;
                        if (in_row >= 0 && in_col >= 0 && in_row < dim_im_in && in_col < dim_im_in)
                        {
                            conv_out += Im_in[(in_row * dim_im_in + in_col) * ch_im_in + i_ch_out] *
                                wt[(i_ker_y * dim_kernel + i_ker_x) * ch_im_out + i_ch_out];
                        }
                    }
                }
                Im_out[(i_out_y * dim_im_out + i_out_x) * ch_im_out + i_ch_out] =
                    (q7_t) __SSAT((conv_out >> out_shift), 8);
            }
        }
    }
}
