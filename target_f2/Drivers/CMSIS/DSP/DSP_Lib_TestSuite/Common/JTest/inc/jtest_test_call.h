#ifndef _JTEST_TEST_CALL_H_
#define _JTEST_TEST_CALL_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/
#include "jtest_test.h"
#include "jtest_test_define.h"
#include "jtest_fw.h"

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Exectute the test in the #JTEST_TEST_t struct associated with the identifier
 *  test_fn and store the result in retval.
 */
#define JTEST_TEST_RUN(retval, test_fn)                                 \
    do                                                                  \
    {                                                                   \
        JTEST_DUMP_STR("Test Name:\n");                                 \
        JTEST_DUMP_STR(JTEST_TEST_STRUCT_NAME(test_fn).test_fn_str);    \
        JTEST_DUMP_STR("Function Under Test:\n");                       \
        JTEST_DUMP_STR(JTEST_TEST_STRUCT_NAME(test_fn).fut_str);        \
        retval = JTEST_TEST_STRUCT_NAME(test_fn).test_fn_ptr();         \
    } while (0)

/**
 *  Update the enclosing #JTEST_GROUP_t's pass/fail information based on
 *  test_retval.
 *
 *  @param test_retval A #JTEST_TEST_RET_enum for the current test.
 *
 *  @warning Only use if #JTEST_TEST_t is called in the context of a
 *  #JTEST_GROUP_t.
 */
#define JTEST_TEST_UPDATE_PARENT_GROUP_PF(test_retval)              \
    do                                                              \
    {                                                               \
        /* Update enclosing JTEST_GROUP_t with pass/fail info */    \
        if (test_retval == JTEST_TEST_PASSED)                       \
        {                                                           \
            JTEST_GROUP_INC_PASSED(JTEST_CURRENT_GROUP_PTR(), 1);   \
        } else {                                                    \
            JTEST_GROUP_INC_FAILED(JTEST_CURRENT_GROUP_PTR(), 1);   \
        }                                                           \
    } while (0)

/**
 *  Update the #JTEST_FW with pass/fail information based on test_retval.
 *
 *  @param test_retval A #JTEST_TEST_RET_enum for the current test.
 */
#define JTEST_TEST_UPDATE_FW_PF(test_retval)                        \
    do                                                              \
    {                                                               \
        /* Update the JTEST_FW with pass/fail info */                \
        if (test_retval == JTEST_TEST_PASSED)                       \
        {                                                           \
            JTEST_FW_INC_PASSED( 1);                                \
        } else {                                                    \
            JTEST_FW_INC_FAILED(1);                                 \
        }                                                           \
    } while (0)

/**
 *  Update the enclosing JTEST_GROUP_t's pass/fail information, or the
 *  #JTEST_FW's if this test has no enclosing #JTEST_GROUP_t.
 *
 *  @param test_retval A #JTEST_TEST_RET_enum for the current test.
 */
#define JTEST_TEST_UPDATE_PARENT_GROUP_OR_FW_PF(test_retval)            \
    do                                                                  \
    {                                                                   \
        /* Update pass-fail information */                              \
        if (JTEST_CURRENT_GROUP_PTR() /* Non-null */)                    \
        {                                                               \
            JTEST_TEST_UPDATE_PARENT_GROUP_PF(test_retval);             \
        } else {                                                        \
            JTEST_TEST_UPDATE_FW_PF(test_retval);                       \
        }                                                               \
    } while (0)

/**
 *  Dump the results of the test to the Keil Debugger.
 */
#define JTEST_TEST_DUMP_RESULTS(test_retval)        \
        do                                          \
        {                                           \
            if (test_retval == JTEST_TEST_PASSED)   \
            {                                       \
                JTEST_DUMP_STR("Test Passed\n");      \
            } else {                                \
                JTEST_DUMP_STR("Test Failed\n");      \
            }                                       \
        } while (0)

/**
 *  Call the #JTEST_TEST_t assocaited with the identifier test_fn.
 */
#define JTEST_TEST_CALL(test_fn)                                        \
    do                                                                  \
    {                                                                   \
        if (JTEST_TEST_IS_ENABLED(&JTEST_TEST_STRUCT_NAME(test_fn)))    \
        {                                                               \
            /* Default to failure */                                    \
            JTEST_TEST_RET_t __jtest_test_ret = JTEST_TEST_FAILED;      \
                                                                        \
            JTEST_ACT_TEST_START();                                     \
            JTEST_TEST_RUN(__jtest_test_ret, test_fn);                  \
                                                                        \
            /* Update pass-fail information */                          \
            JTEST_TEST_UPDATE_PARENT_GROUP_OR_FW_PF(__jtest_test_ret);  \
                                                                        \
            JTEST_TEST_DUMP_RESULTS(__jtest_test_ret);                  \
            JTEST_ACT_TEST_END();                                       \
        }                                                               \
    } while (0)

#endif /* _JTEST_TEST_CALL_H_ */
