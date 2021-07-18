#pragma once

#include <stdbool.h>

typedef struct Loader Loader;

/** Start application
 * @param name - application name
 * @param args - application arguments
 * @retval true on success
 */
bool loader_start(Loader* instance, const char* name, const char* args);

/** Lock application start
 * @retval true on success
 */
bool loader_lock(Loader* instance);

/** Unlock application start */
void loader_unlock(Loader* instance);
