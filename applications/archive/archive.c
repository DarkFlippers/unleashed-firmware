#include "archive_i.h"

static bool archive_get_filenames(ArchiveApp* archive);

static bool is_favourite(ArchiveApp* archive, ArchiveFile_t* file) {
    FS_Common_Api* common_api = &archive->fs_api->common;
    FileInfo file_info;
    FS_Error fr;
    string_t path;

    string_init_printf(path, "favourites/%s", string_get_cstr(file->name));

    fr = common_api->info(string_get_cstr(path), &file_info, NULL, 0);
    FURI_LOG_I("FAV", "%d", fr);

    string_clear(path);
    return fr == 0 || fr == 2;
}

static void update_offset(ArchiveApp* archive) {
    furi_assert(archive);

    with_view_model(
        archive->view_archive_main, (ArchiveViewModel * model) {
            size_t array_size = files_array_size(model->files);
            uint16_t bounds = array_size > 3 ? 2 : array_size;

            if(array_size > 3 && model->idx >= array_size - 1) {
                model->list_offset = model->idx - 3;
            } else if(model->list_offset < model->idx - bounds) {
                model->list_offset = CLAMP(model->list_offset + 1, array_size - bounds, 0);
            } else if(model->list_offset > model->idx - bounds) {
                model->list_offset = CLAMP(model->idx - 1, array_size - bounds, 0);
            }
            return true;
        });
}

static void archive_update_last_idx(ArchiveApp* archive) {
    furi_assert(archive);

    with_view_model(
        archive->view_archive_main, (ArchiveViewModel * model) {
            archive->browser.last_idx[archive->browser.depth] =
                CLAMP(model->idx, files_array_size(model->files) - 1, 0);
            model->idx = 0;
            return true;
        });
}

static void archive_switch_dir(ArchiveApp* archive, const char* path) {
    furi_assert(archive);
    furi_assert(path);
    string_set(archive->browser.path, path);
    archive_get_filenames(archive);
    update_offset(archive);
}

static void archive_switch_tab(ArchiveApp* archive) {
    furi_assert(archive);

    with_view_model(
        archive->view_archive_main, (ArchiveViewModel * model) {
            model->tab_idx = archive->browser.tab_id;
            model->idx = 0;

            return true;
        });

    archive->browser.depth = 0;
    archive_switch_dir(archive, tab_default_paths[archive->browser.tab_id]);
}

static void archive_leave_dir(ArchiveApp* archive) {
    furi_assert(archive);

    char* last_char_ptr = strrchr(string_get_cstr(archive->browser.path), '/');

    if(last_char_ptr) {
        size_t pos = last_char_ptr - string_get_cstr(archive->browser.path);
        string_left(archive->browser.path, pos);
    }

    archive->browser.depth = CLAMP(archive->browser.depth - 1, MAX_DEPTH, 0);

    with_view_model(
        archive->view_archive_main, (ArchiveViewModel * model) {
            model->idx = archive->browser.last_idx[archive->browser.depth];
            model->list_offset =
                model->idx -
                (files_array_size(model->files) > 3 ? 3 : files_array_size(model->files));
            return true;
        });

    archive_switch_dir(archive, string_get_cstr(archive->browser.path));
}

static void archive_enter_dir(ArchiveApp* archive, string_t name) {
    furi_assert(archive);
    furi_assert(name);

    archive_update_last_idx(archive);
    archive->browser.depth = CLAMP(archive->browser.depth + 1, MAX_DEPTH, 0);

    string_cat(archive->browser.path, "/");
    string_cat(archive->browser.path, archive->browser.name);

    archive_switch_dir(archive, string_get_cstr(archive->browser.path));
}

