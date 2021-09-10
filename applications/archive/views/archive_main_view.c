#include <furi.h>
#include "../archive_i.h"
#include "archive_main_view.h"

static const char* flipper_app_name[] = {
    [ArchiveFileTypeIButton] = "iButton",
    [ArchiveFileTypeNFC] = "NFC",
    [ArchiveFileTypeSubGhz] = "Sub-GHz",
    [ArchiveFileTypeLFRFID] = "125 kHz RFID",
    [ArchiveFileTypeIrda] = "Infrared",
};

static const char* ArchiveTabNames[] = {
    [ArchiveTabFavorites] = "Favorites",
    [ArchiveTabIButton] = "iButton",
    [ArchiveTabNFC] = "NFC",
    [ArchiveTabSubGhz] = "Sub-GHz",
    [ArchiveTabLFRFID] = "RFID LF",
    [ArchiveTabIrda] = "Infrared",
    [ArchiveTabBrowser] = "Browser"};

static const Icon* ArchiveItemIcons[] = {
    [ArchiveFileTypeIButton] = &I_ibutt_10px,
    [ArchiveFileTypeNFC] = &I_Nfc_10px,
    [ArchiveFileTypeSubGhz] = &I_sub1_10px,
    [ArchiveFileTypeLFRFID] = &I_125_10px,
    [ArchiveFileTypeIrda] = &I_ir_10px,
    [ArchiveFileTypeFolder] = &I_dir_10px,
    [ArchiveFileTypeUnknown] = &I_unknown_10px,
};

void archive_browser_set_callback(
    ArchiveMainView* main_view,
    ArchiveMainViewCallback callback,
    void* context) {
    furi_assert(main_view);
    furi_assert(callback);
    main_view->callback = callback;
    main_view->context = context;
}

void update_offset(ArchiveMainView* main_view) {
    furi_assert(main_view);

    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
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

size_t archive_file_array_size(ArchiveMainView* main_view) {
    uint16_t size = 0;
    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            size = files_array_size(model->files);
            return true;
        });
    return size;
}

void archive_file_array_remove_selected(ArchiveMainView* main_view) {
    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            files_array_remove_v(model->files, model->idx, model->idx + 1);
            model->idx = CLAMP(model->idx, files_array_size(model->files) - 1, 0);
            return true;
        });

    update_offset(main_view);
}

void archive_file_array_clean(ArchiveMainView* main_view) {
    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            files_array_clean(model->files);
            return true;
        });
}

ArchiveFile_t* archive_get_current_file(ArchiveMainView* main_view) {
    ArchiveFile_t* selected;
    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            selected = files_array_size(model->files) > 0 ?
                           files_array_get(model->files, model->idx) :
                           NULL;
            return true;
        });
    return selected;
}

ArchiveTabEnum archive_get_tab(ArchiveMainView* main_view) {
    ArchiveTabEnum tab_id;
    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            tab_id = model->tab_idx;
            return true;
        });
    return tab_id;
}

void archive_set_tab(ArchiveMainView* main_view, ArchiveTabEnum tab) {
    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            model->tab_idx = tab;
            return true;
        });
}

uint8_t archive_get_depth(ArchiveMainView* main_view) {
    uint8_t depth;
    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            depth = model->depth;
            return true;
        });

    return depth;
}

const char* archive_get_path(ArchiveMainView* main_view) {
    return string_get_cstr(main_view->path);
}
const char* archive_get_name(ArchiveMainView* main_view) {
    ArchiveFile_t* selected = archive_get_current_file(main_view);
    return string_get_cstr(selected->name);
}

void archive_set_name(ArchiveMainView* main_view, const char* name) {
    furi_assert(main_view);
    furi_assert(name);

    string_set(main_view->name, name);
}

void archive_browser_update(ArchiveMainView* main_view) {
    furi_assert(main_view);

    archive_get_filenames(main_view, archive_get_tab(main_view), string_get_cstr(main_view->path));

    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            uint16_t idx = 0;
            while(idx < files_array_size(model->files)) {
                ArchiveFile_t* current = files_array_get(model->files, idx);
                if(!string_search(current->name, string_get_cstr(main_view->name))) {
                    model->idx = idx;
                    break;
                }
                ++idx;
            }
            return true;
        });

    update_offset(main_view);
}

void archive_view_add_item(ArchiveMainView* main_view, FileInfo* file_info, const char* name) {
    furi_assert(main_view);
    furi_assert(file_info);
    furi_assert(name);

    ArchiveFile_t item;

    if(filter_by_extension(file_info, get_tab_ext(archive_get_tab(main_view)), name)) {
        ArchiveFile_t_init(&item);
        string_init_set_str(item.name, name);
        set_file_type(&item, file_info);

        with_view_model(
            main_view->view, (ArchiveMainViewModel * model) {
                files_array_push_back(model->files, item);
                return true;
            });

        ArchiveFile_t_clear(&item);
    }
}

