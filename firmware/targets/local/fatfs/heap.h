#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <cmsis_os.h>

#define configTOTAL_HEAP_SIZE ((size_t)(8192 * 16))
#define configAPPLICATION_ALLOCATED_HEAP 0
#define portBYTE_ALIGNMENT 8

#if portBYTE_ALIGNMENT == 8
#define portBYTE_ALIGNMENT_MASK (0x0007)
#endif

/* No test marker by default. */
#ifndef mtCOVERAGE_TEST_MARKER
#define mtCOVERAGE_TEST_MARKER()
#endif

/* No tracing by default. */
#ifndef traceMALLOC
#define traceMALLOC(pvReturn, xWantedSize)
#endif

/* No tracing by default. */
#ifndef traceFREE
#define traceFREE(pvReturn, xBlockSize)
#endif

/* No assert by default. */
#ifndef configASSERT
#define configASSERT(var)
#endif

bool prvHeapInit(void);

void acquire_memalloc_mutex();
void release_memalloc_mutex();