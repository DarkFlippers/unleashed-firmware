#pragma once

#include <gui/gui.h>
#include <gui/modules/popup.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/dialog_ex.h>
#include <dialogs/dialogs.h>
#include <assets_icons.h>

#include <desktop/desktop_settings.h>
#include <desktop/views/desktop_view_pin_input.h>
#include "views/desktop_settings_view_pin_setup_howto.h"
#include "views/desktop_settings_view_pin_setup_howto2.h"

typedef enum {
    DesktopSettingsAppViewMenu,
    DesktopSettingsAppViewVarItemList,
    DesktopSettingsAppViewIdPopup,
    DesktopSettingsAppViewIdPinInput,
    DesktopSettingsAppViewIdPinSetupHowto,
    DesktopSettingsAppViewIdPinSetupHowto2,
    DesktopSettingsAppViewDialogEx,
} DesktopSettingsAppView;

typedef struct {
    DesktopSettings settings;

    Gui* gui;
    DialogsApp* dialogs;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    VariableItemList* variable_item_list;
    Submenu* submenu;
    Popup* popup;
    DesktopViewPinInput* pin_input_view;
    DesktopSettingsViewPinSetupHowto* pin_setup_howto_view;
    DesktopSettingsViewPinSetupHowto2* pin_setup_howto2_view;
    DialogEx* dialog_ex;

    DesktopPinCode pincode_buffer;
    bool pincode_buffer_filled;

    uint32_t pin_menu_idx;
    uint32_t quick_apps_menu_idx;
    uint32_t quick_apps_direction_menu_idx;
} DesktopSettingsApp;
