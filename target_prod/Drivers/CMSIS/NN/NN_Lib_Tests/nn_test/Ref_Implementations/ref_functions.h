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

#ifndef _REF_FUNCTIONS_H_
#define _REF_FUNCTIONS_H_

#include "arm_math.h"
#include "arm_nnfunctions.h"
//#include "arm_nnsupportfunctions.h"
#include "fully_connected_testing_weights.h"

#ifdef __cplusplus
extern    "C"
{
#endif

/*
 *
 * Convolution reference implemenation
 *
 */

    void      arm_convolve_HWC_q7_ref(const q7_t * Im_in,   // input image
                                      const uint16_t dim_im_in, // input image dimention
                                      const uint16_t ch_im_in,  // number of input image channels
                                      const q7_t * wt,  // kernel weights 
                                      const uint16_t ch_im_out, // number of filters, i.e., output image channels
                                      const uint16_t dim_kernel,    // filter kernel size
                                      const uint16_t padding,   // padding sizes
                                      const uint16_t stride,    // stride
                                      const q7_t * bias,    // bias
                                      const uint16_t bias_shift, const uint16_t out_shift, q7_t * Im_out,   // output image
                                      const uint16_t dim_im_out,    // output image dimension
                                      q15_t * bufferA,  //buffer space for input
                                      q7_t * bufferB    //buffer space for output
        );

    void      arm_convolve_HWC_q7_ref_nonsquare(const q7_t * Im_in, // input image
                                                const uint16_t dim_im_in_x, // input image dimention x
                                                const uint16_t dim_im_in_y, // input image dimention y
                                                const uint16_t ch_im_in,    // number of input image channels
                                                const q7_t * wt,    // kernel weights 
                                                const uint16_t ch_im_out,   // number of filters, i.e., output image channels
                                                const uint16_t dim_kernel_x,    // filter kernel size x
                                                const uint16_t dim_kernel_y,    // filter kernel size y
                                                const uint16_t padding_x,   // padding sizes x
                                                const uint16_t padding_y,   // padding sizes y
                                                const uint16_t stride_x,    // stride x
                                                const uint16_t stride_y,    // stride y
                                                const q7_t * bias,  // bias
                                                const uint16_t bias_shift, const uint16_t out_shift, q7_t * Im_out, // output image
                                                const uint16_t dim_im_out_x,    // output image dimension x
                                                const uint16_t dim_im_out_y,    // output image dimension y
                                                q15_t * bufferA,    //buffer space for input
                                                q7_t * bufferB  //buffer space for output
        );

    void      arm_convolve_HWC_q15_ref(const q15_t * Im_in, // input image
                                       const uint16_t dim_im_in,    // input image dimention
                                       const uint16_t ch_im_in, // number of input image channels
                                       const q15_t * wt,    // kernel weights 
                                       const uint16_t ch_im_out,    // number of filters, i.e., output image channels
                                       const uint16_t dim_kernel,   // filter kernel size
                                       const uint16_t padding,  // padding sizes
                                       const uint16_t stride,   // stride
                                       const q15_t * bias,  // bias
                                       const uint16_t bias_shift, const uint16_t out_shift, q15_t * Im_out, // output image
                                       const uint16_t dim_im_out,   // output image dimension
                                       q15_t * bufferA, //buffer space for input
                                       q7_t * bufferB   //buffer space for output
        );
    void      arm_convolve_HWC_q15_nonsquare_ref(const q15_t * Im_in,
                                                      const uint16_t dim_im_in_x,
                                                      const uint16_t dim_im_in_y,
                                                      const uint16_t ch_im_in,
                                                      const q15_t * wt,
                                                      const uint16_t ch_im_out,
                                                      const uint16_t dim_kernel_x,
                                                      const uint16_t dim_kernel_y,
                                                      const uint16_t padding_x,
                                                      const uint16_t padding_y,
                                                      const uint16_t stride_x,
                                                      const uint16_t stride_y,
                                                      const q15_t * bias,
                                                      const uint16_t bias_shift,
                                                      const uint16_t out_shift,
                                                      q15_t * Im_out,
                                                      const uint16_t dim_im_out_x,
                                                      const uint16_t dim_im_out_y, 
                                                      q15_t * bufferA, 
                                                      q7_t * bufferB);
													  
    void      arm_depthwise_separable_conv_HWC_q7_ref(const q7_t * Im_in,   // input image
                                                      const uint16_t dim_im_in, // input image dimention
                                                      const uint16_t ch_im_in,  // number of input image channels
                                                      const q7_t * wt,  // kernel weights 
                                                      const uint16_t ch_im_out, // number of filters, i.e., output image channels
                                                      const uint16_t dim_kernel,    // filter kernel size
                                                      const uint16_t padding,   // padding sizes
                                                      const uint16_t stride,    // stride
                                                      const q7_t * bias,    // bias
                                                      const uint16_t bias_shift,    // amount of left-shift for bias
                                                      const uint16_t out_shift, // amount of right-shift for output
                                                      q7_t * Im_out,    // output image
                                                      const uint16_t dim_im_out,    // output image dimension
                                                      q15_t * bufferA,  //buffer space for input
                                                      q7_t * bufferB    //buffer space for output
        );
    void      arm_depthwise_separable_conv_HWC_q7_ref_nonsquare(const q7_t * Im_in, // input image
                                                                const uint16_t dim_im_in_x, // input image dimention x
                                                                const uint16_t dim_im_in_y, // input image dimention y
                                                                const uint16_t ch_im_in,    // number of input image channels
                                                                const q7_t * wt,    // kernel weights 
                                                                const uint16_t ch_im_out,   // number of filters, i.e., output image channels
                                                                const uint16_t dim_kernel_x,    // filter kernel size x
                                                                const uint16_t dim_kernel_y,    // filter kernel size y
                                                                const uint16_t padding_x,   // padding sizes x
                                                                const uint16_t padding_y,   // padding sizes y
                                                                const uint16_t stride_x,    // stride x
                                                                const uint16_t stride_y,    // stride y
                                                                const q7_t * bias,  // bias
                                                                const uint16_t bias_shift,  // amount of left-shift for bias
                                                                const uint16_t out_shift,   // amount of right-shift for output
                                                                q7_t * Im_out,  // output image
                                                                const uint16_t dim_im_out_x,    // output image dimension x
                                                                const uint16_t dim_im_out_y,    // output image dimension y
                                                                q15_t * bufferA,    //buffer space for input
                                                                q7_t * bufferB  //buffer space for output
        );

/*
 *
 * Fully-connected reference implemenation
 *
 */

    void      arm_fully_connected_q7_ref(const q7_t * pV,   // pointer to vector
                                         const q7_t * pM,   // pointer to matrix
                                         const uint16_t dim_vec,    // length of the vector
                                         const uint16_t num_of_rows,    // numCol of A
                                         const uint16_t bias_shift, // amount of left-shift for bias
                                         const uint16_t out_shift,  // amount of right-shift for output
                                         const q7_t * bias, q7_t * pOut,    // output operand
                                         q15_t * vec_buffer);

    void      arm_fully_connected_q15_ref(const q15_t * pV, // pointer to vector
                                          const q15_t * pM, // pointer to matrix
                                          const uint16_t dim_vec,   // length of the vector
                                          const uint16_t num_of_rows,   // numCol of A
                                          const uint16_t bias_shift,    // amount of left-shift for bias
                                          const uint16_t out_shift, // amount of right-shift for output
                                          const q15_t * bias, q15_t * pOut, // output operand
                                          q15_t * vec_buffer);

    void      arm_fully_connected_mat_q7_vec_q15_ref(const q15_t * pV,  // pointer to vector
                                                     const q7_t * pM,   // pointer to matrix
                                                     const uint16_t dim_vec,    // length of the vector
                                                     const uint16_t num_of_rows,    // numCol of A
                                                     const uint16_t bias_shift, // amount of left-shift for bias
                                                     const uint16_t out_shift,  // amount of right-shift for output
                                                     const q7_t * bias, q15_t * pOut,   // output operand
                                                     q15_t * vec_buffer);

    void      arm_fully_connected_q7_opt_ref(const q7_t * pV,   // pointer to vector
                                             const q7_t * pM,   // pointer to matrix
                                             const uint16_t dim_vec,    // length of the vector
                                             const uint16_t num_of_rows,    // numCol of A
                                             const uint16_t bias_shift, // amount of left-shift for bias
                                             const uint16_t out_shift,  // amount of right-shift for output
                                             const q7_t * bias, q7_t * pOut,    // output operand
                                             q15_t * vec_buffer);

    void      arm_fully_connected_q15_opt_ref(const q15_t * pV, // pointer to vector
                                              const q15_t * pM, // pointer to matrix
                                              const uint16_t dim_vec,   // length of the vector
                                              const uint16_t num_of_rows,   // numCol of A
                                              const uint16_t bias_shift,    // amount of left-shift for bias
                                              const uint16_t out_shift, // amount of right-shift for output
                                              const q15_t * bias, q15_t * pOut, // output operand
                                              q15_t * vec_buffer);

    void      arm_fully_connected_mat_q7_vec_q15_opt_ref(const q15_t * pV,  // pointer to vector
                                                         const q7_t * pM,   // pointer to matrix
                                                         const uint16_t dim_vec,    // length of the vector
                                                         const uint16_t num_of_rows,    // numCol of A
                                                         const uint16_t bias_shift, // amount of left-shift for bias
                                                         const uint16_t out_shift,  // amount of right-shift for output
                                                         const q7_t * bias, q15_t * pOut,   // output operand
                                                         q15_t * vec_buffer);

/*
 *
 * Pooling reference implemenation
 *
 */

    void      arm_avepool_q7_HWC_ref(const q7_t * Im_in,    // input image
                                     const uint16_t dim_im_in,  // input image dimension
                                     const uint16_t ch_im_in,   // number of input image channels
                                     const uint16_t dim_kernel, // window kernel size
                                     const uint16_t padding,    // padding sizes
                                     const uint16_t stride, // stride
                                     const uint16_t dim_im_out, // output image dimension
                                     q7_t * bufferA,    // a buffer for local storage
                                     q7_t * Im_out);

    void      arm_maxpool_q7_HWC_ref(const q7_t * Im_in,    // input image
                                     const uint16_t dim_im_in,  // input image dimension
                                     const uint16_t ch_im_in,   // number of input image channels
                                     const uint16_t dim_kernel, // window kernel size
                                     const uint16_t padding,    // padding sizes
                                     const uint16_t stride, // stride
                                     const uint16_t dim_im_out, // output image dimension
                                     q7_t * bufferA,    // a buffer for local storage
                                     q7_t * Im_out);

/*
 *
 * Other reference implemenation
 *
 */

    void      arm_relu_q7_ref(q7_t * data, uint16_t size);

    void      arm_relu_q15_ref(q15_t * data, uint16_t size);

    void      arm_nn_mult_q7_ref(q7_t * pSrcA, q7_t * pSrcB, q7_t * pDst, const uint16_t out_shift, uint32_t blockSize);

    void      arm_nn_mult_q15_ref(q15_t * pSrcA, q15_t * pSrcB, q15_t * pDst, const uint16_t out_shift, uint32_t blockSize);

#ifdef __cplusplus
}
#endif

#endif
