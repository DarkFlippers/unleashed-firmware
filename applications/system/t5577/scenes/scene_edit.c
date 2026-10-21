#include "../t5577_app.h"

// Every block a dump can hold, in the order a person reads them off a tag rather than the order
// a write puts them back. Page 1 block 0 is left out for the same reason the restore leaves it
// out: it repeats the configuration and is not a block of its own.
static T5577Target t5577_app_edit_target(uint8_t index) {
    furi_check(index < T5577_APP_EDIT_BLOCKS);

    if(index < LFRFID_T5577_BLOCKS_IN_PAGE_0) return (T5577Target){0, index};

    // Page 1 block 0 is not in the list, so the rest of that page is one further along.
    return (T5577Target){1, index - LFRFID_T5577_BLOCKS_IN_PAGE_0 + 1};
}

static void t5577_app_scene_edit_callback(void* context, uint32_t index) {
    T5577App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void t5577_app_scene_edit_on_enter(void* context) {
    T5577App* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Edit blocks");

    for(uint8_t index = 0; index < T5577_APP_EDIT_BLOCKS; index++) {
        const T5577Target target = t5577_app_edit_target(index);
        // The value is the label: a list of block numbers alone would mean opening each one to
        // find the word being looked for.
        char label[24];

        if(t5577_dump_was_read(&app->dump, target.page, target.block)) {
            snprintf(
                label,
                sizeof(label),
                "%u:%u  %08lX",
                target.page,
                target.block,
                app->dump.block[target.page][target.block]);
        } else {
            snprintf(label, sizeof(label), "%u:%u  --------", target.page, target.block);
        }

        submenu_add_item(app->submenu, label, index, t5577_app_scene_edit_callback, app);
    }

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, T5577AppSceneEdit));

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewSubmenu);
}

bool t5577_app_scene_edit_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type == SceneManagerEventTypeBack) {
        // Asked rather than assumed, and only when the dump and the file really differ.
        if(!t5577_dump_equal(&app->dump, &app->saved)) {
            if(t5577_app_ask(
                   app, "Save changes", "Write the edited blocks\nback to the file", "Save")) {
                if(!t5577_file_save(&app->dump, furi_string_get_cstr(app->path))) {
                    dialog_message_show_storage_error(app->dialogs, "Cannot write\nthe dump");
                    return true;
                }

                app->saved = app->dump;
                notification_message(app->notifications, &sequence_success);
            } else {
                // Declining puts the dump back rather than only stopping the asking: the dump in
                // memory is what Write to Tag, Info and Rename all use, so an edit left in it
                // would reach a tag the user just declined to write to a file.
                app->dump = app->saved;
            }
        }
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, T5577AppSceneFileMenu);

        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event >= T5577_APP_EDIT_BLOCKS) return false;

    // The scene state is both the cursor to come back to and which block the scenes it opens
    // are for, which is one place for one number.
    scene_manager_set_scene_state(app->scene_manager, T5577AppSceneEdit, event.event);

    // Block 0 of page 0 is the one block whose bits have names, so it is edited by name - with
    // the raw word still one press away for everything the pickers cannot say.
    const T5577Target target = t5577_app_edit_target(event.event);
    scene_manager_next_scene(
        app->scene_manager,
        (target.page == 0 && target.block == 0) ? T5577AppSceneEditConfig : T5577AppSceneBlock);

    return true;
}

void t5577_app_scene_edit_on_exit(void* context) {
    T5577App* app = context;
    submenu_reset(app->submenu);
}

T5577Target t5577_app_edit_block(const T5577App* app) {
    return t5577_app_edit_target(
        scene_manager_get_scene_state(app->scene_manager, T5577AppSceneEdit));
}
