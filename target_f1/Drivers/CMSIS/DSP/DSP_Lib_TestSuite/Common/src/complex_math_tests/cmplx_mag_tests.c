#include "jtest.h"
#include "complex_math_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "complex_math_templates.h"
#include "type_abbrev.h"

#define JTEST_ARM_CMPLX_MAG_TEST(suffix, comparison_interface)  \
    COMPLEX_MATH_DEFINE_TEST_TEMPLATE_BUF1_BLK(                 \
        cmplx_mag,                                              \
        suffix,                                                 \
        TYPE_FROM_ABBREV(suffix),                               \
        TYPE_FROM_ABBREV(suffix),                               \
        comparison_interface)

JTEST_ARM_CMPLX_MAG_TEST(f32, COMPLEX_MATH_COMPARE_RE_INTERFACE);
JTEST_ARM_CMPLX_MAG_TEST(q31, COMPLEX_MATH_SNR_COMPARE_RE_INTERFACE);
JTEST_ARM_CMPLX_MAG_TEST(q15, COMPLEX_MATH_SNR_COMPARE_RE_INTERFACE);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(cmplx_mag_tests)
{
    JTEST_TEST_CALL(arm_cmplx_mag_f32_test);
    JTEST_TEST_CALL(arm_cmplx_mag_q31_test);
    JTEST_TEST_CALL(arm_cmplx_mag_q15_test);
}
