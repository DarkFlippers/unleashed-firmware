/* ----------------------------------------------------------------------
* Copyright (C) 2010-2012 ARM Limited. All rights reserved.
*
* $Date:        17. January 2013
* $Revision: 	V1.4.0  b
*
* Project: 	    CMSIS DSP Library
*
* Title:	    math_helper.c
*
* Description:	Definition of all helper functions required.
*
* Target Processor: Cortex-M4/Cortex-M3
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*   - Neither the name of ARM LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
*		Include standard header files
* -------------------------------------------------------------------- */
#include<math.h>

/* ----------------------------------------------------------------------
*		Include project header files
* -------------------------------------------------------------------- */
#include "math_helper.h"

/**
 * @brief  Caluclation of SNR
 * @param[in]  pRef 	Pointer to the reference buffer
 * @param[in]  pTest	Pointer to the test buffer
 * @param[in]  buffSize	total number of samples
 * @return     SNR
 * The function Caluclates signal to noise ratio for the reference output
 * and test output
 */

float arm_snr_f32(float *pRef, float *pTest, uint32_t buffSize)
{
  float EnergySignal = 0.0, EnergyError = 0.0;
  uint32_t i;
  float SNR;
  int temp;
  int *test;

  for (i = 0; i < buffSize; i++)
    {
 	  /* Checking for a NAN value in pRef array */
	  test =   (int *)(&pRef[i]);
      temp =  *test;

	  if (temp == 0x7FC00000)
	  {
	  		return(0);
	  }

	  /* Checking for a NAN value in pTest array */
	  test =   (int *)(&pTest[i]);
      temp =  *test;

	  if (temp == 0x7FC00000)
	  {
	  		return(0);
	  }
      EnergySignal += pRef[i] * pRef[i];
      EnergyError += (pRef[i] - pTest[i]) * (pRef[i] - pTest[i]);
    }

	/* Checking for a NAN value in EnergyError */
	test =   (int *)(&EnergyError);
    temp =  *test;

    if (temp == 0x7FC00000)
    {
  		return(0);
    }


  SNR = 10 * log10 (EnergySignal / EnergyError);

  return (SNR);

}


/**
 * @brief  Provide guard bits for Input buffer
 * @param[in,out]  input_buf   Pointer to input buffer
 * @param[in]       blockSize  block Size
 * @param[in]       guard_bits guard bits
 * @return none
 * The function Provides the guard bits for the buffer
 * to avoid overflow
 */

void arm_provide_guard_bits_q15 (q15_t * input_buf, uint32_t blockSize,
                            uint32_t guard_bits)
{
  uint32_t i;

  for (i = 0; i < blockSize; i++)
    {
      input_buf[i] = input_buf[i] >> guard_bits;
    }
}

/**
 * @brief  Converts float to fixed in q12.20 format
 * @param[in]  pIn         pointer to input buffer
 * @param[out] pOut        pointer to outputbuffer
 * @param[in]  numSamples  number of samples in the input buffer
 * @return none
 * The function converts floating point values to fixed point(q12.20) values
 */

void arm_float_to_q12_20(float *pIn, q31_t * pOut, uint32_t numSamples)
{
  uint32_t i;

  for (i = 0; i < numSamples; i++)
    {
	  /* 1048576.0f corresponds to pow(2, 20) */
      pOut[i] = (q31_t) (pIn[i] * 1048576.0f);

      pOut[i] += pIn[i] > 0 ? 0.5 : -0.5;

      if (pIn[i] == (float) 1.0)
        {
          pOut[i] = 0x000FFFFF;
        }
    }
}

/**
 * @brief  Compare MATLAB Reference Output and ARM Test output
 * @param[in]  pIn         Pointer to Ref buffer
 * @param[in]  pOut        Pointer to Test buffer
 * @param[in]  numSamples  number of samples in the buffer
 * @return maximum difference
 */

