/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mat_scale_f32.c
 * Description:  Multiplies a floating-point matrix by a scalar
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
 * @ingroup groupMatrix
 */

/**
 * @defgroup MatrixScale Matrix Scale
 *
 * Multiplies a matrix by a scalar.  This is accomplished by multiplying each element in the
 * matrix by the scalar.  For example:
 * \image html MatrixScale.gif "Matrix Scaling of a 3 x 3 matrix"
 *
 * The function checks to make sure that the input and output matrices are of the same size.
 *
 * In the fixed-point Q15 and Q31 functions, <code>scale</code> is represented by
 * a fractional multiplication <code>scaleFract</code> and an arithmetic shift <code>shift</code>.
 * The shift allows the gain of the scaling operation to exceed 1.0.
 * The overall scale factor applied to the fixed-point data is
 * <pre>
 *     scale = scaleFract * 2^shift.
 * </pre>
 */

/**
 * @addtogroup MatrixScale
 * @{
 */

/**
 * @brief Floating-point matrix scaling.
 * @param[in]       *pSrc points to input matrix structure
 * @param[in]       scale scale factor to be applied
 * @param[out]      *pDst points to output matrix structure
 * @return     		The function returns either <code>ARM_MATH_SIZE_MISMATCH</code>
 * or <code>ARM_MATH_SUCCESS</code> based on the outcome of size checking.
 *
 */

arm_status arm_mat_scale_f32(
  const arm_matrix_instance_f32 * pSrc,
  float32_t scale,
  arm_matrix_instance_f32 * pDst)
{
  float32_t *pIn = pSrc->pData;                  /* input data matrix pointer */
  float32_t *pOut = pDst->pData;                 /* output data matrix pointer */
  uint32_t numSamples;                           /* total number of elements in the matrix */
  uint32_t blkCnt;                               /* loop counters */
  arm_status status;                             /* status of matrix scaling     */

#if defined (ARM_MATH_DSP)

  float32_t in1, in2, in3, in4;                  /* temporary variables */
  float32_t out1, out2, out3, out4;              /* temporary variables */

#endif //      #if defined (ARM_MATH_DSP)

#ifdef ARM_MATH_MATRIX_CHECK
  /* Check for matrix mismatch condition */
  if ((pSrc->numRows != pDst->numRows) || (pSrc->numCols != pDst->numCols))
  {
    /* Set status as ARM_MATH_SIZE_MISMATCH */
    status = ARM_MATH_SIZE_MISMATCH;
  }
  else
#endif /*    #ifdef ARM_MATH_MATRIX_CHECK    */
  {
    /* Total number of samples in the input matrix */
    numSamples = (uint32_t) pSrc->numRows * pSrc->numCols;

#if defined (ARM_MATH_DSP)

    /* Run the below code for Cortex-M4 and Cortex-M3 */

    /* Loop Unrolling */
    blkCnt = numSamples >> 2;

    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U)
    {
      /* C(m,n) = A(m,n) * scale */
      /* Scaling and results are stored in the destination buffer. */
      in1 = pIn[0];
      in2 = pIn[1];
      in3 = pIn[2];
      in4 = pIn[3];

      out1 = in1 * scale;
      out2 = in2 * scale;
      out3 = in3 * scale;
      out4 = in4 * scale;


      pOut[0] = out1;
      pOut[1] = out2;
      pOut[2] = out3;
      pOut[3] = out4;

      /* update pointers to process next sampels */
      pIn += 4U;
      pOut += 4U;

      /* Decrement the numSamples loop counter */
      blkCnt--;
    }

    /* If the numSamples is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = numSamples % 0x4U;

#else

    /* Run the below code for Cortex-M0 */

    /* Initialize blkCnt with number of samples */
    blkCnt = numSamples;

#endif /* #if defined (ARM_MATH_DSP) */

    while (blkCnt > 0U)
    {
      /* C(m,n) = A(m,n) * scale */
      /* The results are stored in the destination buffer. */
      *pOut++ = (*pIn++) * scale;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* Set status as ARM_MATH_SUCCESS */
    status = ARM_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);
}

/**
 * @} end of MatrixScale group
 */
