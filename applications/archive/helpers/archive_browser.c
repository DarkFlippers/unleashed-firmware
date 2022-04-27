#include "archive_files.h"
#include "archive_apps.h"
#include "archive_browser.h"
#include <math.h>

bool archive_is_item_in_array(ArchiveBrowserViewModel* model, uint32_t idx) {
    size_t array_size = files_array_size(model->files);

    if((idx >= model->array_offset + array_size) || (idx < model->array_offset)) {
        return false;
    }

    return true;
}

void archive_update_offset(ArchiveBrowserView* browser) {
    furi_assert(browser);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            uint16_t bounds = model->item_cnt > 3 ? 2 : model->item_cnt;

            if(model->item_cnt > 3 && model->item_idx >= model->item_cnt - 1) {
                model->list_offset = model->item_idx - 3;
            } else if(model->list_offset < model->item_idx - bounds) {
                model->list_offset = CLAMP(model->item_idx - 2, model->item_cnt - bounds, 0);
            } else if(model->list_offset > model->item_idx - bounds) {
                model->list_offset = CLAMP(model->item_idx - 1, model->item_cnt - bounds, 0);
            }

            return true;
        });
}

void archive_update_focus(ArchiveBrowserView* browser, const char* target) {
    furi_assert(browser);
    furi_assert(target);

    archive_get_filenames(browser, string_get_cstr(browser->path));

    if(!archive_file_get_array_size(browser) && !archive_get_depth(browser)) {
        archive_switch_tab(browser, TAB_RIGHT);
    } else {
        with_view_model(
            browser->view, (ArchiveBrowserViewModel * model) {
                uint16_t idx = 0;
                while(idx < files_array_size(model->files)) {
                    ArchiveFile_t* current = files_array_get(model->files, idx);
                    if(!string_search(current->name, target)) {
                        model->item_idx = idx + model->array_offset;
                        break;
                    }
                    ++idx;
                }
                return false;
            });

        archive_update_offset(browser);
    }
}

size_t archive_file_get_array_size(ArchiveBrowserView* browser) {
    furi_assert(browser);

    uint16_t size = 0;
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            size = files_array_size(model->files);
            return false;
        });
    return size;
}

void archive_set_item_count(ArchiveBrowserView* browser, uint32_t count) {
    furi_assert(browser);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            model->item_cnt = count;
            model->item_idx = CLAMP(model->item_idx, model->item_cnt - 1, 0);
            return false;
        });
}

void archive_file_array_rm_selected(ArchiveBrowserView* browser) {
    furi_assert(browser);
    uint32_t items_cnt = 0;

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            files_array_remove_v(
                model->files,
                model->item_idx - model->array_offset,
                model->item_idx - model->array_offset + 1);
            model->item_cnt--;
            model->item_idx = CLAMP(model->item_idx, model->item_cnt - 1, 0);
            items_cnt = model->item_cnt;
            return false;
        });

    if((items_cnt == 0) && (archive_get_depth(browser) == 0)) {
        archive_switch_tab(browser, TAB_RIGHT);
    }

    archive_update_offset(browser);
}

void archive_file_array_swap(ArchiveBrowserView* browser, int8_t dir) {
    furi_assert(browser);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            ArchiveFile_t temp;
            size_t array_size = files_array_size(model->files) - 1;
            uint8_t swap_idx = CLAMP(model->item_idx + dir, array_size, 0);

            if(model->item_idx == 0 && dir < 0) {
                ArchiveFile_t_init(&temp);
                files_array_pop_at(&temp, model->files, array_size);
                files_array_push_at(model->files, model->item_idx, temp);
                ArchiveFile_t_clear(&temp);
            } else if(model->item_idx == array_size && dir > 0) {
                ArchiveFile_t_init(&temp);
                files_array_pop_at(&temp, model->files, 0);
                files_array_push_at(model->files, array_size, temp);
                ArchiveFile_t_clear(&temp);
            } else {
                files_array_swap_at(model->files, model->item_idx, swap_idx);
            }
            return false;
        });
}

void archive_file_array_rm_all(ArchiveBrowserView* browser) {
    furi_assert(browser);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            files_array_reset(model->files);
            return false;
        });
}

bool archive_file_array_load(ArchiveBrowserView* browser, int8_t dir) {
    furi_assert(browser);

    int32_t offset_new = 0;

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            if(model->item_cnt > FILE_LIST_BUF_LEN) {
                if(dir < 0) {
                    offset_new = model->item_idx - FILE_LIST_BUF_LEN / 4 * 3;
                } else if(dir == 0) {
                    offset_new = model->item_idx - FILE_LIST_BUF_LEN / 4 * 2;
                } else {
                    offset_new = model->item_idx - FILE_LIST_BUF_LEN / 4 * 1;
                }
                offset_new = CLAMP(offset_new, model->item_cnt - FILE_LIST_BUF_LEN, 0);
            }
            return false;
        });

    bool res = archive_dir_read_items(
        browser, string_get_cstr(browser->path), offset_new, FILE_LIST_BUF_LEN);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            model->array_offset = offset_new;
            model->list_loading = false;
            return true;
        });

    return res;
}

