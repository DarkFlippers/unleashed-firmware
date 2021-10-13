#include "archive_browser.h"
#include "math.h"

void archive_update_offset(ArchiveBrowserView* browser) {
    furi_assert(browser);
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            size_t array_size = files_array_size(model->files);
            uint16_t bounds = array_size > 3 ? 2 : array_size;

            if(array_size > 3 && model->idx >= array_size - 1) {
                model->list_offset = model->idx - 3;
            } else if(
                model->last_offset && model->last_offset != model->list_offset &&
                model->tab_idx == model->last_tab) {
                model->list_offset = model->last_offset;
                model->last_offset = !model->last_offset;
            } else if(model->list_offset < model->idx - bounds) {
                model->list_offset = CLAMP(model->idx - 2, array_size - bounds, 0);
            } else if(model->list_offset > model->idx - bounds) {
                model->list_offset = CLAMP(model->idx - 1, array_size - bounds, 0);
            }
            return true;
        });
}

void archive_update_focus(ArchiveBrowserView* browser, const char* target) {
    furi_assert(browser);
    furi_assert(target);

    archive_get_filenames(browser, string_get_cstr(browser->path));

    if(!archive_file_array_size(browser) && !archive_get_depth(browser)) {
        archive_switch_tab(browser, DEFAULT_TAB_DIR);
    } else {
        with_view_model(
            browser->view, (ArchiveBrowserViewModel * model) {
                uint16_t idx = 0;
                while(idx < files_array_size(model->files)) {
                    ArchiveFile_t* current = files_array_get(model->files, idx);
                    if(!string_search(current->name, target)) {
                        model->idx = idx;
                        break;
                    }
                    ++idx;
                }
                return false;
            });

        archive_update_offset(browser);
    }
}

size_t archive_file_array_size(ArchiveBrowserView* browser) {
    uint16_t size = 0;
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            size = files_array_size(model->files);
            return false;
        });
    return size;
}

void archive_file_array_rm_selected(ArchiveBrowserView* browser) {
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            files_array_remove_v(model->files, model->idx, model->idx + 1);
            model->idx = CLAMP(model->idx, files_array_size(model->files) - 1, 0);
            return false;
        });

    if(!archive_file_array_size(browser) && !archive_get_depth(browser)) {
        archive_switch_tab(browser, DEFAULT_TAB_DIR);
    }

    archive_update_offset(browser);
}

void archive_file_array_swap(ArchiveBrowserView* browser, int8_t d) {
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            ArchiveFile_t temp;
            size_t array_size = files_array_size(model->files) - 1;
            uint8_t swap_idx = CLAMP(model->idx + d, array_size, 0);

            if(model->idx == 0 && d < 0) {
                ArchiveFile_t_init(&temp);
                files_array_pop_at(&temp, model->files, array_size);
                files_array_push_at(model->files, model->idx, temp);
                ArchiveFile_t_clear(&temp);
            } else if(model->idx == array_size && d > 0) {
                ArchiveFile_t_init(&temp);
                files_array_pop_at(&temp, model->files, model->last_idx);
                files_array_push_at(model->files, array_size, temp);
                ArchiveFile_t_clear(&temp);
            } else {
                files_array_swap_at(model->files, model->idx, swap_idx);
            }

            return false;
        });
}

void archive_file_array_rm_all(ArchiveBrowserView* browser) {
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            files_array_clean(model->files);
            return false;
        });
}

ArchiveFile_t* archive_get_current_file(ArchiveBrowserView* browser) {
    ArchiveFile_t* selected;
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            selected = files_array_size(model->files) ? files_array_get(model->files, model->idx) :
                                                        NULL;
            return false;
        });
    return selected;
}

ArchiveFile_t* archive_get_file_at(ArchiveBrowserView* browser, size_t idx) {
    ArchiveFile_t* selected;
    idx = CLAMP(idx, archive_file_array_size(browser), 0);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            selected = files_array_size(model->files) ? files_array_get(model->files, idx) : NULL;
            return false;
        });
    return selected;
}

ArchiveTabEnum archive_get_tab(ArchiveBrowserView* browser) {
    ArchiveTabEnum tab_id;
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            tab_id = model->tab_idx;
            return false;
        });
    return tab_id;
}

