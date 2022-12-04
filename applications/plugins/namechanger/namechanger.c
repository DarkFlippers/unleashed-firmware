#include "namechanger.h"
#include "scenes/namechanger_scene.h"

#include <toolbox/path.h>
#include <flipper_format/flipper_format.h>

bool namechanger_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    NameChanger* namechanger = context;
    return scene_manager_handle_custom_event(namechanger->scene_manager, event);
}

bool namechanger_back_event_callback(void* context) {
    furi_assert(context);
    NameChanger* namechanger = context;
    return scene_manager_handle_back_event(namechanger->scene_manager);
}

bool namechanger_make_app_folder(NameChanger* namechanger) {
    bool created = false;
    FURI_LOG_I(TAG, "folder1");

    FuriString* folderpath = furi_string_alloc();
    furi_string_set(folderpath, "/ext/dolphin");
    FURI_LOG_I(TAG, "folder2");

    //Make dir if doesn't exist
    if(!storage_simply_mkdir(namechanger->storage, furi_string_get_cstr(folderpath))) {
        FURI_LOG_I(TAG, "folder3");
        furi_string_set_str(namechanger->error, "Cannot create\napp folder.");
    } else {
        FURI_LOG_I(TAG, "folder4");
        created = true;
    }
    FURI_LOG_I(TAG, "folder5");
    furi_string_free(folderpath);
    FURI_LOG_I(TAG, "folder6");
    return created;
}

bool namechanger_name_read_write(NameChanger* namechanger, char* name, uint8_t mode) {
    FuriString* file_path = furi_string_alloc();
    furi_string_set(file_path, "/ext/dolphin/name.txt");
    FURI_LOG_I(TAG, "name1");

    bool result = false;

    if(mode == 2) {
        FURI_LOG_I(TAG, "name2");
        UNUSED(name);
        FlipperFormat* file = flipper_format_file_alloc(namechanger->storage);
        //read
        FuriString* data = furi_string_alloc();
        FURI_LOG_I(TAG, "name3");

        do {
            FURI_LOG_I(TAG, "name4");
            if(!flipper_format_file_open_existing(file, furi_string_get_cstr(file_path))) {
                break;
            }
            FURI_LOG_I(TAG, "name4a");

            // header
            uint32_t version;

            if(!flipper_format_read_header(file, data, &version)) {
                break;
            }
            FURI_LOG_I(TAG, "name4b");

            if(furi_string_cmp_str(data, NAMECHANGER_HEADER) != 0) {
                break;
            }
            FURI_LOG_I(TAG, "name4c");

            if(version != 1) {
                break;
            }
            FURI_LOG_I(TAG, "name4d");

            // get Name
            if(!flipper_format_read_string(file, "Name", data)) {
                break;
            }
            FURI_LOG_I(TAG, "name4e");

            result = true;
            FURI_LOG_I(TAG, "name5");
        } while(false);

        flipper_format_free(file);
        FURI_LOG_I(TAG, "name6");

        if(!result) {
            FURI_LOG_I(TAG, "name7");
            FURI_LOG_E(TAG, "Cannot load data from file.");
            namechanger_text_store_set(namechanger, "%s", furi_hal_version_get_name_ptr());
        } else {
            FURI_LOG_I(TAG, "name8");
            furi_string_trim(data);

            if(!furi_string_size(data)) {
                FURI_LOG_I(TAG, "name9");
                namechanger_text_store_set(namechanger, "%s", furi_hal_version_get_name_ptr());
            } else {
                FURI_LOG_I(TAG, "name10");
                char newname[8];
                snprintf(newname, 8, "%s", furi_string_get_cstr(data));
                namechanger_text_store_set(namechanger, "%s", newname);
            }
        }
        FURI_LOG_I(TAG, "name11");

        furi_string_free(data);
    } else if(mode == 3) {
        FURI_LOG_I(TAG, "name12");
        //save
        FlipperFormat* file = flipper_format_file_alloc(namechanger->storage);

        do {
            FURI_LOG_I(TAG, "name13");
            // Open file for write
            if(!flipper_format_file_open_always(file, furi_string_get_cstr(file_path))) {
                break;
            }

            // Write header
            if(!flipper_format_write_header_cstr(file, NAMECHANGER_HEADER, 1)) {
                break;
            }

            // Write comments
            if(!flipper_format_write_comment_cstr(
                   file, "Changing the value below will change your FlipperZero device name.")) {
                break;
            }

            if(!flipper_format_write_comment_cstr(
                   file,
                   "Note: This is limited to 8 characters using the following: a-z, A-Z, 0-9, and _")) {
                break;
            }

            if(!flipper_format_write_comment_cstr(
                   file, "It cannot contain any other characters.")) {
                break;
            }

            //If name is eraseerase (set by Revert) - then don't write any name
            //otherwise, write name as set in the variable
            if(strcmp(name, "eraseerase") == 0) {
                if(!flipper_format_write_string_cstr(file, "Name", "")) {
                    break;
                }
            } else {
                if(!flipper_format_write_string_cstr(file, "Name", name)) {
                    break;
                }
            }

            FURI_LOG_I(TAG, "name14");
            result = true;
        } while(false);

        flipper_format_free(file);
        FURI_LOG_I(TAG, "name15");

        if(!result) {
            FURI_LOG_I(TAG, "name16");
            FURI_LOG_E(TAG, "Cannot save name file.");
            furi_string_set_str(namechanger->error, "Cannot save\nname file.");
        }
    } else {
        FURI_LOG_I(TAG, "name17");
        FURI_LOG_E(TAG, "Something broke.");
        furi_string_set_str(namechanger->error, "Something broke.");
    }
    FURI_LOG_I(TAG, "name18");

    return result;
}

