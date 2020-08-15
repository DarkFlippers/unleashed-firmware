#include "jtest.h"
#include "basic_math_test_group.h"
#include "complex_math_test_group.h"
#include "controller_test_group.h"
#include "fast_math_test_group.h"
#include "filtering_test_group.h"
#include "matrix_test_group.h"
#include "statistics_test_group.h"
#include "support_test_group.h"
#include "transform_test_group.h"
#include "intrinsics_test_group.h"

JTEST_DEFINE_GROUP(all_tests)
{
    /*
      To skip a test, comment it out
    */
    JTEST_GROUP_CALL(basic_math_tests);
    JTEST_GROUP_CALL(complex_math_tests);
    JTEST_GROUP_CALL(controller_tests);
    JTEST_GROUP_CALL(fast_math_tests);
    JTEST_GROUP_CALL(filtering_tests);
    JTEST_GROUP_CALL(matrix_tests);
    JTEST_GROUP_CALL(statistics_tests);
    JTEST_GROUP_CALL(support_tests);
    JTEST_GROUP_CALL(transform_tests);
    JTEST_GROUP_CALL(intrinsics_tests);

    return;
}
