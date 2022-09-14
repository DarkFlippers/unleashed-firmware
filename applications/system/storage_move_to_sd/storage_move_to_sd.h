#pragma once
#include "gui/modules/widget_elements/widget_element_i.h"
#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>

#include <gui/modules/widget.h>
#include <gui/modules/popup.h>

#include <storage/storage.h>
#include <storage/storage_sd_api.h>

#include "scenes/storage_move_to_sd_scene.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MoveToSdCustomEventExit,
    MoveToSdCustomEventConfirm,
} MoveToSdCustomEvent;

typedef struct {
    // records
    Gui* gui;
    Widget* widget;
    NotificationApp* notifications;

    // view managment
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    FuriPubSubSubscription* sub;

} StorageMoveToSd;

typedef enum {
    StorageMoveToSdViewWidget,
} StorageMoveToSdView;

bool storage_move_to_sd_perform(void);

#ifdef __cplusplus
}
#endif
