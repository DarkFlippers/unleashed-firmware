#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/popup.h>

#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>

#include "scenes/fuzzer_scene.h"
#include "views/main_menu.h"
#include "views/attack.h"
#include "views/field_editor.h"

#include "helpers/fuzzer_types.h"
#include "lib/worker/fake_worker.h"

#include <flipper_format/flipper_format_i.h>
#include "fuzzer_icons.h"

#define FUZZ_TIME_DELAY_MAX (80)

typedef struct {
    const char* custom_dict_extension;
    const char* custom_dict_folder;
    const char* key_extension;
    const char* path_key_folder;
    const Icon* key_icon;
} FuzzerConsts;

typedef struct {
    Gui* gui;
    NotificationApp* notifications;

    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    Popup* popup;
    DialogsApp* dialogs;
    FuzzerViewMain* main_view;
    FuzzerViewAttack* attack_view;
    FuzzerViewFieldEditor* field_editor_view;

    FuriString* file_path;

    FuzzerState fuzzer_state;
    FuzzerConsts* fuzzer_const;

    FuzzerWorker* worker;
    FuzzerPayload* payload;
} PacsFuzzerApp;