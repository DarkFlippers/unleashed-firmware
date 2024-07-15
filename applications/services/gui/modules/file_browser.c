#include "file_browser.h"
#include "file_browser_worker.h"

#include <gui/elements.h>
#include <assets_icons.h>
#include <toolbox/path.h>

#include <furi.h>
#include <furi_hal_resources.h>

#include <core/check.h>
#include <core/common_defines.h>
#include <core/log.h>
#include <m-array.h>

#define LIST_ITEMS   5u
#define MAX_LEN_PX   110
#define FRAME_HEIGHT 12
#define Y_OFFSET     3

#define ITEM_LIST_LEN_MAX 50

#define CUSTOM_ICON_MAX_SIZE 32

#define SCROLL_INTERVAL (333)
#define SCROLL_DELAY    (2)

typedef enum {
    BrowserItemTypeLoading,
    BrowserItemTypeBack,
    BrowserItemTypeFolder,
    BrowserItemTypeFile,
} BrowserItemType;

typedef struct {
    FuriString* path;
    BrowserItemType type;
    uint8_t* custom_icon_data;
    FuriString* display_name;
} BrowserItem_t;

static void BrowserItem_t_init(BrowserItem_t* obj) {
    obj->type = BrowserItemTypeLoading;
    obj->path = furi_string_alloc();
    obj->display_name = furi_string_alloc();
    obj->custom_icon_data = NULL;
}

static void BrowserItem_t_init_set(BrowserItem_t* obj, const BrowserItem_t* src) {
    obj->type = src->type;
    obj->path = furi_string_alloc_set(src->path);
    obj->display_name = furi_string_alloc_set(src->display_name);
    if(src->custom_icon_data) {
        obj->custom_icon_data = malloc(CUSTOM_ICON_MAX_SIZE);
        memcpy(obj->custom_icon_data, src->custom_icon_data, CUSTOM_ICON_MAX_SIZE);
    } else {
        obj->custom_icon_data = NULL;
    }
}

static void BrowserItem_t_set(BrowserItem_t* obj, const BrowserItem_t* src) {
    obj->type = src->type;
    furi_string_set(obj->path, src->path);
    furi_string_set(obj->display_name, src->display_name);
    if(src->custom_icon_data) {
        memcpy(obj->custom_icon_data, src->custom_icon_data, CUSTOM_ICON_MAX_SIZE);
    } else {
        obj->custom_icon_data = NULL;
    }
}

static void BrowserItem_t_clear(BrowserItem_t* obj) {
    furi_string_free(obj->path);
    furi_string_free(obj->display_name);
    if(obj->custom_icon_data) {
        free(obj->custom_icon_data);
    }
}

ARRAY_DEF(
    items_array,
    BrowserItem_t,
    (INIT(API_2(BrowserItem_t_init)),
     SET(API_6(BrowserItem_t_set)),
     INIT_SET(API_6(BrowserItem_t_init_set)),
     CLEAR(API_2(BrowserItem_t_clear))))

struct FileBrowser {
    View* view;
    BrowserWorker* worker;
    const char* ext_filter;
    const char* base_path;
    bool skip_assets;
    bool hide_dot_files;
    bool hide_ext;

    FileBrowserCallback callback;
    void* context;

    FileBrowserLoadItemCallback item_callback;
    void* item_context;

    FuriString* result_path;
    FuriTimer* scroll_timer;
};

typedef struct {
    items_array_t items;

    bool is_root;
    bool folder_loading;
    bool list_loading;
    uint32_t item_cnt;
    int32_t item_idx;
    int32_t array_offset;
    int32_t list_offset;

    const Icon* file_icon;
    bool hide_ext;
    size_t scroll_counter;

    uint32_t button_held_for_ticks;
} FileBrowserModel;

static const Icon* BrowserItemIcons[] = {
    [BrowserItemTypeLoading] = &I_loading_10px,
    [BrowserItemTypeBack] = &I_back_10px,
    [BrowserItemTypeFolder] = &I_dir_10px,
    [BrowserItemTypeFile] = &I_unknown_10px,
};

