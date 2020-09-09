#include "jtest.h"
#include "all_tests.h"
#include "arm_math.h"


#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
asm(" .global __ARM_use_no_argv\n");
#endif


void debug_init(void)
{
    uint32_t * SHCSR_ptr = (uint32_t *) 0xE000ED24; /* System Handler Control and State Register */
    *SHCSR_ptr |= 0x70000;             /* Enable  UsageFault, BusFault, and MemManage fault*/
}

int main(void)
{
    debug_init();

    JTEST_INIT();               /* Initialize test framework. */

    JTEST_GROUP_CALL(all_tests); /* Run all tests. */

    JTEST_ACT_EXIT_FW();        /* Exit test framework.  */
    while (1);                   /* Never return. */
}
