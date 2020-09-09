#ifndef _JTEST_PF_H_
#define _JTEST_PF_H_

/*--------------------------------------------------------------------------------*/
/* Purpose */
/*--------------------------------------------------------------------------------*/
/* jtest_pf.h Contains macros useful for capturing pass/fail data. */


/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 * Members that can be added to other structs to extend them pass/fail data and
 * corresponding functionality.
 */
#define JTEST_PF_MEMBERS                            \
    uint32_t passed;                                \
    uint32_t failed /* Note the lacking semicolon*/ \

/**
 *  Used for initializing JTEST_PF_MEMBERS in a struct declaration.
 */
#define JTEST_PF_MEMBER_INIT                    \
    0,                                          \
    0

/* Member-Incrementing Macros */
/*--------------------------------------------------------------------------------*/

/**
 *  Dispatch macro for incrementing #JTEST_PF_MEMBERS.
 *
 *  @param xxx Values: 'passed', 'failed'
 */
#define JTEST_PF_INC_XXX(xxx, struct_pf_ptr, amount)    \
    do                                                  \
    {                                                   \
        ((struct_pf_ptr)->xxx) += (amount);             \
    } while (0)

/**
 *  Specialization of the #JTEST_PF_INC_XXX macro to increment the passed
 *  member.
 */
#define JTEST_PF_INC_PASSED(struct_pf_ptr, amount)  \
    JTEST_PF_INC_XXX(passed, struct_pf_ptr, amount)


/**
 *  Specialization of the #JTEST_PF_INC_XXX macro to increment the failed
 *  member.
 */
#define JTEST_PF_INC_FAILED(struct_pf_ptr, amount)  \
    JTEST_PF_INC_XXX(failed, struct_pf_ptr, amount)


/* Member-Resetting Macros */
/*--------------------------------------------------------------------------------*/

/**
 *  Dispatch macro for setting #JTEST_PF_MEMBERS to zero.
 *
 *  @param xxx Values: 'passed', 'failed'
 */
#define JTEST_PF_RESET_XXX(xxx, struct_pf_ptr)  \
    do                                          \
    {                                           \
        ((struct_pf_ptr)->xxx) = UINT32_C(0);   \
    } while (0)

/**
 *  Specialization of #JTEST_PF_RESET_XXX for the 'passed' member.
 */
#define JTEST_PF_RESET_PASSED(struct_pf_ptr)    \
    JTEST_PF_RESET_XXX(passed, struct_pf_ptr)

/**
 *  Specialization of #JTEST_PF_RESET_XXX for the 'failed' member.
 */
#define JTEST_PF_RESET_FAILED(struct_pf_ptr)    \
    JTEST_PF_RESET_XXX(failed, struct_pf_ptr)

#endif /* _JTEST_PF_H_ */
