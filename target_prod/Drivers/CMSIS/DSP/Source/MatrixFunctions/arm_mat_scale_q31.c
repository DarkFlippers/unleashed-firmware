/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mat_scale_q31.c
 * Description:  Multiplies a Q31 matrix by a scalar
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
 * @addtogroup MatrixScale
 * @{
 */

/**
 * @brief Q31 matrix scaling.
 * @param[in]       *pSrc points to input matrix
 * @param[in]       scaleFract fractional portion of the scale factor
 * @param[in]       shift number of bits to shift the result by
 * @param[out]      *pDst points to output matrix structure
 * @return     		The function returns either
 * <code>ARM_MATH_SIZE_MISMATCH</code> or <code>ARM_MATH_SUCCESS</code> based on the outcome of size checking.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The input data <code>*pSrc</code> and <code>scaleFract</code> are in 1.31 format.
 * These are multiplied to yield a 2.62 intermediate result and this is shifted with saturation to 1.31 format.
 */

arm_status arm_mat_scale_q31(
  const arm_matrix_instance_q31 * pSrc,
  q31_t scaleFract,
  int32_t shift,
  arm_matrix_instance_q31 * pDst)
{
  q31_t *pIn = pSrc->pData;                      /* input data matrix pointer */
  q31_t *pOut = pDst->pData;                     /* output data matrix pointer */
  uint32_t numSamples;                           /* total number of elements in the matrix */
  int32_t totShift = shift + 1;                  /* shift to apply after scaling */
  uint32_t blkCnt;                               /* loop counters  */
  arm_status status;                             /* status of matrix scaling      */
  q31_t in1, in2, out1;                          /* temporary variabels */

#if defined (ARM_MATH_DSP)

  q31_t in3, in4, out2, out3, out4;              /* temporary variables */

#endif //      #ifndef ARM_MAT_CM0

#ifdef ARM_MATH_MATRIX_CHECK
  /* Check for matrix mismatch  */
  if ((pSrc->numRows != pDst->numRows) || (pSrc->numCols != pDst->numCols))
  {
    /* Set status as ARM_MATH_SIZE_MISMATCH */
    status = ARM_MATH_SIZE_MISMATCH;
  }
  else
#endif //    #ifdef ARM_MATH_MATRIX_CHECK
  {
    /* Total number of samples in the input matrix */
    numSamples = (uint32_t) pSrc->numRows * pSrc->numCols;

#if defined (ARM_MATH_DSP)

    /* Run the below code for Cortex-M4 and Cortex-M3 */

    /* Loop Unrolling */
    blkCnt = numSamples >> 2U;

    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U)
    {
      /* C(m,n) = A(m,n) * k */
      /* Read values from input */
      in1 = *pIn;
      in2 = *(pIn + 1);
      in3 = *(pIn + 2);
      in4 = *(pIn + 3);

      /* multiply input with scaler value */
      in1 = ((q63_t) in1 * scaleFract) >> 32;
      in2 = ((q63_t) in2 * scaleFract) >> 32;
      in3 = ((q63_t) in3 * scaleFract) >> 32;
      in4 = ((q63_t) in4 * scaleFract) >> 32;

      /* apply shifting */
      out1 = in1 << totShift;
      out2 = in2 << totShift;

      /* saturate the results. */
      if (in1 != (out1 >> totShift))
        out1 = 0x7FFFFFFF ^ (in1 >> 31);

      if (in2 != (out2 >> totShift))
        out2 = 0x7FFFFFFF ^ (in2 >> 31);

      out3 = in3 << totShift;
      out4 = in4 << totShift;

      *pOut = out1;
      *(pOut + 1) = out2;

      if (in3 != (out3 >> totShift))
        out3 = 0x7FFFFFFF ^ (in3 >> 31);

      if (in4 != (out4 >> totShift))
        out4 = 0x7FFFFFFF ^ (in4 >> 31);


      *(pOut + 2) = out3;
      *(pOut + 3) = out4;

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
      /* C(m,n) = A(m,n) * k */
      /* Scale, saturate and then store the results in the destination buffer. */
      in1 = *pIn++;

      in2 = ((q63_t) in1 * scaleFract) >> 32;

      out1 = in2 << totShift;

      if (in2 != (out1 >> totShift))
        out1 = 0x7FFFFFFF ^ (in2 >> 31);

      *pOut++ = out1;

      /* Decrement the numSamples loop counter */
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
