#pragma once

#include "archive.h"
#include <stdint.h>
#include <furi.h>
#include <gui/gui_i.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>
#include <gui/view_stack.h>
#include <dialogs/dialogs.h>
#include <gui/modules/loading.h>
#include <loader/loader.h>

#include "views/archive_browser_view.h"
#include "scenes/archive_scene.h"

typedef enum {
    ArchiveViewBrowser,
    ArchiveViewTextInput,
    ArchiveViewWidget,
    ArchiveViewTotal,
    ArchiveViewStack,
} ArchiveViewEnum;

struct ArchiveApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    ViewStack* view_stack;
    SceneManager* scene_manager;
    ArchiveBrowserView* browser;
    TextInput* text_input;
    Widget* widget;
    DialogsApp* dialogs;
    Loading* loading;
    FuriPubSubSubscription* loader_stop_subscription;

    FuriString* fav_move_str;
    FuriString* dst_path;
    char text_store[MAX_NAME_LEN];
    char file_extension[MAX_EXT_LEN + 1];
};

void archive_show_loading_popup(ArchiveApp* context, bool show);