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

#include "arm_math.h"
#include "arm_nnfunctions.h"

void      arm_nn_mult_q7_ref(q7_t * pSrcA, 
                             q7_t * pSrcB, 
                             q7_t * pDst, 
                             const uint16_t out_shift, 
                             uint32_t blockSize) {
    uint16_t  i;

for (i = 0; i < blockSize; i++)
    {
		q31_t product = pSrcA[i] * pSrcB[i];
#ifndef ARM_NN_TRUNCATE
        pDst[i] = (q7_t)__SSAT((product + (0x1 << (out_shift - 1)))>>out_shift, 8);
#else
        pDst[i] = (q7_t)__SSAT(product >> out_shift, 8);
#endif
    }
}

void     arm_nn_mult_q15_ref(q15_t * pSrcA, 
                             q15_t * pSrcB, 
                             q15_t * pDst, 
                             const uint16_t out_shift, 
                             uint32_t blockSize) {
    uint16_t  i;

for (i = 0; i < blockSize; i++)
    {
		q31_t product = pSrcA[i] * pSrcB[i];
#ifndef ARM_NN_TRUNCATE
        pDst[i] = (q15_t)__SSAT((product + (0x1 << (out_shift - 1)))>>out_shift, 16);
#else
        pDst[i] = (q15_t)__SSAT(product >> out_shift, 16);
#endif


    }
}
