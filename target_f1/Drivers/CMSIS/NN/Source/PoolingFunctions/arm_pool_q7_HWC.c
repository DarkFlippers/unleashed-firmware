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
 * Title:        arm_pool_q7_HWC.c
 * Description:  Pooling function implementations
 *
 * $Date:        17. January 2018
 * $Revision:    V.1.0.0
 *
 * Target Processor:  Cortex-M cores
 *
 * -------------------------------------------------------------------- */

#include "arm_math.h"
#include "arm_nnfunctions.h"

#if defined (ARM_MATH_DSP)

/**
 * @brief A few utility functions used by pooling functions
 *
 * 
 */

static void buffer_scale_back_q15_to_q7(q15_t * buffer, q7_t * target, uint16_t length, uint16_t scale)
{
    int       i;

    for (i = 0; i < length; i++)
    {
        target[i] = (q7_t) (buffer[i] / scale);
    }
}

static void compare_and_replace_if_larger_q7(q7_t * base,   // base data
                                             q7_t * target, // compare target
                                             const uint16_t length  // data length
    )
{
    q7_t     *pIn = base;
    q7_t     *pCom = target;
    union arm_nnword in;
    union arm_nnword com;
    uint16_t  cnt = length >> 2;

    while (cnt > 0u)
    {
        in.word = *__SIMD32(pIn);
        com.word = *__SIMD32(pCom)++;

        // if version
        if (com.bytes[0] > in.bytes[0])
            in.bytes[0] = com.bytes[0];
        if (com.bytes[1] > in.bytes[1])
            in.bytes[1] = com.bytes[1];
        if (com.bytes[2] > in.bytes[2])
            in.bytes[2] = com.bytes[2];
        if (com.bytes[3] > in.bytes[3])
            in.bytes[3] = com.bytes[3];

        *__SIMD32(pIn)++ = in.word;

        cnt--;
    }
}

static void accumulate_q7_to_q15(q15_t * base, q7_t * target, const uint16_t length)
{
    q15_t    *pCnt = base;
    q7_t     *pV = target;
    q31_t     v1, v2, vo1, vo2;
    uint16_t  cnt = length >> 2;
    q31_t     in;

    while (cnt > 0u)
    {
        q31_t     value = *__SIMD32(pV)++;
        v1 = __SXTB16(__ROR(value, 8));
        v2 = __SXTB16(value);
#ifndef ARM_MATH_BIG_ENDIAN

        vo2 = __PKHTB(v1, v2, 16);
        vo1 = __PKHBT(v2, v1, 16);

#else

        vo1 = __PKHTB(v1, v2, 16);
        vo2 = __PKHBT(v2, v1, 16);

#endif

        in = *__SIMD32(pCnt);
        *__SIMD32(pCnt)++ = __QADD16(vo1, in);

        in = *__SIMD32(pCnt);
        *__SIMD32(pCnt)++ = __QADD16(vo2, in);

        cnt--;
    }
    cnt = length & 0x3;
    while (cnt > 0u)
    {
        *pCnt++ += *pV++;
        cnt--;
    }
}

#endif                          // ARM_MATH_DSP

/**
 *  @ingroup groupNN
 */

/**
 * @addtogroup Pooling
 * @{
 */

  /**
   * @brief Q7 max pooling function
   * @param[in, out]  Im_in       pointer to input tensor
   * @param[in]       dim_im_in   input tensor dimention
   * @param[in]       ch_im_in    number of input tensor channels
   * @param[in]       dim_kernel  filter kernel size
   * @param[in]       padding     padding sizes
   * @param[in]       stride      convolution stride
   * @param[in]       dim_im_out  output tensor dimension
   * @param[in,out]   bufferA     pointer to buffer space for input
   * @param[in,out]   Im_out      pointer to output tensor
   * @return none.
   *
   * @details
   *
   * <b>Buffer size:</b>
   *
   * bufferA size:  0
   *
   * The pooling function is implemented as split x-pooling then
   * y-pooling.
   *
   * This pooling function is input-destructive. Input data is undefined
   * after calling this function.
   *
   */

