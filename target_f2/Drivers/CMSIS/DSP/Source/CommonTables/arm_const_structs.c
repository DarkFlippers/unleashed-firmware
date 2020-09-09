/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_const_structs.c
 * Description:  Constant structs that are initialized for user convenience.
 *               For example, some can be given as arguments to the arm_cfft_f32() or arm_rfft_f32() functions.
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

#include "arm_const_structs.h"

/* Floating-point structs */
const arm_cfft_instance_f32 arm_cfft_sR_f32_len16 = {
	16, twiddleCoef_16, armBitRevIndexTable16, ARMBITREVINDEXTABLE_16_TABLE_LENGTH
};

const arm_cfft_instance_f32 arm_cfft_sR_f32_len32 = {
	32, twiddleCoef_32, armBitRevIndexTable32, ARMBITREVINDEXTABLE_32_TABLE_LENGTH
};

const arm_cfft_instance_f32 arm_cfft_sR_f32_len64 = {
	64, twiddleCoef_64, armBitRevIndexTable64, ARMBITREVINDEXTABLE_64_TABLE_LENGTH
};

const arm_cfft_instance_f32 arm_cfft_sR_f32_len128 = {
	128, twiddleCoef_128, armBitRevIndexTable128, ARMBITREVINDEXTABLE_128_TABLE_LENGTH
};

const arm_cfft_instance_f32 arm_cfft_sR_f32_len256 = {
	256, twiddleCoef_256, armBitRevIndexTable256, ARMBITREVINDEXTABLE_256_TABLE_LENGTH
};

const arm_cfft_instance_f32 arm_cfft_sR_f32_len512 = {
	512, twiddleCoef_512, armBitRevIndexTable512, ARMBITREVINDEXTABLE_512_TABLE_LENGTH
};

const arm_cfft_instance_f32 arm_cfft_sR_f32_len1024 = {
	1024, twiddleCoef_1024, armBitRevIndexTable1024, ARMBITREVINDEXTABLE_1024_TABLE_LENGTH
};

const arm_cfft_instance_f32 arm_cfft_sR_f32_len2048 = {
	2048, twiddleCoef_2048, armBitRevIndexTable2048, ARMBITREVINDEXTABLE_2048_TABLE_LENGTH
};

const arm_cfft_instance_f32 arm_cfft_sR_f32_len4096 = {
	4096, twiddleCoef_4096, armBitRevIndexTable4096, ARMBITREVINDEXTABLE_4096_TABLE_LENGTH
};

/* Fixed-point structs */
const arm_cfft_instance_q31 arm_cfft_sR_q31_len16 = {
	16, twiddleCoef_16_q31, armBitRevIndexTable_fixed_16, ARMBITREVINDEXTABLE_FIXED_16_TABLE_LENGTH
};

const arm_cfft_instance_q31 arm_cfft_sR_q31_len32 = {
	32, twiddleCoef_32_q31, armBitRevIndexTable_fixed_32, ARMBITREVINDEXTABLE_FIXED_32_TABLE_LENGTH
};

const arm_cfft_instance_q31 arm_cfft_sR_q31_len64 = {
	64, twiddleCoef_64_q31, armBitRevIndexTable_fixed_64, ARMBITREVINDEXTABLE_FIXED_64_TABLE_LENGTH
};

const arm_cfft_instance_q31 arm_cfft_sR_q31_len128 = {
	128, twiddleCoef_128_q31, armBitRevIndexTable_fixed_128, ARMBITREVINDEXTABLE_FIXED_128_TABLE_LENGTH
};

const arm_cfft_instance_q31 arm_cfft_sR_q31_len256 = {
	256, twiddleCoef_256_q31, armBitRevIndexTable_fixed_256, ARMBITREVINDEXTABLE_FIXED_256_TABLE_LENGTH
};

const arm_cfft_instance_q31 arm_cfft_sR_q31_len512 = {
	512, twiddleCoef_512_q31, armBitRevIndexTable_fixed_512, ARMBITREVINDEXTABLE_FIXED_512_TABLE_LENGTH
};

const arm_cfft_instance_q31 arm_cfft_sR_q31_len1024 = {
	1024, twiddleCoef_1024_q31, armBitRevIndexTable_fixed_1024, ARMBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH
};

const arm_cfft_instance_q31 arm_cfft_sR_q31_len2048 = {
	2048, twiddleCoef_2048_q31, armBitRevIndexTable_fixed_2048, ARMBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH
};

const arm_cfft_instance_q31 arm_cfft_sR_q31_len4096 = {
	4096, twiddleCoef_4096_q31, armBitRevIndexTable_fixed_4096, ARMBITREVINDEXTABLE_FIXED_4096_TABLE_LENGTH
};

const arm_cfft_instance_q15 arm_cfft_sR_q15_len16 = {
	16, twiddleCoef_16_q15, armBitRevIndexTable_fixed_16, ARMBITREVINDEXTABLE_FIXED_16_TABLE_LENGTH
};

