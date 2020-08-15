#include "jtest.h"
#include "filtering_tests.h"

JTEST_DEFINE_GROUP(filtering_tests)
{
    /*
      To skip a test, comment it out.
    */
    JTEST_GROUP_CALL(biquad_tests);
    JTEST_GROUP_CALL(conv_tests);
    JTEST_GROUP_CALL(correlate_tests);
    JTEST_GROUP_CALL(fir_tests);
    JTEST_GROUP_CALL(iir_tests);
    JTEST_GROUP_CALL(lms_tests);

    return;
}
