#pragma once

#include "scenes/file_browser_scene.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/file_browser.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>

typedef struct FileBrowserApp FileBrowserApp;

struct FileBrowserApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    DialogsApp* dialogs;
    Widget* widget;
    FileBrowser* file_browser;

    string_t file_path;
};

typedef enum {
    FileBrowserAppViewStart,
    FileBrowserAppViewBrowser,
    FileBrowserAppViewResult,
} FileBrowserAppView;
