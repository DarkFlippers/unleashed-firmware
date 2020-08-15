#ifndef _COMPLEX_MATH_TEMPLATES_H_
#define _COMPLEX_MATH_TEMPLATES_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/
#include "test_templates.h"

/*--------------------------------------------------------------------------------*/
/* Group Specific Templates */
/*--------------------------------------------------------------------------------*/

/**
 *  Compare the real outputs from the function under test and the reference
 *  function.
 */
#define COMPLEX_MATH_COMPARE_RE_INTERFACE(block_size, output_type)  \
    TEST_ASSERT_BUFFERS_EQUAL(                                      \
        complex_math_output_ref_a.data_ptr,                         \
        complex_math_output_fut_a.data_ptr,                         \
        block_size * sizeof(output_type))

/**
 *  Compare the real and imaginary outputs from the function under test and the
 *  reference function.
 */
#define COMPLEX_MATH_COMPARE_CMPLX_INTERFACE(block_size, output_type)   \
    do                                                                  \
    {                                                                   \
        COMPLEX_MATH_COMPARE_RE_INTERFACE(block_size * 2, output_type); \
    } while (0)


/*
 * Comparison SNR thresholds for the data types used in complex_math_tests.
 */
#define COMPLEX_MATH_SNR_THRESHOLD_float32_t 120
#define COMPLEX_MATH_SNR_THRESHOLD_q31_t 100
#define COMPLEX_MATH_SNR_THRESHOLD_q15_t 75

/**
 *  Compare reference and fut outputs using SNR.
 *
 *  The output_suffix specifies which output buffers to use for the
 *  comparison. An output_suffix of 'a' expands to the following buffers:
 *
 *  - complex_math_output_f32_ref_a
 *  - complex_math_output_f32_fut_a
 *  - complex_math_output_ref_a
 *  - complex_math_output_fut_a
 *
 *  @note The outputs are converted to float32_t before comparison.
 */
#define COMPLEX_MATH_SNR_COMPARE_OUT_INTERFACE(block_size,      \
                                               output_type,     \
                                               output_suffix)   \
    do                                                          \
    {                                                           \
        TEST_CONVERT_AND_ASSERT_SNR(                            \
            complex_math_output_f32_ref_##output_suffix,        \
            complex_math_output_ref_##output_suffix.data_ptr,   \
            complex_math_output_f32_fut_##output_suffix,        \
            complex_math_output_fut_##output_suffix.data_ptr,   \
            block_size,                                         \
            output_type,                                        \
            COMPLEX_MATH_SNR_THRESHOLD_##output_type            \
            );                                                  \
    } while (0)

/**
 *  Specification of #COMPLEX_MATH_SNR_COMPARE_INTERFACE() for real outputs.
 */
#define COMPLEX_MATH_SNR_COMPARE_RE_INTERFACE(block_size,       \
                                                   output_type) \
    COMPLEX_MATH_SNR_COMPARE_OUT_INTERFACE(block_size,          \
                                           output_type,         \
                                           a)

/**
 *  Specification of #COMPLEX_MATH_SNR_COMPARE_INTERFACE() for complex outputs.
 */
#define COMPLEX_MATH_SNR_COMPARE_CMPLX_INTERFACE(block_size,    \
                                                 output_type)   \
        COMPLEX_MATH_SNR_COMPARE_OUT_INTERFACE(block_size * 2,  \
                                               output_type,     \
                                               a)

/**
 *  Compare reference and fut split outputs using SNR.
 *
 *  'Split' refers to two separate output buffers; one for real and one for
 *  complex.
 */
#define COMPLEX_MATH_SNR_COMPARE_SPLIT_INTERFACE(block_size,    \
                                                 output_type)   \
        do                                                      \
        {                                                       \
            COMPLEX_MATH_SNR_COMPARE_OUT_INTERFACE(block_size,  \
                                                   output_type, \
                                                   a);          \
            COMPLEX_MATH_SNR_COMPARE_OUT_INTERFACE(block_size,  \
                                                   output_type, \
                                                   b);          \
        } while (0)


/*--------------------------------------------------------------------------------*/
/* Input Interfaces */
/*--------------------------------------------------------------------------------*/
/*
 *  General:
 *  Input interfaces provide inputs to functions inside test templates.  They
 *  ONLY provide the inputs.  The output variables should be hard coded.
 *
 *  The input interfaces must have the following format:
 *
 *  ARM_xxx_INPUT_INTERFACE() or
 *  REF_xxx_INPUT_INTERFACE()
 *
 *  The xxx must be lowercase, and is intended to be the indentifying substring
 *  in the function's name.  Acceptable values are 'sub' or 'add' from the
 *  functions arm_add_q31.
 */

#define ARM_cmplx_conj_INPUT_INTERFACE(input, block_size)           \
    PAREN(input, complex_math_output_fut_a.data_ptr, block_size)

