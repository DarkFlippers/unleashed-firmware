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

void arm_fully_connected_q7_opt_ref(const q7_t * pV,    // pointer to vector
                                    const q7_t * pM,    // pointer to matrix
                                    const uint16_t dim_vec, // length of the vector
                                    const uint16_t num_of_rows, // numCol of A
                                    const uint16_t bias_shift,  // amount of left-shift for bias
                                    const uint16_t out_shift,   // amount of right-shift for output
                                    const q7_t * bias, q7_t * pOut, // output operand
                                    q15_t * vec_buffer)
{

    uint16_t  rowCnt = num_of_rows >> 2;
    const q7_t *pB = pM;
    const q7_t *pA;
    q7_t     *pO = pOut;
    const q7_t *pBias = bias;

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

        uint16_t  colCnt = dim_vec >> 2;

        while (colCnt)
        {
            q7_t      inA1 = *pA++;
            q7_t      inA3 = *pA++;
            q7_t      inA2 = *pA++;
            q7_t      inA4 = *pA++;

            q7_t      inB1 = *pB++;
            q7_t      inB3 = *pB++;
            q7_t      inB2 = *pB++;
            q7_t      inB4 = *pB++;

            sum += inA1 * inB1 + inA2 * inB2;
            sum2 += inA1 * inB3 + inA2 * inB4;

            inB1 = *pB++;
            inB3 = *pB++;
            inB2 = *pB++;
            inB4 = *pB++;

            sum3 += inA1 * inB1 + inA2 * inB2;
            sum4 += inA1 * inB3 + inA2 * inB4;

            inB1 = *pB++;
            inB3 = *pB++;
            inB2 = *pB++;
            inB4 = *pB++;

            sum += inA3 * inB1 + inA4 * inB2;
            sum2 += inA3 * inB3 + inA4 * inB4;

            inB1 = *pB++;
            inB3 = *pB++;
            inB2 = *pB++;
            inB4 = *pB++;

            sum3 += inA3 * inB1 + inA4 * inB2;
            sum4 += inA3 * inB3 + inA4 * inB4;

            colCnt--;
        }
        colCnt = dim_vec & 0x3;
        while (colCnt)
        {
            q7_t      inA = *pA++;
            q7_t      inB = *pB++;
            sum += inA * inB;
            inB = *pB++;
            sum2 += inA * inB;
            inB = *pB++;
            sum3 += inA * inB;
            inB = *pB++;
            sum4 += inA * inB;

            colCnt--;
        }
        *pO++ = (q7_t) __SSAT((sum >> out_shift), 8);
        *pO++ = (q7_t) __SSAT((sum2 >> out_shift), 8);
        *pO++ = (q7_t) __SSAT((sum3 >> out_shift), 8);
        *pO++ = (q7_t) __SSAT((sum4 >> out_shift), 8);

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
            q7_t      inA = *pA++;
            q7_t      inB = *pB++;
            ip_out += inA * inB;
        }
        *pO++ = (q7_t) __SSAT((ip_out >> out_shift), 8);

        rowCnt--;
    }
}
