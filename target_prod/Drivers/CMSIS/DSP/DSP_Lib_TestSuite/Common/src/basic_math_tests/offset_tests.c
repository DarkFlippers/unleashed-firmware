#include "jtest.h"
#include "basic_math_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "basic_math_templates.h"
#include "type_abbrev.h"

#define JTEST_ARM_OFFSET_TEST(suffix)               \
    BASIC_MATH_DEFINE_TEST_TEMPLATE_BUF1_ELT1_BLK(  \
        offset,                                     \
        suffix,                                     \
        TYPE_FROM_ABBREV(suffix),                   \
        TYPE_FROM_ABBREV(suffix),                   \
        TYPE_FROM_ABBREV(suffix))

JTEST_ARM_OFFSET_TEST(f32);
JTEST_ARM_OFFSET_TEST(q31);
JTEST_ARM_OFFSET_TEST(q15);
JTEST_ARM_OFFSET_TEST(q7);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(offset_tests)
{
    JTEST_TEST_CALL(arm_offset_f32_test);
    JTEST_TEST_CALL(arm_offset_q31_test);
    JTEST_TEST_CALL(arm_offset_q15_test);
    JTEST_TEST_CALL(arm_offset_q7_test);
}
