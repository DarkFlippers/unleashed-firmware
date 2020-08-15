#ifndef _MATRIX_TEST_DATA_H_
#define _MATRIX_TEST_DATA_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "arr_desc.h"
#include "arm_math.h"           /* float32_t */

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/
#define MATRIX_TEST_MAX_ROWS 4
#define MATRIX_TEST_MAX_COLS 4
#define MATRIX_TEST_BIGGEST_INPUT_TYPE float64_t
#define MATRIX_TEST_MAX_ELTS (MATRIX_TEST_MAX_ROWS * MATRIX_TEST_MAX_COLS)
#define MATRIX_MAX_COEFFS_LEN 16
#define MATRIX_MAX_SHIFTS_LEN 5

/**
 *  Declare the matrix inputs defined by MATRIX_DEFINE_INPUTS.
 */
#define MATRIX_DECLARE_INPUTS(suffix)               \
    ARR_DESC_DECLARE(matrix_##suffix##_a_inputs);   \
    ARR_DESC_DECLARE(matrix_##suffix##_b_inputs);   \
    ARR_DESC_DECLARE(matrix_##suffix##_invertible_inputs)


/*--------------------------------------------------------------------------------*/
/* Declare Variables */
/*--------------------------------------------------------------------------------*/

/* Input/Output Buffers */
extern arm_matrix_instance_f32 matrix_output_fut;
extern arm_matrix_instance_f32 matrix_output_ref;
extern arm_matrix_instance_f64 matrix_output_fut64;
extern arm_matrix_instance_f64 matrix_output_ref64;
extern MATRIX_TEST_BIGGEST_INPUT_TYPE matrix_output_f32_fut[MATRIX_TEST_MAX_ELTS];
extern MATRIX_TEST_BIGGEST_INPUT_TYPE matrix_output_f32_ref[MATRIX_TEST_MAX_ELTS];
extern MATRIX_TEST_BIGGEST_INPUT_TYPE matrix_output_scratch[MATRIX_TEST_MAX_ELTS];

/* Matrix Inputs */
MATRIX_DECLARE_INPUTS(f64);
MATRIX_DECLARE_INPUTS(f32);
MATRIX_DECLARE_INPUTS(q31);
MATRIX_DECLARE_INPUTS(q15);

extern const float32_t matrix_f32_scale_values[MATRIX_MAX_COEFFS_LEN];
extern const q31_t matrix_q31_scale_values[MATRIX_MAX_COEFFS_LEN];
extern const q15_t matrix_q15_scale_values[MATRIX_MAX_COEFFS_LEN];
extern const int32_t matrix_shift_values[MATRIX_MAX_SHIFTS_LEN];

#endif /* _MATRIX_TEST_DATA_H_ */