void
arm_maxpool_q7_HWC(q7_t * Im_in,
                   const uint16_t dim_im_in,
                   const uint16_t ch_im_in,
                   const uint16_t dim_kernel,
                   const uint16_t padding,
                   const uint16_t stride, const uint16_t dim_im_out, q7_t * bufferA, q7_t * Im_out)
{

#if defined (ARM_MATH_DSP)
    /* Run the following code for Cortex-M4 and Cortex-M7 */

    int16_t   i_x, i_y;

    /* first does the pooling along x axis */
    for (i_y = 0; i_y < dim_im_in; i_y++)
    {

        for (i_x = 0; i_x < dim_im_out; i_x++)
        {
            /* for each output pixel */
            q7_t     *target = Im_in + (i_y * dim_im_in + i_x) * ch_im_in;
            q7_t     *win_start;
            q7_t     *win_stop;
            if (i_x * stride - padding < 0)
            {
                win_start = target;
            } else
            {
                win_start = Im_in + (i_y * dim_im_in + i_x * stride - padding) * ch_im_in;
            }

            if (i_x * stride - padding + dim_kernel >= dim_im_in)
            {
                win_stop = Im_in + (i_y * dim_im_in + dim_im_in) * ch_im_in;
            } else
            {
                win_stop = Im_in + (i_y * dim_im_in + i_x * stride - padding + dim_kernel) * ch_im_in;
            }

            /* first step is to copy over initial data */
            /* arm_copy_q7(win_start, target, ch_im_in); */
            memmove(target, win_start, ch_im_in);

            /* start the max operation from the second part */
            win_start += ch_im_in;
            for (; win_start < win_stop; win_start += ch_im_in)
            {
                compare_and_replace_if_larger_q7(target, win_start, ch_im_in);
            }
        }
    }

    /* then does the pooling along y axis */
    for (i_y = 0; i_y < dim_im_out; i_y++)
    {

        /* for each output row */
        q7_t     *target = Im_out + i_y * dim_im_out * ch_im_in;
        q7_t     *row_start;
        q7_t     *row_end;
        /* setting the starting row */
        if (i_y * stride - padding < 0)
        {
            row_start = Im_in;
        } else
        {
            row_start = Im_in + (i_y * stride - padding) * dim_im_in * ch_im_in;
        }
        /* setting the stopping row */
        if (i_y * stride - padding + dim_kernel >= dim_im_in)
        {
            row_end = Im_in + dim_im_in * dim_im_in * ch_im_in;
        } else
        {
            row_end = Im_in + (i_y * stride - padding + dim_kernel) * dim_im_in * ch_im_in;
        }

        /* copy over the first row */
        /* arm_copy_q7(row_start, target, dim_im_out * ch_im_in); */
        memmove(target, row_start, dim_im_out * ch_im_in);

        /* move over to next row */
        row_start += ch_im_in * dim_im_in;

        for (; row_start < row_end; row_start += dim_im_in * ch_im_in)
        {
            compare_and_replace_if_larger_q7(target, row_start, dim_im_out * ch_im_in);
        }
    }

#else
    /* Run the following code as reference implementation for Cortex-M0 and Cortex-M3 */

    int16_t   i_ch_in, i_x, i_y;
    int16_t   k_x, k_y;

    for (i_ch_in = 0; i_ch_in < ch_im_in; i_ch_in++)
    {
        for (i_y = 0; i_y < dim_im_out; i_y++)
        {
            for (i_x = 0; i_x < dim_im_out; i_x++)
            {
                int       max = -129;
                for (k_y = i_y * stride - padding; k_y < i_y * stride - padding + dim_kernel; k_y++)
                {
                    for (k_x = i_x * stride - padding; k_x < i_x * stride - padding + dim_kernel; k_x++)
                    {
                        if (k_y >= 0 && k_x >= 0 && k_y < dim_im_in && k_x < dim_im_in)
                        {
                            if (Im_in[i_ch_in + ch_im_in * (k_x + k_y * dim_im_in)] > max)
                            {
                                max = Im_in[i_ch_in + ch_im_in * (k_x + k_y * dim_im_in)];
                            }
                        }
                    }
                }
                Im_out[i_ch_in + ch_im_in * (i_x + i_y * dim_im_out)] = max;
            }
        }
    }

#endif                          /* ARM_MATH_DSP */

}

  /**
   * @brief Q7 average pooling function
   * @param[in,out]   Im_in       pointer to input tensor
   * @param[in]       dim_im_in   input tensor dimention
   * @param[in]       ch_im_in    number of input tensor channels
   * @param[in]       dim_kernel  filter kernel size
   * @param[in]       padding     padding sizes
   * @param[in]       stride      convolution stride
   * @param[in]       dim_im_out  output tensor dimension
   * @param[in,out]   bufferA     pointer to buffer space for input
   * @param[in,out]   Im_out      pointer to output tensor
   * @return none.
   *
   * @details
   *
   * <b>Buffer size:</b>
   *
   * bufferA size:  2*dim_im_out*ch_im_in
   *
   * The pooling function is implemented as split x-pooling then
   * y-pooling.
   *
   * This pooling function is input-destructive. Input data is undefined
   * after calling this function.
   *
   */

