#pragma once
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_LOADER            "loader"
#define LOADER_APPLICATIONS_NAME "Apps"

typedef struct Loader Loader;

typedef enum {
    LoaderStatusOk,
    LoaderStatusErrorAppStarted,
    LoaderStatusErrorUnknownApp,
    LoaderStatusErrorInternal,
    LoaderStatusErrorApiMismatch,
    LoaderStatusErrorApiMismatchExit,
} LoaderStatus;

typedef enum {
    LoaderEventTypeApplicationBeforeLoad,
    LoaderEventTypeApplicationLoadFailed,
    LoaderEventTypeApplicationStopped,
    LoaderEventTypeNoMoreAppsInQueue, //<! The normal `Stopped` event still fires before this one
} LoaderEventType;

typedef struct {
    LoaderEventType type;
} LoaderEvent;

typedef enum {
    LoaderDeferredLaunchFlagNone = 0,
    LoaderDeferredLaunchFlagGui = (1 << 1), //<! Report launch to the user via a dialog
} LoaderDeferredLaunchFlag;

/**
 * @brief Start application
 * @param[in] instance loader instance
 * @param[in] name application name or id
 * @param[in] args application arguments
 * @param[out] error_message detailed error message, can be NULL
 * @return LoaderStatus
 */
LoaderStatus
    loader_start(Loader* instance, const char* name, const char* args, FuriString* error_message);

/**
 * @brief Start application with GUI error message
 * @param[in] instance loader instance
 * @param[in] name application name or id
 * @param[in] args application arguments
 * @return LoaderStatus
 */
LoaderStatus loader_start_with_gui_error(Loader* loader, const char* name, const char* args);

/**
 * @brief Start application detached with GUI error message
 * @param[in] instance loader instance
 * @param[in] name application name or id
 * @param[in] args application arguments
 */
void loader_start_detached_with_gui_error(Loader* loader, const char* name, const char* args);

/**
 * @brief Lock application start
 * @param[in] instance loader instance
 * @return true on success
 */
bool loader_lock(Loader* instance);

/**
 * @brief Unlock application start
 * @param[in] instance loader instance
 */
void loader_unlock(Loader* instance);

/**
 * @brief Check if loader is locked
 * @param[in] instance loader instance
 * @return true if locked
 */
bool loader_is_locked(Loader* instance);

/**
 * @brief Show loader menu
 * @param[in] instance loader instance
 */
void loader_show_menu(Loader* instance);

/**
 * @brief Get loader pubsub
 * @param[in] instance loader instance
 * @return FuriPubSub* 
 */
FuriPubSub* loader_get_pubsub(Loader* instance);

/**
 * @brief Send a signal to the currently running application
 *
 * @param[in] instance pointer to the loader instance
 * @param[in] signal signal value to be sent
 * @param[inout] arg optional argument (can be of any value, including NULL)
 *
 * @return true if the signal was handled by the application, false otherwise
 */
bool loader_signal(Loader* instance, uint32_t signal, void* arg);

/**
 * @brief Get the name of the currently running application
 *
 * @param[in] instance pointer to the loader instance
 * @param[inout] name pointer to the string to contain the name (must be allocated)
 * @return true if it was possible to get an application name, false otherwise
 */
bool loader_get_application_name(Loader* instance, FuriString* name);

/**
 * @brief Get the launch path or name of the currently running application
 * 
 * This is the string that was supplied to `loader_start` such that the current
 * app is running now. It might be a name (in the case of internal apps) or a
 * path (in the case of external apps). This value can be used to launch the
 * same app again.
 *
 * @param[in] instance pointer to the loader instance
 * @param[inout] name pointer to the string to contain the path or name
 *                    (must be allocated)
 * @return true if it was possible to get an application path, false otherwise
 */
bool loader_get_application_launch_path(Loader* instance, FuriString* name);

/**
 * @brief Enqueues a request to launch an application after the current one
 * 
 * @param[in] instance pointer to the loader instance
 * @param[in] name pointer to the name or path of the application, or NULL to
 *                 cancel a previous request
 * @param[in] args pointer to argument to provide to the next app
 * @param[in] flags additional flags. see enum documentation for more info
 */
void loader_enqueue_launch(
    Loader* instance,
    const char* name,
    const char* args,
    LoaderDeferredLaunchFlag flags);

/**
 * @brief Removes all requests to launch applications after the current one
 * exits
 * 
 * @param[in] instance pointer to the loader instance
 */
void loader_clear_launch_queue(Loader* instance);

#ifdef __cplusplus
}
#endif
