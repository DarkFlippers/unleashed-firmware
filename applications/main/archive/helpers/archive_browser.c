#include "archive_files.h"
#include "archive_apps.h"
#include "archive_browser.h"
#include "../views/archive_browser_view.h"

#include <core/common_defines.h>
#include <core/log.h>
#include <gui/modules/file_browser_worker.h>
#include <flipper_application/flipper_application.h>

static void
    archive_folder_open_cb(void* context, uint32_t item_cnt, int32_t file_idx, bool is_root) {
    furi_assert(context);
    ArchiveBrowserView* browser = (ArchiveBrowserView*)context;

    int32_t load_offset = 0;
    browser->is_root = is_root;
    ArchiveTabEnum tab = archive_get_tab(browser);

    if((item_cnt == 0) && (archive_is_home(browser)) && (tab != ArchiveTabBrowser)) {
        archive_switch_tab(browser, browser->last_tab_switch_dir);
    } else if(!furi_string_start_with_str(browser->path, "/app:")) {
        with_view_model(
            browser->view,
            ArchiveBrowserViewModel * model,
            {
                files_array_reset(model->files);
                model->item_cnt = item_cnt;
                model->item_idx = (file_idx > 0) ? file_idx : 0;
                load_offset =
                    CLAMP(model->item_idx - FILE_LIST_BUF_LEN / 2, (int32_t)model->item_cnt, 0);
                model->array_offset = 0;
                model->list_offset = 0;
                model->list_loading = true;
                model->folder_loading = false;
            },
            false);
        archive_update_offset(browser);

        file_browser_worker_load(browser->worker, load_offset, FILE_LIST_BUF_LEN);
    }
}

static void archive_list_load_cb(void* context, uint32_t list_load_offset) {
    furi_assert(context);
    ArchiveBrowserView* browser = (ArchiveBrowserView*)context;

    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        {
            files_array_reset(model->files);
            model->array_offset = list_load_offset;
        },
        false);
}

static void
    archive_list_item_cb(void* context, FuriString* item_path, bool is_folder, bool is_last) {
    furi_assert(context);
    ArchiveBrowserView* browser = (ArchiveBrowserView*)context;

    if(!is_last) {
        archive_add_file_item(browser, is_folder, furi_string_get_cstr(item_path));
    } else {
        bool load_again = false;
        with_view_model(
            browser->view,
            ArchiveBrowserViewModel * model,
            {
                model->list_loading = false;
                if(archive_is_file_list_load_required(model)) {
                    load_again = true;
                }
            },
            true);
        if(load_again) {
            archive_file_array_load(browser, 0);
        }
    }
}

static void archive_long_load_cb(void* context) {
    furi_assert(context);
    ArchiveBrowserView* browser = (ArchiveBrowserView*)context;

    with_view_model(
        browser->view, ArchiveBrowserViewModel * model, { model->folder_loading = true; }, true);
}

static void archive_file_browser_set_path(
    ArchiveBrowserView* browser,
    FuriString* path,
    const char* filter_ext,
    bool skip_assets,
    bool hide_dot_files) {
    furi_assert(browser);
    if(!browser->worker_running) {
        browser->worker =
            file_browser_worker_alloc(path, NULL, filter_ext, skip_assets, hide_dot_files);
        file_browser_worker_set_callback_context(browser->worker, browser);
        file_browser_worker_set_folder_callback(browser->worker, archive_folder_open_cb);
        file_browser_worker_set_list_callback(browser->worker, archive_list_load_cb);
        file_browser_worker_set_item_callback(browser->worker, archive_list_item_cb);
        file_browser_worker_set_long_load_callback(browser->worker, archive_long_load_cb);
        browser->worker_running = true;
    } else {
        furi_assert(browser->worker);
        file_browser_worker_set_config(
            browser->worker, path, filter_ext, skip_assets, hide_dot_files);
    }
}

bool archive_is_item_in_array(ArchiveBrowserViewModel* model, uint32_t idx) {
    size_t array_size = files_array_size(model->files);

    if((idx >= (uint32_t)model->array_offset + array_size) ||
       (idx < (uint32_t)model->array_offset)) {
        return false;
    }

    return true;
}

