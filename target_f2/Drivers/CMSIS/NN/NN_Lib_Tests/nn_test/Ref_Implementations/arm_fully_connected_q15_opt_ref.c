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

void arm_fully_connected_q15_opt_ref(const q15_t * pV,  // pointer to vector
                                     const q15_t * pM,  // pointer to matrix
                                     const uint16_t dim_vec,    // length of the vector
                                     const uint16_t num_of_rows,    // numCol of A
                                     const uint16_t bias_shift, // amount of left-shift for bias
                                     const uint16_t out_shift,  // amount of right-shift for output
                                     const q15_t * bias, q15_t * pOut,  // output operand
                                     q15_t * vec_buffer)
{

    uint16_t  rowCnt = num_of_rows >> 2;
    const q15_t *pB = pM;
    const q15_t *pA;
    q15_t    *pO = pOut;
    const q15_t *pBias = bias;

    while (rowCnt)
    {
        pA = pV;
#ifndef ARM_NN_TRUNCATE
        q31_t     sum = (*pBias++ << bias_shift) + (0x1 << (out_shift - 1));
        q31_t     sum2 = (*pBias++ << bias_shift) + (0x1 << (out_shift - 1));
        q31_t     sum3 = (*pBias++ << bias_shift) + (0x1 << (out_shift - 1));
        q31_t     sum4 = (*pBias++ << bias_shift) + (0x1 << (out_shift - 1));
#else
        q31_t     sum = *pBias++ << bias_shift;
        q31_t     sum2 = *pBias++ << bias_shift;
        q31_t     sum3 = *pBias++ << bias_shift;
        q31_t     sum4 = *pBias++ << bias_shift;
#endif

        uint16_t  colCnt = dim_vec >> 1;

        while (colCnt)
        {
            q15_t     inA1 = *pA++;
            q15_t     inA2 = *pA++;

            q15_t     inB1 = *pB++;
            q15_t     inB2 = *pB++;
            sum += inA1 * inB1 + inA2 * inB2;

            inB1 = *pB++;
            inB2 = *pB++;
            sum2 += inA1 * inB1 + inA2 * inB2;

            inB1 = *pB++;
            inB2 = *pB++;
            sum3 += inA1 * inB1 + inA2 * inB2;

            inB1 = *pB++;
            inB2 = *pB++;
            sum4 += inA1 * inB1 + inA2 * inB2;

            colCnt--;
        }
        colCnt = dim_vec & 0x1;
        while (colCnt)
        {
            q15_t     inA = *pA++;
            q15_t     inB = *pB++;
            sum += inA * inB;
            inB = *pB++;
            sum2 += inA * inB;
            inB = *pB++;
            sum3 += inA * inB;
            inB = *pB++;
            sum4 += inA * inB;
            colCnt--;
        }
        *pO++ = (q15_t) __SSAT((sum >> out_shift), 16);
        *pO++ = (q15_t) __SSAT((sum2 >> out_shift), 16);
        *pO++ = (q15_t) __SSAT((sum3 >> out_shift), 16);
        *pO++ = (q15_t) __SSAT((sum4 >> out_shift), 16);

        rowCnt--;
    }

    rowCnt = num_of_rows & 0x3;

    while (rowCnt)
    {
        pA = pV;
#ifndef ARM_NN_TRUNCATE
        int       ip_out = (*pBias++ << bias_shift) + (0x1 << (out_shift - 1));
#else
        int       ip_out = *pBias++ << bias_shift;
#endif
        for (int j = 0; j < dim_vec; j++)
        {
            q15_t     inA = *pA++;
            q15_t     inB = *pB++;
            ip_out += inA * inB;
        }
        *pO++ = (q15_t) __SSAT((ip_out >> out_shift), 16);

        rowCnt--;
    }
}
