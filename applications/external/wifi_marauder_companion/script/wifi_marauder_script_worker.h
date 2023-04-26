#pragma once

#include "wifi_marauder_script.h"

typedef enum {
    WifiMarauderScriptWorkerStatusSuccess = 0,
    WifiMarauderScriptWorkerStatusInvalidScript = 1,
    WifiMarauderScriptWorkerStatusForceExit = 2
} WifiMarauderScriptWorkerStatus;

typedef struct WifiMarauderScriptWorker {
    WifiMarauderScript* script;
    FuriThread* worker_thread;
    void (*callback_start)(void*);
    void (*callback_stage)(WifiMarauderScriptStage*, void*);
    void* context;
    bool is_running;
} WifiMarauderScriptWorker;

/**
 * @brief Allocates a new instance of WifiMarauderScriptWorker.
 *
 * @return A pointer to the allocated instance or NULL if allocation fails.
 */
WifiMarauderScriptWorker* wifi_marauder_script_worker_alloc();

/**
 * @brief Starts the execution of the worker and sets the callback function to be called after each stage is executed.
 *
 * @param instance A pointer to the instance of WifiMarauderScriptWorker to start.
 * @param script Script to be executed
 * @return True if the worker was successfully started, false otherwise.
 */
bool wifi_marauder_script_worker_start(
    WifiMarauderScriptWorker* instance,
    WifiMarauderScript* script);

/**
 * @brief Frees the memory used by the instance of WifiMarauderScriptWorker.
 *
 * @param script A pointer to the instance of WifiMarauderScriptWorker to free.
 */
void wifi_marauder_script_worker_free(WifiMarauderScriptWorker* script);