bool archive_is_file_list_load_required(ArchiveBrowserViewModel* model) {
    size_t array_size = files_array_size(model->files);

    if((model->list_loading) || (array_size >= model->item_cnt)) {
        return false;
    }

    if((model->array_offset > 0) &&
       (model->item_idx < (model->array_offset + FILE_LIST_BUF_LEN / 4))) {
        return true;
    }

    if(((model->array_offset + array_size) < model->item_cnt) &&
       (model->item_idx > (int32_t)(model->array_offset + array_size - FILE_LIST_BUF_LEN / 4))) {
        return true;
    }

    return false;
}

void archive_update_offset(ArchiveBrowserView* browser) {
    furi_assert(browser);

    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        {
            uint16_t bounds = model->item_cnt > 3 ? 2 : model->item_cnt;

            if((model->item_cnt > 3u) && (model->item_idx >= ((int32_t)model->item_cnt - 1))) {
                model->list_offset = model->item_idx - 3;
            } else if(model->list_offset < model->item_idx - bounds) {
                model->list_offset =
                    CLAMP(model->item_idx - 2, (int32_t)model->item_cnt - bounds, 0);
            } else if(model->list_offset > model->item_idx - bounds) {
                model->list_offset =
                    CLAMP(model->item_idx - 1, (int32_t)model->item_cnt - bounds, 0);
            }
        },
        true);
}

void archive_update_focus(ArchiveBrowserView* browser, const char* target) {
    furi_assert(browser);
    furi_assert(target);

    archive_get_items(browser, furi_string_get_cstr(browser->path));

    if(!archive_file_get_array_size(browser) && archive_is_home(browser)) {
        archive_switch_tab(browser, TAB_RIGHT);
    } else {
        with_view_model(
            browser->view,
            ArchiveBrowserViewModel * model,
            {
                uint16_t idx = 0;
                while(idx < files_array_size(model->files)) {
                    ArchiveFile_t* current = files_array_get(model->files, idx);
                    if(!furi_string_search(current->path, target)) {
                        model->item_idx = idx + model->array_offset;
                        break;
                    }
                    ++idx;
                }
            },
            false);

        archive_update_offset(browser);
    }
}

size_t archive_file_get_array_size(ArchiveBrowserView* browser) {
    furi_assert(browser);

    uint16_t size = 0;
    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        { size = files_array_size(model->files); },
        false);
    return size;
}

void archive_set_item_count(ArchiveBrowserView* browser, uint32_t count) {
    furi_assert(browser);

    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        {
            model->item_cnt = count;
            model->item_idx = CLAMP(model->item_idx, (int32_t)model->item_cnt - 1, 0);
        },
        false);
    archive_update_offset(browser);
}

void archive_file_array_rm_selected(ArchiveBrowserView* browser) {
    furi_assert(browser);
    uint32_t items_cnt = 0;

    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        {
            files_array_remove_v(
                model->files,
                model->item_idx - model->array_offset,
                model->item_idx - model->array_offset + 1);
            model->item_cnt--;
            model->item_idx = CLAMP(model->item_idx, (int32_t)model->item_cnt - 1, 0);
            items_cnt = model->item_cnt;
        },
        false);

    if((items_cnt == 0) && (archive_is_home(browser))) {
        archive_switch_tab(browser, TAB_RIGHT);
    }

    archive_update_offset(browser);
}

void archive_file_array_swap(ArchiveBrowserView* browser, int8_t dir) {
    furi_assert(browser);

    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        {
            ArchiveFile_t temp;
            size_t array_size = files_array_size(model->files) - 1;
            uint8_t swap_idx = CLAMP((size_t)(model->item_idx + dir), array_size, 0u);

            if(model->item_idx == 0 && dir < 0) {
                ArchiveFile_t_init(&temp);
                files_array_pop_at(&temp, model->files, array_size);
                files_array_push_at(model->files, model->item_idx, temp);
                ArchiveFile_t_clear(&temp);
            } else if(((uint32_t)model->item_idx == array_size) && (dir > 0)) {
                ArchiveFile_t_init(&temp);
                files_array_pop_at(&temp, model->files, 0);
                files_array_push_at(model->files, array_size, temp);
                ArchiveFile_t_clear(&temp);
            } else {
                files_array_swap_at(model->files, model->item_idx, swap_idx);
            }
        },
        false);
}

void archive_file_array_rm_all(ArchiveBrowserView* browser) {
    furi_assert(browser);

    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        { files_array_reset(model->files); },
        false);
}

