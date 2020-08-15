#ifndef ARM_BASIC_MATH_TEST_DATA_H
#define ARM_BASIC_MATH_TEST_DATA_H

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "arr_desc.h"
#include "arm_math.h"

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/
#define BASIC_MATH_MAX_INPUT_ELEMENTS 32
#define BASIC_MATH_BIGGEST_INPUT_TYPE float32_t

/*--------------------------------------------------------------------------------*/
/* Declare Variables */
/*--------------------------------------------------------------------------------*/

/* Input/Output Buffers */
ARR_DESC_DECLARE(basic_math_output_fut);
ARR_DESC_DECLARE(basic_math_output_ref);

extern BASIC_MATH_BIGGEST_INPUT_TYPE
basic_math_output_f32_ref[BASIC_MATH_MAX_INPUT_ELEMENTS];

extern BASIC_MATH_BIGGEST_INPUT_TYPE
basic_math_output_f32_fut[BASIC_MATH_MAX_INPUT_ELEMENTS];

/* Block Sizes*/
ARR_DESC_DECLARE(basic_math_block_sizes);

/* Numbers */
ARR_DESC_DECLARE(basic_math_elts);
ARR_DESC_DECLARE(basic_math_elts2);
ARR_DESC_DECLARE(basic_math_eltsf);

/* Float Inputs */
ARR_DESC_DECLARE(basic_math_zeros);
ARR_DESC_DECLARE(basic_math_f_2);
ARR_DESC_DECLARE(basic_math_f_15);
ARR_DESC_DECLARE(basic_math_f_32);
ARR_DESC_DECLARE(basic_math_f_all);

#endif
