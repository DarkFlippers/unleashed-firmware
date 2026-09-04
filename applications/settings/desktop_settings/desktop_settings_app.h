#pragma once

#include <gui/gui.h>
#include <gui/modules/popup.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/text_input.h>
#include <gui/modules/dialog_ex.h>
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
    DesktopSettingsAppViewDialogEx,
} DesktopSettingsAppView;

typedef struct {
    FuriString* file;
    FuriString* name;
} DesktopSettingsMenuStyleEntry;

typedef struct {
    DesktopSettings settings;
    DesktopSettingsMenuStyleEntry* menu_styles;
    size_t menu_styles_count;
    bool menu_styles_loaded;

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
    DialogEx* dialog_ex;

    DesktopPinCode pincode_buffer;
    bool pincode_buffer_filled;

    bool save_name;
    char device_name[FURI_HAL_VERSION_ARRAY_NAME_LENGTH];

    uint8_t menu_idx;
    uint32_t pin_menu_idx;
} DesktopSettingsApp;

/** Scan the loader plugin directory into app->menu_styles, replacing whatever is there - a scan
 * that fails leaves the list empty.
 *
 * Costs an SD manifest read per plugin, so the caller should have something on screen first.
 * Sets menu_styles_loaded only when the directory was read through to the end, so that a scan cut
 * short - nothing there to look at, or a card removed part way - is retried on the next call
 * rather than remembered.
 */
void desktop_settings_menu_styles_load(DesktopSettingsApp* app);
