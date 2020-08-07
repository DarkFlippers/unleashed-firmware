#ifndef _INTRINSICS_TEST_DATA_H_
#define _INTRINSICS_TEST_DATA_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "arm_math.h"

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

#define INTRINSICS_MAX_LEN 1024

/*--------------------------------------------------------------------------------*/
/* Variable Declarations */
/*--------------------------------------------------------------------------------*/

extern q63_t intrinsics_output_fut[INTRINSICS_MAX_LEN];
extern q63_t intrinsics_output_ref[INTRINSICS_MAX_LEN];
extern float32_t intrinsics_output_f32_fut[INTRINSICS_MAX_LEN];
extern float32_t intrinsics_output_f32_ref[INTRINSICS_MAX_LEN];
extern const q63_t intrinsics_q63_inputs[INTRINSICS_MAX_LEN];
extern const q31_t *intrinsics_q31_inputs;

#endif /* _INTRINSICS_TEST_DATA_H_ */
