#pragma once

#include "chip8_app.h"
#include "scenes/chip8_scene.h"
#include "chip8.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <dialogs/dialogs.h>
#include <gui/modules/variable_item_list.h>
#include "views/chip8_view.h"

#define CHIP8_APP_PATH_FOLDER "/any/chip8"
#define CHIP8_APP_EXTENSION ".ch8"

struct Chip8App {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    DialogsApp* dialogs;

    string_t file_name;
    uint8_t** backup_screen;
    Chip8View* chip8_view;
    Chip8Emulator* chip8;
};

typedef enum {
    Chip8FileSelectView,
    Chip8WorkView,
} Chip8AppView;
