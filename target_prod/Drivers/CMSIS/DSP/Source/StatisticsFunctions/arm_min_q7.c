/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_min_q7.c
 * Description:  Minimum value of a Q7 vector
 *
 * $Date:        27. January 2017
 * $Revision:    V.1.5.1
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2017 ARM Limited or its affiliates. All rights reserved.
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

/**
 * @ingroup groupStats
 */


/**
 * @addtogroup Min
 * @{
 */


/**
 * @brief Minimum value of a Q7 vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult minimum value returned here
 * @param[out]      *pIndex index of minimum value returned here
 * @return none.
 */

void arm_min_q7(
  q7_t * pSrc,
  uint32_t blockSize,
  q7_t * pResult,
  uint32_t * pIndex)
{
#if defined (ARM_MATH_DSP)
  /* Run the below code for Cortex-M4 and Cortex-M3 */

  q7_t minVal1, minVal2, out;                    /* Temporary variables to store the output value. */
  uint32_t blkCnt, outIndex, count;              /* loop counter */

  /* Initialise the count value. */
  count = 0U;
  /* Initialise the index value to zero. */
  outIndex = 0U;
  /* Load first input value that act as reference value for comparision */
  out = *pSrc++;

  /* Loop unrolling */
  blkCnt = (blockSize - 1U) >> 2U;

  while (blkCnt > 0U)
  {
    /* Initialize minVal to the next consecutive values one by one */
    minVal1 = *pSrc++;
    minVal2 = *pSrc++;

    /* compare for the minimum value */
    if (out > minVal1)
    {
      /* Update the minimum value and its index */
      out = minVal1;
      outIndex = count + 1U;
    }

    /* compare for the minimum value */
    if (out > minVal2)
    {
      /* Update the minimum value and its index */
      out = minVal2;
      outIndex = count + 2U;
    }

    /* Initialize minVal to the next consecutive values one by one */
    minVal1 = *pSrc++;
    minVal2 = *pSrc++;

    /* compare for the minimum value */
    if (out > minVal1)
    {
      /* Update the minimum value and its index */
      out = minVal1;
      outIndex = count + 3U;
    }

    /* compare for the minimum value */
    if (out > minVal2)
    {
      /* Update the minimum value and its index */
      out = minVal2;
      outIndex = count + 4U;
    }

    count += 4U;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* if (blockSize - 1U) is not multiple of 4 */
  blkCnt = (blockSize - 1U) % 4U;

#else
  /* Run the below code for Cortex-M0 */

  q7_t minVal1, out;                             /* Temporary variables to store the output value. */
  uint32_t blkCnt, outIndex;                     /* loop counter */

  /* Initialise the index value to zero. */
  outIndex = 0U;
  /* Load first input value that act as reference value for comparision */
  out = *pSrc++;

  blkCnt = (blockSize - 1U);

#endif /* #if defined (ARM_MATH_DSP) */

  while (blkCnt > 0U)
  {
    /* Initialize minVal to the next consecutive values one by one */
    minVal1 = *pSrc++;

    /* compare for the minimum value */
    if (out > minVal1)
    {
      /* Update the minimum value and it's index */
      out = minVal1;
      outIndex = blockSize - blkCnt;
    }

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Store the minimum value and it's index into destination pointers */
  *pResult = out;
  *pIndex = outIndex;
}

/**
 * @} end of Min group
 */
