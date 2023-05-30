#pragma once

#include "helpers/subrem_types.h"
#include "helpers/subrem_presets.h"
#include "scenes/subrem_scene.h"

#include "helpers/txrx/subghz_txrx.h"
#include <subrem_configurator_icons.h>

#include "views/remote.h"
#include "views/edit_menu.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <gui/modules/text_input.h>
#include <gui/modules/popup.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>

#include <flipper_format/flipper_format_i.h>

#define SUBREM_APP_FOLDER EXT_PATH("subghz_remote")
#define SUBREM_MAX_LEN_NAME 64

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    DialogsApp* dialogs;
    Widget* widget;
    Popup* popup;
    TextInput* text_input;
    Submenu* submenu;

    FuriString* file_path;
    char file_name_tmp[SUBREM_MAX_LEN_NAME];

    SubRemViewRemote* subrem_remote_view;
    SubRemViewEditMenu* subrem_edit_menu;

    SubRemMapPreset* map_preset;

    SubGhzTxRx* txrx;

    bool map_not_saved;

    uint8_t chusen_sub;
} SubGhzRemoteApp;

SubRemLoadMapState subrem_load_from_file(SubGhzRemoteApp* app);

SubRemLoadMapState subrem_map_file_load(SubGhzRemoteApp* app, const char* file_path);

void subrem_map_preset_reset(SubRemMapPreset* map_preset);

bool subrem_save_map_to_file(SubGhzRemoteApp* app);

void subrem_save_active_sub(void* context);