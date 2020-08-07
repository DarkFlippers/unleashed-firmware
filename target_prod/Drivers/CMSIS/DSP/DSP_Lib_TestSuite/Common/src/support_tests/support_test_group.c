#include "jtest.h"
#include "support_tests.h"

JTEST_DEFINE_GROUP(support_tests)
{
    JTEST_GROUP_CALL(copy_tests);
    JTEST_GROUP_CALL(fill_tests);
    JTEST_GROUP_CALL(x_to_y_tests);
    return;
}