static bool filter_by_extension(ArchiveApp* archive, FileInfo* file_info, const char* name) {
    furi_assert(archive);
    furi_assert(file_info);
    furi_assert(name);

    bool result = false;
    const char* filter_ext_ptr = get_tab_ext(archive->browser.tab_id);

    if(strcmp(filter_ext_ptr, "*") == 0) {
        result = true;
    } else if(strstr(name, filter_ext_ptr) != NULL) {
        result = true;
    } else if(file_info->flags & FSF_DIRECTORY) {
        result = true;
    }

    return result;
}

static void set_file_type(ArchiveFile_t* file, FileInfo* file_info) {
    furi_assert(file);
    furi_assert(file_info);

    for(size_t i = 0; i < SIZEOF_ARRAY(known_ext); i++) {
        if(string_search_str(file->name, known_ext[i], 0) != STRING_FAILURE) {
            file->type = i;
            return;
        }
    }

    if(file_info->flags & FSF_DIRECTORY) {
        file->type = ArchiveFileTypeFolder;
    } else {
        file->type = ArchiveFileTypeUnknown;
    }
}

static bool archive_get_filenames(ArchiveApp* archive) {
    furi_assert(archive);
    FS_Dir_Api* dir_api = &archive->fs_api->dir;
    ArchiveFile_t item;
    FileInfo file_info;
    File directory;
    char name[MAX_NAME_LEN];
    bool result;

    result = dir_api->open(&directory, string_get_cstr(archive->browser.path));

    with_view_model(
        archive->view_archive_main, (ArchiveViewModel * model) {
            files_array_clean(model->files);
            return true;
        });

    if(!result) {
        dir_api->close(&directory);
        return false;
    }

    while(1) {
        result = dir_api->read(&directory, &file_info, name, MAX_NAME_LEN);

        if(directory.error_id == FSE_NOT_EXIST || name[0] == 0) {
            break;
        }

        if(result) {
            uint16_t files_cnt;
            with_view_model(
                archive->view_archive_main, (ArchiveViewModel * model) {
                    files_cnt = files_array_size(model->files);

                    return true;
                });

            if(files_cnt > MAX_FILES) {
                break;
            } else if(directory.error_id == FSE_OK) {
                if(filter_by_extension(archive, &file_info, name)) {
                    ArchiveFile_t_init(&item);
                    string_init_set(item.name, name);
                    set_file_type(&item, &file_info);

                    with_view_model(
                        archive->view_archive_main, (ArchiveViewModel * model) {
                            files_array_push_back(model->files, item);
                            return true;
                        });

                    ArchiveFile_t_clear(&item);
                }
            } else {
                dir_api->close(&directory);
                return false;
            }
        }
    }

    dir_api->close(&directory);
    return true;
}

static void archive_exit_callback(ArchiveApp* archive) {
    furi_assert(archive);

    AppEvent event;
    event.type = EventTypeExit;
    furi_check(osMessageQueuePut(archive->event_queue, &event, 0, osWaitForever) == osOK);
}

static uint32_t archive_previous_callback(void* context) {
    return ArchiveViewMain;
}

/* file menu */
static void archive_add_to_favourites(ArchiveApp* archive) {
    furi_assert(archive);

    FS_Common_Api* common_api = &archive->fs_api->common;
    common_api->mkdir("favourites");

    FS_File_Api* file_api = &archive->fs_api->file;
    File src;
    File dst;

    bool fr;
    uint16_t buffer[MAX_FILE_SIZE];
    uint16_t bw = 0;
    uint16_t br = 0;

    string_t buffer_src;
    string_t buffer_dst;

    string_init_printf(
        buffer_src,
        "%s/%s",
        string_get_cstr(archive->browser.path),
        string_get_cstr(archive->browser.name));
    string_init_printf(buffer_dst, "/favourites/%s", string_get_cstr(archive->browser.name));

    fr = file_api->open(&src, string_get_cstr(buffer_src), FSAM_READ, FSOM_OPEN_EXISTING);
    FURI_LOG_I("FATFS", "OPEN: %d", fr);
    fr = file_api->open(&dst, string_get_cstr(buffer_dst), FSAM_WRITE, FSOM_CREATE_ALWAYS);
    FURI_LOG_I("FATFS", "CREATE: %d", fr);

    for(;;) {
        br = file_api->read(&src, &buffer, sizeof(buffer));
        if(br == 0) break;
        bw = file_api->write(&dst, &buffer, sizeof(buffer));
        if(bw < br) break;
    }

    file_api->close(&src);
    file_api->close(&dst);

    string_clear(buffer_src);
    string_clear(buffer_dst);
}

