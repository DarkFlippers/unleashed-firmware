#include "../sim800_app_i.h"

void sim800_scene_number_input_on_enter(void* context) {
    Sim800App* app = context;

    if(furi_string_size(app->number) == 0) furi_string_set_str(app->number, "+");
    strlcpy(app->text_store, furi_string_get_cstr(app->number), SIM800_NUMBER_MAX);

    sim800_text_input_reset(app->text_input);
    const char* header = "Number for SMS";
    if(app->mode == Sim800ModeCall) {
        header = "Number to call";
    } else if(app->mode == Sim800ModeEditNumber) {
        header = "Change number";
    } else if(app->mode == Sim800ModeNewEntry) {
        header = "New number";
    }
    sim800_text_input_set_header_text(app->text_input, header);
    sim800_text_input_set_minimum_length(app->text_input, 2);
    sim800_text_input_set_result_callback(
        app->text_input,
        sim800_input_done_callback,
        app,
        app->text_store,
        SIM800_NUMBER_MAX,
        false);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewTextInput);
}

bool sim800_scene_number_input_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeCustom &&
       event.event == Sim800CustomEventTextInputDone) {
        furi_string_set_str(app->number, app->text_store);

        if(app->mode == Sim800ModeEditNumber) {
            bool saved =
                sim800_phonebook_save(furi_string_get_cstr(app->entry_name), app->text_store);
            sim800_notify_result(app, saved);

            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, Sim800SceneSavedMenu);
            return true;
        }

        if(app->mode == Sim800ModeNewEntry) {
            app->save_target = Sim800SaveTargetNewEntry;
            scene_manager_next_scene(app->scene_manager, Sim800SceneSaveName);
            return true;
        }

        app->number_saved = false;
        app->redial = false;

        sim800_number_chosen(app);
        return true;
    }
    return false;
}

void sim800_scene_number_input_on_exit(void* context) {
    Sim800App* app = context;
    sim800_text_input_reset(app->text_input);
}
