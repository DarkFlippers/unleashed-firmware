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
 * Title:        arm_relu_q15.c
 * Description:  Q15 version of ReLU
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
 * @addtogroup Acti
 * @{
 */

  /**
   * @brief Q15 RELU function
   * @param[in,out]   data        pointer to input
   * @param[in]       size        number of elements
   * @return none.
   * 
   * @details
   *
   * Optimized relu with QSUB instructions.
   *
   */

void arm_relu_q15(q15_t * data, uint16_t size)
{

#if defined (ARM_MATH_DSP)
    /* Run the following code for Cortex-M4 and Cortex-M7 */

    uint16_t  i = size >> 1;
    q15_t    *pIn = data;
    q15_t    *pOut = data;
    q31_t     in;
    q31_t     buf;
    q31_t     mask;

    while (i)
    {
        in = *__SIMD32(pIn)++;

        /* extract the first bit */
        buf = __ROR(in & 0x80008000, 15);

        /* if MSB=1, mask will be 0xFF, 0x0 otherwise */
        mask = __QSUB16(0x00000000, buf);

        *__SIMD32(pOut)++ = in & (~mask);
        i--;
    }

    if (size & 0x1)
    {
        if (*pIn < 0)
        {
            *pIn = 0;
        }
        pIn++;
    }
#else
    /* Run the following code as reference implementation for Cortex-M0 and Cortex-M3 */
    uint16_t  i;

    for (i = 0; i < size; i++)
    {
        if (data[i] < 0)
            data[i] = 0;
    }

#endif                          /* ARM_MATH_DSP */

}

/**
 * @} end of Acti group
 */
