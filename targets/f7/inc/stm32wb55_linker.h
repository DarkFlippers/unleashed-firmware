/**
 * @file stm32wb55_linker.h
 *
 * Linker defined symbols. Used in various part of firmware to understand
 * hardware boundaries.
 * 
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern const void _stack_end; /**< end of stack */
extern const void _stack_size; /**< stack size */

extern const void _sidata; /**< data initial value start */
extern const void _sdata; /**< data start */
extern const void _edata; /**< data end */

extern const void _sbss; /**< bss start */
extern const void _ebss; /**< bss end */

extern const void _sMB_MEM2; /**< RAM2a start */
extern const void _eMB_MEM2; /**< RAM2a end */

extern const void __heap_start__; /**< RAM1 Heap start */
extern const void __heap_end__; /**< RAM1 Heap end */

extern const void __free_flash_start__; /**< Free Flash space start */

#ifdef __cplusplus
}
#endif
