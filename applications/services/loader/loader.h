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
} LoaderStatus;

typedef enum {
    LoaderEventTypeApplicationBeforeLoad,
    LoaderEventTypeApplicationLoadFailed,
    LoaderEventTypeApplicationStopped
} LoaderEventType;

typedef struct {
    LoaderEventType type;
} LoaderEvent;

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
 * @param[in,out] arg optional argument (can be of any value, including NULL)
 *
 * @return true if the signal was handled by the application, false otherwise
 */
bool loader_signal(Loader* instance, uint32_t signal, void* arg);

/**
 * @brief Get the name of the currently running application
 *
 * @param[in] instance pointer to the loader instance
 * @param[in,out] name pointer to the string to contain the name (must be allocated)
 * @return true if it was possible to get an application name, false otherwise
 */
bool loader_get_application_name(Loader* instance, FuriString* name);

#ifdef __cplusplus
}
#endif