static void render_item_menu(Canvas* canvas, ArchiveMainViewModel* model) {
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 71, 17, 57, 46);
    canvas_set_color(canvas, ColorBlack);
    elements_slightly_rounded_frame(canvas, 70, 16, 58, 48);

    string_t menu[MENU_ITEMS];

    string_init_set_str(menu[0], "Run in app");
    string_init_set_str(menu[1], "Pin");
    string_init_set_str(menu[2], "Rename");
    string_init_set_str(menu[3], "Delete");

    ArchiveFile_t* selected = files_array_get(model->files, model->idx);

    if(!is_known_app(selected->type)) {
        string_set_str(menu[0], "---");
        string_set_str(menu[1], "---");
        string_set_str(menu[2], "---");
    } else if(selected->fav) {
        string_set_str(menu[1], "Unpin");
    }

    for(size_t i = 0; i < MENU_ITEMS; i++) {
        canvas_draw_str(canvas, 82, 27 + i * 11, string_get_cstr(menu[i]));
        string_clear(menu[i]);
    }

    canvas_draw_icon(canvas, 74, 20 + model->menu_idx * 11, &I_ButtonRight_4x7);
}

static void archive_draw_frame(Canvas* canvas, uint16_t idx, bool scrollbar) {
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 15 + idx * FRAME_HEIGHT, scrollbar ? 122 : 127, FRAME_HEIGHT);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_dot(canvas, 0, 15 + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, 1, 15 + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, 0, (15 + idx * FRAME_HEIGHT) + 1);

    canvas_draw_dot(canvas, 0, (15 + idx * FRAME_HEIGHT) + 11);
    canvas_draw_dot(canvas, scrollbar ? 121 : 126, 15 + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, scrollbar ? 121 : 126, (15 + idx * FRAME_HEIGHT) + 11);
}

static void draw_list(Canvas* canvas, ArchiveMainViewModel* model) {
    furi_assert(model);

    size_t array_size = files_array_size(model->files);
    bool scrollbar = array_size > 4;

    for(size_t i = 0; i < MIN(array_size, MENU_ITEMS); ++i) {
        string_t str_buff;
        char cstr_buff[MAX_NAME_LEN];

        size_t idx = CLAMP(i + model->list_offset, array_size, 0);
        ArchiveFile_t* file = files_array_get(model->files, CLAMP(idx, array_size - 1, 0));

        string_init_set(str_buff, file->name);
        string_right(str_buff, string_search_rchar(str_buff, '/') + 1);
        strlcpy(cstr_buff, string_get_cstr(str_buff), string_size(str_buff) + 1);

        if(is_known_app(file->type)) archive_trim_file_ext(cstr_buff);

        string_clean(str_buff);
        string_set_str(str_buff, cstr_buff);

        elements_string_fit_width(canvas, str_buff, scrollbar ? MAX_LEN_PX - 6 : MAX_LEN_PX);

        if(model->idx == idx) {
            archive_draw_frame(canvas, i, scrollbar);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        canvas_draw_icon(canvas, 2, 16 + i * FRAME_HEIGHT, ArchiveItemIcons[file->type]);
        canvas_draw_str(canvas, 15, 24 + i * FRAME_HEIGHT, string_get_cstr(str_buff));
        string_clear(str_buff);
    }

    if(scrollbar) {
        elements_scrollbar_pos(canvas, 126, 15, 49, model->idx, array_size);
    }

    if(model->action == BrowserActionItemMenu) {
        render_item_menu(canvas, model);
    }
}

static void archive_render_status_bar(Canvas* canvas, ArchiveMainViewModel* model) {
    furi_assert(model);

    const char* tab_name = ArchiveTabNames[model->tab_idx];

    canvas_draw_icon(canvas, 0, 0, &I_Background_128x11);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 0, 0, 50, 13);
    canvas_draw_box(canvas, 107, 0, 20, 13);

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, 1, 0, 50, 12);
    canvas_draw_line(canvas, 0, 1, 0, 11);
    canvas_draw_line(canvas, 1, 12, 49, 12);
    canvas_draw_str_aligned(canvas, 26, 9, AlignCenter, AlignBottom, tab_name);

    canvas_draw_frame(canvas, 108, 0, 20, 12);
    canvas_draw_line(canvas, 107, 1, 107, 11);
    canvas_draw_line(canvas, 108, 12, 126, 12);

    if(model->tab_idx > 0) {
        canvas_draw_icon(canvas, 112, 2, &I_ButtonLeft_4x7);
    }
    if(model->tab_idx < SIZEOF_ARRAY(ArchiveTabNames) - 1) {
        canvas_draw_icon(canvas, 120, 2, &I_ButtonRight_4x7);
    }

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_dot(canvas, 50, 0);
    canvas_draw_dot(canvas, 127, 0);

    canvas_set_color(canvas, ColorBlack);
}

