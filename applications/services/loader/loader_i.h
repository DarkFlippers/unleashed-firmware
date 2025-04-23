#pragma once
#include <furi.h>
#include <toolbox/api_lock.h>
#include <flipper_application/flipper_application.h>

#include <gui/gui.h>
#include <gui/view_holder.h>
#include <gui/modules/loading.h>

#include <m-array.h>

#include "loader.h"
#include "loader_menu.h"
#include "loader_applications.h"
#include "loader_queue.h"

typedef struct {
    FuriString* launch_path;
    char* args;
    FuriThread* thread;
    bool insomniac;
    FlipperApplication* fap;
} LoaderAppData;

struct Loader {
    FuriPubSub* pubsub;
    FuriMessageQueue* queue;
    LoaderMenu* loader_menu;
    LoaderApplications* loader_applications;
    LoaderAppData app;

    LoaderLaunchQueue launch_queue;

    Gui* gui;
    ViewHolder* view_holder;
    Loading* loading;
};

typedef enum {
    LoaderMessageTypeStartByName,
    LoaderMessageTypeAppClosed,
    LoaderMessageTypeShowMenu,
    LoaderMessageTypeMenuClosed,
    LoaderMessageTypeApplicationsClosed,
    LoaderMessageTypeLock,
    LoaderMessageTypeUnlock,
    LoaderMessageTypeIsLocked,
    LoaderMessageTypeStartByNameDetachedWithGuiError,
    LoaderMessageTypeSignal,
    LoaderMessageTypeGetApplicationName,
    LoaderMessageTypeGetApplicationLaunchPath,
    LoaderMessageTypeEnqueueLaunch,
    LoaderMessageTypeClearLaunchQueue,
} LoaderMessageType;

typedef struct {
    const char* name;
    const char* args;
    FuriString* error_message;
} LoaderMessageStartByName;

typedef struct {
    uint32_t signal;
    void* arg;
} LoaderMessageSignal;

typedef enum {
    LoaderStatusErrorUnknown,
    LoaderStatusErrorInvalidFile,
    LoaderStatusErrorInvalidManifest,
    LoaderStatusErrorMissingImports,
    LoaderStatusErrorHWMismatch,
    LoaderStatusErrorOutdatedApp,
    LoaderStatusErrorOutOfMemory,
    LoaderStatusErrorOutdatedFirmware,
} LoaderStatusError;

typedef struct {
    LoaderStatus value;
    LoaderStatusError error;
} LoaderMessageLoaderStatusResult;

typedef struct {
    bool value;
} LoaderMessageBoolResult;

typedef struct {
    FuriApiLock api_lock;
    LoaderMessageType type;

    union {
        LoaderMessageStartByName start;
        LoaderDeferredLaunchRecord defer_start;
        LoaderMessageSignal signal;
        FuriString* application_name;
    };

    union {
        LoaderMessageLoaderStatusResult* status_value;
        LoaderMessageBoolResult* bool_value;
    };
} LoaderMessage;