const arm_cfft_instance_q15 arm_cfft_sR_q15_len32 = {
	32, twiddleCoef_32_q15, armBitRevIndexTable_fixed_32, ARMBITREVINDEXTABLE_FIXED_32_TABLE_LENGTH
};

const arm_cfft_instance_q15 arm_cfft_sR_q15_len64 = {
	64, twiddleCoef_64_q15, armBitRevIndexTable_fixed_64, ARMBITREVINDEXTABLE_FIXED_64_TABLE_LENGTH
};

const arm_cfft_instance_q15 arm_cfft_sR_q15_len128 = {
	128, twiddleCoef_128_q15, armBitRevIndexTable_fixed_128, ARMBITREVINDEXTABLE_FIXED_128_TABLE_LENGTH
};

const arm_cfft_instance_q15 arm_cfft_sR_q15_len256 = {
	256, twiddleCoef_256_q15, armBitRevIndexTable_fixed_256, ARMBITREVINDEXTABLE_FIXED_256_TABLE_LENGTH
};

const arm_cfft_instance_q15 arm_cfft_sR_q15_len512 = {
	512, twiddleCoef_512_q15, armBitRevIndexTable_fixed_512, ARMBITREVINDEXTABLE_FIXED_512_TABLE_LENGTH
};

const arm_cfft_instance_q15 arm_cfft_sR_q15_len1024 = {
	1024, twiddleCoef_1024_q15, armBitRevIndexTable_fixed_1024, ARMBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH
};

const arm_cfft_instance_q15 arm_cfft_sR_q15_len2048 = {
	2048, twiddleCoef_2048_q15, armBitRevIndexTable_fixed_2048, ARMBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH
};

const arm_cfft_instance_q15 arm_cfft_sR_q15_len4096 = {
	4096, twiddleCoef_4096_q15, armBitRevIndexTable_fixed_4096, ARMBITREVINDEXTABLE_FIXED_4096_TABLE_LENGTH
};

/* Structure for real-value inputs */
/* Floating-point structs */
const arm_rfft_fast_instance_f32 arm_rfft_fast_sR_f32_len32 = {
	{ 16, twiddleCoef_32, armBitRevIndexTable32, ARMBITREVINDEXTABLE_16_TABLE_LENGTH },
	32U,
	(float32_t *)twiddleCoef_rfft_32
};

const arm_rfft_fast_instance_f32 arm_rfft_fast_sR_f32_len64 = {
	 { 32, twiddleCoef_32, armBitRevIndexTable32, ARMBITREVINDEXTABLE_32_TABLE_LENGTH },
	64U,
	(float32_t *)twiddleCoef_rfft_64
};

const arm_rfft_fast_instance_f32 arm_rfft_fast_sR_f32_len128 = {
	{ 64, twiddleCoef_64, armBitRevIndexTable64, ARMBITREVINDEXTABLE_64_TABLE_LENGTH },
	128U,
	(float32_t *)twiddleCoef_rfft_128
};

const arm_rfft_fast_instance_f32 arm_rfft_fast_sR_f32_len256 = {
	{ 128, twiddleCoef_128, armBitRevIndexTable128, ARMBITREVINDEXTABLE_128_TABLE_LENGTH },
	256U,
	(float32_t *)twiddleCoef_rfft_256
};

const arm_rfft_fast_instance_f32 arm_rfft_fast_sR_f32_len512 = {
	{ 256, twiddleCoef_256, armBitRevIndexTable256, ARMBITREVINDEXTABLE_256_TABLE_LENGTH },
	512U,
	(float32_t *)twiddleCoef_rfft_512
};

const arm_rfft_fast_instance_f32 arm_rfft_fast_sR_f32_len1024 = {
	{ 512, twiddleCoef_512, armBitRevIndexTable512, ARMBITREVINDEXTABLE_512_TABLE_LENGTH },
	1024U,
	(float32_t *)twiddleCoef_rfft_1024
};

const arm_rfft_fast_instance_f32 arm_rfft_fast_sR_f32_len2048 = {
	{ 1024, twiddleCoef_1024, armBitRevIndexTable1024, ARMBITREVINDEXTABLE_1024_TABLE_LENGTH },
	2048U,
	(float32_t *)twiddleCoef_rfft_2048
};

const arm_rfft_fast_instance_f32 arm_rfft_fast_sR_f32_len4096 = {
	{ 2048, twiddleCoef_2048, armBitRevIndexTable2048, ARMBITREVINDEXTABLE_2048_TABLE_LENGTH },
	4096U,
	(float32_t *)twiddleCoef_rfft_4096
};

/* Fixed-point structs */
/* q31_t */
extern const q31_t realCoefAQ31[8192];
extern const q31_t realCoefBQ31[8192];

