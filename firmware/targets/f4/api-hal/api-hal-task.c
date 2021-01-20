#include "cmsis_os.h"
#include "api-hal-task.h"

//-----------------------------cmsis_os2.c-------------------------------
// helpers to get isr context
// get arch
#ifndef __ARM_ARCH_6M__
#define __ARM_ARCH_6M__ 0
#endif
#ifndef __ARM_ARCH_7M__
#define __ARM_ARCH_7M__ 0
#endif
#ifndef __ARM_ARCH_7EM__
#define __ARM_ARCH_7EM__ 0
#endif
#ifndef __ARM_ARCH_8M_MAIN__
#define __ARM_ARCH_8M_MAIN__ 0
#endif
#ifndef __ARM_ARCH_7A__
#define __ARM_ARCH_7A__ 0
#endif

// get masks
#if((__ARM_ARCH_7M__ == 1U) || (__ARM_ARCH_7EM__ == 1U) || (__ARM_ARCH_8M_MAIN__ == 1U))
#define IS_IRQ_MASKED() ((__get_PRIMASK() != 0U) || (__get_BASEPRI() != 0U))
#elif(__ARM_ARCH_6M__ == 1U)
#define IS_IRQ_MASKED() (__get_PRIMASK() != 0U)
#elif(__ARM_ARCH_7A__ == 1U)
/* CPSR mask bits */
#define CPSR_MASKBIT_I 0x80U

#define IS_IRQ_MASKED() ((__get_CPSR() & CPSR_MASKBIT_I) != 0U)
#else
#define IS_IRQ_MASKED() (__get_PRIMASK() != 0U)
#endif

// get is irq mode
#if(__ARM_ARCH_7A__ == 1U)
/* CPSR mode bitmasks */
#define CPSR_MODE_USER 0x10U
#define CPSR_MODE_SYSTEM 0x1FU

#define IS_IRQ_MODE() ((__get_mode() != CPSR_MODE_USER) && (__get_mode() != CPSR_MODE_SYSTEM))
#else
#define IS_IRQ_MODE() (__get_IPSR() != 0U)
#endif

// added osKernelGetState(), because KernelState is a static var
#define IS_IRQ() (IS_IRQ_MODE() || (IS_IRQ_MASKED() && (osKernelGetState() == osKernelRunning)))
//-------------------------end of cmsis_os2.c----------------------------

bool task_is_isr_context(void) {
    return IS_IRQ();
}
