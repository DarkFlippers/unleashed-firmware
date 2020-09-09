/*
 * Copyright (C) 2010-2018 Arm Limited or its affiliates. All rights reserved.
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

/* ----------------------------------------------------------------------
 * Project:      CMSIS NN Library
 * Title:        arm_nnsupportfunctions.h
 * Description:  Public header file of support functions for CMSIS NN Library
 *
 * $Date:        13. July 2018
 * $Revision:    V.1.0.0
 *
 * Target Processor:  Cortex-M cores
 * -------------------------------------------------------------------- */

#ifndef _ARM_NNSUPPORTFUNCTIONS_H_
#define _ARM_NNSUPPORTFUNCTIONS_H_

#include "arm_math.h"
#include "arm_common_tables.h"
//#include <cstring>

#ifdef __cplusplus
extern    "C"
{
#endif

/**
 * @brief Union for SIMD access of Q31/Q15/Q7 types
 */
union arm_nnword
{
    q31_t     word;
               /**< Q31 type */
    q15_t     half_words[2];
               /**< Q15 type */
    q7_t      bytes[4];
               /**< Q7 type */
};

/**
 * @brief Struct for specifying activation function types
 *
 */
typedef enum
{
    ARM_SIGMOID = 0,
                /**< Sigmoid activation function */
    ARM_TANH = 1,
             /**< Tanh activation function */
} arm_nn_activation_type;

/**
 * @defgroup nndata_convert Neural Network Data Conversion Functions
 *
 * Perform data type conversion in-between neural network operations
 *
 */

/**
 * @brief Converts the elements of the Q7 vector to Q15 vector without left-shift 
 * @param[in]       *pSrc points to the Q7 input vector    
 * @param[out]      *pDst points to the Q15 output vector   
 * @param[in]       blockSize length of the input vector    
 * @return none.    
 *
 */

void      arm_q7_to_q15_no_shift(const q7_t * pSrc, q15_t * pDst, uint32_t blockSize);

/**
 * @brief  Converts the elements of the Q7 vector to reordered Q15 vector without left-shift
 * @param[in]       *pSrc points to the Q7 input vector    
 * @param[out]      *pDst points to the Q15 output vector   
 * @param[in]       blockSize length of the input vector    
 * @return none.    
 *
 */

void      arm_q7_to_q15_reordered_no_shift(const q7_t * pSrc, q15_t * pDst, uint32_t blockSize);

#if defined (ARM_MATH_DSP)

/**
 * @brief read and expand one Q7 word into two Q15 words
 */

__STATIC_FORCEINLINE void *read_and_pad(void *source, q31_t * out1, q31_t * out2)
{
        q31_t     inA = *__SIMD32(source)++;
        q31_t     inAbuf1 = __SXTB16(__ROR(inA, 8));
        q31_t     inAbuf2 = __SXTB16(inA);

#ifndef ARM_MATH_BIG_ENDIAN
        *out2 = __PKHTB(inAbuf1, inAbuf2, 16);
        *out1 = __PKHBT(inAbuf2, inAbuf1, 16);
#else
        *out1 = __PKHTB(inAbuf1, inAbuf2, 16);
        *out2 = __PKHBT(inAbuf2, inAbuf1, 16);
#endif

        return source;
}

/**
 * @brief read and expand one Q7 word into two Q15 words with reordering
 */

__STATIC_FORCEINLINE void *read_and_pad_reordered(void *source, q31_t * out1, q31_t * out2)
{
        q31_t     inA = *__SIMD32(source)++;
#ifndef ARM_MATH_BIG_ENDIAN
        *out2 = __SXTB16(__ROR(inA, 8));
        *out1 = __SXTB16(inA);
#else
        *out1 = __SXTB16(__ROR(inA, 8));
        *out2 = __SXTB16(inA);
#endif

        return source;
}
#endif

/**
 * @defgroup NNBasicMath Basic Math Functions for Neural Network Computation
 *
 * Basic Math Functions for Neural Network Computation
 *
 */

/**
 * @brief           Q7 vector multiplication with variable output shifts
 * @param[in]       *pSrcA        pointer to the first input vector
 * @param[in]       *pSrcB        pointer to the second input vector
 * @param[out]      *pDst         pointer to the output vector
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in]       blockSize     number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q15 range [0x8000 0x7FFF] will be saturated.
 */

void arm_nn_mult_q15(
  q15_t * pSrcA,
  q15_t * pSrcB,
  q15_t * pDst,
  const uint16_t out_shift,
  uint32_t blockSize);
  
/**
 * @brief           Q7 vector multiplication with variable output shifts
 * @param[in]       *pSrcA        pointer to the first input vector
 * @param[in]       *pSrcB        pointer to the second input vector
 * @param[out]      *pDst         pointer to the output vector
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in]       blockSize     number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q7 range [0x80 0x7F] will be saturated.
 */

void arm_nn_mult_q7(
  q7_t * pSrcA,
  q7_t * pSrcB,
  q7_t * pDst,
  const uint16_t out_shift,
  uint32_t blockSize);
 
/**
 * @brief defition to adding rouding offset
 */
#ifndef ARM_NN_TRUNCATE
    #define NN_ROUND(out_shift) ( 0x1 << (out_shift - 1) )
#else
    #define NN_ROUND(out_shift) 0
#endif

#ifdef __cplusplus
}
#endif

#endif
