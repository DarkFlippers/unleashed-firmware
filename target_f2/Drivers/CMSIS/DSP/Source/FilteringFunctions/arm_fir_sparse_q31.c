/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fir_sparse_q31.c
 * Description:  Q31 sparse FIR filter processing function
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
 * @addtogroup FIR_Sparse
 * @{
 */

/**
 * @brief Processing function for the Q31 sparse FIR filter.
 * @param[in]  *S          points to an instance of the Q31 sparse FIR structure.
 * @param[in]  *pSrc       points to the block of input data.
 * @param[out] *pDst       points to the block of output data
 * @param[in]  *pScratchIn points to a temporary buffer of size blockSize.
 * @param[in]  blockSize   number of input samples to process per call.
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using an internal 32-bit accumulator.
 * The 1.31 x 1.31 multiplications are truncated to 2.30 format.
 * This leads to loss of precision on the intermediate multiplications and provides only a single guard bit.
 * If the accumulator result overflows, it wraps around rather than saturate.
 * In order to avoid overflows the input signal or coefficients must be scaled down by log2(numTaps) bits.
 */

void arm_fir_sparse_q31(
  arm_fir_sparse_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  q31_t * pScratchIn,
  uint32_t blockSize)
{

  q31_t *pState = S->pState;                     /* State pointer */
  q31_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q31_t *px;                                     /* Scratch buffer pointer */
  q31_t *py = pState;                            /* Temporary pointers for state buffer */
  q31_t *pb = pScratchIn;                        /* Temporary pointers for scratch buffer */
  q31_t *pOut;                                   /* Destination pointer */
  q63_t out;                                     /* Temporary output variable */
  int32_t *pTapDelay = S->pTapDelay;             /* Pointer to the array containing offset of the non-zero tap values. */
  uint32_t delaySize = S->maxDelay + blockSize;  /* state length */
  uint16_t numTaps = S->numTaps;                 /* Filter order */
  int32_t readIndex;                             /* Read index of the state buffer */
  uint32_t tapCnt, blkCnt;                       /* loop counters */
  q31_t coeff = *pCoeffs++;                      /* Read the first coefficient value */
  q31_t in;


  /* BlockSize of Input samples are copied into the state buffer */
  /* StateIndex points to the starting position to write in the state buffer */
  arm_circularWrite_f32((int32_t *) py, delaySize, &S->stateIndex, 1,
                        (int32_t *) pSrc, 1, blockSize);

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = (int32_t) (S->stateIndex - blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if (readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Working pointer for state buffer is updated */
  py = pState;

  /* blockSize samples are read from the state buffer */
  arm_circularRead_f32((int32_t *) py, delaySize, &readIndex, 1,
                       (int32_t *) pb, (int32_t *) pb, blockSize, 1,
                       blockSize);

  /* Working pointer for the scratch buffer of state values */
  px = pb;

  /* Working pointer for scratch buffer of output values */
  pOut = pDst;


#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  /* Loop over the blockSize. Unroll by a factor of 4.
   * Compute 4 Multiplications at a time. */
  blkCnt = blockSize >> 2;

  while (blkCnt > 0U)
  {
    /* Perform Multiplications and store in the destination buffer */
    *pOut++ = (q31_t) (((q63_t) * px++ * coeff) >> 32);
    *pOut++ = (q31_t) (((q63_t) * px++ * coeff) >> 32);
    *pOut++ = (q31_t) (((q63_t) * px++ * coeff) >> 32);
    *pOut++ = (q31_t) (((q63_t) * px++ * coeff) >> 32);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4,
   * compute the remaining samples */
  blkCnt = blockSize % 0x4U;

  while (blkCnt > 0U)
  {
    /* Perform Multiplications and store in the destination buffer */
    *pOut++ = (q31_t) (((q63_t) * px++ * coeff) >> 32);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Load the coefficient value and
   * increment the coefficient buffer for the next set of state values */
  coeff = *pCoeffs++;

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = (int32_t) (S->stateIndex - blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if (readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Loop over the number of taps. */
  tapCnt = (uint32_t) numTaps - 2U;

  while (tapCnt > 0U)
  {
    /* Working pointer for state buffer is updated */
    py = pState;

    /* blockSize samples are read from the state buffer */
    arm_circularRead_f32((int32_t *) py, delaySize, &readIndex, 1,
                         (int32_t *) pb, (int32_t *) pb, blockSize, 1,
                         blockSize);

    /* Working pointer for the scratch buffer of state values */
    px = pb;

    /* Working pointer for scratch buffer of output values */
    pOut = pDst;

    /* Loop over the blockSize. Unroll by a factor of 4.
     * Compute 4 MACS at a time. */
    blkCnt = blockSize >> 2;

    while (blkCnt > 0U)
    {
      out = *pOut;
      out += ((q63_t) * px++ * coeff) >> 32;
      *pOut++ = (q31_t) (out);

      out = *pOut;
      out += ((q63_t) * px++ * coeff) >> 32;
      *pOut++ = (q31_t) (out);

      out = *pOut;
      out += ((q63_t) * px++ * coeff) >> 32;
      *pOut++ = (q31_t) (out);

      out = *pOut;
      out += ((q63_t) * px++ * coeff) >> 32;
      *pOut++ = (q31_t) (out);

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* If the blockSize is not a multiple of 4,
     * compute the remaining samples */
    blkCnt = blockSize % 0x4U;

    while (blkCnt > 0U)
    {
      /* Perform Multiply-Accumulate */
      out = *pOut;
      out += ((q63_t) * px++ * coeff) >> 32;
      *pOut++ = (q31_t) (out);

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* Load the coefficient value and
     * increment the coefficient buffer for the next set of state values */
    coeff = *pCoeffs++;

    /* Read Index, from where the state buffer should be read, is calculated. */
    readIndex = (int32_t) (S->stateIndex - blockSize) - *pTapDelay++;

    /* Wraparound of readIndex */
    if (readIndex < 0)
    {
      readIndex += (int32_t) delaySize;
    }

    /* Decrement the tap loop counter */
    tapCnt--;
  }

	/* Compute last tap without the final read of pTapDelay */

	/* Working pointer for state buffer is updated */
	py = pState;

	/* blockSize samples are read from the state buffer */
	arm_circularRead_f32((int32_t *) py, delaySize, &readIndex, 1,
											 (int32_t *) pb, (int32_t *) pb, blockSize, 1,
											 blockSize);

	/* Working pointer for the scratch buffer of state values */
	px = pb;

	/* Working pointer for scratch buffer of output values */
	pOut = pDst;

	/* Loop over the blockSize. Unroll by a factor of 4.
	 * Compute 4 MACS at a time. */
	blkCnt = blockSize >> 2;

	while (blkCnt > 0U)
	{
		out = *pOut;
		out += ((q63_t) * px++ * coeff) >> 32;
		*pOut++ = (q31_t) (out);

		out = *pOut;
		out += ((q63_t) * px++ * coeff) >> 32;
		*pOut++ = (q31_t) (out);

		out = *pOut;
		out += ((q63_t) * px++ * coeff) >> 32;
		*pOut++ = (q31_t) (out);

		out = *pOut;
		out += ((q63_t) * px++ * coeff) >> 32;
		*pOut++ = (q31_t) (out);

		/* Decrement the loop counter */
		blkCnt--;
	}

	/* If the blockSize is not a multiple of 4,
	 * compute the remaining samples */
	blkCnt = blockSize % 0x4U;

	while (blkCnt > 0U)
	{
		/* Perform Multiply-Accumulate */
		out = *pOut;
		out += ((q63_t) * px++ * coeff) >> 32;
		*pOut++ = (q31_t) (out);

		/* Decrement the loop counter */
		blkCnt--;
	}

  /* Working output pointer is updated */
  pOut = pDst;

  /* Output is converted into 1.31 format. */
  /* Loop over the blockSize. Unroll by a factor of 4.
   * process 4 output samples at a time. */
  blkCnt = blockSize >> 2;

  while (blkCnt > 0U)
  {
    in = *pOut << 1;
    *pOut++ = in;
    in = *pOut << 1;
    *pOut++ = in;
    in = *pOut << 1;
    *pOut++ = in;
    in = *pOut << 1;
    *pOut++ = in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4,
   * process the remaining output samples */
  blkCnt = blockSize % 0x4U;

  while (blkCnt > 0U)
  {
    in = *pOut << 1;
    *pOut++ = in;

    /* Decrement the loop counter */
    blkCnt--;
  }

#else

  /* Run the below code for Cortex-M0 */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* Perform Multiplications and store in the destination buffer */
    *pOut++ = (q31_t) (((q63_t) * px++ * coeff) >> 32);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Load the coefficient value and
   * increment the coefficient buffer for the next set of state values */
  coeff = *pCoeffs++;

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = (int32_t) (S->stateIndex - blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if (readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Loop over the number of taps. */
  tapCnt = (uint32_t) numTaps - 2U;

  while (tapCnt > 0U)
  {
    /* Working pointer for state buffer is updated */
    py = pState;

    /* blockSize samples are read from the state buffer */
    arm_circularRead_f32((int32_t *) py, delaySize, &readIndex, 1,
                         (int32_t *) pb, (int32_t *) pb, blockSize, 1,
                         blockSize);

    /* Working pointer for the scratch buffer of state values */
    px = pb;

    /* Working pointer for scratch buffer of output values */
    pOut = pDst;

    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
      /* Perform Multiply-Accumulate */
      out = *pOut;
      out += ((q63_t) * px++ * coeff) >> 32;
      *pOut++ = (q31_t) (out);

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* Load the coefficient value and
     * increment the coefficient buffer for the next set of state values */
    coeff = *pCoeffs++;

    /* Read Index, from where the state buffer should be read, is calculated. */
    readIndex = (int32_t) (S->stateIndex - blockSize) - *pTapDelay++;

    /* Wraparound of readIndex */
    if (readIndex < 0)
    {
      readIndex += (int32_t) delaySize;
    }

    /* Decrement the tap loop counter */
    tapCnt--;
  }

	/* Compute last tap without the final read of pTapDelay */

	/* Working pointer for state buffer is updated */
	py = pState;

	/* blockSize samples are read from the state buffer */
	arm_circularRead_f32((int32_t *) py, delaySize, &readIndex, 1,
											 (int32_t *) pb, (int32_t *) pb, blockSize, 1,
											 blockSize);

	/* Working pointer for the scratch buffer of state values */
	px = pb;

	/* Working pointer for scratch buffer of output values */
	pOut = pDst;

	blkCnt = blockSize;

	while (blkCnt > 0U)
	{
		/* Perform Multiply-Accumulate */
		out = *pOut;
		out += ((q63_t) * px++ * coeff) >> 32;
		*pOut++ = (q31_t) (out);

		/* Decrement the loop counter */
		blkCnt--;
	}

  /* Working output pointer is updated */
  pOut = pDst;

  /* Output is converted into 1.31 format. */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    in = *pOut << 1;
    *pOut++ = in;

    /* Decrement the loop counter */
    blkCnt--;
  }

#endif /*   #if defined (ARM_MATH_DSP) */

}

/**
 * @} end of FIR_Sparse group
 */
