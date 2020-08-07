#include "jtest.h"
#include "support_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "support_templates.h"
#include "type_abbrev.h"

#define JTEST_ARM_COPY_TEST(suffix)             \
    SUPPORT_DEFINE_TEST_TEMPLATE_BUF1_BLK(      \
        copy,                                   \
        suffix,                                 \
        TYPE_FROM_ABBREV(suffix),               \
        TYPE_FROM_ABBREV(suffix),               \
        SUPPORT_COMPARE_INTERFACE)

JTEST_ARM_COPY_TEST(f32);
JTEST_ARM_COPY_TEST(q31);
JTEST_ARM_COPY_TEST(q15);
JTEST_ARM_COPY_TEST(q7);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(copy_tests)
{
    JTEST_TEST_CALL(arm_copy_f32_test);
    JTEST_TEST_CALL(arm_copy_q31_test);
    JTEST_TEST_CALL(arm_copy_q15_test);
    JTEST_TEST_CALL(arm_copy_q7_test);
}
