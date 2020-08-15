#ifndef _TRANSFORM_TEMPLATES_H_
#define _TRANSFORM_TEMPLATES_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "test_templates.h"
#include <string.h>             /* memcpy() */

/*--------------------------------------------------------------------------------*/
/* Group Specific Templates */
/*--------------------------------------------------------------------------------*/

/**
 * Comparison SNR thresholds for the data types used in transform_tests.
 */
#define TRANSFORM_SNR_THRESHOLD_float32_t 90
#define TRANSFORM_SNR_THRESHOLD_q31_t     90
#define TRANSFORM_SNR_THRESHOLD_q15_t     30

#define DCT4_TRANSFORM_SNR_THRESHOLD_float32_t 80
#define DCT4_TRANSFORM_SNR_THRESHOLD_q31_t     75
#define DCT4_TRANSFORM_SNR_THRESHOLD_q15_t     11

/**
 *  Compare the outputs from the function under test and the reference
 *  function using SNR.
 */
#define TRANSFORM_SNR_COMPARE_INTERFACE(block_size,     \
                                        output_type)    \
    do                                                  \
    {                                                   \
        TEST_CONVERT_AND_ASSERT_SNR(                    \
            transform_fft_output_f32_ref,               \
            (output_type *) transform_fft_output_ref,   \
            transform_fft_output_f32_fut,               \
            (output_type *) transform_fft_output_fut,   \
            block_size,                                 \
            output_type,                                \
            TRANSFORM_SNR_THRESHOLD_##output_type       \
            );                                          \
    } while (0)
    
/**
 *  Compare the outputs from the function under test and the reference
 *  function using SNR.
 */
#define DCT_TRANSFORM_SNR_COMPARE_INTERFACE(block_size,  \
                                            output_type) \
    do                                                   \
    {                                                    \
        TEST_CONVERT_AND_ASSERT_SNR(                     \
            transform_fft_output_f32_ref,                \
            (output_type *) transform_fft_output_ref,    \
            transform_fft_output_f32_fut,                \
            (output_type *) transform_fft_output_fut,    \
            block_size,                                  \
            output_type,                                 \
            DCT4_TRANSFORM_SNR_THRESHOLD_##output_type   \
            );                                           \
    } while (0)                                           \

/**
 *  Specialization on #TRANSFORM_SNR_COMPARE_INTERFACE() to fix the block_size
 *  for complex datasets.
 */
#define TRANSFORM_SNR_COMPARE_CMPLX_INTERFACE(block_size, output_type)  \
    /* Complex numbers have two components*/                            \
    TRANSFORM_SNR_COMPARE_INTERFACE(block_size * 2, output_type )

/**
 * This macro copys data from the input_ptr into input arrays.
 *
 * Some functions modify their input data; in order to provide the same data to
 * multiple tests, copies must be made so the changes from one function don't
 * impact the others.
 */
#define TRANSFORM_COPY_INPUTS(input_ptr,        \
                              bytes)            \
    do                                          \
    {                                           \
        memcpy(                                 \
            transform_fft_input_fut,            \
            input_ptr,                          \
            bytes);                             \
        memcpy(                                 \
            transform_fft_input_ref,            \
            input_ptr,                          \
            bytes);                             \
    } while (0)

/**
 * This macro copys data from the input_ptr into input arrays. It also creates
 * symmetric input data for rfft inverse.
 *
 * The 4.534234f just makes the middle entry of the array semi random.  It's
 * actual value doesn't seem to matter much.
 *
 * Some functions modify their input data; in order to provide the same data to
 * multiple tests, copies must be made so the changes from one function don't
 * impact the others.
 */
#define TRANSFORM_PREPARE_INVERSE_INPUTS(input_ptr,                              \
                              fftlen, input_type, bytes)                         \
    do                                                                           \
    {                                                                            \
        uint32_t i;                                                              \
                                                                                 \
        memcpy(                                                                  \
            transform_fft_input_fut,                                             \
            input_ptr,                                                           \
            bytes);                                                              \
                                                                                 \
        ((input_type*)transform_fft_input_fut)[1] = 0;                           \
        ((input_type*)transform_fft_input_fut)[fftlen + 0] = 0;                  \
        ((input_type*)transform_fft_input_fut)[fftlen + 1] = 0;                  \
        for(i=1;i<fftlen/2;i++)                                                  \
        {                                                                        \
           *((input_type*)transform_fft_input_fut + fftlen + 2*i + 0) =          \
               *((input_type*)transform_fft_input_fut + fftlen - 2*i + 0);       \
           *((input_type*)transform_fft_input_fut + fftlen + 2*i + 1) =          \
               -(*((input_type*)transform_fft_input_fut + fftlen - 2*i + 1));    \
                                                                                 \
        }                                                                        \
                                                                                 \
        memcpy(                                                                  \
            transform_fft_input_ref,                                             \
            transform_fft_input_fut,                                             \
            bytes * 2);                                                          \
    } while (0)

/**
 * This macro copys data from the input_ptr into the in-place input arrays.
 *
 * Some functions modify their input data; in order to provide the same data to
 * multiple tests, copies must be made so the changes from one function don't
 * impact the others.
 */
#define TRANSFORM_PREPARE_INPLACE_INPUTS_DOWNSHIFT(input_ptr,       \
                                         bytes,                     \
                                         type)                      \
    do                                                              \
    {                                                               \
        uint32_t i;                                                 \
        memcpy(                                                     \
            transform_fft_inplace_input_fut,                        \
            input_ptr,                                              \
            bytes);                                                 \
        memcpy(                                                     \
            transform_fft_inplace_input_ref,                        \
            input_ptr,                                              \
            bytes);                                                 \
        for(i=0;i<bytes/sizeof(type);i++) {                         \
            *((type*)transform_fft_inplace_input_fut + i) >>= 1;    \
            *((type*)transform_fft_inplace_input_ref + i) >>= 1;}   \
    } while (0)

/**
 * This macro copys data from the input_ptr into the in-place input arrays.
 *
 * Some functions modify their input data; in order to provide the same data to
 * multiple tests, copies must be made so the changes from one function don't
 * impact the others.
 */
#define TRANSFORM_PREPARE_INPLACE_INPUTS(input_ptr, \
                                         bytes)     \
    do                                              \
    {                                               \
        memcpy(                                     \
            transform_fft_inplace_input_fut,        \
            input_ptr,                              \
            bytes);                                 \
        memcpy(                                     \
            transform_fft_inplace_input_ref,        \
            input_ptr,                              \
            bytes);                                 \
    } while (0)


#endif /* _TRANSFORM_TEMPLATES_H_ */
