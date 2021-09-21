#pragma once

#include "archive.h"
#include <stdint.h>
#include <furi.h>
#include <gui/gui_i.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_input.h>
#include <loader/loader.h>

#include "views/archive_browser_view.h"
#include "scenes/archive_scene.h"

typedef enum {
    ArchiveViewBrowser,
    ArchiveViewTextInput,
    ArchiveViewTotal,
} ArchiveViewEnum;

struct ArchiveApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    ArchiveBrowserView* browser;
    TextInput* text_input;
    char text_store[MAX_NAME_LEN];
};