static void file_browser_view_draw_callback(Canvas* canvas, void* _model);
static bool file_browser_view_input_callback(InputEvent* event, void* context);

static void
    browser_folder_open_cb(void* context, uint32_t item_cnt, int32_t file_idx, bool is_root);
static void browser_list_load_cb(void* context, uint32_t list_load_offset);
static void
    browser_list_item_cb(void* context, FuriString* item_path, bool is_folder, bool is_last);
static void browser_long_load_cb(void* context);

static void file_browser_scroll_timer_callback(void* context) {
    furi_check(context);
    FileBrowser* browser = context;
    with_view_model(browser->view, FileBrowserModel * model, { model->scroll_counter++; }, true);
}

static void file_browser_view_enter_callback(void* context) {
    furi_check(context);
    FileBrowser* browser = context;
    with_view_model(browser->view, FileBrowserModel * model, { model->scroll_counter = 0; }, true);
    furi_timer_start(browser->scroll_timer, SCROLL_INTERVAL);
}

static void file_browser_view_exit_callback(void* context) {
    furi_check(context);
    FileBrowser* browser = context;
    furi_timer_stop(browser->scroll_timer);
}

FileBrowser* file_browser_alloc(FuriString* result_path) {
    furi_check(result_path);

    FileBrowser* browser = malloc(sizeof(FileBrowser));
    browser->view = view_alloc();
    view_allocate_model(browser->view, ViewModelTypeLocking, sizeof(FileBrowserModel));
    view_set_context(browser->view, browser);
    view_set_draw_callback(browser->view, file_browser_view_draw_callback);
    view_set_input_callback(browser->view, file_browser_view_input_callback);
    view_set_enter_callback(browser->view, file_browser_view_enter_callback);
    view_set_exit_callback(browser->view, file_browser_view_exit_callback);

    browser->scroll_timer =
        furi_timer_alloc(file_browser_scroll_timer_callback, FuriTimerTypePeriodic, browser);

    browser->result_path = result_path;

    with_view_model(
        browser->view, FileBrowserModel * model, { items_array_init(model->items); }, false);

    return browser;
}

void file_browser_free(FileBrowser* browser) {
    furi_check(browser);

    furi_timer_free(browser->scroll_timer);

    with_view_model(
        browser->view, FileBrowserModel * model, { items_array_clear(model->items); }, false);

    view_free(browser->view);
    free(browser);
}

View* file_browser_get_view(FileBrowser* browser) {
    furi_check(browser);
    return browser->view;
}

void file_browser_configure(
    FileBrowser* browser,
    const char* extension,
    const char* base_path,
    bool skip_assets,
    bool hide_dot_files,
    const Icon* file_icon,
    bool hide_ext) {
    furi_check(browser);

    browser->ext_filter = extension;
    browser->skip_assets = skip_assets;
    browser->hide_ext = hide_ext;
    browser->base_path = base_path;
    browser->hide_dot_files = hide_dot_files;

    with_view_model(
        browser->view,
        FileBrowserModel * model,
        {
            model->file_icon = file_icon;
            model->hide_ext = hide_ext;
        },
        false);
}

void file_browser_start(FileBrowser* browser, FuriString* path) {
    furi_check(browser);
    browser->worker = file_browser_worker_alloc(
        path,
        browser->base_path,
        browser->ext_filter,
        browser->skip_assets,
        browser->hide_dot_files);
    file_browser_worker_set_callback_context(browser->worker, browser);
    file_browser_worker_set_folder_callback(browser->worker, browser_folder_open_cb);
    file_browser_worker_set_list_callback(browser->worker, browser_list_load_cb);
    file_browser_worker_set_item_callback(browser->worker, browser_list_item_cb);
    file_browser_worker_set_long_load_callback(browser->worker, browser_long_load_cb);
}

void file_browser_stop(FileBrowser* browser) {
    furi_check(browser);
    file_browser_worker_free(browser->worker);
    with_view_model(
        browser->view,
        FileBrowserModel * model,
        {
            items_array_reset(model->items);
            model->item_cnt = 0;
            model->item_idx = 0;
            model->array_offset = 0;
            model->list_offset = 0;
        },
        false);
}