ArchiveFile_t* archive_get_current_file(ArchiveBrowserView* browser) {
    furi_assert(browser);

    ArchiveFile_t* selected;
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            selected = files_array_size(model->files) ?
                           files_array_get(model->files, model->item_idx - model->array_offset) :
                           NULL;
            return false;
        });
    return selected;
}

ArchiveFile_t* archive_get_file_at(ArchiveBrowserView* browser, size_t idx) {
    furi_assert(browser);

    ArchiveFile_t* selected;

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            idx = CLAMP(idx - model->array_offset, files_array_size(model->files), 0);
            selected = files_array_size(model->files) ? files_array_get(model->files, idx) : NULL;
            return false;
        });
    return selected;
}

ArchiveTabEnum archive_get_tab(ArchiveBrowserView* browser) {
    furi_assert(browser);

    ArchiveTabEnum tab_id;
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            tab_id = model->tab_idx;
            return false;
        });
    return tab_id;
}

uint8_t archive_get_depth(ArchiveBrowserView* browser) {
    furi_assert(browser);

    uint8_t depth;
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            depth = idx_last_array_size(model->idx_last);
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
    furi_assert(browser);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            model->tab_idx = tab;
            return false;
        });
}
void archive_set_last_tab(ArchiveBrowserView* browser, ArchiveTabEnum tab) {
    furi_assert(browser);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            model->last_tab = model->tab_idx;
            return false;
        });
}

void archive_add_app_item(ArchiveBrowserView* browser, const char* name) {
    furi_assert(browser);
    furi_assert(name);

    ArchiveFile_t item;

    string_t full_name;

    string_init_set(full_name, browser->path);
    string_cat_printf(full_name, "/%s", name);

    char* app_name = strchr(string_get_cstr(full_name), ':');
    if(app_name == NULL) {
        string_clear(full_name);
        return;
    }

    ArchiveFile_t_init(&item);
    string_init_set_str(item.name, name);
    archive_set_file_type(&item, NULL, app_name + 1, true);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            files_array_push_back(model->files, item);
            model->item_cnt = files_array_size(model->files);
            return false;
        });
    ArchiveFile_t_clear(&item);
    string_clear(full_name);
}

void archive_add_file_item(ArchiveBrowserView* browser, FileInfo* file_info, const char* name) {
    furi_assert(browser);
    furi_assert(file_info);
    furi_assert(name);

    ArchiveFile_t item;

    if(archive_filter_by_extension(
           file_info, archive_get_tab_ext(archive_get_tab(browser)), name)) {
        ArchiveFile_t_init(&item);
        string_init_set_str(item.name, name);
        archive_set_file_type(&item, file_info, archive_get_path(browser), false);

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
            if(show) {
                if(archive_is_item_in_array(model, model->item_idx)) {
                    model->menu = true;
                    model->menu_idx = 0;
                    ArchiveFile_t* selected =
                        files_array_get(model->files, model->item_idx - model->array_offset);
                    selected->fav = archive_is_favorite("%s", string_get_cstr(selected->name));
                }
            } else {
                model->menu = false;
                model->menu_idx = 0;
            }

            return true;
        });
}

void archive_favorites_move_mode(ArchiveBrowserView* browser, bool active) {
    furi_assert(browser);

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

    const char* path = archive_get_default_path(tab);
    bool tab_empty = true;
    if(tab == ArchiveTabFavorites) {
        if(archive_favorites_count(browser) > 0) tab_empty = false;
    } else if(strncmp(path, "/app:", 5) == 0) {
        if(archive_app_is_available(browser, path)) tab_empty = false;
    } else {
        uint32_t files_cnt = archive_dir_count_items(browser, archive_get_default_path(tab));
        if(files_cnt > 0) tab_empty = false;
    }

    if((tab_empty) && (tab != ArchiveTabBrowser)) {
        archive_switch_tab(browser, key);
    } else {
        with_view_model(
            browser->view, (ArchiveBrowserViewModel * model) {
                if(model->last_tab != model->tab_idx) {
                    model->item_idx = 0;
                    model->array_offset = 0;
                    idx_last_array_reset(model->idx_last);
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

    if(string_size(name) >= (MAX_NAME_LEN - 1)) {
        return;
    }

    archive_dir_count_items(browser, string_get_cstr(name));

    if(string_cmp(browser->path, name) != 0) {
        with_view_model(
            browser->view, (ArchiveBrowserViewModel * model) {
                idx_last_array_push_back(model->idx_last, model->item_idx);
                model->array_offset = 0;
                model->item_idx = 0;
                return false;
            });

        string_set(browser->path, name);
    }

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

    archive_dir_count_items(browser, path);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            idx_last_array_pop_back(&model->item_idx, model->idx_last);
            return false;
        });

    archive_switch_dir(browser, path);
}
