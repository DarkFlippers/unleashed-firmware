#pragma once

#include <stdint.h>
#include <stdbool.h>

// FreeRTOS part
#include <FreeRTOS.h>

#ifdef __cplusplus
extern "C" {
#endif

// Timeout value.
#define osWaitForever 0xFFFFFFFFU ///< Wait forever timeout value.

// Flags options (\ref furi_thread_flags_wait and \ref osEventFlagsWait).
#define osFlagsWaitAny 0x00000000U ///< Wait for any flag (default).
#define osFlagsWaitAll 0x00000001U ///< Wait for all flags.
#define osFlagsNoClear 0x00000002U ///< Do not clear flags which have been specified to wait for.

// Flags errors (returned by osThreadFlagsXxxx and osEventFlagsXxxx).
#define osFlagsError 0x80000000U ///< Error indicator.
#define osFlagsErrorUnknown 0xFFFFFFFFU ///< osError (-1).
#define osFlagsErrorTimeout 0xFFFFFFFEU ///< osErrorTimeout (-2).
#define osFlagsErrorResource 0xFFFFFFFDU ///< osErrorResource (-3).
#define osFlagsErrorParameter 0xFFFFFFFCU ///< osErrorParameter (-4).
#define osFlagsErrorISR 0xFFFFFFFAU ///< osErrorISR (-6).

/// Status code values returned by CMSIS-RTOS functions.
typedef enum {
    osOK = 0, ///< Operation completed successfully.
    osError = -1, ///< Unspecified RTOS error: run-time error but no other error message fits.
    osErrorTimeout = -2, ///< Operation not completed within the timeout period.
    osErrorResource = -3, ///< Resource not available.
    osErrorParameter = -4, ///< Parameter error.
    osErrorNoMemory =
        -5, ///< System is out of memory: it was impossible to allocate or reserve memory for the operation.
    osErrorISR =
        -6, ///< Not allowed in ISR context: the function cannot be called from interrupt service routines.
    osStatusReserved = 0x7FFFFFFF ///< Prevents enum down-size compiler optimization.
} osStatus_t;

#ifdef __cplusplus
}
#endif
