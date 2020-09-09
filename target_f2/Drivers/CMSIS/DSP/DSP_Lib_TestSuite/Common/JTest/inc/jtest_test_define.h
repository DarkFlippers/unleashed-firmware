#ifndef _JTEST_TEST_DEFINE_H_
#define _JTEST_TEST_DEFINE_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "jtest_util.h"
#include "jtest_define.h"
#include "jtest_test.h"

/* For defining macros with optional arguments */
#include "opt_arg/opt_arg.h"

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Prefix for all #JTEST_TEST_t structs.
 */
#define JTEST_TEST_STRUCT_NAME_PREFIX G_JTEST_TEST_STRUCT_

/**
 *  Define test template used by #JTEST_TEST_t tests.
 */
#define JTEST_TEST_FN_TEMPLATE(test_fn)                         \
    JTEST_TEST_RET_t test_fn(void)

#define JTEST_TEST_FN_PROTOTYPE JTEST_TEST_FN_TEMPLATE /**< Alias for
                                                        * #JTEST_TEST_FN_TEMPLATE. */

/**
 *  Evaluate to the name of the #JTEST_TEST_t struct associated with test_fn.
 */
#define JTEST_TEST_STRUCT_NAME(test_fn)                         \
    JTEST_STRUCT_NAME(JTEST_TEST_STRUCT_NAME_PREFIX, test_fn)

/**
 *  Define a #JTEST_TEST_t struct based on the given test_fn.
 */
#define JTEST_TEST_DEFINE_STRUCT(test_fn)                   \
    JTEST_DEFINE_STRUCT(JTEST_TEST_t,                       \
                        JTEST_TEST_STRUCT_NAME(test_fn))

/**
 *  Declare a #JTEST_TEST_t struct based on the given test_fn.
 */
#define JTEST_TEST_DECLARE_STRUCT(test_fn)      \
    JTEST_DECLARE_STRUCT(JTEST_TEST_DEFINE_STRUCT(test_fn))

/**
 *  Contents needed to initialize a JTEST_TEST_t struct.
 */
#define JTEST_TEST_STRUCT_INIT(test_fn, fut, enable)    \
    test_fn,                                            \
        STR_NL(test_fn),                                   \
        STR_NL(fut),                                       \
    {                                                   \
        {                                               \
            enable,                                     \
                0                                       \
        }                                               \
    }                                                   \
        

/**
 *  Initialize the contents of a #JTEST_TEST_t struct.
 */
#define JTEST_TEST_INIT(test_fn, fut, enable)              \
    JTEST_TEST_DEFINE_STRUCT(test_fn) = {                  \
        JTEST_TEST_STRUCT_INIT(test_fn, fut, enable)       \
    }

/* Test Definition Macro */
/*--------------------------------------------------------------------------------*/

/**
 *  Define a #JTEST_TEST_t object and a test function.
 */
#define _JTEST_DEFINE_TEST(test_fn, fut, enable)           \
    JTEST_TEST_FN_PROTOTYPE(test_fn);                      \
    JTEST_TEST_INIT(test_fn, fut, enable);                 \
    JTEST_TEST_FN_PROTOTYPE(test_fn) /* Notice the lacking semicolon */

/**
 *  Declare a #JTEST_TEST_t object and a test function prototype.
 */
#define JTEST_DECLARE_TEST(test_fn)                                     \
    JTEST_TEST_FN_PROTOTYPE(test_fn);                                   \
    JTEST_TEST_DECLARE_STRUCT(test_fn) /* Note the lacking semicolon */

/*--------------------------------------------------------------------------------*/
/* Macros with optional arguments */
/*--------------------------------------------------------------------------------*/

/* Top-level Interface */
#define JTEST_DEFINE_TEST(...)                             \
    JTEST_DEFINE_TEST_(PP_NARG(__VA_ARGS__), ##__VA_ARGS__)

/* Dispatch Macro*/
#define JTEST_DEFINE_TEST_(N, ...)                         \
    SPLICE(JTEST_DEFINE_TEST_, N)(__VA_ARGS__)

/* Default Arguments */
#define JTEST_DEFINE_TEST_DEFAULT_FUT /* Blank */
#define JTEST_DEFINE_TEST_DEFAULT_ENABLE                   \
    JTEST_TRUE                                 /* Tests enabled by
                                                * default. */ 

/* Dispatch Cases*/
#define JTEST_DEFINE_TEST_1(_1)                            \
    _JTEST_DEFINE_TEST(                                    \
        _1,                                                \
        JTEST_DEFINE_TEST_DEFAULT_FUT,                     \
        JTEST_DEFINE_TEST_DEFAULT_ENABLE                   \
        )

#define JTEST_DEFINE_TEST_2(_1, _2)                        \
    _JTEST_DEFINE_TEST(                                    \
        _1,                                                \
        _2,                                                \
        JTEST_DEFINE_TEST_DEFAULT_ENABLE                   \
        )

#define JTEST_DEFINE_TEST_3(_1, _2, _3)                    \
    _JTEST_DEFINE_TEST(                                    \
        _1,                                                \
        _2,                                                \
        _3                                                 \
        )

#endif /* _JTEST_TEST_DEFINE_H_ */
