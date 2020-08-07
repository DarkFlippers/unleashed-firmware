#include "jtest.h"
#include "matrix_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "matrix_templates.h"
#include "type_abbrev.h"

#define JTEST_ARM_MAT_ADD_TEST(suffix)          \
    MATRIX_DEFINE_TEST_TEMPLATE_ELT2(           \
        mat_add,                                \
        suffix,                                 \
        MATRIX_TEST_CONFIG_ADDITIVE_OUTPUT,     \
        MATRIX_TEST_VALID_ADDITIVE_DIMENSIONS,  \
        MATRIX_COMPARE_INTERFACE)

JTEST_ARM_MAT_ADD_TEST(f32);
JTEST_ARM_MAT_ADD_TEST(q31);
JTEST_ARM_MAT_ADD_TEST(q15);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(mat_add_tests)
{
    JTEST_TEST_CALL(arm_mat_add_f32_test);
    JTEST_TEST_CALL(arm_mat_add_q31_test);
    JTEST_TEST_CALL(arm_mat_add_q15_test);
}