void file_browser_set_callback(FileBrowser* browser, FileBrowserCallback callback, void* context) {
    furi_check(browser);
    browser->context = context;
    browser->callback = callback;
}

void file_browser_set_item_callback(
    FileBrowser* browser,
    FileBrowserLoadItemCallback callback,
    void* context) {
    furi_check(browser);

    browser->item_context = context;
    browser->item_callback = callback;
}

static bool browser_is_item_in_array(FileBrowserModel* model, uint32_t idx) {
    size_t array_size = items_array_size(model->items);

    if((idx >= (uint32_t)model->array_offset + array_size) ||
       (idx < (uint32_t)model->array_offset)) {
        return false;
    }
    return true;
}

static bool browser_is_list_load_required(FileBrowserModel* model) {
    size_t array_size = items_array_size(model->items);
    if((array_size > 0) && (!model->is_root) && (model->array_offset == 0)) {
        array_size--;
    }
    uint32_t item_cnt = (model->is_root) ? (model->item_cnt) : (model->item_cnt - 1);

    if((model->list_loading) || (array_size >= item_cnt)) {
        return false;
    }

    if((model->array_offset > 0) &&
       (model->item_idx < (model->array_offset + ITEM_LIST_LEN_MAX / 4))) {
        return true;
    }

    if(((model->array_offset + array_size) < item_cnt) &&
       (model->item_idx > (int32_t)(model->array_offset + array_size - ITEM_LIST_LEN_MAX / 4))) {
        return true;
    }

    return false;
}

static void browser_list_rollover(FileBrowserModel* model) {
    if(!model->list_loading && items_array_size(model->items) < model->item_cnt) {
        items_array_reset(model->items);
    }
}

static void browser_update_offset(FileBrowser* browser) {
    furi_check(browser);

    with_view_model(
        browser->view,
        FileBrowserModel * model,
        {
            uint16_t bounds = model->item_cnt > (LIST_ITEMS - 1) ? 2 : model->item_cnt;

            if((model->item_cnt > (LIST_ITEMS - 1)) &&
               (model->item_idx >= ((int32_t)model->item_cnt - 1))) {
                model->list_offset = model->item_idx - (LIST_ITEMS - 1);
            } else if(model->list_offset < model->item_idx - bounds) {
                model->list_offset = CLAMP(
                    model->item_idx - (int32_t)(LIST_ITEMS - 2),
                    (int32_t)model->item_cnt - bounds,
                    0);
            } else if(model->list_offset > model->item_idx - bounds) {
                model->list_offset =
                    CLAMP(model->item_idx - 1, (int32_t)model->item_cnt - bounds, 0);
            }
        },
        false);
}

static void
    browser_folder_open_cb(void* context, uint32_t item_cnt, int32_t file_idx, bool is_root) {
    furi_check(context);
    FileBrowser* browser = (FileBrowser*)context;

    int32_t load_offset = 0;

    with_view_model(
        browser->view,
        FileBrowserModel * model,
        {
            items_array_reset(model->items);
            if(is_root) {
                model->item_cnt = item_cnt;
                model->item_idx = (file_idx > 0) ? file_idx : 0;
                load_offset =
                    CLAMP(model->item_idx - ITEM_LIST_LEN_MAX / 2, (int32_t)model->item_cnt, 0);
            } else {
                model->item_cnt = item_cnt + 1;
                model->item_idx = file_idx + 1;
                load_offset = CLAMP(
                    model->item_idx - ITEM_LIST_LEN_MAX / 2 - 1, (int32_t)model->item_cnt - 1, 0);
            }
            model->array_offset = 0;
            model->list_offset = 0;
            model->is_root = is_root;
            model->list_loading = true;
            model->folder_loading = false;
        },
        true);
    browser_update_offset(browser);

    file_browser_worker_load(browser->worker, load_offset, ITEM_LIST_LEN_MAX);
}

