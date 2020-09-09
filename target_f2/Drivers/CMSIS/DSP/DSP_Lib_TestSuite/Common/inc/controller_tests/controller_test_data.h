#ifndef _CONTROLLER_TEST_DATA_H_
#define _CONTROLLER_TEST_DATA_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "arm_math.h"

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

#define CONTROLLER_MAX_LEN 1024
#define CONTROLLER_MAX_COEFFS_LEN (12 * 3)
#define TRANFORM_BIGGEST_INPUT_TYPE float32_t

/*--------------------------------------------------------------------------------*/
/* Variable Declarations */
/*--------------------------------------------------------------------------------*/

extern float32_t controller_output_fut[CONTROLLER_MAX_LEN];
extern float32_t controller_output_ref[CONTROLLER_MAX_LEN];
extern float32_t controller_output_f32_fut[CONTROLLER_MAX_LEN];
extern float32_t controller_output_f32_ref[CONTROLLER_MAX_LEN];
extern const float32_t controller_f32_inputs[CONTROLLER_MAX_LEN];
extern const q31_t controller_q31_inputs[CONTROLLER_MAX_LEN];
extern const q15_t * controller_q15_inputs;
extern const float32_t controller_f32_coeffs[CONTROLLER_MAX_COEFFS_LEN];
extern const q31_t controller_q31_coeffs[CONTROLLER_MAX_COEFFS_LEN];
extern const q15_t controller_q15_coeffs[CONTROLLER_MAX_COEFFS_LEN];

#endif /* _CONTROLLER_TEST_DATA_H_ */
