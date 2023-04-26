#include "../wifi_marauder_app_i.h"

static void
    wifi_marauder_scene_script_stage_edit_list_add_callback(void* context, uint32_t index) {
    WifiMarauderApp* app = context;

    // Creates new item
    WifiMarauderScriptStageListItem* new_item =
        (WifiMarauderScriptStageListItem*)malloc(sizeof(WifiMarauderScriptStageListItem));
    new_item->value = malloc(64);
    new_item->next_item = NULL;

    if(app->script_stage_edit_first_item == NULL) {
        app->script_stage_edit_first_item = new_item;
    } else {
        WifiMarauderScriptStageListItem* last_item = app->script_stage_edit_first_item;
        while(last_item->next_item != NULL) {
            last_item = last_item->next_item;
        }
        last_item->next_item = new_item;
    }

    scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptStageEditList, index);
    app->user_input_type = WifiMarauderUserInputTypeString;
    app->user_input_string_reference = &new_item->value;
    scene_manager_next_scene(app->scene_manager, WifiMarauderSceneUserInput);
}

static void wifi_marauder_scene_script_stage_edit_list_deallocate_items(WifiMarauderApp* app) {
    WifiMarauderScriptStageListItem* current_item = app->script_stage_edit_first_item;
    while(current_item != NULL) {
        WifiMarauderScriptStageListItem* next_item = current_item->next_item;
        free(current_item->value);
        free(current_item);
        current_item = next_item;
    }
    app->script_stage_edit_first_item = NULL;
}

static void wifi_marauder_scene_script_stage_edit_list_save_strings(WifiMarauderApp* app) {
    WifiMarauderScriptStageListItem* current_item = app->script_stage_edit_first_item;
    int array_size = 0;

    // Calculates the required array size
    while(current_item != NULL) {
        array_size++;
        current_item = current_item->next_item;
    }

    // Reallocate the array of strings if necessary
    if(*app->script_stage_edit_string_count_reference < array_size) {
        *app->script_stage_edit_strings_reference =
            realloc(*app->script_stage_edit_strings_reference, array_size * sizeof(char*));
    }

    // Fills the array of strings
    current_item = app->script_stage_edit_first_item;
    int i = 0;
    while(current_item != NULL) {
        char* current_str = malloc(strlen(current_item->value) + 1);
        strncpy(current_str, current_item->value, strlen(current_item->value) + 1);
        (*app->script_stage_edit_strings_reference)[i] = current_str;
        current_item = current_item->next_item;
        i++;
    }

    *app->script_stage_edit_string_count_reference = array_size;
}

static void wifi_marauder_scene_script_stage_edit_list_save_numbers(WifiMarauderApp* app) {
    WifiMarauderScriptStageListItem* current_item = app->script_stage_edit_first_item;
    int array_size = 0;

    // Calculates the required array size
    while(current_item != NULL) {
        array_size++;
        current_item = current_item->next_item;
    }

    // Reallocate the array of integers if necessary
    if(*app->script_stage_edit_number_count_reference < array_size) {
        *app->script_stage_edit_numbers_reference =
            realloc(*app->script_stage_edit_numbers_reference, array_size * sizeof(int));
    }

    // Fills the array of integers
    current_item = app->script_stage_edit_first_item;
    int i = 0;
    while(current_item != NULL) {
        (*app->script_stage_edit_numbers_reference)[i] = atoi(current_item->value);
        current_item = current_item->next_item;
        i++;
    }

    *app->script_stage_edit_number_count_reference = array_size;
}

static void
    wifi_marauder_scene_script_stage_edit_list_save_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    if(app->script_stage_edit_strings_reference != NULL &&
       app->script_stage_edit_string_count_reference != NULL) {
        wifi_marauder_scene_script_stage_edit_list_save_strings(app);
    }

    if(app->script_stage_edit_numbers_reference != NULL &&
       app->script_stage_edit_number_count_reference != NULL) {
        wifi_marauder_scene_script_stage_edit_list_save_numbers(app);
    }

    wifi_marauder_scene_script_stage_edit_list_deallocate_items(app);
    scene_manager_previous_scene(app->scene_manager);
}

static void
    wifi_marauder_scene_script_stage_edit_list_clear_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    wifi_marauder_scene_script_stage_edit_list_deallocate_items(app);

    submenu_reset(app->submenu);
    submenu_add_item(
        app->submenu,
        "[+] ADD ITEM",
        99,
        wifi_marauder_scene_script_stage_edit_list_add_callback,
        app);
    submenu_add_item(
        app->submenu,
        "[*] SAVE ITEMS",
        99,
        wifi_marauder_scene_script_stage_edit_list_save_callback,
        app);
    submenu_add_item(
        app->submenu,
        "[-] CLEAR LIST",
        99,
        wifi_marauder_scene_script_stage_edit_list_clear_callback,
        app);
}

void wifi_marauder_scene_script_stage_edit_list_on_enter(void* context) {
    WifiMarauderApp* app = context;
    int item_index = 0;
    WifiMarauderScriptStageListItem* current_item = app->script_stage_edit_first_item;

    while(current_item != NULL) {
        submenu_add_item(app->submenu, current_item->value, item_index++, NULL, app);
        current_item = current_item->next_item;
    }
    submenu_add_item(
        app->submenu,
        "[+] ADD ITEM",
        99,
        wifi_marauder_scene_script_stage_edit_list_add_callback,
        app);
    submenu_add_item(
        app->submenu,
        "[*] SAVE ITEMS",
        99,
        wifi_marauder_scene_script_stage_edit_list_save_callback,
        app);
    submenu_add_item(
        app->submenu,
        "[-] CLEAR LIST",
        99,
        wifi_marauder_scene_script_stage_edit_list_clear_callback,
        app);

    submenu_set_selected_item(
        app->submenu,
        scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneScriptStageEditList));
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewSubmenu);
}

bool wifi_marauder_scene_script_stage_edit_list_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void wifi_marauder_scene_script_stage_edit_list_on_exit(void* context) {
    WifiMarauderApp* app = context;
    submenu_reset(app->submenu);
}
