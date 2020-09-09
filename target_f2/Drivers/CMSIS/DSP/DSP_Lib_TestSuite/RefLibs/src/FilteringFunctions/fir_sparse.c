#include "ref.h"

void ref_fir_sparse_f32(
  arm_fir_sparse_instance_f32 * S,
  float32_t * pSrc,
  float32_t * pDst,
  float32_t * pScratchIn,
  uint32_t blockSize)
{
  float32_t *pState = S->pState;                 /* State pointer */
  float32_t *pCoeffs = S->pCoeffs;               /* Coefficient pointer */
  float32_t *px;                                 /* Scratch buffer pointer */
  float32_t *py = pState;                        /* Temporary pointers for state buffer */
  float32_t *pb = pScratchIn;                    /* Temporary pointers for scratch buffer */
  float32_t *pOut;                               /* Destination pointer */
  int32_t *pTapDelay = S->pTapDelay;             /* Pointer to the array containing offset of the non-zero tap values. */
  uint32_t delaySize = S->maxDelay + blockSize;  /* state length */
  uint16_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter  */
  int32_t readIndex;                             /* Read index of the state buffer */
  uint32_t tapCnt, blkCnt;                       /* loop counters */
  float32_t coeff = *pCoeffs++;                  /* Read the first coefficient value */


  /* BlockSize of Input samples are copied into the state buffer */
  /* StateIndex points to the starting position to write in the state buffer */
  arm_circularWrite_f32((int32_t *) py, delaySize, &S->stateIndex, 1,
                        (int32_t *) pSrc, 1, blockSize);


  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = ((int32_t) S->stateIndex - (int32_t) blockSize) - *pTapDelay++;

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

  /* Working pointer for the scratch buffer */
  px = pb;

  /* Working pointer for destination buffer */
  pOut = pDst;

  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* Perform Multiplications and store in destination buffer */
    *pOut++ = *px++ * coeff;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Loop over the number of taps. */
  tapCnt = (uint32_t) numTaps - 1U;

  while (tapCnt > 0U)
  {
    /* Load the coefficient value and
     * increment the coefficient buffer for the next set of state values */
    coeff = *pCoeffs++;

    /* Read Index, from where the state buffer should be read, is calculated. */
    readIndex = ((int32_t) S->stateIndex - (int32_t) blockSize) - *pTapDelay++;

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

    /* Working pointer for the scratch buffer */
    px = pb;

    /* Working pointer for destination buffer */
    pOut = pDst;

    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
      /* Perform Multiply-Accumulate */
      *pOut++ += *px++ * coeff;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* Decrement the tap loop counter */
    tapCnt--;
  }
}

