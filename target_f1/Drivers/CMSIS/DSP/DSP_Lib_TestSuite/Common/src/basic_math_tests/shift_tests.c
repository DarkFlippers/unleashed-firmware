#include "jtest.h"
#include "basic_math_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "basic_math_templates.h"
#include "type_abbrev.h"

#define JTEST_ARM_SHIFT_TEST(suffix)                \
    BASIC_MATH_DEFINE_TEST_TEMPLATE_BUF1_ELT1_BLK(  \
        shift,                                      \
        suffix,                                     \
        TYPE_FROM_ABBREV(suffix),                   \
        int8_t, /*elt_type*/                        \
        TYPE_FROM_ABBREV(suffix))

JTEST_ARM_SHIFT_TEST(q31);
JTEST_ARM_SHIFT_TEST(q15);
JTEST_ARM_SHIFT_TEST(q7);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(shift_tests)
{
    JTEST_TEST_CALL(arm_shift_q31_test);
    JTEST_TEST_CALL(arm_shift_q15_test);
    JTEST_TEST_CALL(arm_shift_q7_test);
}
