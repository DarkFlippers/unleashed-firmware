#ifndef _JTEST_SYSTICK_H_
#define _JTEST_SYSTICK_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

/* Get access to the SysTick structure. */
#if   defined ARMCM0
  #include "ARMCM0.h"
#elif defined ARMCM0P
  #include "ARMCM0plus.h"
#elif defined ARMCM3
  #include "ARMCM3.h"
#elif defined ARMCM4
  #include "ARMCM4.h"
#elif defined ARMCM4_FP
  #include "ARMCM4_FP.h"
#elif defined ARMCM7
  #include "ARMCM7.h" 
#elif defined ARMCM7_SP
  #include "ARMCM7_SP.h"
#elif defined ARMCM7_DP
  #include "ARMCM7_DP.h"
#elif defined ARMSC000
  #include "ARMSC000.h"
#elif defined ARMSC300
  #include "ARMSC300.h"
#elif defined ARMv8MBL
  #include "ARMv8MBL.h"
#elif defined ARMv8MML
  #include "ARMv8MML.h"
#elif defined ARMv8MML_DSP
  #include "ARMv8MML_DSP.h"
#elif defined ARMv8MML_SP
  #include "ARMv8MML_SP.h"
#elif defined ARMv8MML_DSP_SP
  #include "ARMv8MML_DSP_SP.h"
#elif defined ARMv8MML_DP
  #include "ARMv8MML_DP.h"
#elif defined ARMv8MML_DSP_DP
  #include "ARMv8MML_DSP_DP.h"

#else
  #warning "no appropriate header file found!"
#endif

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Initial value for the SysTick module.
 *
 *  @note This is also the maximum value, important as SysTick is a decrementing
 *  counter.
 */
#define JTEST_SYSTICK_INITIAL_VALUE 0xFFFFFF

/**
 *  Reset the SysTick, decrementing timer to it's maximum value and disable it.
 *
 *  This macro should leave the SysTick timer in a state that's ready for cycle
 *  counting.
 */
#define JTEST_SYSTICK_RESET(systick_ptr)                    \
    do                                                      \
    {                                                       \
        (systick_ptr)->LOAD = JTEST_SYSTICK_INITIAL_VALUE;  \
        (systick_ptr)->VAL = 1;                             \
                                                            \
        /* Disable the SysTick module. */                   \
        (systick_ptr)->CTRL = UINT32_C(0x000000);           \
    } while (0)

/**
 *  Start the SysTick timer, sourced by the processor clock.
 */
#define JTEST_SYSTICK_START(systick_ptr)                    \
    do                                                      \
    {                                                       \
        (systick_ptr)->CTRL =                               \
            SysTick_CTRL_ENABLE_Msk |                       \
            SysTick_CTRL_CLKSOURCE_Msk; /* Internal clk*/   \
    } while (0)

/**
 *  Evaluate to the current value of the SysTick timer.
 */
#define JTEST_SYSTICK_VALUE(systick_ptr)        \
    ((systick_ptr)->VAL)
           
#endif /* _JTEST_SYSTICK_H_ */
