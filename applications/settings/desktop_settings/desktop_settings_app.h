#pragma once

#include <gui/gui.h>
#include <gui/modules/popup.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/text_input.h>
#include <dialogs/dialogs.h>
#include <assets_icons.h>

#include <desktop/desktop_settings.h>
#include <desktop/views/desktop_view_pin_input.h>
#include "views/desktop_settings_view_pin_setup_howto.h"
#include "views/desktop_settings_view_pin_setup_howto2.h"

#include <furi_hal_version.h>

typedef enum {
    DesktopSettingsAppViewMenu,
    DesktopSettingsAppViewVarItemList,
    DesktopSettingsAppViewIdPopup,
    DesktopSettingsAppViewIdPinInput,
    DesktopSettingsAppViewIdPinSetupHowto,
    DesktopSettingsAppViewIdPinSetupHowto2,
    DesktopSettingsAppViewTextInput,
} DesktopSettingsAppView;

typedef struct {
    DesktopSettings settings;

    Gui* gui;
    DialogsApp* dialogs;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    VariableItemList* variable_item_list;
    Submenu* submenu;
    TextInput* text_input;
    Popup* popup;
    DesktopViewPinInput* pin_input_view;
    DesktopSettingsViewPinSetupHowto* pin_setup_howto_view;
    DesktopSettingsViewPinSetupHowto2* pin_setup_howto2_view;

    DesktopPinCode pincode_buffer;
    bool pincode_buffer_filled;

    bool save_name;
    char device_name[FURI_HAL_VERSION_ARRAY_NAME_LENGTH];

    uint8_t menu_idx;
    uint32_t pin_menu_idx;
} DesktopSettingsApp;
