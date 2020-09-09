/* ----------------------------------------------------------------------   
* Copyright (C) 2010-2013 ARM Limited. All rights reserved.   
*   
* $Date:        17. January 2013  
* $Revision: 	V1.4.0   
*  
* Project: 	    CMSIS DSP Library 
*
* Title:	    math_helper.h
* 
* Description:	Prototypes of all helper functions required.  
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


#include "arm_math.h"

#ifndef MATH_HELPER_H
#define MATH_HELPER_H

float arm_snr_f32(float *pRef, float *pTest,  uint32_t buffSize);  
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

