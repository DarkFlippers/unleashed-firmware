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
 * Title:        arm_softmax_q15.c
 * Description:  Q15 softmax function
 *
 * $Date:        20. February 2018
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
 * @addtogroup Softmax
 * @{
 */

  /**
   * @brief Q15 softmax function
   * @param[in]       vec_in      pointer to input vector
   * @param[in]       dim_vec     input vector dimention
   * @param[out]      p_out       pointer to output vector
   * @return none.
   *
   * @details
   *
   *  Here, instead of typical e based softmax, we use
   *  2-based softmax, i.e.,:
   *
   *  y_i = 2^(x_i) / sum(2^x_j)
   *
   *  The relative output will be different here.
   *  But mathematically, the gradient will be the same
   *  with a log(2) scaling factor.
   *
   */

void arm_softmax_q15(const q15_t * vec_in, const uint16_t dim_vec, q15_t * p_out)
{
    q31_t     sum;
    int16_t   i;
    uint8_t   shift;
    q31_t     base;
    base = -1 * 0x100000;
    for (i = 0; i < dim_vec; i++)
    {
        if (vec_in[i] > base)
        {
            base = vec_in[i];
        }
    }

    /* we ignore really small values  
     * anyway, they will be 0 after shrinking
     * to q15_t
     */
    base = base - 16;

    sum = 0;

    for (i = 0; i < dim_vec; i++)
    {
        if (vec_in[i] > base)
        {
            shift = (uint8_t)__USAT(vec_in[i] - base, 5);
            sum += 0x1 << shift;
        }
    }

    /* This is effectively (0x1 << 32) / sum */
    int64_t div_base = 0x100000000LL;
    int output_base = (int32_t)(div_base / sum);

    /* Final confidence will be output_base >> ( 17 - (vec_in[i] - base) )
     * so 32768 (0x1<<15) -> 100% confidence when sum = 0x1 << 16, output_base = 0x1 << 16
     * and vec_in[i]-base = 16
     */
    for (i = 0; i < dim_vec; i++)
    {
        if (vec_in[i] > base) 
        {
            /* Here minimum value of 17+base-vec[i] will be 1 */
            shift = (uint8_t)__USAT(17+base-vec_in[i], 5);
            p_out[i] = (q15_t) __SSAT((output_base >> shift), 16);
        } else
        {
            p_out[i] = 0;
        }
    }

}

/**
 * @} end of Softmax group
 */
