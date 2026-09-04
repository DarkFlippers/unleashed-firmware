#include <furi.h>
#include <gui/modules/popup.h>
#include <gui/modules/dialog_ex.h>
#include <gui/scene_manager.h>
#include <namechanger/namechanger.h>
#include <flipper_format/flipper_format.h>
#include <power/power_service/power.h>

#include <desktop/desktop.h>
#include <desktop/views/desktop_view_pin_input.h>

#include <desktop/desktop.h>
#include <desktop/views/desktop_view_pin_input.h>

#include "desktop_settings_app.h"
#include "scenes/desktop_settings_scene.h"

#include <storage/storage.h>
#include <flipper_application/flipper_application.h>
#include <loader/loader.h>

#define TAG "DesktopSettings"

// variable_item_list_add() takes a uint8_t values count, and the start scene passes
// menu_styles_count plus one for "Default" - this is what stops that from wrapping
#define MENU_STYLES_MAX (UINT8_MAX - 1)

static void desktop_settings_menu_styles_free(DesktopSettingsApp* app);

void desktop_settings_menu_styles_load(DesktopSettingsApp* app) {
    desktop_settings_menu_styles_free(app);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* dir = storage_file_alloc(storage);
    FuriString* path = furi_string_alloc();
    FuriString* name = furi_string_alloc();
    FuriString* file_name = furi_string_alloc();
    char file[64];
    uint8_t icon[FAP_MANIFEST_MAX_ICON_SIZE];
    uint8_t* icon_ptr = icon;

    if(storage_dir_open(dir, LOADER_MENU_STYLES_PATH)) {
        while(storage_dir_read(dir, NULL, file, sizeof(file))) {
            // The directory holds every loader plugin, so filter on the menu style appid prefix
            furi_string_set_str(file_name, file);
            if(!furi_string_start_with_str(file_name, LOADER_MENU_STYLE_PREFIX) ||
               !furi_string_end_with_str(file_name, ".fal")) {
                continue;
            }
            // This one is a menu style we cannot offer, rather than something we filtered out
            if(furi_string_size(file_name) >= sizeof(app->settings.menu_style)) {
                FURI_LOG_W(TAG, "Menu style name too long, ignoring %s", file);
                continue;
            }
            if(app->menu_styles_count >= MENU_STYLES_MAX) {
                FURI_LOG_W(TAG, "More than %u menu styles, ignoring the rest", MENU_STYLES_MAX);
                break;
            }
            furi_string_printf(path, "%s/%s", LOADER_MENU_STYLES_PATH, file);
            if(!flipper_application_load_name_and_icon(path, storage, &icon_ptr, name)) {
                FURI_LOG_W(TAG, "Skipping unreadable menu style %s", file);
                continue;
            }
            size_t pos = app->menu_styles_count;
            app->menu_styles =
                realloc(app->menu_styles, (pos + 1) * sizeof(DesktopSettingsMenuStyleEntry));
            while(pos && furi_string_cmp(app->menu_styles[pos - 1].name, name) > 0) {
                app->menu_styles[pos] = app->menu_styles[pos - 1];
                pos--;
            }
            app->menu_styles[pos].file = furi_string_alloc_set_str(file);
            app->menu_styles[pos].name = furi_string_alloc_set(name);
            app->menu_styles_count++;
        }
        app->menu_styles_loaded = true;
    } else {
        // Not the same as having no styles installed - leave it uncached so that a card which
        // shows up later, or a directory that is not there yet, is picked up on the next entry
        FURI_LOG_W(TAG, "Cannot open %s, no menu styles offered", LOADER_MENU_STYLES_PATH);
    }

    storage_dir_close(dir);
    storage_file_free(dir);
    furi_string_free(path);
    furi_string_free(name);
    furi_string_free(file_name);
    furi_record_close(RECORD_STORAGE);
}

static void desktop_settings_menu_styles_free(DesktopSettingsApp* app) {
    for(size_t i = 0; i < app->menu_styles_count; i++) {
        furi_string_free(app->menu_styles[i].file);
        furi_string_free(app->menu_styles[i].name);
    }
    free(app->menu_styles);
    app->menu_styles = NULL;
    app->menu_styles_count = 0;
    app->menu_styles_loaded = false;
}

