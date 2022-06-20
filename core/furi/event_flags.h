#pragma once

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Attributes structure for event flags.
typedef struct {
    const char* name; ///< name of the event flags
    uint32_t attr_bits; ///< attribute bits
    void* cb_mem; ///< memory for control block
    uint32_t cb_size; ///< size of provided memory for control block
} osEventFlagsAttr_t;

/// \details Event Flags ID identifies the event flags.
typedef void* osEventFlagsId_t;

/// Create and Initialize an Event Flags object.
/// \param[in]     attr          event flags attributes; NULL: default values.
/// \return event flags ID for reference by other functions or NULL in case of error.
osEventFlagsId_t osEventFlagsNew(const osEventFlagsAttr_t* attr);

/// Get name of an Event Flags object.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \return name as null-terminated string.
const char* osEventFlagsGetName(osEventFlagsId_t ef_id);

/// Set the specified Event Flags.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \param[in]     flags         specifies the flags that shall be set.
/// \return event flags after setting or error code if highest bit set.
uint32_t osEventFlagsSet(osEventFlagsId_t ef_id, uint32_t flags);

/// Clear the specified Event Flags.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \param[in]     flags         specifies the flags that shall be cleared.
/// \return event flags before clearing or error code if highest bit set.
uint32_t osEventFlagsClear(osEventFlagsId_t ef_id, uint32_t flags);

/// Get the current Event Flags.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \return current event flags.
uint32_t osEventFlagsGet(osEventFlagsId_t ef_id);

/// Wait for one or more Event Flags to become signaled.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \param[in]     flags         specifies the flags to wait for.
/// \param[in]     options       specifies flags options (osFlagsXxxx).
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return event flags before clearing or error code if highest bit set.
uint32_t
    osEventFlagsWait(osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout);

/// Delete an Event Flags object.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osEventFlagsDelete(osEventFlagsId_t ef_id);

#ifdef __cplusplus
}
#endif
