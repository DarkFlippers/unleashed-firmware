#include "jtest.h"
#include "matrix_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "matrix_templates.h"
#include "type_abbrev.h"

#define JTEST_ARM_MAT_MULT_FAST_TEST(suffix)            \
    MATRIX_DEFINE_TEST_TEMPLATE_ELT2(                   \
        mat_mult_fast,                                  \
        suffix,                                         \
        MATRIX_TEST_CONFIG_MULTIPLICATIVE_OUTPUT,       \
        MATRIX_TEST_VALID_MULTIPLICATIVE_DIMENSIONS,    \
        MATRIX_SNR_COMPARE_INTERFACE)

JTEST_ARM_MAT_MULT_FAST_TEST(q31);

/*--------------------------------------------------------------------------------*/
/* Q15 Uses a Different interface than the others. */
/*--------------------------------------------------------------------------------*/

#define ARM_mat_mult_fast_q15_INPUT_INTERFACE(input_a_ptr, input_b_ptr) \
    PAREN(input_a_ptr, input_b_ptr,                                     \
          (void *) &matrix_output_fut,                                  \
          (q15_t *) matrix_output_scratch)

JTEST_DEFINE_TEST(arm_mat_mult_fast_q15_test, arm_mat_mult_fast_q15)
{
    MATRIX_TEST_TEMPLATE_ELT2(
        matrix_q15_a_inputs,
        matrix_q15_b_inputs,
        arm_matrix_instance_q15 * ,
        arm_matrix_instance_q15,
        TYPE_FROM_ABBREV(q15),
        arm_mat_mult_fast_q15,
        ARM_mat_mult_fast_q15_INPUT_INTERFACE,
        ref_mat_mult_fast_q15,
        REF_mat_mult_fast_INPUT_INTERFACE,
        MATRIX_TEST_CONFIG_MULTIPLICATIVE_OUTPUT,
        MATRIX_TEST_VALID_MULTIPLICATIVE_DIMENSIONS,
        MATRIX_SNR_COMPARE_INTERFACE);
}

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(mat_mult_fast_tests)
{
    /*
      To skip a test, comment it out.
    */
    JTEST_TEST_CALL(arm_mat_mult_fast_q31_test);
    JTEST_TEST_CALL(arm_mat_mult_fast_q15_test);
}