uint32_t arm_compare_fixed_q15(q15_t *pIn, q15_t *pOut, uint32_t numSamples)
{
  uint32_t i;
  int32_t diff, diffCrnt = 0;
  uint32_t maxDiff = 0;

  for (i = 0; i < numSamples; i++)
  {
  	diff = pIn[i] - pOut[i];
  	diffCrnt = (diff > 0) ? diff : -diff;

	if (diffCrnt > maxDiff)
	{
		maxDiff = diffCrnt;
	}
  }

  return(maxDiff);
}

/**
 * @brief  Compare MATLAB Reference Output and ARM Test output
 * @param[in]  pIn         Pointer to Ref buffer
 * @param[in]  pOut        Pointer to Test buffer
 * @param[in]  numSamples number of samples in the buffer
 * @return maximum difference
 */

uint32_t arm_compare_fixed_q31(q31_t *pIn, q31_t * pOut, uint32_t numSamples)
{
  uint32_t i;
  int32_t diff, diffCrnt = 0;
  uint32_t maxDiff = 0;

  for (i = 0; i < numSamples; i++)
  {
  	diff = pIn[i] - pOut[i];
  	diffCrnt = (diff > 0) ? diff : -diff;

	if (diffCrnt > maxDiff)
	{
		maxDiff = diffCrnt;
	}
  }

  return(maxDiff);
}

/**
 * @brief  Provide guard bits for Input buffer
 * @param[in,out]  input_buf   Pointer to input buffer
 * @param[in]       blockSize  block Size
 * @param[in]       guard_bits guard bits
 * @return none
 * The function Provides the guard bits for the buffer
 * to avoid overflow
 */

void arm_provide_guard_bits_q31 (q31_t * input_buf,
								 uint32_t blockSize,
                                 uint32_t guard_bits)
{
  uint32_t i;

  for (i = 0; i < blockSize; i++)
    {
      input_buf[i] = input_buf[i] >> guard_bits;
    }
}

/**
 * @brief  Provide guard bits for Input buffer
 * @param[in,out]  input_buf   Pointer to input buffer
 * @param[in]       blockSize  block Size
 * @param[in]       guard_bits guard bits
 * @return none
 * The function Provides the guard bits for the buffer
 * to avoid overflow
 */

void arm_provide_guard_bits_q7 (q7_t * input_buf,
								uint32_t blockSize,
                                uint32_t guard_bits)
{
  uint32_t i;

  for (i = 0; i < blockSize; i++)
    {
      input_buf[i] = input_buf[i] >> guard_bits;
    }
}



/**
 * @brief  Caluclates number of guard bits
 * @param[in]  num_adds 	number of additions
 * @return guard bits
 * The function Caluclates the number of guard bits
 * depending on the numtaps
 */

uint32_t arm_calc_guard_bits (uint32_t num_adds)
{
  uint32_t i = 1, j = 0;

  if (num_adds == 1)
    {
      return (0);
    }

  while (i < num_adds)
    {
      i = i * 2;
      j++;
    }

  return (j);
}

/**
 * @brief  Apply guard bits to buffer
 * @param[in,out]  pIn         pointer to input buffer
 * @param[in]      numSamples  number of samples in the input buffer
 * @param[in]      guard_bits  guard bits
 * @return none
 */

void arm_apply_guard_bits (float32_t *pIn,
						   uint32_t numSamples,
						   uint32_t guard_bits)
{
  uint32_t i;

  for (i = 0; i < numSamples; i++)
    {
      pIn[i] = pIn[i] * arm_calc_2pow(guard_bits);
    }
}

/**
 * @brief  Calculates pow(2, numShifts)
 * @param[in]  numShifts 	number of shifts
 * @return pow(2, numShifts)
 */
uint32_t arm_calc_2pow(uint32_t numShifts)
{

  uint32_t i, val = 1;

  for (i = 0; i < numShifts; i++)
    {
      val = val * 2;
    }

  return(val);
}



