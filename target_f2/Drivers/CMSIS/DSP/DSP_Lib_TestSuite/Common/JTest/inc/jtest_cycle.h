#ifndef _JTEST_CYCLE_H_
#define _JTEST_CYCLE_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "jtest_fw.h"           /* JTEST_DUMP_STRF() */
#include "jtest_systick.h"
#include "jtest_util.h"         /* STR() */

/*--------------------------------------------------------------------------------*/
/* Declare Module Variables */
/*--------------------------------------------------------------------------------*/
extern const char * JTEST_CYCLE_STRF;

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Wrap the function call, fn_call, to count execution cycles and display the
 *  results.
 */
/* skipp function name + param
#define JTEST_COUNT_CYCLES(fn_call)                     \
    do                                                  \
    {                                                   \
        uint32_t __jtest_cycle_end_count;               \
                                                        \
        JTEST_SYSTICK_RESET(SysTick);                   \
        JTEST_SYSTICK_START(SysTick);                   \
                                                        \
        fn_call;                                        \
                                                        \
        __jtest_cycle_end_count =                       \
            JTEST_SYSTICK_VALUE(SysTick);               \
                                                        \
		JTEST_SYSTICK_RESET(SysTick);                   \
        JTEST_DUMP_STRF(JTEST_CYCLE_STRF,               \
                        STR(fn_call),                   \
                        (JTEST_SYSTICK_INITIAL_VALUE -  \
                         __jtest_cycle_end_count));     \
    } while (0)
*/
#define JTEST_COUNT_CYCLES(fn_call)                     \
    do                                                  \
    {                                                   \
        uint32_t __jtest_cycle_end_count;               \
                                                        \
        JTEST_SYSTICK_RESET(SysTick);                   \
        JTEST_SYSTICK_START(SysTick);                   \
                                                        \
        fn_call;                                        \
                                                        \
        __jtest_cycle_end_count =                       \
            JTEST_SYSTICK_VALUE(SysTick);               \
                                                        \
		JTEST_SYSTICK_RESET(SysTick);                   \
        JTEST_DUMP_STRF(JTEST_CYCLE_STRF,               \
                        (JTEST_SYSTICK_INITIAL_VALUE -  \
                         __jtest_cycle_end_count));     \
    } while (0)

#endif /* _JTEST_CYCLE_H_ */
