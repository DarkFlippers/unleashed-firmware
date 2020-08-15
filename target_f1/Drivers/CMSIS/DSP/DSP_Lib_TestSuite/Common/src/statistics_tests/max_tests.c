#include "jtest.h"
#include "statistics_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "statistics_templates.h"
#include "type_abbrev.h"

#define JTEST_ARM_MAX_TEST(suffix)              \
    STATISTICS_DEFINE_TEST_TEMPLATE_BUF1_BLK(   \
        max,                                    \
        suffix,                                 \
        TYPE_FROM_ABBREV(suffix),               \
        TYPE_FROM_ABBREV(suffix),               \
        STATISTICS_COMPARE_INTERFACE)

JTEST_ARM_MAX_TEST(f32);
JTEST_ARM_MAX_TEST(q31);
JTEST_ARM_MAX_TEST(q15);
JTEST_ARM_MAX_TEST(q7);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(max_tests)
{
    /*
      To skip a test, comment it out.
    */
    JTEST_TEST_CALL(arm_max_f32_test);
    JTEST_TEST_CALL(arm_max_q31_test);
    JTEST_TEST_CALL(arm_max_q15_test);
    JTEST_TEST_CALL(arm_max_q7_test);
}