/**
 * @brief  Converts float to fixed q14
 * @param[in]  pIn         pointer to input buffer
 * @param[out] pOut        pointer to output buffer
 * @param[in]  numSamples  number of samples in the buffer
 * @return none
 * The function converts floating point values to fixed point values
 */

void arm_float_to_q14 (float *pIn, q15_t *pOut, uint32_t numSamples)
{
  uint32_t i;

  for (i = 0; i < numSamples; i++)
    {
	  /* 16384.0f corresponds to pow(2, 14) */
      pOut[i] = (q15_t) (pIn[i] * 16384.0f);

      pOut[i] += pIn[i] > 0 ? 0.5 : -0.5;

      if (pIn[i] == (float) 2.0)
        {
          pOut[i] = 0x7FFF;
        }

    }

}


/**
 * @brief  Converts float to fixed q30 format
 * @param[in]  pIn         pointer to input buffer
 * @param[out] pOut        pointer to output buffer
 * @param[in]  numSamples  number of samples in the buffer
 * @return none
 * The function converts floating point values to fixed point values
 */

void arm_float_to_q30 (float *pIn, q31_t * pOut, uint32_t numSamples)
{
  uint32_t i;

  for (i = 0; i < numSamples; i++)
    {
	  /* 1073741824.0f corresponds to pow(2, 30) */
      pOut[i] = (q31_t) (pIn[i] * 1073741824.0f);

      pOut[i] += pIn[i] > 0 ? 0.5 : -0.5;

      if (pIn[i] == (float) 2.0)
        {
          pOut[i] = 0x7FFFFFFF;
        }
    }
}

/**
 * @brief  Converts float to fixed q30 format
 * @param[in]  pIn         pointer to input buffer
 * @param[out] pOut        pointer to output buffer
 * @param[in]  numSamples  number of samples in the buffer
 * @return none
 * The function converts floating point values to fixed point values
 */

void arm_float_to_q29 (float *pIn, q31_t *pOut, uint32_t numSamples)
{
  uint32_t i;

  for (i = 0; i < numSamples; i++)
    {
	  /* 1073741824.0f corresponds to pow(2, 30) */
      pOut[i] = (q31_t) (pIn[i] * 536870912.0f);

      pOut[i] += pIn[i] > 0 ? 0.5 : -0.5;

      if (pIn[i] == (float) 4.0)
        {
          pOut[i] = 0x7FFFFFFF;
        }
    }
}


/**
 * @brief  Converts float to fixed q28 format
 * @param[in]  pIn         pointer to input buffer
 * @param[out] pOut        pointer to output buffer
 * @param[in]  numSamples  number of samples in the buffer
 * @return none
 * The function converts floating point values to fixed point values
 */

void arm_float_to_q28 (float *pIn, q31_t *pOut, uint32_t numSamples)
{
  uint32_t i;

  for (i = 0; i < numSamples; i++)
    {
	/* 268435456.0f corresponds to pow(2, 28) */
      pOut[i] = (q31_t) (pIn[i] * 268435456.0f);

      pOut[i] += pIn[i] > 0 ? 0.5 : -0.5;

      if (pIn[i] == (float) 8.0)
        {
          pOut[i] = 0x7FFFFFFF;
        }
    }
}

/**
 * @brief  Clip the float values to +/- 1
 * @param[in,out]  pIn           input buffer
 * @param[in]      numSamples    number of samples in the buffer
 * @return none
 * The function converts floating point values to fixed point values
 */

void arm_clip_f32 (float *pIn, uint32_t numSamples)
{
  uint32_t i;

  for (i = 0; i < numSamples; i++)
    {
      if (pIn[i] > 1.0f)
	  {
	    pIn[i] = 1.0;
	  }
	  else if ( pIn[i] < -1.0f)
	  {
	    pIn[i] = -1.0;
	  }

    }
}