#define REF_cmplx_conj_INPUT_INTERFACE(input, block_size)           \
    PAREN(input, complex_math_output_ref_a.data_ptr, block_size)

#define ARM_cmplx_dot_prod_INPUT_INTERFACE(input_a, input_b, block_size) \
    PAREN(input_a, input_b, block_size,                                 \
          complex_math_output_fut_a.data_ptr,                          \
          complex_math_output_fut_b.data_ptr)

#define REF_cmplx_dot_prod_INPUT_INTERFACE(input_a, input_b, block_size) \
    PAREN(input_a, input_b, block_size,                                 \
          complex_math_output_ref_a.data_ptr,                          \
          complex_math_output_ref_b.data_ptr)

#define ARM_cmplx_mag_INPUT_INTERFACE(input, block_size)            \
    PAREN(input, complex_math_output_fut_a.data_ptr, block_size)

#define REF_cmplx_mag_INPUT_INTERFACE(input, block_size)            \
    PAREN(input, complex_math_output_ref_a.data_ptr, block_size)

#define ARM_cmplx_mag_squared_INPUT_INTERFACE(input, block_size)    \
    PAREN(input, complex_math_output_fut_a.data_ptr, block_size)

#define REF_cmplx_mag_squared_INPUT_INTERFACE(input, block_size)    \
    PAREN(input, complex_math_output_ref_a.data_ptr, block_size)

#define ARM_cmplx_mult_cmplx_INPUT_INTERFACE(input_a, input_b, block_size) \
    PAREN(input_a, input_b, complex_math_output_fut_a.data_ptr, block_size)

#define REF_cmplx_mult_cmplx_INPUT_INTERFACE(input_a, input_b, block_size) \
    PAREN(input_a, input_b, complex_math_output_ref_a.data_ptr, block_size)

#define ARM_cmplx_mult_real_INPUT_INTERFACE(input_a, input_b, block_size) \
    PAREN(input_a, input_b, complex_math_output_fut_a.data_ptr, block_size)

#define REF_cmplx_mult_real_INPUT_INTERFACE(input_a, input_b, block_size) \
    PAREN(input_a, input_b, complex_math_output_ref_a.data_ptr, block_size)

/*--------------------------------------------------------------------------------*/
/* Test Templates */
/*--------------------------------------------------------------------------------*/

/**
 *  Specialization of #TEST_TEMPLATE_BUF1_BLK() for complex math tests.
 *
 *  @note This macro relies on the existance of ARM_xxx_INPUT_INTERFACE and
 *  REF_xxx_INPUT_INTERFACEs.
 */
#define COMPLEX_MATH_DEFINE_TEST_TEMPLATE_BUF1_BLK(fn_name,         \
                                                   suffix,          \
                                                   input_type,      \
                                                   output_type,     \
                                                   comparison_interface) \
    JTEST_DEFINE_TEST(arm_##fn_name##_##suffix##_test,              \
                      arm_##fn_name##_##suffix)                     \
    {                                                               \
        TEST_TEMPLATE_BUF1_BLK(                                     \
            complex_math_f_all,                                     \
            complex_math_block_sizes,                               \
            input_type,                                             \
            output_type,                                            \
            arm_##fn_name##_##suffix,                               \
            ARM_##fn_name##_INPUT_INTERFACE,                        \
            ref_##fn_name##_##suffix,                               \
            REF_##fn_name##_INPUT_INTERFACE,                        \
            comparison_interface);                                  \
    }

/**
 *  Specialization of #TEST_TEMPLATE_BUF2_BLK1() for complex math tests.
 *
 *  @note This macro relies on the existance of ARM_xxx_INPUT_INTERFACE and
 *  REF_xxx_INPUT_INTERFACEs.
 */
#define COMPLEX_MATH_DEFINE_TEST_TEMPLATE_BUF2_BLK(fn_name,         \
                                                   suffix,          \
                                                   input_type,      \
                                                   output_type,     \
                                                   comparison_interface) \
    JTEST_DEFINE_TEST(arm_##fn_name##_##suffix##_test,              \
                      arm_##fn_name##_##suffix)                     \
    {                                                               \
        TEST_TEMPLATE_BUF2_BLK(                                     \
            complex_math_f_all,                                     \
            complex_math_f_all,                                     \
            complex_math_block_sizes,                               \
            input_type,                                             \
            output_type,                                            \
            arm_##fn_name##_##suffix,                               \
            ARM_##fn_name##_INPUT_INTERFACE,                        \
            ref_##fn_name##_##suffix,                               \
            REF_##fn_name##_INPUT_INTERFACE,                        \
            comparison_interface);                                  \
    }

#endif /* _COMPLEX_MATH_TEMPLATES_H_ */
