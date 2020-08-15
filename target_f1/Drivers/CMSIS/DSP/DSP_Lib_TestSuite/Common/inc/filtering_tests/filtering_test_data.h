#ifndef FILTERING_TEST_DATA_H
#define FILTERING_TEST_DATA_H

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "arr_desc.h"
#include "arm_math.h"

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

#define FILTERING_MAX_BLOCKSIZE  33
#define LMS_MAX_BLOCKSIZE        512
#define FILTERING_MAX_NUMTAPS		34
#define FILTERING_MAX_NUMSTAGES  14
#define FILTERING_MAX_POSTSHIFT  8
#define FILTERING_MAX_TAP_DELAY	0xFF
#define FILTERING_MAX_L				3
#define FILTERING_MAX_M				33

/*--------------------------------------------------------------------------------*/
/* Declare Variables */
/*--------------------------------------------------------------------------------*/

/* Input/Output Buffers */
extern float32_t filtering_output_fut[LMS_MAX_BLOCKSIZE*2];
extern float32_t filtering_output_ref[LMS_MAX_BLOCKSIZE*2];
extern float32_t filtering_output_f32_fut[LMS_MAX_BLOCKSIZE*2];
extern float32_t filtering_output_f32_ref[LMS_MAX_BLOCKSIZE*2];
extern float32_t filtering_input_lms[LMS_MAX_BLOCKSIZE*2];
extern float32_t filtering_pState[LMS_MAX_BLOCKSIZE + FILTERING_MAX_NUMTAPS];
extern float32_t filtering_scratch[FILTERING_MAX_BLOCKSIZE * 3];
extern float32_t filtering_scratch2[FILTERING_MAX_BLOCKSIZE * 3];
extern float32_t filtering_coeffs_lms[FILTERING_MAX_NUMTAPS];

extern const float64_t filtering_f64_inputs[FILTERING_MAX_BLOCKSIZE * FILTERING_MAX_M + FILTERING_MAX_NUMTAPS];
extern const float32_t filtering_f32_inputs[FILTERING_MAX_BLOCKSIZE * FILTERING_MAX_M + FILTERING_MAX_NUMTAPS];
extern const q31_t filtering_q31_inputs[FILTERING_MAX_BLOCKSIZE * FILTERING_MAX_M + FILTERING_MAX_NUMTAPS];
extern const q15_t * filtering_q15_inputs;
extern const q7_t * filtering_q7_inputs;

/* Block Sizes */
ARR_DESC_DECLARE(filtering_blocksizes);
ARR_DESC_DECLARE(lms_blocksizes);
ARR_DESC_DECLARE(filtering_numtaps);
ARR_DESC_DECLARE(filtering_numtaps2);
ARR_DESC_DECLARE(filtering_postshifts);
ARR_DESC_DECLARE(filtering_numstages);
ARR_DESC_DECLARE(filtering_Ls);
ARR_DESC_DECLARE(filtering_Ms);

/* Coefficient Lists */
extern const float64_t filtering_coeffs_f64[FILTERING_MAX_NUMSTAGES * 6 + 2];
extern const float64_t filtering_coeffs_b_f64[FILTERING_MAX_NUMSTAGES * 6 + 2];
extern const float32_t filtering_coeffs_f32[FILTERING_MAX_NUMSTAGES * 6 + 2];
extern const float32_t filtering_coeffs_b_f32[FILTERING_MAX_NUMSTAGES * 6 + 2];
extern const float32_t *filtering_coeffs_c_f32;
extern float32_t filtering_coeffs_lms_f32[FILTERING_MAX_NUMTAPS];
extern const q31_t filtering_coeffs_q31[FILTERING_MAX_NUMSTAGES * 6 + 2];
extern const q31_t *filtering_coeffs_b_q31;
extern const q31_t *filtering_coeffs_c_q31;
extern q31_t filtering_coeffs_lms_q31[FILTERING_MAX_NUMTAPS];
extern const q15_t filtering_coeffs_q15[FILTERING_MAX_NUMSTAGES * 6 + 4];
extern const q15_t *filtering_coeffs_b_q15;
extern const q15_t *filtering_coeffs_c_q15;
extern q15_t filtering_coeffs_lms_q15[FILTERING_MAX_NUMTAPS];
extern const q7_t filtering_coeffs_q7[FILTERING_MAX_NUMSTAGES * 6 + 8];
extern const q7_t *filtering_coeffs_b_q7;
extern const q7_t *filtering_coeffs_c_q7;

/* Tap Delay Lists */
extern const int32_t filtering_tap_delay[FILTERING_MAX_NUMTAPS];

/* Numbers */

/* Float Inputs */

#endif
