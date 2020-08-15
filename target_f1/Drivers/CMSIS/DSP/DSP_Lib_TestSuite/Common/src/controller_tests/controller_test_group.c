#include "jtest.h"
#include "controller_tests.h"

JTEST_DEFINE_GROUP(controller_tests)
{
    /*
      To skip a test, comment it out.
    */
    JTEST_GROUP_CALL(pid_reset_tests);
    JTEST_GROUP_CALL(pid_tests);
    JTEST_GROUP_CALL(sin_cos_tests);
    return;
}
