#include "file_browser.h"
#include "assets_icons.h"
#include "cmsis_os2.h"
#include "file_browser_worker.h"
#include "furi/check.h"
#include "furi/common_defines.h"
#include "furi/log.h"
#include "furi_hal_resources.h"
#include "m-string.h"
#include <m-array.h>
#include <gui/elements.h>
#include <furi.h>
#include "toolbox/path.h"

#define LIST_ITEMS 5u
#define MAX_LEN_PX 110
#define FRAME_HEIGHT 12
#define Y_OFFSET 3

#define ITEM_LIST_LEN_MAX 100

typedef enum {
    BrowserItemTypeLoading,
    BrowserItemTypeBack,
    BrowserItemTypeFolder,
    BrowserItemTypeFile,
} BrowserItemType;

typedef struct {
    string_t path;
    BrowserItemType type;
} BrowserItem_t;

static void BrowserItem_t_init(BrowserItem_t* obj) {
    obj->type = BrowserItemTypeLoading;
    string_init(obj->path);
}

static void BrowserItem_t_init_set(BrowserItem_t* obj, const BrowserItem_t* src) {
    obj->type = src->type;
    string_init_set(obj->path, src->path);
}

static void BrowserItem_t_set(BrowserItem_t* obj, const BrowserItem_t* src) {
    obj->type = src->type;
    string_set(obj->path, src->path);
}

