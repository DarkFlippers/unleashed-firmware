#ifndef _TRANSFORM_TEST_DATA_H_
#define _TRANSFORM_TEST_DATA_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "arr_desc.h"
#include "arm_math.h"

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

#define TRANSFORM_MAX_FFT_LEN 4096
#define TRANFORM_BIGGEST_INPUT_TYPE float32_t

/*--------------------------------------------------------------------------------*/
/* Variable Declarations */
/*--------------------------------------------------------------------------------*/

/* Lengths are multiplied by 2 to accomodate complex numbers*/
extern float32_t transform_fft_output_fut[TRANSFORM_MAX_FFT_LEN * 2];
extern float32_t transform_fft_output_ref[TRANSFORM_MAX_FFT_LEN * 2];
extern float32_t transform_fft_input_fut[TRANSFORM_MAX_FFT_LEN * 2];
extern float32_t transform_fft_input_ref[TRANSFORM_MAX_FFT_LEN * 2];
extern float32_t transform_fft_output_f32_fut[TRANSFORM_MAX_FFT_LEN * 2];
extern float32_t transform_fft_output_f32_ref[TRANSFORM_MAX_FFT_LEN * 2];
extern float32_t * transform_fft_inplace_input_fut;
extern float32_t * transform_fft_inplace_input_ref;
extern float32_t transform_fft_f32_inputs[TRANSFORM_MAX_FFT_LEN * 2];
extern q31_t transform_fft_q31_inputs[TRANSFORM_MAX_FFT_LEN * 2];
extern q15_t * transform_fft_q15_inputs;
extern q15_t dct4_transform_fft_q15_inputs[TRANSFORM_MAX_FFT_LEN * 2];

/* FFT Lengths */
ARR_DESC_DECLARE(transform_radix2_fftlens);
ARR_DESC_DECLARE(transform_radix4_fftlens);
ARR_DESC_DECLARE(transform_rfft_fftlens);
ARR_DESC_DECLARE(transform_rfft_fast_fftlens);
ARR_DESC_DECLARE(transform_dct_fftlens);

/* CFFT Structs */
ARR_DESC_DECLARE(transform_cfft_f32_structs);
ARR_DESC_DECLARE(transform_cfft_q31_structs);
ARR_DESC_DECLARE(transform_cfft_q15_structs);

#endif /* _TRANSFORM_TEST_DATA_H_ */