static void archive_text_input_callback(void* context) {
    furi_assert(context);

    ArchiveApp* archive = (ArchiveApp*)context;
    FS_Common_Api* common_api = &archive->fs_api->common;

    string_t buffer_src;
    string_t buffer_dst;

    string_init_printf(
        buffer_src,
        "%s/%s",
        string_get_cstr(archive->browser.path),
        string_get_cstr(archive->browser.name));
    string_init_printf(
        buffer_dst,
        "%s/%s",
        string_get_cstr(archive->browser.path),
        archive->browser.text_input_buffer);

    // append extension

    ArchiveFile_t* file;

    with_view_model(
        archive->view_archive_main, (ArchiveViewModel * model) {
            file = files_array_get(
                model->files, CLAMP(model->idx, files_array_size(model->files) - 1, 0));
            return true;
        });

    string_cat(buffer_dst, known_ext[file->type]);
    common_api->rename(string_get_cstr(buffer_src), string_get_cstr(buffer_dst));

    view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewMain);

    string_clear(buffer_src);
    string_clear(buffer_dst);

    archive_get_filenames(archive);
}

static void archive_enter_text_input(ArchiveApp* archive) {
    furi_assert(archive);
    *archive->browser.text_input_buffer = '\0';

    strlcpy(
        archive->browser.text_input_buffer, string_get_cstr(archive->browser.name), MAX_NAME_LEN);

    archive_trim_file_ext(archive->browser.text_input_buffer);

    text_input_set_header_text(archive->text_input, "Rename:");

    text_input_set_result_callback(
        archive->text_input,
        archive_text_input_callback,
        archive,
        archive->browser.text_input_buffer,
        MAX_NAME_LEN);

    view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewTextInput);
}

static void archive_show_file_menu(ArchiveApp* archive) {
    furi_assert(archive);

    archive->browser.menu = true;

    with_view_model(
        archive->view_archive_main, (ArchiveViewModel * model) {
            ArchiveFile_t* selected;
            selected = files_array_get(model->files, model->idx);
            model->menu = true;
            model->menu_idx = 0;
            selected->fav = is_favourite(archive, selected);

            return true;
        });
}

static void archive_close_file_menu(ArchiveApp* archive) {
    furi_assert(archive);

    archive->browser.menu = false;

    with_view_model(
        archive->view_archive_main, (ArchiveViewModel * model) {
            model->menu = false;
            model->menu_idx = 0;
            return true;
        });
}

static void archive_open_app(ArchiveApp* archive, const char* app_name, const char* args) {
    furi_assert(archive);
    furi_assert(app_name);

    app_loader_start(app_name, args);
}

static void archive_delete_file(ArchiveApp* archive, ArchiveFile_t* file, bool fav, bool orig) {
    furi_assert(archive);
    furi_assert(file);

    FS_Common_Api* common_api = &archive->fs_api->common;
    string_t path;
    string_init(path);

    if(!fav && !orig) {
        string_printf(
            path, "%s/%s", string_get_cstr(archive->browser.path), string_get_cstr(file->name));
        common_api->remove(string_get_cstr(path));

    } else { // remove from favorites
        string_printf(path, "favourites/%s", string_get_cstr(file->name));
        common_api->remove(string_get_cstr(path));

        if(orig) { // remove original file
            string_printf(
                path, "%s/%s", get_default_path(file->type), string_get_cstr(file->name));
            common_api->remove(string_get_cstr(path));
        }
    }

    string_clear(path);
    archive_get_filenames(archive);

    with_view_model(
        archive->view_archive_main, (ArchiveViewModel * model) {
            model->idx = CLAMP(model->idx, files_array_size(model->files) - 1, 0);
            return true;
        });

    update_offset(archive);
}