NameChanger* namechanger_alloc() {
    NameChanger* namechanger = malloc(sizeof(namechanger));

    namechanger->scene_manager = scene_manager_alloc(&namechanger_scene_handlers, namechanger);

    namechanger->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(namechanger->view_dispatcher);
    view_dispatcher_set_event_callback_context(namechanger->view_dispatcher, namechanger);
    view_dispatcher_set_custom_event_callback(
        namechanger->view_dispatcher, namechanger_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        namechanger->view_dispatcher, namechanger_back_event_callback);

    namechanger->gui = furi_record_open(RECORD_GUI);
    namechanger->storage = furi_record_open(RECORD_STORAGE);

    namechanger->submenu = submenu_alloc();
    view_dispatcher_add_view(
        namechanger->view_dispatcher,
        NameChangerViewSubmenu,
        submenu_get_view(namechanger->submenu));

    namechanger->text_input = text_input_alloc();
    view_dispatcher_add_view(
        namechanger->view_dispatcher,
        NameChangerViewTextInput,
        text_input_get_view(namechanger->text_input));

    namechanger->popup = popup_alloc();
    view_dispatcher_add_view(
        namechanger->view_dispatcher, NameChangerViewPopup, popup_get_view(namechanger->popup));

    namechanger->widget = widget_alloc();
    view_dispatcher_add_view(
        namechanger->view_dispatcher, NameChangerViewWidget, widget_get_view(namechanger->widget));

    return namechanger;
}

void namechanger_free(NameChanger* namechanger) {
    furi_assert(namechanger);

    view_dispatcher_remove_view(namechanger->view_dispatcher, NameChangerViewWidget);
    widget_free(namechanger->widget);
    view_dispatcher_remove_view(namechanger->view_dispatcher, NameChangerViewPopup);
    popup_free(namechanger->popup);

    view_dispatcher_remove_view(namechanger->view_dispatcher, NameChangerViewTextInput);
    text_input_free(namechanger->text_input);

    view_dispatcher_remove_view(namechanger->view_dispatcher, NameChangerViewSubmenu);
    submenu_free(namechanger->submenu);

    view_dispatcher_free(namechanger->view_dispatcher);
    scene_manager_free(namechanger->scene_manager);

    furi_string_free(namechanger->error);

    furi_record_close(RECORD_STORAGE);

    furi_record_close(RECORD_GUI);

    free(namechanger);
}

void namechanger_text_store_set(NameChanger* namechanger, const char* text, ...) {
    va_list args;
    va_start(args, text);

    vsnprintf(namechanger->text_store, NAMECHANGER_TEXT_STORE_SIZE, text, args);

    va_end(args);
}

void namechanger_text_store_clear(NameChanger* namechanger) {
    memset(namechanger->text_store, 0, NAMECHANGER_TEXT_STORE_SIZE);
}

int32_t namechanger_app() {
    NameChanger* namechanger = namechanger_alloc();

    namechanger->error = furi_string_alloc();
    furi_string_set(namechanger->error, "Default");

    view_dispatcher_attach_to_gui(
        namechanger->view_dispatcher, namechanger->gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(namechanger->scene_manager, NameChangerSceneStart);

    view_dispatcher_run(namechanger->view_dispatcher);

    namechanger_free(namechanger);
    return 0;
}