static void BrowserItem_t_clear(BrowserItem_t* obj) {
    string_clear(obj->path);
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
    bool skip_assets;

    FileBrowserCallback callback;
    void* context;

    string_ptr result_path;
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
static void browser_list_item_cb(void* context, string_t item_path, bool is_folder, bool is_last);
static void browser_long_load_cb(void* context);

FileBrowser* file_browser_alloc(string_ptr result_path) {
    furi_assert(result_path);
    FileBrowser* browser = malloc(sizeof(FileBrowser));
    browser->view = view_alloc();
    view_allocate_model(browser->view, ViewModelTypeLocking, sizeof(FileBrowserModel));
    view_set_context(browser->view, browser);
    view_set_draw_callback(browser->view, file_browser_view_draw_callback);
    view_set_input_callback(browser->view, file_browser_view_input_callback);

    browser->result_path = result_path;

    with_view_model(
        browser->view, (FileBrowserModel * model) {
            items_array_init(model->items);
            return false;
        });

    return browser;
}

void file_browser_free(FileBrowser* browser) {
    furi_assert(browser);

    with_view_model(
        browser->view, (FileBrowserModel * model) {
            items_array_clear(model->items);
            return false;
        });

    view_free(browser->view);
    free(browser);
}

View* file_browser_get_view(FileBrowser* browser) {
    furi_assert(browser);
    return browser->view;
}

void file_browser_configure(
    FileBrowser* browser,
    const char* extension,
    bool skip_assets,
    const Icon* file_icon,
    bool hide_ext) {
    furi_assert(browser);

    browser->ext_filter = extension;
    browser->skip_assets = skip_assets;

    with_view_model(
        browser->view, (FileBrowserModel * model) {
            model->file_icon = file_icon;
            model->hide_ext = hide_ext;
            return false;
        });
}

void file_browser_start(FileBrowser* browser, string_t path) {
    furi_assert(browser);
    browser->worker = file_browser_worker_alloc(path, browser->ext_filter, browser->skip_assets);
    file_browser_worker_set_callback_context(browser->worker, browser);
    file_browser_worker_set_folder_callback(browser->worker, browser_folder_open_cb);
    file_browser_worker_set_list_callback(browser->worker, browser_list_load_cb);
    file_browser_worker_set_item_callback(browser->worker, browser_list_item_cb);
    file_browser_worker_set_long_load_callback(browser->worker, browser_long_load_cb);
}

void file_browser_stop(FileBrowser* browser) {
    furi_assert(browser);
    file_browser_worker_free(browser->worker);
    with_view_model(
        browser->view, (FileBrowserModel * model) {
            items_array_reset(model->items);
            model->item_cnt = 0;
            model->item_idx = 0;
            model->array_offset = 0;
            model->list_offset = 0;
            return false;
        });
}

void file_browser_set_callback(FileBrowser* browser, FileBrowserCallback callback, void* context) {
    browser->context = context;
    browser->callback = callback;
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
    uint32_t item_cnt = (model->is_root) ? model->item_cnt : model->item_cnt - 1;

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

static void browser_update_offset(FileBrowser* browser) {
    furi_assert(browser);

    with_view_model(
        browser->view, (FileBrowserModel * model) {
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

            return false;
        });
}

static void
    browser_folder_open_cb(void* context, uint32_t item_cnt, int32_t file_idx, bool is_root) {
    furi_assert(context);
    FileBrowser* browser = (FileBrowser*)context;

    int32_t load_offset = 0;

    with_view_model(
        browser->view, (FileBrowserModel * model) {
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
            return true;
        });
    browser_update_offset(browser);

    file_browser_worker_load(browser->worker, load_offset, ITEM_LIST_LEN_MAX);
}

static void browser_list_load_cb(void* context, uint32_t list_load_offset) {
    furi_assert(context);
    FileBrowser* browser = (FileBrowser*)context;

    BrowserItem_t back_item;
    BrowserItem_t_init(&back_item);
    back_item.type = BrowserItemTypeBack;

    with_view_model(
        browser->view, (FileBrowserModel * model) {
            items_array_reset(model->items);
            model->array_offset = list_load_offset;
            if(!model->is_root) {
                if(list_load_offset == 0) {
                    items_array_push_back(model->items, back_item);
                } else {
                    model->array_offset += 1;
                }
            }
            return false;
        });

    BrowserItem_t_clear(&back_item);
}

static void browser_list_item_cb(void* context, string_t item_path, bool is_folder, bool is_last) {
    furi_assert(context);
    FileBrowser* browser = (FileBrowser*)context;

    BrowserItem_t item;

    if(!is_last) {
        BrowserItem_t_init(&item);
        string_set(item.path, item_path);
        item.type = (is_folder) ? BrowserItemTypeFolder : BrowserItemTypeFile;

        with_view_model(
            browser->view, (FileBrowserModel * model) {
                items_array_push_back(model->items, item);
                return false;
            });
        BrowserItem_t_clear(&item);
    } else {
        with_view_model(
            browser->view, (FileBrowserModel * model) {
                model->list_loading = false;
                return true;
            });
    }
}

static void browser_long_load_cb(void* context) {
    furi_assert(context);
    FileBrowser* browser = (FileBrowser*)context;

    with_view_model(
        browser->view, (FileBrowserModel * model) {
            model->folder_loading = true;
            return true;
        });
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
    uint8_t width = 49;
    uint8_t height = 47;
    uint8_t x = 128 / 2 - width / 2;
    uint8_t y = 64 / 2 - height / 2;

    UNUSED(model);

    elements_bold_rounded_frame(canvas, x, y, width, height);

    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text(canvas, x + 7, y + 13, "Loading...");

    canvas_draw_icon(canvas, x + 13, y + 19, &A_Loading_24);
}

static void browser_draw_list(Canvas* canvas, FileBrowserModel* model) {
    uint32_t array_size = items_array_size(model->items);
    bool show_scrollbar = model->item_cnt > LIST_ITEMS;

    string_t filename;
    string_init(filename);

    for(uint32_t i = 0; i < MIN(model->item_cnt, LIST_ITEMS); i++) {
        int32_t idx = CLAMP((uint32_t)(i + model->list_offset), model->item_cnt, 0u);

        BrowserItemType item_type = BrowserItemTypeLoading;

        if(browser_is_item_in_array(model, idx)) {
            BrowserItem_t* item = items_array_get(
                model->items, CLAMP(idx - model->array_offset, (int32_t)(array_size - 1), 0));
            item_type = item->type;
            path_extract_filename(
                item->path, filename, (model->hide_ext) && (item_type == BrowserItemTypeFile));
        } else {
            string_set_str(filename, "---");
        }

        if(item_type == BrowserItemTypeBack) {
            string_set_str(filename, ". .");
        }

        elements_string_fit_width(
            canvas, filename, (show_scrollbar ? MAX_LEN_PX - 6 : MAX_LEN_PX));

        if(model->item_idx == idx) {
            browser_draw_frame(canvas, i, show_scrollbar);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        if((item_type == BrowserItemTypeFile) && (model->file_icon)) {
            canvas_draw_icon(canvas, 2, Y_OFFSET + 1 + i * FRAME_HEIGHT, model->file_icon);
        } else if(BrowserItemIcons[item_type] != NULL) {
            canvas_draw_icon(
                canvas, 2, Y_OFFSET + 1 + i * FRAME_HEIGHT, BrowserItemIcons[item_type]);
        }
        canvas_draw_str(canvas, 15, Y_OFFSET + 9 + i * FRAME_HEIGHT, string_get_cstr(filename));
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

    string_clear(filename);
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
    furi_assert(browser);
    bool consumed = false;
    bool is_loading = false;

    with_view_model(
        browser->view, (FileBrowserModel * model) {
            is_loading = model->folder_loading;
            return false;
        });

    if(is_loading) {
        return false;
    } else if(event->key == InputKeyUp || event->key == InputKeyDown) {
        if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
            with_view_model(
                browser->view, (FileBrowserModel * model) {
                    if(event->key == InputKeyUp) {
                        model->item_idx =
                            ((model->item_idx - 1) + model->item_cnt) % model->item_cnt;
                        if(browser_is_list_load_required(model)) {
                            model->list_loading = true;
                            int32_t load_offset = CLAMP(
                                model->item_idx - ITEM_LIST_LEN_MAX / 4 * 3,
                                (int32_t)model->item_cnt - ITEM_LIST_LEN_MAX,
                                0);
                            file_browser_worker_load(
                                browser->worker, load_offset, ITEM_LIST_LEN_MAX);
                        }
                    } else if(event->key == InputKeyDown) {
                        model->item_idx = (model->item_idx + 1) % model->item_cnt;
                        if(browser_is_list_load_required(model)) {
                            model->list_loading = true;
                            int32_t load_offset = CLAMP(
                                model->item_idx - ITEM_LIST_LEN_MAX / 4 * 1,
                                (int32_t)model->item_cnt - ITEM_LIST_LEN_MAX,
                                0);
                            file_browser_worker_load(
                                browser->worker, load_offset, ITEM_LIST_LEN_MAX);
                        }
                    }
                    return true;
                });
            browser_update_offset(browser);
            consumed = true;
        }
    } else if(event->key == InputKeyOk) {
        if(event->type == InputTypeShort) {
            BrowserItem_t* selected_item = NULL;
            int32_t select_index = 0;
            with_view_model(
                browser->view, (FileBrowserModel * model) {
                    if(browser_is_item_in_array(model, model->item_idx)) {
                        selected_item =
                            items_array_get(model->items, model->item_idx - model->array_offset);
                        select_index = model->item_idx;
                        if((!model->is_root) && (select_index > 0)) {
                            select_index -= 1;
                        }
                    }
                    return false;
                });

            if(selected_item) {
                if(selected_item->type == BrowserItemTypeBack) {
                    file_browser_worker_folder_exit(browser->worker);
                } else if(selected_item->type == BrowserItemTypeFolder) {
                    file_browser_worker_folder_enter(
                        browser->worker, selected_item->path, select_index);
                } else if(selected_item->type == BrowserItemTypeFile) {
                    string_set(browser->result_path, selected_item->path);
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
                browser->view, (FileBrowserModel * model) {
                    is_root = model->is_root;
                    return false;
                });
            if(!is_root) {
                file_browser_worker_folder_exit(browser->worker);
            }
            consumed = true;
        }
    }

    return consumed;
}
