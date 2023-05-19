#pragma once

#include "helpers/subrem_types.h"
#include <assets_icons.h>

#include "views/remote.h"

#include "scenes/subrem_scene.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <notification/notification_messages.h>
#include <gui/modules/text_input.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>
#include <gui/modules/popup.h>

#include <flipper_format/flipper_format_i.h>

#include <lib/subghz/protocols/raw.h>

#include <lib/subghz/subghz_setting.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/transmitter.h>

#define SUBREM_APP_FOLDER EXT_PATH("subghz_remote")
#define SUBREM_MAX_LEN_NAME 64

typedef struct {
    uint32_t frequency;
    uint8_t* data;
} FreqPreset;

// Sub File preset
typedef struct {
    FlipperFormat* fff_data;
    FreqPreset freq_preset;
    FuriString* file_path;
    FuriString* protocaol_name;
    FuriString* label;
    SubGhzProtocolType type;
} SubRemSubFilePreset;

SubRemSubFilePreset* subrem_sub_file_preset_alloc();

void subrem_sub_file_preset_free(SubRemSubFilePreset* sub_preset);

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    DialogsApp* dialogs;
    Submenu* submenu;
    FuriString* file_path;
    char file_name_tmp[SUBREM_MAX_LEN_NAME];

    SubRemViewRemote* subrem_remote_view;

    SubRemSubFilePreset* subs_preset[SubRemSubKeyNameMaxCount];

    SubGhzSetting* setting;
    SubGhzEnvironment* environment;
    SubGhzReceiver* receiver;
    SubGhzTransmitter* transmitter;

    bool tx_running;

    uint8_t chusen_sub;

    // TODO: LoadFileError
} SubGhzRemoteApp;

SubRemLoadMapState subrem_load_from_file(SubGhzRemoteApp* app);

bool subrem_tx_start_sub(
    SubGhzRemoteApp* app,
    SubRemSubFilePreset* sub_preset,
    SubGhzProtocolEncoderRAWCallbackEnd callback);

bool subrem_tx_stop_sub(SubGhzRemoteApp* app, bool forced);