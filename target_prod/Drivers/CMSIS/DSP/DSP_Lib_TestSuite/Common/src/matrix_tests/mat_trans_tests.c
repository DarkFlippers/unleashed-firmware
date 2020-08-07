#include "jtest.h"
#include "matrix_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "matrix_templates.h"
#include "type_abbrev.h"

#define JTEST_ARM_MAT_TRANS_TEST(suffix)        \
    MATRIX_DEFINE_TEST_TEMPLATE_ELT1(           \
        mat_trans,                              \
        suffix,                                 \
        MATRIX_TEST_CONFIG_TRANSPOSE_OUTPUT,    \
        MATRIX_TEST_VALID_DIMENSIONS_ALWAYS)

JTEST_ARM_MAT_TRANS_TEST(f32);
JTEST_ARM_MAT_TRANS_TEST(q31);
JTEST_ARM_MAT_TRANS_TEST(q15);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(mat_trans_tests)
{
    /*
      To skip a test, comment it out.
    */
    JTEST_TEST_CALL(arm_mat_trans_f32_test);
    JTEST_TEST_CALL(arm_mat_trans_q31_test);
    JTEST_TEST_CALL(arm_mat_trans_q15_test);
}
