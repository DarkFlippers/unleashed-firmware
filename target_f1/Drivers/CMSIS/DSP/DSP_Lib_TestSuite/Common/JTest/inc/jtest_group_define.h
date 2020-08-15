#ifndef _JTEST_GROUP_DEFINE_H_
#define _JTEST_GROUP_DEFINE_H_


/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "jtest_util.h"
#include "jtest_define.h"
#include "jtest_group.h"

/* For defining macros with optional arguments */
#include "opt_arg/opt_arg.h"

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Prefix for all #JTEST_GROUP_t structs.
 */
#define JTEST_GROUP_STRUCT_NAME_PREFIX G_JTEST_GROUP_STRUCT_

/**
 *  Define test template used by #JTEST_GROUP_t tests.
 */
#define JTEST_GROUP_FN_TEMPLATE(group_fn)    \
    void group_fn(void)

#define JTEST_GROUP_FN_PROTOTYPE JTEST_GROUP_FN_TEMPLATE /**< Alias for
                                                            #JTEST_GROUP_FN_TEMPLATE. */

/**
 *  Evaluate to the name of the #JTEST_GROUP_t struct associated with group_fn.
 */
#define JTEST_GROUP_STRUCT_NAME(group_fn)    \
    JTEST_STRUCT_NAME(JTEST_GROUP_STRUCT_NAME_PREFIX, group_fn)

/**
 *  Define a #JTEST_GROUP_t struct based on the given group_fn.
 */
#define JTEST_GROUP_DEFINE_STRUCT(group_fn)  \
    JTEST_DEFINE_STRUCT(JTEST_GROUP_t,       \
                        JTEST_GROUP_STRUCT_NAME(group_fn))

/**
 *  Declare a #JTEST_GROUP_t struct based on the given group_fn.
 */
#define JTEST_GROUP_DECLARE_STRUCT(group_fn) \
    JTEST_DECLARE_STRUCT(JTEST_GROUP_DEFINE_STRUCT(group_fn))

/**
 *  Contents needed to initialize a JTEST_GROUP_t struct.
 */
#define JTEST_GROUP_STRUCT_INIT(group_fn)    \
    group_fn,                                \
        STR_NL(group_fn),                       \
        JTEST_PF_MEMBER_INIT

/**
 *  Initialize the contents of a #JTEST_GROUP_t struct.
 */
#define JTEST_GROUP_INIT(group_fn)           \
    JTEST_GROUP_DEFINE_STRUCT(group_fn) = {  \
        JTEST_GROUP_STRUCT_INIT(group_fn)    \
    }

/* Test Definition Macro */
/*--------------------------------------------------------------------------------*/

/**
 *  Define a #JTEST_GROUP_t object and a test function.
 */
#define JTEST_DEFINE_GROUP(group_fn)         \
    JTEST_GROUP_FN_PROTOTYPE(group_fn);      \
    JTEST_GROUP_INIT(group_fn);              \
    JTEST_GROUP_FN_PROTOTYPE(group_fn) /* Notice the lacking semicolon */

/**
 *  Declare a #JTEST_GROUP_t object and a test function prototype.
 */
#define JTEST_DECLARE_GROUP(group_fn)        \
    JTEST_GROUP_FN_PROTOTYPE(group_fn);      \
    JTEST_GROUP_DECLARE_STRUCT(group_fn) /* Note the lacking semicolon */

#endif /* _JTEST_GROUP_DEFINE_H_ */
