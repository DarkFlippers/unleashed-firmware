#include "../chip8_app_i.h"
#include "furi_hal_power.h"

static bool chip8_file_select(Chip8App* chip8) {
    furi_assert(chip8);
    string_init(chip8->file_name);
    string_set_str(chip8->file_name, CHIP8_APP_PATH_FOLDER);
    // string_set_str(file_path, chip8->file_name);

    bool res = dialog_file_browser_show(
        chip8->dialogs,
        chip8->file_name,
        chip8->file_name,
        CHIP8_APP_EXTENSION,
        true,
        &I_unknown_10px,
        false);

    FURI_LOG_I(
        "Chip8_file_browser_show", "chip8->file_name: %s", string_get_cstr(chip8->file_name));
    FURI_LOG_I("Chip8_file_browser_show", "res: %d", res);
    return res;
}

void chip8_scene_file_select_on_enter(void* context) {
    Chip8App* chip8 = context;

    if(chip8_file_select(chip8)) {
        FURI_LOG_I(
            "Chip8", "chip8_file_select, file_name = %s", string_get_cstr(chip8->file_name));
        scene_manager_next_scene(chip8->scene_manager, Chip8WorkView);
    } else {
        view_dispatcher_stop(chip8->view_dispatcher);
    }
}

bool chip8_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void chip8_scene_file_select_on_exit(void* context) {
    UNUSED(context);
}