void archive_view_render(Canvas* canvas, void* model) {
    ArchiveMainViewModel* m = model;

    archive_render_status_bar(canvas, model);

    if(files_array_size(m->files) > 0) {
        draw_list(canvas, m);
    } else {
        canvas_draw_str_aligned(
            canvas, GUI_DISPLAY_WIDTH / 2, 40, AlignCenter, AlignCenter, "Empty");
    }
}

View* archive_main_get_view(ArchiveMainView* main_view) {
    furi_assert(main_view);
    return main_view->view;
}

static void archive_show_file_menu(ArchiveMainView* main_view) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            ArchiveFile_t* selected;
            selected = files_array_get(model->files, model->idx);
            model->action = BrowserActionItemMenu;
            model->menu_idx = 0;
            selected->fav = is_known_app(selected->type) ? archive_is_favorite(
                                                               string_get_cstr(main_view->path),
                                                               string_get_cstr(selected->name)) :
                                                           false;

            return true;
        });
}

static void archive_close_file_menu(ArchiveMainView* main_view) {
    furi_assert(main_view);

    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            model->action = BrowserActionBrowse;
            model->menu_idx = 0;
            return true;
        });
}

static void archive_run_in_app(
    ArchiveMainView* main_view,
    ArchiveFile_t* selected,
    bool full_path_provided) {
    Loader* loader = furi_record_open("loader");

    string_t full_path;

    if(!full_path_provided) {
        string_init_printf(
            full_path, "%s/%s", string_get_cstr(main_view->path), string_get_cstr(selected->name));
    } else {
        string_init_set(full_path, selected->name);
    }
    loader_start(loader, flipper_app_name[selected->type], string_get_cstr(full_path));

    string_clear(full_path);
    furi_record_close("loader");
}

static void archive_file_menu_callback(ArchiveMainView* main_view) {
    furi_assert(main_view);

    ArchiveFile_t* selected = archive_get_current_file(main_view);
    const char* path = archive_get_path(main_view);
    const char* name = archive_get_name(main_view);

    uint8_t idx;
    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            idx = model->menu_idx;
            return true;
        });

    switch(idx) {
    case 0:
        if(is_known_app(selected->type)) {
            archive_run_in_app(main_view, selected, false);
        }
        break;
    case 1:
        if(is_known_app(selected->type)) {
            if(!archive_is_favorite(path, name)) {
                archive_set_name(main_view, string_get_cstr(selected->name));
                archive_add_to_favorites(path, name);
            } else {
                // delete from favorites
                archive_favorites_delete(path, name);
            }
            archive_close_file_menu(main_view);
        }
        break;
    case 2:
        // open rename view
        if(is_known_app(selected->type)) {
            main_view->callback(ArchiveBrowserEventRename, main_view->context);
        }
        break;
    case 3:
        // confirmation?
        archive_delete_file(main_view, main_view->path, selected->name);
        archive_close_file_menu(main_view);
        break;

    default:
        archive_close_file_menu(main_view);
        break;
    }
    selected = NULL;
}

static void archive_switch_dir(ArchiveMainView* main_view, const char* path) {
    furi_assert(main_view);
    furi_assert(path);

    string_set(main_view->path, path);
    archive_get_filenames(main_view, archive_get_tab(main_view), string_get_cstr(main_view->path));
    update_offset(main_view);
}

void archive_switch_tab(ArchiveMainView* main_view) {
    furi_assert(main_view);

    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            model->idx = 0;
            model->depth = 0;
            return true;
        });

    archive_switch_dir(main_view, tab_default_paths[archive_get_tab(main_view)]);
}

static void archive_enter_dir(ArchiveMainView* main_view, string_t name) {
    furi_assert(main_view);
    furi_assert(name);

    // update last index
    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            model->last_idx[model->depth] =
                CLAMP(model->idx, files_array_size(model->files) - 1, 0);
            model->idx = 0;
            model->depth = CLAMP(model->depth + 1, MAX_DEPTH, 0);
            return true;
        });

    string_cat(main_view->path, "/");
    string_cat(main_view->path, main_view->name);

    archive_switch_dir(main_view, string_get_cstr(main_view->path));
}