void
arm_avepool_q7_HWC(q7_t * Im_in,
                   const uint16_t dim_im_in,
                   const uint16_t ch_im_in,
                   const uint16_t dim_kernel,
                   const uint16_t padding,
                   const uint16_t stride, const uint16_t dim_im_out, q7_t * bufferA, q7_t * Im_out)
{

#if defined (ARM_MATH_DSP)
    /* Run the following code for Cortex-M4 and Cortex-M7 */

    q15_t    *buffer = (q15_t *) bufferA;
    int16_t   i_x, i_y;
    int16_t   count = 0;

    /* first does the pooling along x axis */
    for (i_y = 0; i_y < dim_im_in; i_y++)
    {

        for (i_x = 0; i_x < dim_im_out; i_x++)
        {
            /* for each output pixel */
            q7_t     *target = Im_in + (i_y * dim_im_in + i_x) * ch_im_in;
            q7_t     *win_start;
            q7_t     *win_stop;
            if (i_x * stride - padding < 0)
            {
                win_start = target;
            } else
            {
                win_start = Im_in + (i_y * dim_im_in + i_x * stride - padding) * ch_im_in;
            }

            if (i_x * stride - padding + dim_kernel >= dim_im_in)
            {
                win_stop = Im_in + (i_y * dim_im_in + dim_im_in) * ch_im_in;
            } else
            {
                win_stop = Im_in + (i_y * dim_im_in + i_x * stride - padding + dim_kernel) * ch_im_in;
            }

            /* first step is to copy over initial data */
            arm_q7_to_q15_no_shift(win_start, buffer, ch_im_in);
            count = 1;

            /* start the max operation from the second part */
            win_start += ch_im_in;
            for (; win_start < win_stop; win_start += ch_im_in)
            {
                accumulate_q7_to_q15(buffer, win_start, ch_im_in);
                count++;
            }
            buffer_scale_back_q15_to_q7(buffer, target, ch_im_in, count);
        }
    }

    /* then does the pooling along y axis */
    for (i_y = 0; i_y < dim_im_out; i_y++)
    {
        /* for each output row */
        q7_t     *target = Im_out + i_y * dim_im_out * ch_im_in;
        q7_t     *row_start;
        q7_t     *row_end;
        /* setting the starting row */
        if (i_y * stride - padding < 0)
        {
            row_start = Im_in;
        } else
        {
            row_start = Im_in + (i_y * stride - padding) * dim_im_in * ch_im_in;
        }
        /* setting the stopping row */
        if (i_y * stride - padding + dim_kernel >= dim_im_in)
        {
            row_end = Im_in + dim_im_in * dim_im_in * ch_im_in;
        } else
        {
            row_end = Im_in + (i_y * stride - padding + dim_kernel) * dim_im_in * ch_im_in;
        }

        /* copy over the first row */
        arm_q7_to_q15_no_shift(row_start, buffer, dim_im_out * ch_im_in);
        count = 1;

        /* move over to next row */
        row_start += ch_im_in * dim_im_in;

        for (; row_start < row_end; row_start += dim_im_in * ch_im_in)
        {
            accumulate_q7_to_q15(buffer, row_start, dim_im_out * ch_im_in);
            count++;
        }
        buffer_scale_back_q15_to_q7(buffer, target, dim_im_out * ch_im_in, count);
    }

#else
    /* Run the following code as reference implementation for Cortex-M0 and Cortex-M3 */

    int16_t   i_ch_in, i_x, i_y;
    int16_t   k_x, k_y;

    for (i_ch_in = 0; i_ch_in < ch_im_in; i_ch_in++)
    {
        for (i_y = 0; i_y < dim_im_out; i_y++)
        {
            for (i_x = 0; i_x < dim_im_out; i_x++)
            {
                int       sum = 0;
                int       count = 0;
                for (k_y = i_y * stride - padding; k_y < i_y * stride - padding + dim_kernel; k_y++)
                {
                    for (k_x = i_x * stride - padding; k_x < i_x * stride - padding + dim_kernel; k_x++)
                    {
                        if (k_y >= 0 && k_x >= 0 && k_y < dim_im_in && k_x < dim_im_in)
                        {
                            sum += Im_in[i_ch_in + ch_im_in * (k_x + k_y * dim_im_in)];
                            count++;
                        }
                    }
                }
                Im_out[i_ch_in + ch_im_in * (i_x + i_y * dim_im_out)] = sum / count;
            }
        }
    }

#endif                          /* ARM_MATH_DSP */

}

/**
 * @} end of Pooling group
 */
