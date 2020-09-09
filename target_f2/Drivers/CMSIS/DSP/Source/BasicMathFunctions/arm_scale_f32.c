/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_scale_f32.c
 * Description:  Multiplies a floating-point vector by a scalar
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
 * @ingroup groupMath
 */

/**
 * @defgroup scale Vector Scale
 *
 * Multiply a vector by a scalar value.  For floating-point data, the algorithm used is:
 *
 * <pre>
 *     pDst[n] = pSrc[n] * scale,   0 <= n < blockSize.
 * </pre>
 *
 * In the fixed-point Q7, Q15, and Q31 functions, <code>scale</code> is represented by
 * a fractional multiplication <code>scaleFract</code> and an arithmetic shift <code>shift</code>.
 * The shift allows the gain of the scaling operation to exceed 1.0.
 * The algorithm used with fixed-point data is:
 *
 * <pre>
 *     pDst[n] = (pSrc[n] * scaleFract) << shift,   0 <= n < blockSize.
 * </pre>
 *
 * The overall scale factor applied to the fixed-point data is
 * <pre>
 *     scale = scaleFract * 2^shift.
 * </pre>
 *
 * The functions support in-place computation allowing the source and destination
 * pointers to reference the same memory buffer.
 */

/**
 * @addtogroup scale
 * @{
 */

/**
 * @brief Multiplies a floating-point vector by a scalar.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       scale scale factor to be applied
 * @param[out]      *pDst points to the output vector
 * @param[in]       blockSize number of samples in the vector
 * @return none.
 */


void arm_scale_f32(
  float32_t * pSrc,
  float32_t scale,
  float32_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */
  float32_t in1, in2, in3, in4;                  /* temporary variabels */

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = A * scale */
    /* Scale the input and then store the results in the destination buffer. */
    /* read input samples from source */
    in1 = *pSrc;
    in2 = *(pSrc + 1);

    /* multiply with scaling factor */
    in1 = in1 * scale;

    /* read input sample from source */
    in3 = *(pSrc + 2);

    /* multiply with scaling factor */
    in2 = in2 * scale;

    /* read input sample from source */
    in4 = *(pSrc + 3);

    /* multiply with scaling factor */
    in3 = in3 * scale;
    in4 = in4 * scale;
    /* store the result to destination */
    *pDst = in1;
    *(pDst + 1) = in2;
    *(pDst + 2) = in3;
    *(pDst + 3) = in4;

    /* update pointers to process next samples */
    pSrc += 4U;
    pDst += 4U;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

#else

  /* Run the below code for Cortex-M0 */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

#endif /* #if defined (ARM_MATH_DSP) */

  while (blkCnt > 0U)
  {
    /* C = A * scale */
    /* Scale the input and then store the result in the destination buffer. */
    *pDst++ = (*pSrc++) * scale;

    /* Decrement the loop counter */
    blkCnt--;
  }
}

/**
 * @} end of scale group
 */
