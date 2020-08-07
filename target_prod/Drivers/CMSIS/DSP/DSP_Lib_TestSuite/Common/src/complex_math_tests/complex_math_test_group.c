#include "jtest.h"
#include "complex_math_tests.h"

JTEST_DEFINE_GROUP(complex_math_tests)
{
    JTEST_GROUP_CALL(cmplx_conj_tests);
    JTEST_GROUP_CALL(cmplx_dot_prod_tests);
    JTEST_GROUP_CALL(cmplx_mag_tests);
    JTEST_GROUP_CALL(cmplx_mag_squared_tests);
    JTEST_GROUP_CALL(cmplx_mult_cmplx_tests);
    JTEST_GROUP_CALL(cmplx_mult_real_tests);
    
    return;
}
