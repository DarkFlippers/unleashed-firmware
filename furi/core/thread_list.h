#pragma once

#include "base.h"
#include "common_defines.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    FuriThread* thread; /**< Pointer to FuriThread, valid while it is running */
    const char* app_id; /**< Thread application id, valid while it is running */
    const char* name; /**< Thread name, valid while it is running */
    FuriThreadPriority priority; /**< Thread priority */
    uint32_t stack_address; /**< Thread stack address */
    size_t heap; /**< Thread heap size if tracking enabled, 0 - otherwise */
    uint32_t stack_size; /**< Thread stack size */
    uint32_t stack_min_free; /**< Thread minimum of the stack size ever reached */
    const char*
        state; /**< Thread state, can be: "Running", "Ready", "Blocked", "Suspended", "Deleted", "Invalid" */
    float cpu; /**< Thread CPU usage time in percents (including interrupts happened while running) */

    // Service variables
    uint32_t counter_previous; /**< Thread previous runtime counter */
    uint32_t counter_current; /**< Thread current runtime counter */
    uint32_t tick; /**< Thread last seen tick */
} FuriThreadListItem;

/** Anonymous FuriThreadList type */
typedef struct FuriThreadList FuriThreadList;

/** Allocate FuriThreadList instance
 *
 * @return     FuriThreadList instance
 */
FuriThreadList* furi_thread_list_alloc(void);

/** Free FuriThreadList instance
 *
 * @param      instance  The FuriThreadList instance to free
 */
void furi_thread_list_free(FuriThreadList* instance);

/** Get FuriThreadList instance size
 *
 * @param      instance  The instance
 *
 * @return     Item count
 */
size_t furi_thread_list_size(FuriThreadList* instance);

/** Get item at position
 *
 * @param      instance  The FuriThreadList instance
 * @param[in]  position  The position of the item
 *
 * @return     The FuriThreadListItem instance
 */
FuriThreadListItem* furi_thread_list_get_at(FuriThreadList* instance, size_t position);

/** Get item by thread FuriThread pointer
 *
 * @param      instance  The FuriThreadList instance
 * @param      thread    The FuriThread pointer
 *
 * @return     The FuriThreadListItem instance
 */
FuriThreadListItem* furi_thread_list_get_or_insert(FuriThreadList* instance, FuriThread* thread);

/** Get percent of time spent in ISR
 *
 * @param      instance  The instance
 *
 * @return     percent of time spent in ISR
 */
float furi_thread_list_get_isr_time(FuriThreadList* instance);

#ifdef __cplusplus
}
#endif
