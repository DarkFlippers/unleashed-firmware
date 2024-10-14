#pragma once

#include "thread_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Process items in the FuriThreadList instance
 *
 * @param      instance  The instance
 * @param[in]  runtime   The runtime of the system since start
 * @param[in]  tick      The tick when processing happened
 */
void furi_thread_list_process(FuriThreadList* instance, uint32_t runtime, uint32_t tick);

#ifdef __cplusplus
}
#endif
