#include "jtest.h"
#include "support_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "support_templates.h"
#include "type_abbrev.h"

/* Aliases to play nicely with templates. */
#define arm_f32_to_q31 arm_float_to_q31
#define arm_f32_to_q15 arm_float_to_q15
#define arm_f32_to_q7  arm_float_to_q7
#define arm_q31_to_f32 arm_q31_to_float
#define arm_q15_to_f32 arm_q15_to_float
#define arm_q7_to_f32  arm_q7_to_float
#define ref_f32_to_q31 ref_float_to_q31
#define ref_f32_to_q15 ref_float_to_q15
#define ref_f32_to_q7  ref_float_to_q7
#define ref_q31_to_f32 ref_q31_to_float
#define ref_q15_to_f32 ref_q15_to_float
#define ref_q7_to_f32  ref_q7_to_float

#define JTEST_ARM_X_TO_Y_TEST(prefix, suffix)               \
    JTEST_DEFINE_TEST(arm_##prefix##_to_##suffix##_test,    \
                      arm_##prefix##_to_##suffix)           \
    {                                                       \
        TEST_TEMPLATE_BUF1_BLK(                             \
            support_f_all,                                  \
            support_block_sizes,                            \
            TYPE_FROM_ABBREV(prefix),                       \
            TYPE_FROM_ABBREV(suffix),                       \
            arm_##prefix##_to_##suffix,                     \
            ARM_x_to_y_INPUT_INTERFACE,                     \
            ref_##prefix##_to_##suffix,                     \
            REF_x_to_y_INPUT_INTERFACE,                     \
            SUPPORT_COMPARE_INTERFACE);                     \
    }

JTEST_ARM_X_TO_Y_TEST(f32, q31);
JTEST_ARM_X_TO_Y_TEST(f32, q15);
JTEST_ARM_X_TO_Y_TEST(f32, q7);

JTEST_ARM_X_TO_Y_TEST(q31, f32);
JTEST_ARM_X_TO_Y_TEST(q31, q15);
JTEST_ARM_X_TO_Y_TEST(q31, q7);

JTEST_ARM_X_TO_Y_TEST(q15, f32);
JTEST_ARM_X_TO_Y_TEST(q15, q31);
JTEST_ARM_X_TO_Y_TEST(q15, q7);

JTEST_ARM_X_TO_Y_TEST(q7, f32);
JTEST_ARM_X_TO_Y_TEST(q7, q31);
JTEST_ARM_X_TO_Y_TEST(q7, q15);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(x_to_y_tests)
{
    /*
      To skip a test, comment it out.
    */
    JTEST_TEST_CALL(arm_f32_to_q31_test);
    JTEST_TEST_CALL(arm_f32_to_q15_test);
    JTEST_TEST_CALL(arm_f32_to_q7_test);

    JTEST_TEST_CALL(arm_q31_to_f32_test);
    JTEST_TEST_CALL(arm_q31_to_q15_test);
    JTEST_TEST_CALL(arm_q31_to_q7_test);

    JTEST_TEST_CALL(arm_q15_to_f32_test);
    JTEST_TEST_CALL(arm_q15_to_q31_test);
    JTEST_TEST_CALL(arm_q15_to_q7_test);

    JTEST_TEST_CALL(arm_q7_to_f32_test);
    JTEST_TEST_CALL(arm_q7_to_q31_test);
    JTEST_TEST_CALL(arm_q7_to_q15_test);
}
