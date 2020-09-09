/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_scale_q31.c
 * Description:  Multiplies a Q31 vector by a scalar
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
 * @addtogroup scale
 * @{
 */

/**
 * @brief Multiplies a Q31 vector by a scalar.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       scaleFract fractional portion of the scale value
 * @param[in]       shift number of bits to shift the result by
 * @param[out]      *pDst points to the output vector
 * @param[in]       blockSize number of samples in the vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The input data <code>*pSrc</code> and <code>scaleFract</code> are in 1.31 format.
 * These are multiplied to yield a 2.62 intermediate result and this is shifted with saturation to 1.31 format.
 */

void arm_scale_q31(
  q31_t * pSrc,
  q31_t scaleFract,
  int8_t shift,
  q31_t * pDst,
  uint32_t blockSize)
{
  int8_t kShift = shift + 1;                     /* Shift to apply after scaling */
  int8_t sign = (kShift & 0x80);
  uint32_t blkCnt;                               /* loop counter */
  q31_t in, out;

#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */

  q31_t in1, in2, in3, in4;                      /* temporary input variables */
  q31_t out1, out2, out3, out4;                  /* temporary output variabels */


  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  if (sign == 0U)
  {
    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U)
    {
      /* read four inputs from source */
      in1 = *pSrc;
      in2 = *(pSrc + 1);
      in3 = *(pSrc + 2);
      in4 = *(pSrc + 3);

      /* multiply input with scaler value */
      in1 = ((q63_t) in1 * scaleFract) >> 32;
      in2 = ((q63_t) in2 * scaleFract) >> 32;
      in3 = ((q63_t) in3 * scaleFract) >> 32;
      in4 = ((q63_t) in4 * scaleFract) >> 32;

      /* apply shifting */
      out1 = in1 << kShift;
      out2 = in2 << kShift;

      /* saturate the results. */
      if (in1 != (out1 >> kShift))
        out1 = 0x7FFFFFFF ^ (in1 >> 31);

      if (in2 != (out2 >> kShift))
        out2 = 0x7FFFFFFF ^ (in2 >> 31);

      out3 = in3 << kShift;
      out4 = in4 << kShift;

      *pDst = out1;
      *(pDst + 1) = out2;

      if (in3 != (out3 >> kShift))
        out3 = 0x7FFFFFFF ^ (in3 >> 31);

      if (in4 != (out4 >> kShift))
        out4 = 0x7FFFFFFF ^ (in4 >> 31);

      /* Store result destination */
      *(pDst + 2) = out3;
      *(pDst + 3) = out4;

      /* Update pointers to process next sampels */
      pSrc += 4U;
      pDst += 4U;

      /* Decrement the loop counter */
      blkCnt--;
    }

  }
  else
  {
    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U)
    {
      /* read four inputs from source */
      in1 = *pSrc;
      in2 = *(pSrc + 1);
      in3 = *(pSrc + 2);
      in4 = *(pSrc + 3);

      /* multiply input with scaler value */
      in1 = ((q63_t) in1 * scaleFract) >> 32;
      in2 = ((q63_t) in2 * scaleFract) >> 32;
      in3 = ((q63_t) in3 * scaleFract) >> 32;
      in4 = ((q63_t) in4 * scaleFract) >> 32;

      /* apply shifting */
      out1 = in1 >> -kShift;
      out2 = in2 >> -kShift;

      out3 = in3 >> -kShift;
      out4 = in4 >> -kShift;

      /* Store result destination */
      *pDst = out1;
      *(pDst + 1) = out2;

      *(pDst + 2) = out3;
      *(pDst + 3) = out4;

      /* Update pointers to process next sampels */
      pSrc += 4U;
      pDst += 4U;

      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

#else

  /* Run the below code for Cortex-M0 */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

#endif /* #if defined (ARM_MATH_DSP) */

  if (sign == 0)
  {
	  while (blkCnt > 0U)
	  {
		/* C = A * scale */
		/* Scale the input and then store the result in the destination buffer. */
		in = *pSrc++;
		in = ((q63_t) in * scaleFract) >> 32;

		out = in << kShift;

		if (in != (out >> kShift))
			out = 0x7FFFFFFF ^ (in >> 31);

		*pDst++ = out;

		/* Decrement the loop counter */
		blkCnt--;
	  }
  }
  else
  {
	  while (blkCnt > 0U)
	  {
		/* C = A * scale */
		/* Scale the input and then store the result in the destination buffer. */
		in = *pSrc++;
		in = ((q63_t) in * scaleFract) >> 32;

		out = in >> -kShift;

		*pDst++ = out;

		/* Decrement the loop counter */
		blkCnt--;
	  }

  }
}

/**
 * @} end of scale group
 */
