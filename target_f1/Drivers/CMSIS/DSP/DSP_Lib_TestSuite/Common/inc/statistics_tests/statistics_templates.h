#ifndef _STATISTICS_TEMPLATES_H_
#define _STATISTICS_TEMPLATES_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "test_templates.h"

/*--------------------------------------------------------------------------------*/
/* Group Specific Templates */
/*--------------------------------------------------------------------------------*/

/**
 *  Compare the outputs from the function under test and the reference function.
 */
#define STATISTICS_COMPARE_INTERFACE(block_size,    \
                                     output_type)   \
    do                                              \
    {                                               \
        TEST_ASSERT_BUFFERS_EQUAL(                  \
            statistics_output_ref.data_ptr,         \
            statistics_output_fut.data_ptr,         \
            1 * sizeof(output_type) /* All fns return one value*/   \
            );                                      \
        TEST_ASSERT_EQUAL(                          \
            statistics_idx_fut,                     \
            statistics_idx_ref);                    \
    } while (0)                                      \

/*
 * Comparison SNR thresholds for the data types used in statistics_tests.
 */
#define STATISTICS_SNR_THRESHOLD_float32_t 120
#define STATISTICS_SNR_THRESHOLD_q31_t 100
#define STATISTICS_SNR_THRESHOLD_q15_t 60
#define STATISTICS_SNR_THRESHOLD_q7_t 30

/**
 *  Compare reference and fut outputs using SNR.
 *
 *  @note The outputs are converted to float32_t before comparison.
 */
#define STATISTICS_SNR_COMPARE_INTERFACE(block_size,    \
                                         output_type)   \
    do                                                  \
    {                                                   \
        TEST_CONVERT_AND_ASSERT_SNR(                    \
            statistics_output_f32_ref,                  \
            statistics_output_ref.data_ptr,             \
            statistics_output_f32_fut,                  \
            statistics_output_fut.data_ptr,             \
                1, /* All fns return one element*/      \
            output_type,                                \
            STATISTICS_SNR_THRESHOLD_##output_type      \
            );                                          \
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

#define ARM_max_INPUT_INTERFACE(input, block_size)              \
    PAREN(input, block_size,                                    \
          statistics_output_fut.data_ptr, &statistics_idx_fut)

#define REF_max_INPUT_INTERFACE(input, block_size)              \
    PAREN(input, block_size,                                    \
          statistics_output_ref.data_ptr, &statistics_idx_ref)

#define ARM_mean_INPUT_INTERFACE(input, block_size)             \
    PAREN(input, block_size, statistics_output_fut.data_ptr)

#define REF_mean_INPUT_INTERFACE(input, block_size)             \
    PAREN(input, block_size, statistics_output_ref.data_ptr)

#define ARM_min_INPUT_INTERFACE(input, block_size)              \
    PAREN(input, block_size,                                    \
          statistics_output_fut.data_ptr, &statistics_idx_fut)

#define REF_min_INPUT_INTERFACE(input, block_size)              \
    PAREN(input, block_size,                                    \
          statistics_output_ref.data_ptr, &statistics_idx_ref)

#define ARM_power_INPUT_INTERFACE(input, block_size)            \
    PAREN(input, block_size, statistics_output_fut.data_ptr)

#define REF_power_INPUT_INTERFACE(input, block_size)            \
    PAREN(input, block_size, statistics_output_ref.data_ptr)

#define ARM_rms_INPUT_INTERFACE(input, block_size)              \
    PAREN(input, block_size, statistics_output_fut.data_ptr)

#define REF_rms_INPUT_INTERFACE(input, block_size)              \
    PAREN(input, block_size, statistics_output_ref.data_ptr)

#define ARM_std_INPUT_INTERFACE(input, block_size)              \
    PAREN(input, block_size, statistics_output_fut.data_ptr)

#define REF_std_INPUT_INTERFACE(input, block_size)              \
    PAREN(input, block_size, statistics_output_ref.data_ptr)

#define ARM_var_INPUT_INTERFACE(input, block_size)              \
    PAREN(input, block_size, statistics_output_fut.data_ptr)

#define REF_var_INPUT_INTERFACE(input, block_size)              \
    PAREN(input, block_size, statistics_output_ref.data_ptr)


/*--------------------------------------------------------------------------------*/
/* Test Templates */
/*--------------------------------------------------------------------------------*/

/**
 *  Specialization of #TEST_TEMPLATE_BUF1_BLK() for statistics tests.
 *
 *  @note This macro relies on the existance of ARM_xxx_INPUT_INTERFACE and
 *  REF_xxx_INPUT_INTERFACEs.
 */
#define STATISTICS_DEFINE_TEST_TEMPLATE_BUF1_BLK(fn_name,       \
                                                 suffix,        \
                                                 input_type,    \
                                                 output_type,   \
                                                 comparison_interface)  \
    JTEST_DEFINE_TEST(arm_##fn_name##_##suffix##_test,          \
                      arm_##fn_name##_##suffix)                 \
    {                                                           \
        TEST_TEMPLATE_BUF1_BLK(                                 \
            statistics_f_all,                                   \
            statistics_block_sizes,                             \
            input_type,                                         \
            output_type,                                        \
            arm_##fn_name##_##suffix,                           \
            ARM_##fn_name##_INPUT_INTERFACE,                    \
            ref_##fn_name##_##suffix,                           \
            REF_##fn_name##_INPUT_INTERFACE,                    \
            comparison_interface);                              \
    }


#endif /* _STATISTICS_TEMPLATES_H_ */
