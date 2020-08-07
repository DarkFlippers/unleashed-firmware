#include "jtest.h"
#include "arr_desc.h"
#include "arm_math.h"
#include "type_abbrev.h"
#include "test_templates.h"

/* Bucket of zeros. For comparison with the output of arm_pid_reset_xxx. */
ARR_DESC_DEFINE(float32_t, zeroes, 3, CURLY(0));

/**
 *  Define a JTEST_TEST_t for the function arm_pid_reset_xxx function having
 *  suffix.
 */
#define ARM_PID_RESET_TEST(suffix)                                      \
    JTEST_DEFINE_TEST(arm_pid_reset_##suffix##_test,                    \
                      arm_pid_reset_##suffix)                           \
    {                                                                   \
        /* Initialise the pid_instance */                               \
        arm_pid_instance_##suffix pid_inst = { 0 };                     \
            pid_inst.state[0] = (TYPE_FROM_ABBREV(suffix)) 0xffffffff;  \
            pid_inst.state[1] = (TYPE_FROM_ABBREV(suffix)) 0xffffffff;  \
            pid_inst.state[2] = (TYPE_FROM_ABBREV(suffix)) 0xffffffff;  \
                                                                        \
            /* Display cycle count and run test */                      \
            JTEST_COUNT_CYCLES(arm_pid_reset_##suffix(&pid_inst));      \
                                                                        \
            /* Test correctness */                                      \
            TEST_ASSERT_BUFFERS_EQUAL(                                  \
                pid_inst.state,                                         \
                zeroes.data_ptr,                                        \
                3 * sizeof(TYPE_FROM_ABBREV(suffix)));                  \
                                                                        \
            return JTEST_TEST_PASSED;                                   \
    }

ARM_PID_RESET_TEST(f32);
ARM_PID_RESET_TEST(q31);
ARM_PID_RESET_TEST(q15);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(pid_reset_tests)
{
    /*
      To skip a test, comment it out.
    */
    JTEST_TEST_CALL(arm_pid_reset_f32_test);
    JTEST_TEST_CALL(arm_pid_reset_q31_test);
    JTEST_TEST_CALL(arm_pid_reset_q15_test);
}
