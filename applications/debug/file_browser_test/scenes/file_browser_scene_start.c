#include "../file_browser_app_i.h"

#include <furi_hal.h>
#include <gui/modules/widget_elements/widget_element_i.h>
#include <storage/storage.h>

static void
    file_browser_scene_start_ok_callback(GuiButtonType result, InputType type, void* context) {
    UNUSED(result);
    furi_assert(context);
    FileBrowserApp* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, type);
    }
}

bool file_browser_scene_start_on_event(void* context, SceneManagerEvent event) {
    FileBrowserApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        furi_string_set(app->file_path, EXT_PATH("badusb/demo_windows.txt"));
        scene_manager_next_scene(app->scene_manager, FileBrowserSceneBrowser);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
    }
    return consumed;
}

void file_browser_scene_start_on_enter(void* context) {
    FileBrowserApp* app = context;

    widget_add_string_multiline_element(
        app->widget, 64, 20, AlignCenter, AlignTop, FontSecondary, "Press OK to start");

    widget_add_button_element(
        app->widget, GuiButtonTypeCenter, "Ok", file_browser_scene_start_ok_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, FileBrowserAppViewStart);
}

void file_browser_scene_start_on_exit(void* context) {
    UNUSED(context);
    FileBrowserApp* app = context;
    widget_reset(app->widget);
}