static bool desktop_settings_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    DesktopSettingsApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool desktop_settings_back_event_callback(void* context) {
    furi_assert(context);
    DesktopSettingsApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

DesktopSettingsApp* desktop_settings_app_alloc(void) {
    DesktopSettingsApp* app = malloc(sizeof(DesktopSettingsApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&desktop_settings_scene_handlers, app);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, desktop_settings_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, desktop_settings_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Claim the screen at the first moment we are able to. Until a view is current the ViewPort
    // stays disabled and the GUI draws whatever is underneath - the menu we were opened from -
    // and the loader lets go of its own loading animation as soon as this thread starts.
    app->loading = loading_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, DesktopSettingsAppViewLoading, loading_get_view(app->loading));
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewLoading);

    app->popup = popup_alloc();
    app->submenu = submenu_alloc();
    app->variable_item_list = variable_item_list_alloc();
    app->pin_input_view = desktop_view_pin_input_alloc();
    app->pin_setup_howto_view = desktop_settings_view_pin_setup_howto_alloc();
    app->pin_setup_howto2_view = desktop_settings_view_pin_setup_howto2_alloc();
    app->dialog_ex = dialog_ex_alloc();

    view_dispatcher_add_view(
        app->view_dispatcher, DesktopSettingsAppViewMenu, submenu_get_view(app->submenu));
    view_dispatcher_add_view(
        app->view_dispatcher,
        DesktopSettingsAppViewVarItemList,
        variable_item_list_get_view(app->variable_item_list));
    view_dispatcher_add_view(
        app->view_dispatcher, DesktopSettingsAppViewIdPopup, popup_get_view(app->popup));
    view_dispatcher_add_view(
        app->view_dispatcher,
        DesktopSettingsAppViewIdPinInput,
        desktop_view_pin_input_get_view(app->pin_input_view));
    view_dispatcher_add_view(
        app->view_dispatcher,
        DesktopSettingsAppViewIdPinSetupHowto,
        desktop_settings_view_pin_setup_howto_get_view(app->pin_setup_howto_view));
    view_dispatcher_add_view(
        app->view_dispatcher,
        DesktopSettingsAppViewIdPinSetupHowto2,
        desktop_settings_view_pin_setup_howto2_get_view(app->pin_setup_howto2_view));
    view_dispatcher_add_view(
        app->view_dispatcher, DesktopSettingsAppViewDialogEx, dialog_ex_get_view(app->dialog_ex));

    // Text Input
    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        DesktopSettingsAppViewTextInput,
        text_input_get_view(app->text_input));

    return app;
}

void desktop_settings_app_free(DesktopSettingsApp* app) {
    furi_assert(app);

    bool temp_save_name = app->save_name;
    // Save name if set or remove file
    if(temp_save_name) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        if(strcmp(app->device_name, "") == 0) {
            storage_simply_remove(storage, NAMECHANGER_PATH);
        } else {
            FlipperFormat* file = flipper_format_file_alloc(storage);

            do {
                if(!flipper_format_file_open_always(file, NAMECHANGER_PATH)) break;
                if(!flipper_format_write_header_cstr(file, NAMECHANGER_HEADER, NAMECHANGER_VERSION))
                    break;
                if(!flipper_format_write_string_cstr(file, "Name", app->device_name)) break;
            } while(0);

            flipper_format_free(file);
        }
        furi_record_close(RECORD_STORAGE);
    }

    // Variable item list
    view_dispatcher_remove_view(app->view_dispatcher, DesktopSettingsAppViewMenu);
    view_dispatcher_remove_view(app->view_dispatcher, DesktopSettingsAppViewVarItemList);
    view_dispatcher_remove_view(app->view_dispatcher, DesktopSettingsAppViewIdPopup);
    view_dispatcher_remove_view(app->view_dispatcher, DesktopSettingsAppViewIdPinInput);
    view_dispatcher_remove_view(app->view_dispatcher, DesktopSettingsAppViewIdPinSetupHowto);
    view_dispatcher_remove_view(app->view_dispatcher, DesktopSettingsAppViewIdPinSetupHowto2);
    view_dispatcher_remove_view(app->view_dispatcher, DesktopSettingsAppViewDialogEx);
    view_dispatcher_remove_view(app->view_dispatcher, DesktopSettingsAppViewLoading);
    // TextInput
    view_dispatcher_remove_view(app->view_dispatcher, DesktopSettingsAppViewTextInput);
    text_input_free(app->text_input);

    variable_item_list_free(app->variable_item_list);
    submenu_free(app->submenu);
    popup_free(app->popup);
    desktop_view_pin_input_free(app->pin_input_view);
    desktop_settings_view_pin_setup_howto_free(app->pin_setup_howto_view);
    desktop_settings_view_pin_setup_howto2_free(app->pin_setup_howto2_view);
    dialog_ex_free(app->dialog_ex);
    loading_free(app->loading);
    desktop_settings_menu_styles_free(app);
    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);
    // Records
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_GUI);
    free(app);

    if(temp_save_name) {
        Power* power = furi_record_open(RECORD_POWER);
        power_reboot(power, PowerBootModeNormal);
    }
}

extern int32_t desktop_settings_app(void* p) {
    UNUSED(p);

    DesktopSettingsApp* app = desktop_settings_app_alloc();
    Desktop* desktop = furi_record_open(RECORD_DESKTOP);

    desktop_api_get_settings(desktop, &app->settings);

    scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneStart);

    view_dispatcher_run(app->view_dispatcher);

    desktop_api_set_settings(desktop, &app->settings);
    furi_record_close(RECORD_DESKTOP);

    desktop_settings_app_free(app);

    return 0;
}
