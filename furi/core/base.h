#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <furi_config.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FuriWaitForever = 0xFFFFFFFFU,
} FuriWait;

typedef enum {
    FuriFlagWaitAny = 0x00000000U, ///< Wait for any flag (default).
    FuriFlagWaitAll = 0x00000001U, ///< Wait for all flags.
    FuriFlagNoClear = 0x00000002U, ///< Do not clear flags which have been specified to wait for.

    FuriFlagError = 0x80000000U, ///< Error indicator.
    FuriFlagErrorUnknown = 0xFFFFFFFFU, ///< FuriStatusError (-1).
    FuriFlagErrorTimeout = 0xFFFFFFFEU, ///< FuriStatusErrorTimeout (-2).
    FuriFlagErrorResource = 0xFFFFFFFDU, ///< FuriStatusErrorResource (-3).
    FuriFlagErrorParameter = 0xFFFFFFFCU, ///< FuriStatusErrorParameter (-4).
    FuriFlagErrorISR = 0xFFFFFFFAU, ///< FuriStatusErrorISR (-6).
} FuriFlag;

typedef enum {
    FuriStatusOk = 0, ///< Operation completed successfully.
    FuriStatusError =
        -1, ///< Unspecified RTOS error: run-time error but no other error message fits.
    FuriStatusErrorTimeout = -2, ///< Operation not completed within the timeout period.
    FuriStatusErrorResource = -3, ///< Resource not available.
    FuriStatusErrorParameter = -4, ///< Parameter error.
    FuriStatusErrorNoMemory =
        -5, ///< System is out of memory: it was impossible to allocate or reserve memory for the operation.
    FuriStatusErrorISR =
        -6, ///< Not allowed in ISR context: the function cannot be called from interrupt service routines.
    FuriStatusReserved = 0x7FFFFFFF ///< Prevents enum down-size compiler optimization.
} FuriStatus;

typedef enum {
    FuriSignalExit, /**< Request (graceful) exit. */
    // Other standard signals may be added in the future
    FuriSignalCustom = 100, /**< Custom signal values start from here. */
} FuriSignal;

#ifdef __cplusplus
}
#endif
