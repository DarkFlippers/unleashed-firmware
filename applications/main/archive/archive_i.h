#pragma once

#include "archive.h"
#include <stdint.h>
#include <furi.h>
#include <gui/gui_i.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>
#include <loader/loader.h>

#include "views/archive_browser_view.h"
#include "scenes/archive_scene.h"

typedef enum {
    ArchiveViewBrowser,
    ArchiveViewTextInput,
    ArchiveViewWidget,
    ArchiveViewTotal,
} ArchiveViewEnum;

struct ArchiveApp {
    Gui* gui;
    Loader* loader;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    ArchiveBrowserView* browser;
    TextInput* text_input;
    Widget* widget;
    FuriPubSubSubscription* loader_stop_subscription;
    FuriString* fav_move_str;
    char text_store[MAX_NAME_LEN];
    char file_extension[MAX_EXT_LEN + 1];
};