static void archive_file_menu_callback(ArchiveApp* archive) {
    furi_assert(archive);

    ArchiveFile_t* selected;
    uint8_t idx = 0;

    with_view_model(
        archive->view_archive_main, (ArchiveViewModel * model) {
            selected = files_array_get(model->files, model->idx);
            idx = model->menu_idx;
            return true;
        });

    switch(idx) {
    case 0:
        if(is_known_app(selected->type)) {
            string_t full_path;
            string_init_printf(
                full_path,
                "%s/%s",
                string_get_cstr(archive->browser.path),
                string_get_cstr(selected->name));

            archive_open_app(
                archive, flipper_app_name[selected->type], string_get_cstr(full_path));

            string_clear(full_path);
        }
        break;
    case 1:
        if(is_known_app(selected->type)) {
            if(!is_favourite(archive, selected)) {
                string_set(archive->browser.name, selected->name);
                archive_add_to_favourites(archive);
            } else {
                // delete from favourites
                archive_delete_file(archive, selected, true, false);
            }
            archive_close_file_menu(archive);
        }
        break;
    case 2:
        // open rename view
        if(is_known_app(selected->type)) {
            archive_enter_text_input(archive);
        }
        break;
    case 3:
        // confirmation?
        if(is_favourite(archive, selected)) {
            //delete both fav & original
            archive_delete_file(archive, selected, true, true);
        } else {
            archive_delete_file(archive, selected, false, false);
        }

        archive_close_file_menu(archive);

        break;

    default:
        archive_close_file_menu(archive);
        break;
    }
    selected = NULL;
}

static void menu_input_handler(ArchiveApp* archive, InputEvent* event) {
    furi_assert(archive);
    furi_assert(archive);

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp || event->key == InputKeyDown) {
            with_view_model(
                archive->view_archive_main, (ArchiveViewModel * model) {
                    if(event->key == InputKeyUp) {
                        model->menu_idx = ((model->menu_idx - 1) + MENU_ITEMS) % MENU_ITEMS;
                    } else if(event->key == InputKeyDown) {
                        model->menu_idx = (model->menu_idx + 1) % MENU_ITEMS;
                    }
                    return true;
                });
        }

        if(event->key == InputKeyOk) {
            archive_file_menu_callback(archive);
        } else if(event->key == InputKeyBack) {
            archive_close_file_menu(archive);
        }
    }
}

/* main controls */

static bool archive_view_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    ArchiveApp* archive = context;
    bool in_menu = archive->browser.menu;

    if(in_menu) {
        menu_input_handler(archive, event);
        return true;
    }

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyLeft) {
            if(archive->browser.tab_id > 0) {
                archive->browser.tab_id = CLAMP(archive->browser.tab_id - 1, ArchiveTabTotal, 0);
                archive_switch_tab(archive);
                return true;
            }
        } else if(event->key == InputKeyRight) {
            if(archive->browser.tab_id < ArchiveTabTotal - 1) {
                archive->browser.tab_id =
                    CLAMP(archive->browser.tab_id + 1, ArchiveTabTotal - 1, 0);
                archive_switch_tab(archive);
                return true;
            }

        } else if(event->key == InputKeyBack) {
            if(archive->browser.depth == 0) {
                archive_exit_callback(archive);
            } else {
                archive_leave_dir(archive);
            }

            return true;
        }
    }
    if(event->key == InputKeyUp || event->key == InputKeyDown) {
        with_view_model(
            archive->view_archive_main, (ArchiveViewModel * model) {
                uint16_t num_elements = (uint16_t)files_array_size(model->files);
                if((event->type == InputTypeShort || event->type == InputTypeRepeat)) {
                    if(event->key == InputKeyUp) {
                        model->idx = ((model->idx - 1) + num_elements) % num_elements;
                    } else if(event->key == InputKeyDown) {
                        model->idx = (model->idx + 1) % num_elements;
                    }
                }

                return true;
            });
        update_offset(archive);
    }

    if(event->key == InputKeyOk) {
        ArchiveFile_t* selected;

        with_view_model(
            archive->view_archive_main, (ArchiveViewModel * model) {
                selected = files_array_size(model->files) > 0 ?
                               files_array_get(model->files, model->idx) :
                               NULL;
                return true;
            });

        if(selected) {
            string_set(archive->browser.name, selected->name);

            if(selected->type == ArchiveFileTypeFolder) {
                if(event->type == InputTypeShort) {
                    archive_enter_dir(archive, archive->browser.name);
                } else if(event->type == InputTypeLong) {
                    archive_show_file_menu(archive);
                }
            } else {
                if(event->type == InputTypeShort) {
                    archive_show_file_menu(archive);
                }
            }
        }
    }

    update_offset(archive);

    return true;
}

