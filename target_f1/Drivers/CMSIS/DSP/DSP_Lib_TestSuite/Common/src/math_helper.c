/* ----------------------------------------------------------------------
* Copyright (C) 2010 ARM Limited. All rights reserved.
*
* $Date:        29. November 2010
* $Revision:    V1.0.3
*
* Project:      CMSIS DSP Library
*
* Title:        math_helper.c
*
* Description:  Definition of all helper functions required.
*
* Target Processor: Cortex-M4/Cortex-M3
*
* Version 1.0.3 2010/11/29
*    Re-organized the CMSIS folders and updated documentation.
*
* Version 1.0.2 2010/11/11
*    Documentation updated.
*
* Version 1.0.1 2010/10/05
*    Production release and review comments incorporated.
*
* Version 1.0.0 2010/09/20
*    Production release and review comments incorporated.
*
* Version 0.0.7  2010/06/10
*    Misra-C changes done
* -------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
*       Include standard header files
* -------------------------------------------------------------------- */
#include<math.h>

/* ----------------------------------------------------------------------
*       Include project header files
* -------------------------------------------------------------------- */
#include "math_helper.h"

/**
 * @brief  Caluclation of SNR
 * @param  float*   Pointer to the reference buffer
 * @param  float*   Pointer to the test buffer
 * @param  uint32_t     total number of samples
 * @return float    SNR
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


  SNR = 10 * log10f (EnergySignal / EnergyError);

	return (SNR);

}



double arm_snr_f64(double *pRef, double *pTest, uint32_t buffSize)
{
  double EnergySignal = 0.0, EnergyError = 0.0;
  uint32_t i;
  double SNR;
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
 * @param  q15_t*       Pointer to input buffer
 * @param  uint32_t     blockSize
 * @param  uint32_t     guard_bits
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
 * @param  uint32_t     number of samples in the buffer
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
 * @param  q15_t*   Pointer to Ref buffer
 * @param  q15_t*   Pointer to Test buffer
 * @param  uint32_t     number of samples in the buffer
 * @return none
 */

uint32_t arm_compare_fixed_q15(q15_t *pIn, q15_t * pOut, uint32_t numSamples)
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
 * @param  q31_t*   Pointer to Ref buffer
 * @param  q31_t*   Pointer to Test buffer
 * @param  uint32_t     number of samples in the buffer
 * @return none
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
 * @param  q31_t*   Pointer to input buffer
 * @param  uint32_t     blockSize
 * @param  uint32_t     guard_bits
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
 * @param  q31_t*   Pointer to input buffer
 * @param  uint32_t     blockSize
 * @param  uint32_t     guard_bits
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
 * @param  uint32_t     number of additions
 * @return none
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
 * @brief  Converts Q15 to floating-point
 * @param  uint32_t     number of samples in the buffer
 * @return none
 */

void arm_apply_guard_bits (float32_t * pIn,
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
 * @param  uint32_t     number of shifts
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
 * @param  uint32_t     number of samples in the buffer
 * @return none
 * The function converts floating point values to fixed point values
 */

void arm_float_to_q14 (float *pIn, q15_t * pOut,
                       uint32_t numSamples)
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
 * @param  uint32_t     number of samples in the buffer
 * @return none
 * The function converts floating point values to fixed point values
 */

void arm_float_to_q30 (float *pIn, q31_t * pOut,
                       uint32_t numSamples)
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
 * @param  uint32_t     number of samples in the buffer
 * @return none
 * The function converts floating point values to fixed point values
 */

void arm_float_to_q29 (float *pIn, q31_t * pOut,
                       uint32_t numSamples)
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
 * @param  uint32_t     number of samples in the buffer
 * @return none
 * The function converts floating point values to fixed point values
 */

void arm_float_to_q28 (float *pIn, q31_t * pOut,
                       uint32_t numSamples)
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
 * @param  pIn      input buffer
 * @param  numSamples   number of samples in the buffer
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