void archive_file_array_load(ArchiveBrowserView* browser, int8_t dir) {
    furi_assert(browser);

    int32_t offset_new = 0;

    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        {
            if(model->item_cnt > FILE_LIST_BUF_LEN) {
                if(dir < 0) {
                    offset_new = model->item_idx - FILE_LIST_BUF_LEN / 4 * 3;
                } else if(dir == 0) {
                    offset_new = model->item_idx - FILE_LIST_BUF_LEN / 4 * 2;
                } else {
                    offset_new = model->item_idx - FILE_LIST_BUF_LEN / 4 * 1;
                }
                if(offset_new > 0) {
                    offset_new = CLAMP(offset_new, (int32_t)model->item_cnt, 0);
                } else {
                    offset_new = 0;
                }
            }
        },
        false);

    file_browser_worker_load(browser->worker, offset_new, FILE_LIST_BUF_LEN);
}

ArchiveFile_t* archive_get_current_file(ArchiveBrowserView* browser) {
    furi_assert(browser);

    ArchiveFile_t* selected = NULL;
    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        {
            selected = files_array_size(model->files) ?
                           files_array_get(model->files, model->item_idx - model->array_offset) :
                           NULL;
        },
        false);
    return selected;
}

ArchiveFile_t* archive_get_file_at(ArchiveBrowserView* browser, size_t idx) {
    furi_assert(browser);

    ArchiveFile_t* selected = NULL;

    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        {
            idx = CLAMP(idx - model->array_offset, files_array_size(model->files), 0u);
            selected = files_array_size(model->files) ? files_array_get(model->files, idx) : NULL;
        },
        false);
    return selected;
}

ArchiveTabEnum archive_get_tab(ArchiveBrowserView* browser) {
    furi_assert(browser);

    ArchiveTabEnum tab_id = 0;
    with_view_model(
        browser->view, ArchiveBrowserViewModel * model, { tab_id = model->tab_idx; }, false);
    return tab_id;
}

bool archive_is_home(ArchiveBrowserView* browser) {
    furi_assert(browser);

    if(browser->is_root) {
        return true;
    }

    const char* default_path = archive_get_default_path(archive_get_tab(browser));
    return furi_string_cmp_str(browser->path, default_path) == 0;
}

const char* archive_get_name(ArchiveBrowserView* browser) {
    ArchiveFile_t* selected = archive_get_current_file(browser);
    return furi_string_get_cstr(selected->path);
}

void archive_set_tab(ArchiveBrowserView* browser, ArchiveTabEnum tab) {
    furi_assert(browser);

    with_view_model(
        browser->view, ArchiveBrowserViewModel * model, { model->tab_idx = tab; }, false);
}

void archive_add_app_item(ArchiveBrowserView* browser, const char* name) {
    furi_assert(browser);
    furi_assert(name);

    ArchiveFile_t item;
    ArchiveFile_t_init(&item);
    furi_string_set(item.path, name);
    archive_set_file_type(&item, name, false, true);

    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        {
            files_array_push_back(model->files, item);
            model->item_cnt = files_array_size(model->files);
        },
        false);
    ArchiveFile_t_clear(&item);
}

static bool archive_get_fap_meta(FuriString* file_path, FuriString* fap_name, uint8_t** icon_ptr) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool success = false;
    if(flipper_application_load_name_and_icon(file_path, storage, icon_ptr, fap_name)) {
        success = true;
    }
    furi_record_close(RECORD_STORAGE);
    return success;
}

void archive_add_file_item(ArchiveBrowserView* browser, bool is_folder, const char* name) {
    furi_assert(browser);
    furi_assert(name);

    ArchiveFile_t item;
    ArchiveFile_t_init(&item);

    furi_string_set(item.path, name);
    archive_set_file_type(&item, furi_string_get_cstr(browser->path), is_folder, false);
    if(item.type == ArchiveFileTypeApplication) {
        item.custom_icon_data = malloc(FAP_MANIFEST_MAX_ICON_SIZE);
        if(!archive_get_fap_meta(item.path, item.custom_name, &item.custom_icon_data)) {
            free(item.custom_icon_data);
            item.custom_icon_data = NULL;
        }
    }
    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        { files_array_push_back(model->files, item); },
        false);
    ArchiveFile_t_clear(&item);
}

