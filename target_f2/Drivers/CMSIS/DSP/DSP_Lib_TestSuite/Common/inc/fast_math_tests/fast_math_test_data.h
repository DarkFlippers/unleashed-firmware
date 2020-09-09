#ifndef _FAST_MATH_TEST_DATA_H_
#define _FAST_MATH_TEST_DATA_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "arm_math.h"

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

#define FAST_MATH_MAX_LEN 1024
#define TRANFORM_BIGGEST_INPUT_TYPE float32_t

/*--------------------------------------------------------------------------------*/
/* Variable Declarations */
/*--------------------------------------------------------------------------------*/

extern float32_t fast_math_output_fut[FAST_MATH_MAX_LEN];
extern float32_t fast_math_output_ref[FAST_MATH_MAX_LEN];
extern float32_t fast_math_output_f32_fut[FAST_MATH_MAX_LEN];
extern float32_t fast_math_output_f32_ref[FAST_MATH_MAX_LEN];
extern const float32_t fast_math_f32_inputs[FAST_MATH_MAX_LEN];
extern const q31_t fast_math_q31_inputs[FAST_MATH_MAX_LEN];
extern const q15_t * fast_math_q15_inputs;

#endif /* _FAST_MATH_TEST_DATA_H_ */