static void browser_list_load_cb(void* context, uint32_t list_load_offset) {
    furi_check(context);
    FileBrowser* browser = (FileBrowser*)context;

    BrowserItem_t back_item;
    BrowserItem_t_init(&back_item);
    back_item.type = BrowserItemTypeBack;

    with_view_model(
        browser->view,
        FileBrowserModel * model,
        {
            items_array_reset(model->items);
            model->array_offset = list_load_offset;
            if(!model->is_root) {
                if(list_load_offset == 0) {
                    items_array_push_back(model->items, back_item);
                } else {
                    model->array_offset += 1;
                }
            }
        },
        false);

    BrowserItem_t_clear(&back_item);
}

static void
    browser_list_item_cb(void* context, FuriString* item_path, bool is_folder, bool is_last) {
    furi_check(context);
    FileBrowser* browser = (FileBrowser*)context;

    BrowserItem_t item;
    item.custom_icon_data = NULL;

    if(!is_last) {
        item.path = furi_string_alloc_set(item_path);
        item.display_name = furi_string_alloc();
        if(is_folder) {
            item.type = BrowserItemTypeFolder;
        } else {
            item.type = BrowserItemTypeFile;
            if(browser->item_callback) {
                item.custom_icon_data = malloc(CUSTOM_ICON_MAX_SIZE);
                if(!browser->item_callback(
                       item_path,
                       browser->item_context,
                       &item.custom_icon_data,
                       item.display_name)) {
                    free(item.custom_icon_data);
                    item.custom_icon_data = NULL;
                }
            }
        }

        if(furi_string_empty(item.display_name)) {
            path_extract_filename(
                item_path,
                item.display_name,
                (browser->hide_ext) && (item.type == BrowserItemTypeFile));
        }

        // We shouldn't update screen on each item if custom callback is not set
        // Otherwise it will cause screen flickering
        bool instant_update = (browser->item_callback != NULL);
        with_view_model(
            browser->view,
            FileBrowserModel * model,
            { items_array_push_back(model->items, item); },
            instant_update);

        furi_string_free(item.display_name);
        furi_string_free(item.path);
        if(item.custom_icon_data) {
            free(item.custom_icon_data);
        }
    } else {
        with_view_model(
            browser->view,
            FileBrowserModel * model,
            {
                model->list_loading = false;
                if(browser_is_list_load_required(model)) {
                    model->list_loading = true;
                    int32_t load_offset = CLAMP(
                        model->item_idx - ITEM_LIST_LEN_MAX / 2, (int32_t)model->item_cnt, 0);
                    file_browser_worker_load(browser->worker, load_offset, ITEM_LIST_LEN_MAX);
                }
            },
            true);
    }
}

static void browser_long_load_cb(void* context) {
    furi_check(context);
    FileBrowser* browser = (FileBrowser*)context;

    with_view_model(
        browser->view, FileBrowserModel * model, { model->folder_loading = true; }, true);
}

static void browser_draw_frame(Canvas* canvas, uint16_t idx, bool scrollbar) {
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(
        canvas, 0, Y_OFFSET + idx * FRAME_HEIGHT, (scrollbar ? 122 : 127), FRAME_HEIGHT);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_dot(canvas, 0, Y_OFFSET + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, 1, Y_OFFSET + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, 0, (Y_OFFSET + idx * FRAME_HEIGHT) + 1);

    canvas_draw_dot(canvas, 0, (Y_OFFSET + idx * FRAME_HEIGHT) + (FRAME_HEIGHT - 1));
    canvas_draw_dot(canvas, scrollbar ? 121 : 126, Y_OFFSET + idx * FRAME_HEIGHT);
    canvas_draw_dot(
        canvas, scrollbar ? 121 : 126, (Y_OFFSET + idx * FRAME_HEIGHT) + (FRAME_HEIGHT - 1));
}

static void browser_draw_loading(Canvas* canvas, FileBrowserModel* model) {
    UNUSED(model);

    uint8_t x = 128 / 2 - 24 / 2;
    uint8_t y = 64 / 2 - 24 / 2;

    canvas_draw_icon(canvas, x, y, &A_Loading_24);
}