void archive_show_file_menu(ArchiveBrowserView* browser, bool show) {
    furi_assert(browser);
    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        {
            if(show) {
                if(archive_is_item_in_array(model, model->item_idx)) {
                    model->menu = true;
                    model->menu_idx = 0;
                    ArchiveFile_t* selected =
                        files_array_get(model->files, model->item_idx - model->array_offset);
                    selected->fav =
                        archive_is_favorite("%s", furi_string_get_cstr(selected->path));
                }
            } else {
                model->menu = false;
                model->menu_idx = 0;
            }
        },
        true);
}

void archive_favorites_move_mode(ArchiveBrowserView* browser, bool active) {
    furi_assert(browser);

    with_view_model(
        browser->view, ArchiveBrowserViewModel * model, { model->move_fav = active; }, true);
}

static bool archive_is_dir_exists(FuriString* path) {
    bool state = false;
    FileInfo file_info;
    Storage* storage = furi_record_open(RECORD_STORAGE);

    if(furi_string_equal(path, STORAGE_EXT_PATH_PREFIX)) {
        state = storage_sd_status(storage) == FSE_OK;
    } else if(storage_common_stat(storage, furi_string_get_cstr(path), &file_info) == FSE_OK) {
        state = file_info_is_dir(&file_info);
    }
    furi_record_close(RECORD_STORAGE);
    return state;
}

void archive_switch_tab(ArchiveBrowserView* browser, InputKey key) {
    furi_assert(browser);
    ArchiveTabEnum tab = archive_get_tab(browser);

    browser->last_tab_switch_dir = key;

    if(key == InputKeyLeft) {
        tab = ((tab - 1) + ArchiveTabTotal) % ArchiveTabTotal;
    } else {
        tab = (tab + 1) % ArchiveTabTotal;
    }

    browser->is_root = true;
    archive_set_tab(browser, tab);

    furi_string_set(browser->path, archive_get_default_path(tab));
    bool tab_empty = true;
    if(tab == ArchiveTabFavorites) {
        if(archive_favorites_count(browser) > 0) {
            tab_empty = false;
        }
    } else if(furi_string_start_with_str(browser->path, "/app:")) {
        char* app_name = strchr(furi_string_get_cstr(browser->path), ':');
        if(app_name != NULL) {
            if(archive_app_is_available(browser, furi_string_get_cstr(browser->path))) {
                tab_empty = false;
            }
        }
    } else {
        tab = archive_get_tab(browser);
        if(archive_is_dir_exists(browser->path)) {
            bool skip_assets = (strcmp(archive_get_tab_ext(tab), "*") == 0) ? false : true;
            // Hide dot files everywhere except Browser
            bool hide_dot_files = (strcmp(archive_get_tab_ext(tab), "*") == 0) ? false : true;
            archive_file_browser_set_path(
                browser, browser->path, archive_get_tab_ext(tab), skip_assets, hide_dot_files);
            tab_empty = false; // Empty check will be performed later
        }
    }

    if((tab_empty) && (tab != ArchiveTabBrowser)) {
        archive_switch_tab(browser, key);
    } else {
        with_view_model(
            browser->view,
            ArchiveBrowserViewModel * model,
            {
                model->item_idx = 0;
                model->array_offset = 0;
            },
            false);
        archive_get_items(browser, furi_string_get_cstr(browser->path));
        archive_update_offset(browser);
    }
}

void archive_enter_dir(ArchiveBrowserView* browser, FuriString* path) {
    furi_assert(browser);
    furi_assert(path);

    int32_t idx_temp = 0;

    with_view_model(
        browser->view, ArchiveBrowserViewModel * model, { idx_temp = model->item_idx; }, false);

    furi_string_set(browser->path, path);

    file_browser_worker_folder_enter(browser->worker, path, idx_temp);
}

void archive_leave_dir(ArchiveBrowserView* browser) {
    furi_assert(browser);

    size_t dirname_start = furi_string_search_rchar(browser->path, '/');
    furi_string_left(browser->path, dirname_start);

    file_browser_worker_folder_exit(browser->worker);
}

void archive_refresh_dir(ArchiveBrowserView* browser) {
    furi_assert(browser);

    int32_t idx_temp = 0;

    with_view_model(
        browser->view, ArchiveBrowserViewModel * model, { idx_temp = model->item_idx; }, false);
    file_browser_worker_folder_refresh(browser->worker, idx_temp);
}
