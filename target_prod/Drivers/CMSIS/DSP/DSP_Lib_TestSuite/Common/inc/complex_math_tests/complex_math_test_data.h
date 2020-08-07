#ifndef _COMPLEX_MATH_TEST_DATA_H_
#define _COMPLEX_MATH_TEST_DATA_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "arr_desc.h"
#include "arm_math.h"

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/
#define COMPLEX_MATH_MAX_INPUT_ELEMENTS 32
#define COMPLEX_MATH_BIGGEST_INPUT_TYPE float32_t

/*--------------------------------------------------------------------------------*/
/* Decalare Variables */
/*--------------------------------------------------------------------------------*/

/* Input/Output Buffers */
ARR_DESC_DECLARE(complex_math_output_fut_a);
ARR_DESC_DECLARE(complex_math_output_fut_b);
ARR_DESC_DECLARE(complex_math_output_ref_a);
ARR_DESC_DECLARE(complex_math_output_ref_b);

extern COMPLEX_MATH_BIGGEST_INPUT_TYPE
complex_math_output_f32_ref_a[COMPLEX_MATH_MAX_INPUT_ELEMENTS * 2];

extern COMPLEX_MATH_BIGGEST_INPUT_TYPE
complex_math_output_f32_ref_b[COMPLEX_MATH_MAX_INPUT_ELEMENTS * 2];

extern COMPLEX_MATH_BIGGEST_INPUT_TYPE
complex_math_output_f32_fut_a[COMPLEX_MATH_MAX_INPUT_ELEMENTS * 2];

extern COMPLEX_MATH_BIGGEST_INPUT_TYPE
complex_math_output_f32_fut_b[COMPLEX_MATH_MAX_INPUT_ELEMENTS * 2];

/* Block Sizes*/
ARR_DESC_DECLARE(complex_math_block_sizes);

/* Float Inputs */
ARR_DESC_DECLARE(complex_math_zeros);
ARR_DESC_DECLARE(complex_math_f_2);
ARR_DESC_DECLARE(complex_math_f_15);
ARR_DESC_DECLARE(complex_math_f_32);
ARR_DESC_DECLARE(complex_math_f_all);


#endif /* _COMPLEX_MATH_TEST_DATA_H_ */
