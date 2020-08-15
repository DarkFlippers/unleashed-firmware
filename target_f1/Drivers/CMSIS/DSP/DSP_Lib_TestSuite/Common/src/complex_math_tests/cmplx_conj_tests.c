#include "jtest.h"
#include "complex_math_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "complex_math_templates.h"
#include "type_abbrev.h"

#define JTEST_ARM_CMPLX_CONJ_TEST(suffix)           \
    COMPLEX_MATH_DEFINE_TEST_TEMPLATE_BUF1_BLK(     \
        cmplx_conj,                                 \
        suffix,                                     \
        TYPE_FROM_ABBREV(suffix),                   \
        TYPE_FROM_ABBREV(suffix),                   \
        COMPLEX_MATH_SNR_COMPARE_CMPLX_INTERFACE)

JTEST_ARM_CMPLX_CONJ_TEST(f32);
JTEST_ARM_CMPLX_CONJ_TEST(q31);
JTEST_ARM_CMPLX_CONJ_TEST(q15);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(cmplx_conj_tests)
{
    JTEST_TEST_CALL(arm_cmplx_conj_f32_test);
    JTEST_TEST_CALL(arm_cmplx_conj_q31_test);
    JTEST_TEST_CALL(arm_cmplx_conj_q15_test);
}