const arm_rfft_instance_q31 arm_rfft_sR_q31_len32 = {
	32U,
	0,
	1,
	256U,
	(q31_t*)realCoefAQ31,
	(q31_t*)realCoefBQ31,
	&arm_cfft_sR_q31_len16
};

const arm_rfft_instance_q31 arm_rfft_sR_q31_len64 = {
	64U,
	0,
	1,
	128U,
	(q31_t*)realCoefAQ31,
	(q31_t*)realCoefBQ31,
	&arm_cfft_sR_q31_len32
};

const arm_rfft_instance_q31 arm_rfft_sR_q31_len128 = {
	128U,
	0,
	1,
	64U,
	(q31_t*)realCoefAQ31,
	(q31_t*)realCoefBQ31,
	&arm_cfft_sR_q31_len64
};

const arm_rfft_instance_q31 arm_rfft_sR_q31_len256 = {
	256U,
	0,
	1,
	32U,
	(q31_t*)realCoefAQ31,
	(q31_t*)realCoefBQ31,
	&arm_cfft_sR_q31_len128
};

const arm_rfft_instance_q31 arm_rfft_sR_q31_len512 = {
	512U,
	0,
	1,
	16U,
	(q31_t*)realCoefAQ31,
	(q31_t*)realCoefBQ31,
	&arm_cfft_sR_q31_len256
};

const arm_rfft_instance_q31 arm_rfft_sR_q31_len1024 = {
	1024U,
	0,
	1,
	8U,
	(q31_t*)realCoefAQ31,
	(q31_t*)realCoefBQ31,
	&arm_cfft_sR_q31_len512
};

const arm_rfft_instance_q31 arm_rfft_sR_q31_len2048 = {
	2048U,
	0,
	1,
	4U,
	(q31_t*)realCoefAQ31,
	(q31_t*)realCoefBQ31,
	&arm_cfft_sR_q31_len1024
};

const arm_rfft_instance_q31 arm_rfft_sR_q31_len4096 = {
	4096U,
	0,
	1,
	2U,
	(q31_t*)realCoefAQ31,
	(q31_t*)realCoefBQ31,
	&arm_cfft_sR_q31_len2048
};

const arm_rfft_instance_q31 arm_rfft_sR_q31_len8192 = {
	8192U,
	0,
	1,
	1U,
	(q31_t*)realCoefAQ31,
	(q31_t*)realCoefBQ31,
	&arm_cfft_sR_q31_len4096
};

/* q15_t */
extern const q15_t realCoefAQ15[8192];
extern const q15_t realCoefBQ15[8192];

const arm_rfft_instance_q15 arm_rfft_sR_q15_len32 = {
	32U,
	0,
	1,
	256U,
	(q15_t*)realCoefAQ15,
	(q15_t*)realCoefBQ15,
	&arm_cfft_sR_q15_len16
};

const arm_rfft_instance_q15 arm_rfft_sR_q15_len64 = {
	64U,
	0,
	1,
	128U,
	(q15_t*)realCoefAQ15,
	(q15_t*)realCoefBQ15,
	&arm_cfft_sR_q15_len32
};

const arm_rfft_instance_q15 arm_rfft_sR_q15_len128 = {
	128U,
	0,
	1,
	64U,
	(q15_t*)realCoefAQ15,
	(q15_t*)realCoefBQ15,
	&arm_cfft_sR_q15_len64
};

const arm_rfft_instance_q15 arm_rfft_sR_q15_len256 = {
	256U,
	0,
	1,
	32U,
	(q15_t*)realCoefAQ15,
	(q15_t*)realCoefBQ15,
	&arm_cfft_sR_q15_len128
};

const arm_rfft_instance_q15 arm_rfft_sR_q15_len512 = {
	512U,
	0,
	1,
	16U,
	(q15_t*)realCoefAQ15,
	(q15_t*)realCoefBQ15,
	&arm_cfft_sR_q15_len256
};

const arm_rfft_instance_q15 arm_rfft_sR_q15_len1024 = {
	1024U,
	0,
	1,
	8U,
	(q15_t*)realCoefAQ15,
	(q15_t*)realCoefBQ15,
	&arm_cfft_sR_q15_len512
};

const arm_rfft_instance_q15 arm_rfft_sR_q15_len2048 = {
	2048U,
	0,
	1,
	4U,
	(q15_t*)realCoefAQ15,
	(q15_t*)realCoefBQ15,
	&arm_cfft_sR_q15_len1024
};

const arm_rfft_instance_q15 arm_rfft_sR_q15_len4096 = {
	4096U,
	0,
	1,
	2U,
	(q15_t*)realCoefAQ15,
	(q15_t*)realCoefBQ15,
	&arm_cfft_sR_q15_len2048
};

const arm_rfft_instance_q15 arm_rfft_sR_q15_len8192 = {
	8192U,
	0,
	1,
	1U,
	(q15_t*)realCoefAQ15,
	(q15_t*)realCoefBQ15,
	&arm_cfft_sR_q15_len4096
};
