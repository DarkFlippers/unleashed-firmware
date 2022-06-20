#pragma once

#include "base.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

// Mutex attributes (attr_bits in \ref osMutexAttr_t).
#define osMutexRecursive 0x00000001U ///< Recursive mutex.
#define osMutexPrioInherit 0x00000002U ///< Priority inherit protocol.
#define osMutexRobust 0x00000008U ///< Robust mutex.

/// Attributes structure for mutex.
typedef struct {
    const char* name; ///< name of the mutex
    uint32_t attr_bits; ///< attribute bits
    void* cb_mem; ///< memory for control block
    uint32_t cb_size; ///< size of provided memory for control block
} osMutexAttr_t;

/// \details Mutex ID identifies the mutex.
typedef void* osMutexId_t;

/// Create and Initialize a Mutex object.
/// \param[in]     attr          mutex attributes; NULL: default values.
/// \return mutex ID for reference by other functions or NULL in case of error.
osMutexId_t osMutexNew(const osMutexAttr_t* attr);

/// Get name of a Mutex object.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \return name as null-terminated string.
const char* osMutexGetName(osMutexId_t mutex_id);

/// Acquire a Mutex or timeout if it is locked.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout);

/// Release a Mutex that was acquired by \ref osMutexAcquire.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osMutexRelease(osMutexId_t mutex_id);

/// Delete a Mutex object.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osMutexDelete(osMutexId_t mutex_id);

FuriThreadId osMutexGetOwner(osMutexId_t mutex_id);

#ifdef __cplusplus
}
#endif
