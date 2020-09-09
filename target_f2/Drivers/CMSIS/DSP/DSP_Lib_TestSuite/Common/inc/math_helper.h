/* ----------------------------------------------------------------------
* Copyright (C) 2010 ARM Limited. All rights reserved.
*
* $Date:        29. November 2010
* $Revision:    V1.0.3
*
* Project:      CMSIS DSP Library
*
* Title:        math_helper.h
*
*
* Description:	Prototypes of all helper functions required.
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

#ifndef MATH_HELPER_H
#define MATH_HELPER_H

#include "arm_math.h"

float arm_snr_f32(float *pRef, float *pTest,  uint32_t buffSize);
double arm_snr_f64(double *pRef, double *pTest,  uint32_t buffSize);
void arm_float_to_q12_20(float *pIn, q31_t * pOut, uint32_t numSamples);
void arm_provide_guard_bits_q15(q15_t *input_buf, uint32_t blockSize, uint32_t guard_bits);
void arm_provide_guard_bits_q31(q31_t *input_buf, uint32_t blockSize, uint32_t guard_bits);
void arm_float_to_q14(float *pIn, q15_t *pOut, uint32_t numSamples);
void arm_float_to_q29(float *pIn, q31_t *pOut, uint32_t numSamples);
void arm_float_to_q28(float *pIn, q31_t *pOut, uint32_t numSamples);
void arm_float_to_q30(float *pIn, q31_t *pOut, uint32_t numSamples);
void arm_clip_f32(float *pIn, uint32_t numSamples);
uint32_t arm_calc_guard_bits(uint32_t num_adds);
void arm_apply_guard_bits (float32_t * pIn, uint32_t numSamples, uint32_t guard_bits);
uint32_t arm_compare_fixed_q15(q15_t *pIn, q15_t * pOut, uint32_t numSamples);
uint32_t arm_compare_fixed_q31(q31_t *pIn, q31_t *pOut, uint32_t numSamples);
uint32_t arm_calc_2pow(uint32_t guard_bits);
#endif