static void archive_leave_dir(ArchiveMainView* main_view) {
    furi_assert(main_view);

    char* last_char_ptr = strrchr(string_get_cstr(main_view->path), '/');

    if(last_char_ptr) {
        size_t pos = last_char_ptr - string_get_cstr(main_view->path);
        string_left(main_view->path, pos);
    }

    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            model->depth = CLAMP(model->depth - 1, MAX_DEPTH, 0);
            model->idx = model->last_idx[model->depth];
            return true;
        });

    archive_switch_dir(main_view, string_get_cstr(main_view->path));
}

bool archive_view_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    ArchiveMainView* main_view = context;

    BrowserActionEnum action;
    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            action = model->action;
            return true;
        });

    switch(action) {
    case BrowserActionItemMenu:

        if(event->type == InputTypeShort) {
            if(event->key == InputKeyUp || event->key == InputKeyDown) {
                with_view_model(
                    main_view->view, (ArchiveMainViewModel * model) {
                        if(event->key == InputKeyUp) {
                            model->menu_idx = ((model->menu_idx - 1) + MENU_ITEMS) % MENU_ITEMS;
                        } else if(event->key == InputKeyDown) {
                            model->menu_idx = (model->menu_idx + 1) % MENU_ITEMS;
                        }
                        return true;
                    });
            }

            if(event->key == InputKeyOk) {
                archive_file_menu_callback(main_view);
            } else if(event->key == InputKeyBack) {
                archive_close_file_menu(main_view);
            }
        }
        break;

    case BrowserActionBrowse:

        if(event->type == InputTypeShort) {
            if(event->key == InputKeyLeft) {
                ArchiveTabEnum tab = archive_get_tab(main_view);
                if(tab) {
                    archive_set_tab(main_view, CLAMP(tab - 1, ArchiveTabTotal, 0));
                    archive_switch_tab(main_view);
                    return true;
                }
            } else if(event->key == InputKeyRight) {
                ArchiveTabEnum tab = archive_get_tab(main_view);

                if(tab < ArchiveTabTotal - 1) {
                    archive_set_tab(main_view, CLAMP(tab + 1, ArchiveTabTotal - 1, 0));
                    archive_switch_tab(main_view);
                    return true;
                }

            } else if(event->key == InputKeyBack) {
                if(!archive_get_depth(main_view)) {
                    main_view->callback(ArchiveBrowserEventExit, main_view->context);
                } else {
                    archive_leave_dir(main_view);
                }

                return true;
            }
        }
        if(event->key == InputKeyUp || event->key == InputKeyDown) {
            with_view_model(
                main_view->view, (ArchiveMainViewModel * model) {
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
            update_offset(main_view);
        }

        if(event->key == InputKeyOk) {
            ArchiveFile_t* selected = archive_get_current_file(main_view);

            if(selected) {
                archive_set_name(main_view, string_get_cstr(selected->name));
                if(selected->type == ArchiveFileTypeFolder) {
                    if(event->type == InputTypeShort) {
                        archive_enter_dir(main_view, main_view->name);
                    } else if(event->type == InputTypeLong) {
                        archive_show_file_menu(main_view);
                    }
                } else {
                    if(event->type == InputTypeShort) {
                        if(archive_get_tab(main_view) == ArchiveTabFavorites) {
                            if(is_known_app(selected->type)) {
                                archive_run_in_app(main_view, selected, true);
                            }
                        } else {
                            archive_show_file_menu(main_view);
                        }
                    }
                }
            }
        }
        break;
    default:
        break;
    }

    return true;
}

ArchiveMainView* main_view_alloc() {
    ArchiveMainView* main_view = furi_alloc(sizeof(ArchiveMainView));
    main_view->view = view_alloc();
    view_allocate_model(main_view->view, ViewModelTypeLocking, sizeof(ArchiveMainViewModel));
    view_set_context(main_view->view, main_view);
    view_set_draw_callback(main_view->view, (ViewDrawCallback)archive_view_render);
    view_set_input_callback(main_view->view, archive_view_input);

    string_init(main_view->name);
    string_init(main_view->path);

    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            files_array_init(model->files);
            return true;
        });

    return main_view;
}

void main_view_free(ArchiveMainView* main_view) {
    furi_assert(main_view);

    with_view_model(
        main_view->view, (ArchiveMainViewModel * model) {
            files_array_clear(model->files);
            return false;
        });

    string_clear(main_view->name);
    string_clear(main_view->path);

    view_free(main_view->view);
    free(main_view);
}
