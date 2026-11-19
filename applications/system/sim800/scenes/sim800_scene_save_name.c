#include "../sim800_app_i.h"

static bool sim800_save_name_is_preset(Sim800SaveTarget target) {
    return target == Sim800SaveTargetPreset || target == Sim800SaveTargetNewPreset ||
           target == Sim800SaveTargetRenamePreset;
}

static bool sim800_save_name_apply(Sim800App* app) {
    FuriString* new_path = furi_string_alloc();
    bool saved = false;

    switch(app->save_target) {
    case Sim800SaveTargetPreset:
    case Sim800SaveTargetNewPreset:
        saved = sim800_preset_save(app->text_store, furi_string_get_cstr(app->sms_text));
        break;

    case Sim800SaveTargetRenamePreset:
        // Rename is write-then-remove, so the old file only goes once the new one is on disk
        sim800_preset_path(app->text_store, new_path);
        saved = sim800_preset_save(app->text_store, furi_string_get_cstr(app->sms_text));
        if(saved && furi_string_cmp(new_path, app->file_path) != 0) {
            sim800_preset_delete(furi_string_get_cstr(app->file_path));
        }
        break;

    case Sim800SaveTargetRenameEntry:
        sim800_phonebook_path(app->text_store, new_path);
        saved = sim800_phonebook_save(app->text_store, furi_string_get_cstr(app->number));
        if(saved) {
            if(furi_string_cmp(new_path, app->file_path) != 0) {
                sim800_phonebook_delete(furi_string_get_cstr(app->file_path));
            }
            furi_string_set(app->file_path, new_path);
            furi_string_set_str(app->entry_name, app->text_store);
        }
        break;

    case Sim800SaveTargetNumber:
    case Sim800SaveTargetNewEntry:
    default:
        saved = sim800_phonebook_save(app->text_store, furi_string_get_cstr(app->number));
        if(saved) {
            app->number_saved = true;
            furi_string_set_str(app->entry_name, app->text_store);
            sim800_phonebook_path(app->text_store, app->file_path);
            sim800_call_view_set_saved(app->call_view, true);
        }
        break;
    }

    furi_string_free(new_path);
    return saved;
}

void sim800_scene_save_name_on_enter(void* context) {
    Sim800App* app = context;

    if(app->save_target == Sim800SaveTargetRenameEntry ||
       app->save_target == Sim800SaveTargetRenamePreset) {
        strlcpy(app->text_store, furi_string_get_cstr(app->entry_name), SIM800_NAME_MAX);
    } else if(sim800_save_name_is_preset(app->save_target)) {
        strlcpy(app->text_store, "Preset", SIM800_NAME_MAX);
    } else if(furi_string_size(app->entry_name) > 0) {
        strlcpy(app->text_store, furi_string_get_cstr(app->entry_name), SIM800_NAME_MAX);
    } else {
        strlcpy(app->text_store, furi_string_get_cstr(app->number), SIM800_NAME_MAX);
    }

    sim800_text_input_reset(app->text_input);
    sim800_text_input_set_header_text(
        app->text_input,
        sim800_save_name_is_preset(app->save_target) ? "Preset name" : "Entry name");
    sim800_text_input_set_minimum_length(app->text_input, 1);
    sim800_text_input_set_result_callback(
        app->text_input, sim800_input_done_callback, app, app->text_store, SIM800_NAME_MAX, false);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewTextInput);
}

bool sim800_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type != SceneManagerEventTypeCustom ||
       event.event != Sim800CustomEventTextInputDone) {
        return false;
    }

    bool saved = sim800_save_name_apply(app);
    sim800_notify_result(app, saved);

    switch(app->save_target) {
    case Sim800SaveTargetNewPreset:
    case Sim800SaveTargetRenamePreset:
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, Sim800ScenePresets);
        break;
    case Sim800SaveTargetRenameEntry:
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, Sim800SceneSavedMenu);
        break;
    case Sim800SaveTargetNewEntry:
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, Sim800SceneStart);
        break;
    default:
        scene_manager_previous_scene(app->scene_manager);
        break;
    }

    return true;
}

void sim800_scene_save_name_on_exit(void* context) {
    Sim800App* app = context;
    sim800_text_input_reset(app->text_input);
}
