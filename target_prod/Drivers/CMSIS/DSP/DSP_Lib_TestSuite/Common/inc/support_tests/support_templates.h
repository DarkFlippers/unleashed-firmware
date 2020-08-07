#ifndef _SUPPORT_TEMPLATES_H_
#define _SUPPORT_TEMPLATES_H_

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
#define SUPPORT_COMPARE_INTERFACE(block_size,   \
                                  output_type)  \
    do                                          \
    {                                           \
        TEST_ASSERT_BUFFERS_EQUAL(              \
            support_output_ref.data_ptr,        \
            support_output_fut.data_ptr,        \
            block_size * sizeof(output_type));  \
    } while (0)                                  \

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

#define ARM_copy_INPUT_INTERFACE(input, block_size)         \
    PAREN(input, support_output_fut.data_ptr, block_size)

#define REF_copy_INPUT_INTERFACE(input, block_size)         \
    PAREN(input, support_output_ref.data_ptr, block_size)

#define ARM_fill_INPUT_INTERFACE(elt, block_size)       \
    PAREN(elt, support_output_fut.data_ptr, block_size)

#define REF_fill_INPUT_INTERFACE(elt, block_size)       \
    PAREN(elt, support_output_ref.data_ptr, block_size)

#define ARM_x_to_y_INPUT_INTERFACE(input, block_size)       \
    PAREN(input, support_output_fut.data_ptr, block_size)

#define REF_x_to_y_INPUT_INTERFACE(input, block_size)       \
    PAREN(input, support_output_ref.data_ptr, block_size)

/*--------------------------------------------------------------------------------*/
/* Test Templates */
/*--------------------------------------------------------------------------------*/


/**
 *  Specialization of #TEST_TEMPLATE_BUF1_BLK() for support tests.
 *
 *  @note This macro relies on the existance of ARM_xxx_INPUT_INTERFACE and
 *  REF_xxx_INPUT_INTERFACEs.
 */
#define SUPPORT_DEFINE_TEST_TEMPLATE_BUF1_BLK(fn_name,              \
                                              suffix,               \
                                              input_type,           \
                                              output_type,          \
                                              comparison_interface) \
    JTEST_DEFINE_TEST(arm_##fn_name##_##suffix##_test,              \
                      arm_##fn_name##_##suffix)                     \
    {                                                               \
        TEST_TEMPLATE_BUF1_BLK(                                     \
            support_f_all,                                          \
            support_block_sizes,                                    \
            input_type,                                             \
            output_type,                                            \
            arm_##fn_name##_##suffix,                               \
            ARM_##fn_name##_INPUT_INTERFACE,                        \
            ref_##fn_name##_##suffix,                               \
            REF_##fn_name##_INPUT_INTERFACE,                        \
            comparison_interface);                                  \
    }

/**
 *  Specialization of #TEST_TEMPLATE_ELT1_BLK() for support tests.
 *
 *  @note This macro relies on the existance of ARM_xxx_INPUT_INTERFACE and
 *  REF_xxx_INPUT_INTERFACEs.
 */
#define SUPPORT_DEFINE_TEST_TEMPLATE_ELT1_BLK(fn_name,              \
                                              suffix,               \
                                              elt_type,             \
                                              output_type,          \
                                              comparison_interface) \
    JTEST_DEFINE_TEST(arm_##fn_name##_##suffix##_test,              \
                      arm_##fn_name##_##suffix)                     \
    {                                                               \
        TEST_TEMPLATE_ELT1_BLK(                                     \
            support_elts,                                           \
            support_block_sizes,                                    \
            elt_type,                                               \
            output_type,                                            \
            arm_##fn_name##_##suffix,                               \
            ARM_##fn_name##_INPUT_INTERFACE,                        \
            ref_##fn_name##_##suffix,                               \
            REF_##fn_name##_INPUT_INTERFACE,                        \
            comparison_interface);                                  \
    }

#endif /* _SUPPORT_TEMPLATES_H_ */