static void browser_draw_list(Canvas* canvas, FileBrowserModel* model) {
    uint32_t array_size = items_array_size(model->items);
    bool show_scrollbar = model->item_cnt > LIST_ITEMS;

    FuriString* filename;
    filename = furi_string_alloc();

    for(uint32_t i = 0; i < MIN(model->item_cnt, LIST_ITEMS); i++) {
        int32_t idx = CLAMP((uint32_t)(i + model->list_offset), model->item_cnt, 0u);

        BrowserItemType item_type = BrowserItemTypeLoading;
        uint8_t* custom_icon_data = NULL;

        if(browser_is_item_in_array(model, idx)) {
            BrowserItem_t* item = items_array_get(
                model->items, CLAMP(idx - model->array_offset, (int32_t)(array_size - 1), 0));
            item_type = item->type;
            furi_string_set(filename, item->display_name);
            if(item_type == BrowserItemTypeFile) {
                custom_icon_data = item->custom_icon_data;
            }
        } else {
            furi_string_set(filename, "---");
        }

        if(item_type == BrowserItemTypeBack) {
            furi_string_set(filename, ". .");
        }

        size_t scroll_counter = model->scroll_counter;
        if(model->item_idx == idx) {
            browser_draw_frame(canvas, i, show_scrollbar);
            if(scroll_counter < SCROLL_DELAY) {
                scroll_counter = 0;
            } else {
                scroll_counter -= SCROLL_DELAY;
            }
        } else {
            canvas_set_color(canvas, ColorBlack);
            scroll_counter = 0;
        }

        if(custom_icon_data) { //-V547
            // Currently only 10*10 icons are supported
            canvas_draw_bitmap(
                canvas, 2, Y_OFFSET + 1 + i * FRAME_HEIGHT, 10, 10, custom_icon_data);
        } else if((item_type == BrowserItemTypeFile) && (model->file_icon)) {
            canvas_draw_icon(canvas, 2, Y_OFFSET + 1 + i * FRAME_HEIGHT, model->file_icon);
        } else if(BrowserItemIcons[item_type] != NULL) {
            canvas_draw_icon(
                canvas, 2, Y_OFFSET + 1 + i * FRAME_HEIGHT, BrowserItemIcons[item_type]);
        }
        elements_scrollable_text_line(
            canvas,
            15,
            Y_OFFSET + 9 + i * FRAME_HEIGHT,
            (show_scrollbar ? MAX_LEN_PX - 6 : MAX_LEN_PX),
            filename,
            scroll_counter,
            (model->item_idx != idx));
    }

    if(show_scrollbar) {
        elements_scrollbar_pos(
            canvas,
            126,
            Y_OFFSET,
            canvas_height(canvas) - Y_OFFSET,
            model->item_idx,
            model->item_cnt);
    }

    uint32_t folder_item_cnt = (model->is_root) ? (model->item_cnt) : (model->item_cnt - 1);
    if(folder_item_cnt == 0) {
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_str_aligned(
            canvas,
            canvas_width(canvas) / 2,
            canvas_height(canvas) / 2,
            AlignCenter,
            AlignCenter,
            "<Empty>");
    }

    furi_string_free(filename);
}

static void file_browser_view_draw_callback(Canvas* canvas, void* _model) {
    FileBrowserModel* model = _model;

    if(model->folder_loading) {
        browser_draw_loading(canvas, model);
    } else {
        browser_draw_list(canvas, model);
    }
}

