#include "jtest.h"
#include "basic_math_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "basic_math_templates.h"
#include "type_abbrev.h"


#define JTEST_ARM_SCALE_TEST(suffix)                \
    BASIC_MATH_DEFINE_TEST_TEMPLATE_BUF1_ELT2_BLK(  \
        scale,                                      \
        suffix,                                     \
        TYPE_FROM_ABBREV(suffix),                   \
        TYPE_FROM_ABBREV(suffix), /*elt1_type*/     \
        int8_t, /*elt2_type*/                       \
        TYPE_FROM_ABBREV(suffix))

/* float32_t defined separately because it has less arguments */
JTEST_DEFINE_TEST(arm_scale_f32_test,
                  arm_scale_f32)
{
    TEST_TEMPLATE_BUF1_ELT1_BLK(
        basic_math_f_all,
        basic_math_eltsf,
        basic_math_block_sizes,
        float32_t,
        float32_t,
        float32_t,
        arm_scale_f32,
        ARM_scale_float_INPUT_INTERFACE,
        ref_scale_f32,
        REF_scale_float_INPUT_INTERFACE,
        BASIC_MATH_COMPARE_INTERFACE);
}

JTEST_ARM_SCALE_TEST(q31);
JTEST_ARM_SCALE_TEST(q15);
JTEST_ARM_SCALE_TEST(q7);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(scale_tests)
{
    JTEST_TEST_CALL(arm_scale_f32_test);
    JTEST_TEST_CALL(arm_scale_q31_test);
    JTEST_TEST_CALL(arm_scale_q15_test);
    JTEST_TEST_CALL(arm_scale_q7_test);
}
