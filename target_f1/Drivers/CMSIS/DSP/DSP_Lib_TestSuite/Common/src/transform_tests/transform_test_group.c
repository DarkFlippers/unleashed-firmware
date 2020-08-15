#include "jtest.h"
#include "transform_tests.h"

JTEST_DEFINE_GROUP(transform_tests)
{
    JTEST_GROUP_CALL(cfft_tests);
    JTEST_GROUP_CALL(cfft_family_tests);
    JTEST_GROUP_CALL(rfft_tests);
    JTEST_GROUP_CALL(rfft_fast_tests);
    JTEST_GROUP_CALL(dct4_tests);
}
