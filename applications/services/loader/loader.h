#pragma once

#include <core/pubsub.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_LOADER "loader"

typedef struct Loader Loader;

typedef enum {
    LoaderStatusOk,
    LoaderStatusErrorAppStarted,
    LoaderStatusErrorUnknownApp,
    LoaderStatusErrorInternal,
} LoaderStatus;

typedef enum {
    LoaderEventTypeApplicationStarted,
    LoaderEventTypeApplicationStopped
} LoaderEventType;

typedef struct {
    LoaderEventType type;
} LoaderEvent;

/** Start application
 * @param name - application name
 * @param args - application arguments
 * @retval true on success
 */
LoaderStatus loader_start(Loader* instance, const char* name, const char* args);

/** Lock application start
 * @retval true on success
 */
bool loader_lock(Loader* instance);

/** Unlock application start */
void loader_unlock(Loader* instance);

/** Get loader lock status */
bool loader_is_locked(const Loader* instance);

/** Show primary loader */
void loader_show_menu();

/** Show primary loader */
void loader_update_menu();

/** Show primary loader */
FuriPubSub* loader_get_pubsub(Loader* instance);

#ifdef __cplusplus
}
#endif