static bool file_browser_view_input_callback(InputEvent* event, void* context) {
    FileBrowser* browser = context;
    furi_check(browser);
    bool consumed = false;
    bool is_loading = false;

    with_view_model(
        browser->view, FileBrowserModel * model, { is_loading = model->folder_loading; }, false);

    if(is_loading) {
        return false;
    } else if(event->key == InputKeyUp || event->key == InputKeyDown) {
        if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
            with_view_model(
                browser->view,
                FileBrowserModel * model,
                {
                    int32_t scroll_speed = 1;
                    if(model->button_held_for_ticks > 5) {
                        if(model->button_held_for_ticks % 2) {
                            scroll_speed = 0;
                        } else {
                            scroll_speed = model->button_held_for_ticks > 9 ? 5 : 3;
                        }
                    }

                    if(event->key == InputKeyUp) {
                        if(model->item_idx < scroll_speed) {
                            model->button_held_for_ticks = 0;
                            model->item_idx = model->item_cnt - 1;
                            browser_list_rollover(model);
                        } else {
                            model->item_idx =
                                ((model->item_idx - scroll_speed) + model->item_cnt) %
                                model->item_cnt;
                        }

                        if(browser_is_list_load_required(model)) {
                            model->list_loading = true;
                            int32_t load_offset = CLAMP(
                                model->item_idx - ITEM_LIST_LEN_MAX / 4 * 3,
                                (int32_t)model->item_cnt,
                                0);
                            file_browser_worker_load(
                                browser->worker, load_offset, ITEM_LIST_LEN_MAX);
                        }
                        model->scroll_counter = 0;

                        model->button_held_for_ticks += 1;
                    } else if(event->key == InputKeyDown) {
                        if(model->item_idx + scroll_speed >= (int32_t)model->item_cnt) {
                            model->button_held_for_ticks = 0;
                            model->item_idx = 0;
                            browser_list_rollover(model);
                        } else {
                            model->item_idx = (model->item_idx + scroll_speed) % model->item_cnt;
                        }

                        if(browser_is_list_load_required(model)) {
                            model->list_loading = true;
                            int32_t load_offset = CLAMP(
                                model->item_idx - ITEM_LIST_LEN_MAX / 4 * 1,
                                (int32_t)model->item_cnt,
                                0);
                            file_browser_worker_load(
                                browser->worker, load_offset, ITEM_LIST_LEN_MAX);
                        }
                        model->scroll_counter = 0;

                        model->button_held_for_ticks += 1;
                    }
                },
                true);
            browser_update_offset(browser);
            consumed = true;
        } else if(event->type == InputTypeRelease) {
            with_view_model(
                browser->view,
                FileBrowserModel * model,
                { model->button_held_for_ticks = 0; },
                true);
        }
    } else if(event->key == InputKeyOk) {
        if(event->type == InputTypeShort) {
            BrowserItem_t* selected_item = NULL;
            int32_t select_index = 0;
            with_view_model(
                browser->view,
                FileBrowserModel * model,
                {
                    if(browser_is_item_in_array(model, model->item_idx)) {
                        selected_item =
                            items_array_get(model->items, model->item_idx - model->array_offset);
                        select_index = model->item_idx;
                        if((!model->is_root) && (select_index > 0)) {
                            select_index -= 1;
                        }
                    }
                },
                false);

            if(selected_item) {
                if(selected_item->type == BrowserItemTypeBack) {
                    file_browser_worker_folder_exit(browser->worker);
                } else if(selected_item->type == BrowserItemTypeFolder) {
                    file_browser_worker_folder_enter(
                        browser->worker, selected_item->path, select_index);
                } else if(selected_item->type == BrowserItemTypeFile) {
                    furi_string_set(browser->result_path, selected_item->path);
                    if(browser->callback) {
                        browser->callback(browser->context);
                    }
                }
            }
            consumed = true;
        }
    } else if(event->key == InputKeyLeft) {
        if(event->type == InputTypeShort) {
            bool is_root = false;
            with_view_model(
                browser->view, FileBrowserModel * model, { is_root = model->is_root; }, false);
            if(!is_root) {
                file_browser_worker_folder_exit(browser->worker);
            }
            consumed = true;
        }
    } else if(event->key == InputKeyBack) {
        if(event->type == InputTypeShort) {
            bool is_root = false;
            with_view_model(
                browser->view, FileBrowserModel * model, { is_root = model->is_root; }, false);

            if(!is_root && !file_browser_worker_is_in_start_folder(browser->worker)) {
                consumed = true;
                file_browser_worker_folder_exit(browser->worker);
            }
        }
    }

    return consumed;
}
