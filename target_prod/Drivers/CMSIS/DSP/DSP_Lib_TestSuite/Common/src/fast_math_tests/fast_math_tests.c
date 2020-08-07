#include "jtest.h"
#include "ref.h"
#include "arr_desc.h"
#include "fast_math_templates.h"
#include "fast_math_test_data.h"
#include "type_abbrev.h"

SQRT_TEST_TEMPLATE_ELT1(q31);
SQRT_TEST_TEMPLATE_ELT1(q15);

SIN_COS_TEST_TEMPLATE_ELT1(f32, float32_t, sin);
SIN_COS_TEST_TEMPLATE_ELT1(q31, q31_t, sin);
SIN_COS_TEST_TEMPLATE_ELT1(q15, q15_t, sin);

SIN_COS_TEST_TEMPLATE_ELT1(f32, float32_t, cos);
SIN_COS_TEST_TEMPLATE_ELT1(q31, q31_t, cos);
SIN_COS_TEST_TEMPLATE_ELT1(q15, q15_t, cos);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(fast_math_tests)
{
    /*
      To skip a test, comment it out.
    */
    JTEST_TEST_CALL(arm_sqrt_q31_test);
    JTEST_TEST_CALL(arm_sqrt_q15_test);

    JTEST_TEST_CALL(arm_sin_f32_test);
    JTEST_TEST_CALL(arm_sin_q31_test);
    JTEST_TEST_CALL(arm_sin_q15_test);

    JTEST_TEST_CALL(arm_cos_f32_test);
    JTEST_TEST_CALL(arm_cos_q31_test);
    JTEST_TEST_CALL(arm_cos_q15_test);
}
