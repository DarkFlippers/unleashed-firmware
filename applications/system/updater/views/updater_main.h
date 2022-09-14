#pragma once

#include <gui/view.h>

typedef struct UpdaterMainView UpdaterMainView;
typedef struct FuriPubSubSubscription FuriPubSubSubscription;
typedef struct ViewDispatcher ViewDispatcher;
typedef void (*UpdaterMainInputCallback)(InputType type, void* context);

View* updater_main_get_view(UpdaterMainView* main_view);

UpdaterMainView* updater_main_alloc();

void updater_main_free(UpdaterMainView* main_view);

void updater_main_model_set_state(
    UpdaterMainView* main_view,
    const char* message,
    uint8_t progress,
    bool failed);

void updater_main_set_storage_pubsub(UpdaterMainView* main_view, FuriPubSubSubscription* sub);

FuriPubSubSubscription* updater_main_get_storage_pubsub(UpdaterMainView* main_view);

void updater_main_set_view_dispatcher(UpdaterMainView* main_view, ViewDispatcher* view_dispatcher);