void archive_free(ArchiveApp* archive) {
    furi_assert(archive);

    view_dispatcher_remove_view(archive->view_dispatcher, ArchiveViewMain);
    view_dispatcher_remove_view(archive->view_dispatcher, ArchiveViewTextInput);
    view_dispatcher_free(archive->view_dispatcher);

    with_view_model(
        archive->view_archive_main, (ArchiveViewModel * model) {
            files_array_clear(model->files);
            return false;
        });

    view_free(archive->view_archive_main);

    string_clear(archive->browser.name);
    string_clear(archive->browser.path);

    text_input_free(archive->text_input);

    furi_record_close("sdcard");
    archive->fs_api = NULL;
    furi_record_close("gui");
    archive->gui = NULL;
    furi_thread_free(archive->app_thread);
    furi_check(osMessageQueueDelete(archive->event_queue) == osOK);

    free(archive);
}

ArchiveApp* archive_alloc() {
    ArchiveApp* archive = furi_alloc(sizeof(ArchiveApp));

    archive->event_queue = osMessageQueueNew(8, sizeof(AppEvent), NULL);
    archive->app_thread = furi_thread_alloc();
    archive->gui = furi_record_open("gui");
    archive->fs_api = furi_record_open("sdcard");
    archive->text_input = text_input_alloc();
    archive->view_archive_main = view_alloc();

    furi_check(archive->event_queue);

    view_allocate_model(
        archive->view_archive_main, ViewModelTypeLocking, sizeof(ArchiveViewModel));
    with_view_model(
        archive->view_archive_main, (ArchiveViewModel * model) {
            files_array_init(model->files);
            return false;
        });

    view_set_context(archive->view_archive_main, archive);
    view_set_draw_callback(archive->view_archive_main, archive_view_render);
    view_set_input_callback(archive->view_archive_main, archive_view_input);
    view_set_previous_callback(
        text_input_get_view(archive->text_input), archive_previous_callback);

    // View Dispatcher
    archive->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_add_view(
        archive->view_dispatcher, ArchiveViewMain, archive->view_archive_main);
    view_dispatcher_add_view(
        archive->view_dispatcher, ArchiveViewTextInput, text_input_get_view(archive->text_input));
    view_dispatcher_attach_to_gui(
        archive->view_dispatcher, archive->gui, ViewDispatcherTypeFullscreen);

    view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveTabFavourites);

    return archive;
}

int32_t app_archive(void* p) {
    ArchiveApp* archive = archive_alloc();

    // default tab
    archive_switch_tab(archive);

    AppEvent event;
    while(1) {
        furi_check(osMessageQueueGet(archive->event_queue, &event, NULL, osWaitForever) == osOK);
        if(event.type == EventTypeExit) {
            break;
        }
    }

    archive_free(archive);
    return 0;
}
