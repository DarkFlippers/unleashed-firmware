#include "../desktop_settings_app.h"
#include "applications.h"
#include "desktop_settings_scene.h"
#include "desktop_settings_scene_i.h"
#include <flipper_application/flipper_application.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>

#define APPS_COUNT (FLIPPER_APPS_COUNT + FLIPPER_EXTERNAL_APPS_COUNT)

#define DEFAULT_INDEX (0)
#define EXTERNAL_BROWSER_NAME ("Apps Menu (Default)")
#define PASSPORT_NAME ("Passport (Default)")

#define EXTERNAL_APPLICATION_INDEX (1)
#define EXTERNAL_APPLICATION_NAME ("[Select App]")

#define PRESELECTED_SPECIAL 0xffffffff

static const char* favorite_fap_get_app_name(size_t i) {
    const char* name;
    if(i < FLIPPER_APPS_COUNT) {
        name = FLIPPER_APPS[i].name;
    } else {
        name = FLIPPER_EXTERNAL_APPS[i - FLIPPER_APPS_COUNT].name;
    }

    return name;
}

static bool favorite_fap_selector_item_callback(
    FuriString* file_path,
    void* context,
    uint8_t** icon_ptr,
    FuriString* item_name) {
    UNUSED(context);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool success = flipper_application_load_name_and_icon(file_path, storage, icon_ptr, item_name);
    furi_record_close(RECORD_STORAGE);
    return success;
}

static bool favorite_fap_selector_file_exists(char* file_path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool exists = storage_file_exists(storage, file_path);
    furi_record_close(RECORD_STORAGE);
    return exists;
}

static void desktop_settings_scene_favorite_submenu_callback(void* context, uint32_t index) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void desktop_settings_scene_favorite_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    Submenu* submenu = app->submenu;
    submenu_reset(submenu);

    uint32_t favorite_id =
        scene_manager_get_scene_state(app->scene_manager, DesktopSettingsAppSceneFavorite);
    uint32_t pre_select_item = PRESELECTED_SPECIAL;
    FavoriteApp* curr_favorite_app = NULL;
    bool is_dummy_app = false;
    bool default_passport = false;

    if((favorite_id & SCENE_STATE_SET_DUMMY_APP) == 0) {
        furi_assert(favorite_id < FavoriteAppNumber);
        curr_favorite_app = &app->settings.favorite_apps[favorite_id];
        if(favorite_id == FavoriteAppRightShort) {
            default_passport = true;
        }
    } else {
        favorite_id &= ~(SCENE_STATE_SET_DUMMY_APP);
        furi_assert(favorite_id < DummyAppNumber);
        curr_favorite_app = &app->settings.dummy_apps[favorite_id];
        is_dummy_app = true;
        default_passport = true;
    }

    // Special case: Application browser
    submenu_add_item(
        submenu,
        default_passport ? (PASSPORT_NAME) : (EXTERNAL_BROWSER_NAME),
        DEFAULT_INDEX,
        desktop_settings_scene_favorite_submenu_callback,
        app);

    // Special case: Specific application
    submenu_add_item(
        submenu,
        EXTERNAL_APPLICATION_NAME,
        EXTERNAL_APPLICATION_INDEX,
        desktop_settings_scene_favorite_submenu_callback,
        app);

    if(!is_dummy_app) {
        for(size_t i = 0; i < APPS_COUNT; i++) {
            const char* name = favorite_fap_get_app_name(i);

            submenu_add_item(
                submenu, name, i + 2, desktop_settings_scene_favorite_submenu_callback, app);

            // Select favorite item in submenu
            if(!strcmp(name, curr_favorite_app->name_or_path)) {
                pre_select_item = i + 2;
            }
        }
    }

    if(pre_select_item == PRESELECTED_SPECIAL) {
        if(curr_favorite_app->name_or_path[0] == '\0') {
            pre_select_item = DEFAULT_INDEX;
        } else {
            pre_select_item = EXTERNAL_APPLICATION_INDEX;
        }
    }

    submenu_set_header(submenu, is_dummy_app ? ("Dummy Mode app:") : ("Favorite app:"));
    submenu_set_selected_item(submenu, pre_select_item); // If set during loop, visual glitch.

    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewMenu);
}

bool desktop_settings_scene_favorite_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;
    FuriString* temp_path = furi_string_alloc_set_str(EXT_PATH("apps"));

    uint32_t favorite_id =
        scene_manager_get_scene_state(app->scene_manager, DesktopSettingsAppSceneFavorite);
    FavoriteApp* curr_favorite_app = NULL;
    if((favorite_id & SCENE_STATE_SET_DUMMY_APP) == 0) {
        furi_assert(favorite_id < FavoriteAppNumber);
        curr_favorite_app = &app->settings.favorite_apps[favorite_id];
    } else {
        favorite_id &= ~(SCENE_STATE_SET_DUMMY_APP);
        furi_assert(favorite_id < DummyAppNumber);
        curr_favorite_app = &app->settings.dummy_apps[favorite_id];
    }

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DEFAULT_INDEX) {
            curr_favorite_app->name_or_path[0] = '\0';
            consumed = true;
        } else if(event.event == EXTERNAL_APPLICATION_INDEX) {
            const DialogsFileBrowserOptions browser_options = {
                .extension = ".fap",
                .icon = &I_unknown_10px,
                .skip_assets = true,
                .hide_ext = true,
                .item_loader_callback = favorite_fap_selector_item_callback,
                .item_loader_context = app,
                .base_path = EXT_PATH("apps"),
            };

            // Select favorite fap in file browser
            if(favorite_fap_selector_file_exists(curr_favorite_app->name_or_path)) {
                furi_string_set_str(temp_path, curr_favorite_app->name_or_path);
            }

            if(dialog_file_browser_show(app->dialogs, temp_path, temp_path, &browser_options)) {
                submenu_reset(app->submenu); // Prevent menu from being shown when we exiting scene
                strncpy(
                    curr_favorite_app->name_or_path,
                    furi_string_get_cstr(temp_path),
                    MAX_APP_LENGTH);
                consumed = true;
            }
        } else {
            size_t app_index = event.event - 2;
            const char* name = favorite_fap_get_app_name(app_index);
            if(name) strncpy(curr_favorite_app->name_or_path, name, MAX_APP_LENGTH);
            consumed = true;
        }
        if(consumed) {
            scene_manager_previous_scene(app->scene_manager);
        };
        consumed = true;
    }

    furi_string_free(temp_path);
    return consumed;
}

void desktop_settings_scene_favorite_on_exit(void* context) {
    DesktopSettingsApp* app = context;
    DESKTOP_SETTINGS_SAVE(&app->settings);
    submenu_reset(app->submenu);
}
