#ifndef _JTEST_GROUP_H_
#define _JTEST_GROUP_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "jtest_pf.h"
#include "jtest_util.h"

/*--------------------------------------------------------------------------------*/
/* Type Definitions */
/*--------------------------------------------------------------------------------*/

/**
 *  A struct which represents a group of #JTEST_TEST_t structs. This struct is
 *  used to run the group of tests, and report on their outcomes.
 */
typedef struct JTEST_GROUP_struct
{
    void (* group_fn_ptr) (void); /**< Pointer to the test group */
    char * name_str;              /**< Name of the group */
    
    /* Extend the #JTEST_GROUP_t with Pass/Fail information.*/
    JTEST_PF_MEMBERS;
} JTEST_GROUP_t;

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Set the name of JTEST_GROUP_t.
 */
#define JTEST_GROUP_SET_NAME(group_ptr, name)     \
    JTEST_SET_STRUCT_ATTRIBUTE(group_ptr, name_str, name)

#define JTEST_GROUP_SET_FN(group_ptr, fn_ptr)     \
    JTEST_SET_STRUCT_ATTRIBUTE(group_ptr, group_fn_ptr, fn_ptr)

/**
 *  Increment the number of tests passed in the JTEST_GROUP_t pointed to by
 *  group_ptr.
 */
#define JTEST_GROUP_INC_PASSED(group_ptr, amount) \
    JTEST_PF_INC_PASSED(group_ptr, amount)

/**
 *  Increment the number of tests failed in the JTEST_GROUP_t pointed to by
 *  group_ptr.
 */
#define JTEST_GROUP_INC_FAILED(group_ptr, amount) \
    JTEST_PF_INC_FAILED(group_ptr, amount)

/**
 *  Reset the pass/fail information of the #JTEST_GROUP_t pointed to by
 *  group_ptr.
 */
#define JTEST_GROUP_RESET_PF(group_ptr)         \
    do                                          \
    {                                           \
        JTEST_PF_RESET_PASSED(group_ptr);       \
        JTEST_PF_RESET_FAILED(group_ptr);       \
    } while (0)

#endif /* _JTEST_GROUP_H_ */
