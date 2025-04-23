#pragma once
#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <assets_icons.h>
#include <notification/notification_messages.h>

#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/popup.h>

#include <storage/storage.h>
#include <storage/storage_sd_api.h>

#include "scenes/storage_settings_scene.h"

#include <settings_helpers/submenu_based.h>

#define STORAGE_SETTINGS_MOUNT_INDEX 2

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    // records
    Gui* gui;
    NotificationApp* notification;
    Storage* fs_api;

    // view management
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    // view modules
    Submenu* submenu;
    DialogEx* dialog_ex;

    // text
    FuriString* text_string;

    // helpers
    SubmenuSettingsHelperDescriptor* helper_descriptor;
    SubmenuSettingsHelper* settings_helper;
} StorageSettings;

typedef enum {
    StorageSettingsViewSubmenu,
    StorageSettingsViewDialogEx,
} StorageSettingsView;

#ifdef __cplusplus
}
#endif
