#ifndef _JTEST_GROUP_CALL_H_
#define _JTEST_GROUP_CALL_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "jtest_fw.h"
#include <inttypes.h>

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Execute the test in the #JTEST_GROUP_t struct associated witht he identifier
 *  group_fn.
 */
#define JTEST_GROUP_RUN(group_fn)                                   \
    do                                                              \
    {                                                               \
        JTEST_DUMP_STR("Group Name:\n");                            \
        JTEST_DUMP_STR(JTEST_GROUP_STRUCT_NAME(group_fn).name_str); \
        JTEST_GROUP_STRUCT_NAME(group_fn).group_fn_ptr();           \
    } while (0)


/**
 *  Update the enclosing #JTEST_GROUP_t's pass/fail information using the
 *  current #JTEST_GROUP_t's.
 *
 *  @param group_ptr Pointer to the current #JTEST_GROUP_t.
 *  @param parent_ptr Pointer to the enclosing #JTEST_GROUP_t.
 *
 *  @warning Only run this if the current #JTEST_GROUP_t is being called within
 *  the context of another #JTEST_GROUP_t.
 */
#define JTEST_GROUP_UPDATE_PARENT_GROUP_PF(group_ptr, parent_group_ptr) \
    do                                                                  \
    {                                                                   \
        JTEST_GROUP_INC_PASSED(parent_group_ptr,                        \
                               (group_ptr)->passed);                    \
        JTEST_GROUP_INC_FAILED(parent_group_ptr,                        \
                               (group_ptr)->failed);                    \
    } while (0)

/**
 *  Update the #JTEST_FW's pass/fail information using the current
 *  #JTEST_GROUP_t's.
 */
#define JTEST_GROUP_UPDATE_FW_PF(group_ptr)                     \
    do                                                          \
    {                                                           \
        JTEST_FW_INC_PASSED((group_ptr)->passed);               \
        JTEST_FW_INC_FAILED((group_ptr)->failed);               \
    } while (0)

/**
 *  Update the enclosing context with the current #JTEST_GROUP_t's pass/fail
 *  information. If this group isn't in an enclosing group, it updates the
 *  #JTEST_FW's pass/fail info by default.
 */
#define JTEST_GROUP_UPDATE_PARENT_GROUP_OR_FW_PF(group_ptr,         \
                                                 parent_group_ptr)  \
    do                                                              \
    {                                                               \
        /* Update the pass fail counts in the parent group */       \
        if (parent_group_ptr /* Null implies Top*/)                 \
        {                                                           \
            JTEST_GROUP_UPDATE_PARENT_GROUP_PF(                     \
                group_ptr,                                          \
                parent_group_ptr);                                  \
        } else {                                                    \
            JTEST_GROUP_UPDATE_FW_PF(                               \
                group_ptr);                                         \
        }                                                           \
    } while (0)

/**
 *  Dump the results of running the #JTEST_GROUP_t to the Keil Debugger.
 */
#define JTEST_GROUP_DUMP_RESULTS(group_ptr)                             \
        do                                                              \
        {                                                               \
            JTEST_DUMP_STRF(                                            \
                "Tests Run: %" PRIu32 "\n"                              \
                "----------\n"                                          \
                "   Passed: %" PRIu32 "\n"                              \
                "   Failed: %" PRIu32 "\n",                             \
                (group_ptr)->passed + (group_ptr)->failed,              \
                (group_ptr)->passed,                                    \
                (group_ptr)->failed);                                   \
        } while (0)

/**
 *  Call the #JTEST_GROUP_t associated with the identifier group_fn.
 */
#define JTEST_GROUP_CALL(group_fn)                                      \
        do                                                              \
        {   /* Save the current group from JTEST_FW_t before swapping */ \
            /* it to this group (in order to restore it later )*/       \
            JTEST_GROUP_t * __jtest_temp_group_ptr =                    \
                JTEST_CURRENT_GROUP_PTR();                              \
            JTEST_SET_CURRENT_GROUP(&JTEST_GROUP_STRUCT_NAME(group_fn)); \
                                                                        \
            /* Reset this group's pass/fail count. Each group */        \
            /* should only remember counts for its last execution. */   \
            JTEST_GROUP_RESET_PF(JTEST_CURRENT_GROUP_PTR());            \
                                                                        \
            /* Run the current group */                                 \
            JTEST_ACT_GROUP_START();                                    \
            JTEST_GROUP_RUN(group_fn);                                  \
            JTEST_ACT_GROUP_END();                                      \
                                                                        \
            /* Update the pass fail counts in the parent group (or FW) */ \
            JTEST_GROUP_UPDATE_PARENT_GROUP_OR_FW_PF(                   \
                JTEST_CURRENT_GROUP_PTR(),                              \
                __jtest_temp_group_ptr);                                \
                                                                        \
            JTEST_GROUP_DUMP_RESULTS(JTEST_CURRENT_GROUP_PTR());        \
                                                                        \
            /* Restore the previously current group */                  \
            JTEST_SET_CURRENT_GROUP(__jtest_temp_group_ptr);            \
        } while (0)

#endif /* _JTEST_GROUP_CALL_H_ */
