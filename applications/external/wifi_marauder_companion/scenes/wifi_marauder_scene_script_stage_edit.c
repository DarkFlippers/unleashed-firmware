#include "../wifi_marauder_app_i.h"

void wifi_marauder_scene_script_stage_edit_create_list_strings(
    WifiMarauderApp* app,
    char** strings,
    int string_count) {
    // Deallocates the existing list
    WifiMarauderScriptStageListItem* current_item = app->script_stage_edit_first_item;
    while(current_item != NULL) {
        WifiMarauderScriptStageListItem* next_item = current_item->next_item;
        free(current_item->value);
        free(current_item);
        current_item = next_item;
    }

    // Create a new list with numbers
    WifiMarauderScriptStageListItem* first_item = NULL;
    WifiMarauderScriptStageListItem* previous_item = NULL;
    for(int i = 0; i < string_count; i++) {
        WifiMarauderScriptStageListItem* item = malloc(sizeof(WifiMarauderScriptStageListItem));
        item->value = strdup(strings[i]);
        item->next_item = NULL;

        if(previous_item == NULL) {
            first_item = item;
        } else {
            previous_item->next_item = item;
        }
        previous_item = item;
    }

    app->script_stage_edit_first_item = first_item;
}

void wifi_marauder_scene_script_stage_edit_create_list_numbers(
    WifiMarauderApp* app,
    int* numbers,
    int number_count) {
    // Deallocates the existing list
    WifiMarauderScriptStageListItem* current_item = app->script_stage_edit_first_item;
    while(current_item != NULL) {
        WifiMarauderScriptStageListItem* next_item = current_item->next_item;
        free(current_item->value);
        free(current_item);
        current_item = next_item;
    }

    // Create a new list with numbers
    WifiMarauderScriptStageListItem* first_item = NULL;
    WifiMarauderScriptStageListItem* previous_item = NULL;
    for(int i = 0; i < number_count; i++) {
        char number_str[32];
        snprintf(number_str, sizeof(number_str), "%d", numbers[i]);

        WifiMarauderScriptStageListItem* item = malloc(sizeof(WifiMarauderScriptStageListItem));
        item->value = strdup(number_str);
        item->next_item = NULL;

        if(previous_item == NULL) {
            first_item = item;
        } else {
            previous_item->next_item = item;
        }
        previous_item = item;
    }

    app->script_stage_edit_first_item = first_item;
}

static void
    wifi_marauder_scene_script_stage_edit_list_enter_callback(void* context, uint32_t index) {
    WifiMarauderApp* app = context;
    const WifiMarauderScriptMenuItem* menu_item = &app->script_stage_menu->items[index];

    // Fixed delete item
    if(index == app->script_stage_menu->num_items) {
        uint32_t deleted_stage_index =
            scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneScriptEdit);
        if(deleted_stage_index > 0) {
            scene_manager_set_scene_state(
                app->scene_manager, WifiMarauderSceneScriptEdit, deleted_stage_index - 1);
        }
        WifiMarauderScriptStage* previous_stage = NULL;
        WifiMarauderScriptStage* current_stage = app->script->first_stage;
        uint32_t current_stage_index = 0;

        while(current_stage != NULL && current_stage_index < deleted_stage_index) {
            previous_stage = current_stage;
            current_stage = current_stage->next_stage;
            current_stage_index++;
        }

        // Delete the stage
        if(current_stage != NULL) {
            if(previous_stage != NULL) {
                if(current_stage->next_stage != NULL) {
                    previous_stage->next_stage = current_stage->next_stage;
                } else {
                    previous_stage->next_stage = NULL;
                    app->script->last_stage = previous_stage;
                }
            } else {
                if(current_stage->next_stage != NULL) {
                    app->script->first_stage = current_stage->next_stage;
                } else {
                    app->script->first_stage = NULL;
                    app->script->last_stage = NULL;
                }
            }
        }
        app->script_edit_selected_stage = NULL;

        scene_manager_previous_scene(app->scene_manager);
        return;
    }

    if(menu_item->select_callback == NULL) {
        return;
    }
    if(menu_item->type == WifiMarauderScriptMenuItemTypeNumber) {
        // Accepts user number input, assigning the value to the reference passed as a parameter
        menu_item->select_callback(app);
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptStageEdit, index);
        app->user_input_type = WifiMarauderUserInputTypeNumber;
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneUserInput);
    } else if(menu_item->type == WifiMarauderScriptMenuItemTypeString) {
        // Accepts user string input, assigning the value to the reference passed as a parameter
        menu_item->select_callback(app);
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptStageEdit, index);
        app->user_input_type = WifiMarauderUserInputTypeString;
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneUserInput);
    } else if(menu_item->type == WifiMarauderScriptMenuItemTypeListString) {
        // Accepts the strings that compose the list
        menu_item->select_callback(app);
        wifi_marauder_scene_script_stage_edit_create_list_strings(
            app,
            *app->script_stage_edit_strings_reference,
            *app->script_stage_edit_string_count_reference);
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptStageEdit, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneScriptStageEditList);
    } else if(menu_item->type == WifiMarauderScriptMenuItemTypeListNumber) {
        // Accepts the numbers that compose the list
        menu_item->select_callback(app);
        wifi_marauder_scene_script_stage_edit_create_list_numbers(
            app,
            *app->script_stage_edit_numbers_reference,
            *app->script_stage_edit_number_count_reference);
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptStageEdit, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneScriptStageEditList);
    }
}

void wifi_marauder_scene_script_stage_edit_on_enter(void* context) {
    WifiMarauderApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    variable_item_list_set_enter_callback(
        app->var_item_list, wifi_marauder_scene_script_stage_edit_list_enter_callback, app);
    app->script_stage_menu =
        wifi_marauder_script_stage_menu_create(app->script_edit_selected_stage->type);

    if(app->script_stage_menu->items != NULL) {
        for(uint32_t i = 0; i < app->script_stage_menu->num_items; i++) {
            WifiMarauderScriptMenuItem* stage_item = &app->script_stage_menu->items[i];

            // Changes the list item to handle it in callbacks
            VariableItem* list_item = variable_item_list_add(
                app->var_item_list,
                stage_item->name,
                stage_item->num_options,
                stage_item->change_callback,
                app);

            variable_item_list_set_selected_item(app->var_item_list, i);
            if(stage_item->setup_callback != NULL) {
                stage_item->setup_callback(list_item);
            }
            if(stage_item->change_callback != NULL) {
                stage_item->change_callback(list_item);
            }
        }
    }

    variable_item_list_add(app->var_item_list, "[-] DELETE STAGE", 0, NULL, app);

    variable_item_list_set_selected_item(
        var_item_list,
        scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneScriptStageEdit));
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewVarItemList);
}

bool wifi_marauder_scene_script_stage_edit_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void wifi_marauder_scene_script_stage_edit_on_exit(void* context) {
    WifiMarauderApp* app = context;
    wifi_marauder_script_stage_menu_free(app->script_stage_menu);
    app->script_stage_menu = NULL;
    variable_item_list_reset(app->var_item_list);
}