void ref_fir_sparse_q31(
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
  
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* Perform Multiplications and store in the destination buffer */
    *pOut++ = (q31_t) (((q63_t) * px++ * coeff) >> 32);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Loop over the number of taps. */
  tapCnt = (uint32_t) numTaps - 1U;

  while (tapCnt > 0U)
  {
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

    /* Decrement the tap loop counter */
    tapCnt--;
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
}

void ref_fir_sparse_q15(
  arm_fir_sparse_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  q15_t * pScratchIn,
  q31_t * pScratchOut,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer */
  q15_t *pIn = pSrc;                             /* Working pointer for input */
  q15_t *pOut = pDst;                            /* Working pointer for output */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *px;                                     /* Temporary pointers for scratch buffer */
  q15_t *pb = pScratchIn;                        /* Temporary pointers for scratch buffer */
  q15_t *py = pState;                            /* Temporary pointers for state buffer */
  int32_t *pTapDelay = S->pTapDelay;             /* Pointer to the array containing offset of the non-zero tap values. */
  uint32_t delaySize = S->maxDelay + blockSize;  /* state length */
  uint16_t numTaps = S->numTaps;                 /* Filter order */
  int32_t readIndex;                             /* Read index of the state buffer */
  uint32_t tapCnt, blkCnt;                       /* loop counters */
  q15_t coeff = *pCoeffs++;                      /* Read the first coefficient value */
  q31_t *pScr2 = pScratchOut;                    /* Working pointer for pScratchOut */

  /* BlockSize of Input samples are copied into the state buffer */
  /* StateIndex points to the starting position to write in the state buffer */
  arm_circularWrite_q15(py, delaySize, &S->stateIndex, 1, pIn, 1, blockSize);

  /* Loop over the number of taps. */
  tapCnt = numTaps;

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = (S->stateIndex - blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if (readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Working pointer for state buffer is updated */
  py = pState;

  /* blockSize samples are read from the state buffer */
  arm_circularRead_q15(py, delaySize, &readIndex, 1,
                       pb, pb, blockSize, 1, blockSize);

  /* Working pointer for the scratch buffer of state values */
  px = pb;

  /* Working pointer for scratch buffer of output values */
  pScratchOut = pScr2;

  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* Perform multiplication and store in the scratch buffer */
    *pScratchOut++ = ((q31_t) * px++ * coeff);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Loop over the number of taps. */
  tapCnt = (uint32_t) numTaps - 1U;

  while (tapCnt > 0U)
  {
    /* Load the coefficient value and           
     * increment the coefficient buffer for the next set of state values */
    coeff = *pCoeffs++;

    /* Read Index, from where the state buffer should be read, is calculated. */
    readIndex = (S->stateIndex - blockSize) - *pTapDelay++;

    /* Wraparound of readIndex */
    if (readIndex < 0)
    {
      readIndex += (int32_t) delaySize;
    }

    /* Working pointer for state buffer is updated */
    py = pState;

    /* blockSize samples are read from the state buffer */
    arm_circularRead_q15(py, delaySize, &readIndex, 1,
                         pb, pb, blockSize, 1, blockSize);

    /* Working pointer for the scratch buffer of state values */
    px = pb;

    /* Working pointer for scratch buffer of output values */
    pScratchOut = pScr2;

    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
      /* Perform Multiply-Accumulate */
      *pScratchOut++ += (q31_t) * px++ * coeff;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* Decrement the tap loop counter */
    tapCnt--;
  }

  /* All the output values are in pScratchOut buffer.       
     Convert them into 1.15 format, saturate and store in the destination buffer. */
  /* Loop over the blockSize. */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    *pOut++ = (q15_t) __SSAT(*pScr2++ >> 15, 16);
    blkCnt--;
  }
}

void ref_fir_sparse_q7(
  arm_fir_sparse_instance_q7 * S,
  q7_t *pSrc,
  q7_t *pDst,
  q7_t *pScratchIn,
  q31_t * pScratchOut,
  uint32_t blockSize)
{
  q7_t *pState = S->pState;                      /* State pointer */
  q7_t *pCoeffs = S->pCoeffs;                    /* Coefficient pointer */
  q7_t *px;                                      /* Scratch buffer pointer */
  q7_t *py = pState;                             /* Temporary pointers for state buffer */
  q7_t *pb = pScratchIn;                         /* Temporary pointers for scratch buffer */
  q7_t *pOut = pDst;                             /* Destination pointer */
  int32_t *pTapDelay = S->pTapDelay;             /* Pointer to the array containing offset of the non-zero tap values. */
  uint32_t delaySize = S->maxDelay + blockSize;  /* state length */
  uint16_t numTaps = S->numTaps;                 /* Filter order */
  int32_t readIndex;                             /* Read index of the state buffer */
  uint32_t tapCnt, blkCnt;                       /* loop counters */
  q7_t coeff = *pCoeffs++;                       /* Read the coefficient value */
  q31_t *pScr2 = pScratchOut;                    /* Working pointer for scratch buffer of output values */
  q31_t in;

  /* BlockSize of Input samples are copied into the state buffer */
  /* StateIndex points to the starting position to write in the state buffer */
  arm_circularWrite_q7(py, (int32_t) delaySize, &S->stateIndex, 1, pSrc, 1,
                       blockSize);

  /* Loop over the number of taps. */
  tapCnt = numTaps;

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = ((int32_t) S->stateIndex - (int32_t) blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if (readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Working pointer for state buffer is updated */
  py = pState;

  /* blockSize samples are read from the state buffer */
  arm_circularRead_q7(py, (int32_t) delaySize, &readIndex, 1, pb, pb,
                      (int32_t) blockSize, 1, blockSize);

  /* Working pointer for the scratch buffer of state values */
  px = pb;

  /* Working pointer for scratch buffer of output values */
  pScratchOut = pScr2;

  /* Loop over the blockSize */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* Perform multiplication and store in the scratch buffer */
    *pScratchOut++ = ((q31_t) * px++ * coeff);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Loop over the number of taps. */
  tapCnt = (uint32_t) numTaps - 1U;

  while (tapCnt > 0U)
  {
    /* Load the coefficient value and           
     * increment the coefficient buffer for the next set of state values */
    coeff = *pCoeffs++;

    /* Read Index, from where the state buffer should be read, is calculated. */
    readIndex = ((int32_t) S->stateIndex - (int32_t) blockSize) - *pTapDelay++;

    /* Wraparound of readIndex */
    if (readIndex < 0)
    {
      readIndex += (int32_t) delaySize;
    }

    /* Working pointer for state buffer is updated */
    py = pState;

    /* blockSize samples are read from the state buffer */
    arm_circularRead_q7(py, (int32_t) delaySize, &readIndex, 1, pb, pb,
                        (int32_t) blockSize, 1, blockSize);

    /* Working pointer for the scratch buffer of state values */
    px = pb;

    /* Working pointer for scratch buffer of output values */
    pScratchOut = pScr2;

    /* Loop over the blockSize */
    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
      /* Perform Multiply-Accumulate */
      in = *pScratchOut + ((q31_t) * px++ * coeff);
      *pScratchOut++ = in;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* Decrement the tap loop counter */
    tapCnt--;
  }

  /* All the output values are in pScratchOut buffer.       
     Convert them into 1.15 format, saturate and store in the destination buffer. */
  /* Loop over the blockSize. */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    *pOut++ = (q7_t) __SSAT(*pScr2++ >> 7, 8);

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }
}
