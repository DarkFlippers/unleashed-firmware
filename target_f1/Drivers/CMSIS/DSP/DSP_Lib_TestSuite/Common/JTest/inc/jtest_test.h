#ifndef _JTEST_TEST_H_
#define _JTEST_TEST_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include <stdint.h>
#include "jtest_util.h"
#include "jtest_test_ret.h"

/*--------------------------------------------------------------------------------*/
/* Type Definitions */
/*--------------------------------------------------------------------------------*/

/**
 *  A struct which represents a Test in the JTEST framework.  This struct is
 *  used to enable, run, and describe the test it represents.
 */
typedef struct JTEST_TEST_struct
{
    JTEST_TEST_RET_t ( * test_fn_ptr)(void); /**< Pointer to the test function. */
    char   * test_fn_str;                    /**< Name of the test function */
    char   * fut_str;           /**< Name of the function under test. */

    /**
     *  Flags that govern how the #JTEST_TEST_t behaves.
     */
    union {
        struct {
            unsigned enabled : 1;
            unsigned unused  : 7;
        } bits;
        uint8_t byte;           /* Access all flags at once. */
    } flags;
    
} JTEST_TEST_t;

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Assign a test function to the #JTEST_TEST_t struct.
 */
#define JTEST_TEST_SET_FN(jtest_test_ptr, fn_ptr)                   \
    JTEST_SET_STRUCT_ATTRIBUTE(jtest_test_ptr, test_fn_ptr, fn_ptr)

/**
 *  Specify a function under test (FUT) for the #JTEST_TEST_t struct.
 */
#define JTEST_TEST_SET_FUT(jtest_test_ptr, str)                 \
    JTEST_SET_STRUCT_ATTRIBUTE(jtest_test_ptr, fut_str, str)

/* Macros concerning JTEST_TEST_t flags */
/*--------------------------------------------------------------------------------*/

#define JTEST_TEST_FLAG_SET 1 /**< Value of a set #JTEST_TEST_t flag. */
#define JTEST_TEST_FLAG_CLR 0 /**< Value of a cleared #JTEST_TEST_t flag. */

/**
 *  Evaluate to the flag in #JTEST_TEST_t having flag_name.
 */
#define JTEST_TEST_FLAG(jtest_test_ptr, flag_name)  \
    ((jtest_test_ptr)->flags.bits.flag_name)

/**
 *  Dispatch macro for setting and clearing #JTEST_TEST_t flags.
 *
 *  @param jtest_test_ptr Pointer to a #JTEST_TEST_t struct.
 *  @param flag_name      Name of the flag to set in #JTEST_TEST_t.flags.bits
 *  @param xxx            Vaid values: "SET" or "CLR"
 *
 *  @note This function depends on JTEST_TEST_FLAG_SET and JTEST_TEST_FLAG_CLR.
 */
#define JTEST_TEST_XXX_FLAG(jtest_test_ptr, flag_name, xxx)                  \
    do                                                                       \
    {                                                                        \
        JTEST_TEST_FLAG(jtest_test_ptr, flag_name) = JTEST_TEST_FLAG_##xxx ; \
    } while (0)

/**
 *  Specification of #JTEST_TEST_XXX_FLAG to set #JTEST_TEST_t flags.
 */
#define JTEST_TEST_SET_FLAG(jtest_test_ptr, flag_name)                       \
    JTEST_TEST_XXX_FLAG(jtest_test_ptr, flag_name, SET)

/**
 *  Specification of #JTEST_TEST_XXX_FLAG to clear #JTEST_TEST_t flags.
 */
#define JTEST_TEST_CLR_FLAG(jtest_test_ptr, flag_name)                       \
    JTEST_TEST_XXX_FLAG(jtest_test_ptr, flag_name, CLR)

/**
 *  Evaluate to true if the #JTEST_TEST_t is enabled.
 */
#define JTEST_TEST_IS_ENABLED(jtest_test_ptr)                           \
    (JTEST_TEST_FLAG(jtest_test_ptr, enabled) == JTEST_TEST_FLAG_SET)

#endif /* _JTEST_TEST_H_ */
