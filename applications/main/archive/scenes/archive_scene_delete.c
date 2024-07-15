#include "../archive_i.h"
#include "../helpers/archive_files.h"
#include "../helpers/archive_apps.h"
#include "../helpers/archive_browser.h"
#include "toolbox/path.h"

#define SCENE_DELETE_CUSTOM_EVENT (0UL)
#define MAX_TEXT_INPUT_LEN        22

void archive_scene_delete_widget_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void archive_scene_delete_on_enter(void* context) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;

    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Cancel", archive_scene_delete_widget_callback, app);
    widget_add_button_element(
        app->widget, GuiButtonTypeRight, "Delete", archive_scene_delete_widget_callback, app);

    FuriString* filename;
    filename = furi_string_alloc();

    ArchiveFile_t* current = archive_get_current_file(app->browser);

    FuriString* filename_no_ext = furi_string_alloc();
    path_extract_filename(current->path, filename_no_ext, true);
    strlcpy(app->text_store, furi_string_get_cstr(filename_no_ext), MAX_NAME_LEN);
    furi_string_free(filename_no_ext);

    path_extract_filename(current->path, filename, false);

    char delete_str[64];
    snprintf(delete_str, sizeof(delete_str), "\e#Delete %s?\e#", furi_string_get_cstr(filename));
    widget_add_text_box_element(
        app->widget, 0, 0, 128, 23, AlignCenter, AlignCenter, delete_str, false);

    furi_string_free(filename);

    view_dispatcher_switch_to_view(app->view_dispatcher, ArchiveViewWidget);
}

bool archive_scene_delete_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;

    ArchiveBrowserView* browser = app->browser;
    ArchiveFile_t* selected = archive_get_current_file(browser);
    const char* name = archive_get_name(browser);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            if(selected->is_app) {
                archive_app_delete_file(browser, name);
            } else {
                archive_delete_file(browser, "%s", name);
            }
            archive_show_file_menu(browser, false);
            return scene_manager_previous_scene(app->scene_manager);
        } else if(event.event == GuiButtonTypeLeft) {
            return scene_manager_previous_scene(app->scene_manager);
        }
    }
    return false;
}

void archive_scene_delete_on_exit(void* context) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;

    widget_reset(app->widget);
}