uint8_t archive_get_depth(ArchiveBrowserView* browser) {
    uint8_t depth;
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            depth = model->depth;
            return false;
        });

    return depth;
}

const char* archive_get_path(ArchiveBrowserView* browser) {
    return string_get_cstr(browser->path);
}

const char* archive_get_name(ArchiveBrowserView* browser) {
    ArchiveFile_t* selected = archive_get_current_file(browser);
    return string_get_cstr(selected->name);
}

void archive_set_tab(ArchiveBrowserView* browser, ArchiveTabEnum tab) {
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            model->tab_idx = tab;
            return false;
        });
}
void archive_set_last_tab(ArchiveBrowserView* browser, ArchiveTabEnum tab) {
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            model->last_tab = model->tab_idx;
            return false;
        });
}

void archive_add_item(ArchiveBrowserView* browser, FileInfo* file_info, const char* name) {
    furi_assert(browser);
    furi_assert(file_info);
    furi_assert(name);

    ArchiveFile_t item;

    if(filter_by_extension(file_info, get_tab_ext(archive_get_tab(browser)), name)) {
        ArchiveFile_t_init(&item);
        string_init_set_str(item.name, name);
        set_file_type(&item, file_info);

        with_view_model(
            browser->view, (ArchiveBrowserViewModel * model) {
                files_array_push_back(model->files, item);
                return false;
            });

        ArchiveFile_t_clear(&item);
    }
}

void archive_show_file_menu(ArchiveBrowserView* browser, bool show) {
    furi_assert(browser);
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            model->menu = show;
            model->menu_idx = 0;

            if(show) {
                ArchiveFile_t* selected = files_array_get(model->files, model->idx);
                selected->fav = archive_is_favorite(
                    "%s/%s", string_get_cstr(browser->path), string_get_cstr(selected->name));
            }

            return true;
        });
}

void archive_favorites_move_mode(ArchiveBrowserView* browser, bool active) {
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            model->move_fav = active;
            return true;
        });
}

void archive_switch_dir(ArchiveBrowserView* browser, const char* path) {
    furi_assert(browser);
    furi_assert(path);

    string_set(browser->path, path);
    archive_get_filenames(browser, string_get_cstr(browser->path));
    archive_update_offset(browser);
}

void archive_switch_tab(ArchiveBrowserView* browser, InputKey key) {
    furi_assert(browser);
    ArchiveTabEnum tab = archive_get_tab(browser);

    if(key == InputKeyLeft) {
        tab = ((tab - 1) + ArchiveTabTotal) % ArchiveTabTotal;
    } else if(key == InputKeyRight) {
        tab = (tab + 1) % ArchiveTabTotal;
    }

    archive_set_tab(browser, tab);

    if((tab != ArchiveTabFavorites &&
        !archive_dir_empty(browser, archive_get_default_path(tab))) ||
       (tab == ArchiveTabFavorites && !archive_favorites_count(browser))) {
        if(tab != ArchiveTabBrowser) {
            archive_switch_tab(browser, key);
        }
    } else {
        with_view_model(
            browser->view, (ArchiveBrowserViewModel * model) {
                if(model->last_tab != model->tab_idx) {
                    model->idx = 0;
                    model->depth = 0;
                }
                return false;
            });
        archive_switch_dir(browser, archive_get_default_path(tab));
    }
    archive_set_last_tab(browser, tab);
}

void archive_enter_dir(ArchiveBrowserView* browser, string_t name) {
    furi_assert(browser);
    furi_assert(name);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            model->last_idx = model->idx;
            model->last_offset = model->list_offset;
            model->idx = 0;
            model->depth = CLAMP(model->depth + 1, MAX_DEPTH, 0);
            return false;
        });

    string_cat(browser->path, "/");
    string_cat(browser->path, name);

    archive_switch_dir(browser, string_get_cstr(browser->path));
}

void archive_leave_dir(ArchiveBrowserView* browser) {
    furi_assert(browser);

    const char* path = archive_get_path(browser);
    char* last_char_ptr = strrchr(path, '/');

    if(last_char_ptr) {
        size_t pos = last_char_ptr - path;
        string_left(browser->path, pos);
    }

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            model->depth = CLAMP(model->depth - 1, MAX_DEPTH, 0);
            model->idx = model->last_idx;
            return false;
        });

    archive_switch_dir(browser, path);
}
