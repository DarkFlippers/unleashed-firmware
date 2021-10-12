#include <furi.h>
#include "../archive_i.h"
#include "archive_browser_view.h"
#include "../helpers/archive_browser.h"

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
    ArchiveBrowserView* browser,
    ArchiveBrowserViewCallback callback,
    void* context) {
    furi_assert(browser);
    furi_assert(callback);
    browser->callback = callback;
    browser->context = context;
}

static void render_item_menu(Canvas* canvas, ArchiveBrowserViewModel* model) {
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
    } else if(model->tab_idx == ArchiveTabFavorites) {
        string_set_str(menu[1], "Unpin");
        string_set_str(menu[2], "Move");
    }

    for(size_t i = 0; i < MENU_ITEMS; i++) {
        canvas_draw_str(canvas, 82, 27 + i * 11, string_get_cstr(menu[i]));
        string_clear(menu[i]);
    }

    canvas_draw_icon(canvas, 74, 20 + model->menu_idx * 11, &I_ButtonRight_4x7);
}

static void archive_draw_frame(Canvas* canvas, uint16_t idx, bool scrollbar, bool moving) {
    uint8_t x_offset = moving ? MOVE_OFFSET : 0;

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(
        canvas,
        0 + x_offset,
        15 + idx * FRAME_HEIGHT,
        (scrollbar ? 122 : 127) - x_offset,
        FRAME_HEIGHT);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_dot(canvas, 0 + x_offset, 15 + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, 1 + x_offset, 15 + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, 0 + x_offset, (15 + idx * FRAME_HEIGHT) + 1);

    canvas_draw_dot(canvas, 0 + x_offset, (15 + idx * FRAME_HEIGHT) + 11);
    canvas_draw_dot(canvas, scrollbar ? 121 : 126, 15 + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, scrollbar ? 121 : 126, (15 + idx * FRAME_HEIGHT) + 11);
}

static void draw_list(Canvas* canvas, ArchiveBrowserViewModel* model) {
    furi_assert(model);

    size_t array_size = files_array_size(model->files);
    bool scrollbar = array_size > 4;

    for(size_t i = 0; i < MIN(array_size, MENU_ITEMS); ++i) {
        string_t str_buff;
        char cstr_buff[MAX_NAME_LEN];
        size_t idx = CLAMP(i + model->list_offset, array_size, 0);
        uint8_t x_offset = (model->move_fav && model->idx == idx) ? MOVE_OFFSET : 0;

        ArchiveFile_t* file = files_array_get(model->files, CLAMP(idx, array_size - 1, 0));

        strlcpy(cstr_buff, string_get_cstr(file->name), string_size(file->name) + 1);
        archive_trim_file_path(cstr_buff, is_known_app(file->type));
        string_init_set_str(str_buff, cstr_buff);
        elements_string_fit_width(
            canvas, str_buff, (scrollbar ? MAX_LEN_PX - 6 : MAX_LEN_PX) - x_offset);

        if(model->idx == idx) {
            archive_draw_frame(canvas, i, scrollbar, model->move_fav);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        canvas_draw_icon(
            canvas, 2 + x_offset, 16 + i * FRAME_HEIGHT, ArchiveItemIcons[file->type]);
        canvas_draw_str(canvas, 15 + x_offset, 24 + i * FRAME_HEIGHT, string_get_cstr(str_buff));
        string_clear(str_buff);
    }

    if(scrollbar) {
        elements_scrollbar_pos(canvas, 126, 15, 49, model->idx, array_size);
    }

    if(model->menu) {
        render_item_menu(canvas, model);
    }
}

static void archive_render_status_bar(Canvas* canvas, ArchiveBrowserViewModel* model) {
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

    if(model->move_fav) {
        canvas_draw_icon(canvas, 111, 4, &I_ButtonUp_7x4);
        canvas_draw_icon(canvas, 118, 4, &I_ButtonDown_7x4);
    } else {
        canvas_draw_icon(canvas, 112, 2, &I_ButtonLeft_4x7);
        canvas_draw_icon(canvas, 120, 2, &I_ButtonRight_4x7);
    }

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_dot(canvas, 50, 0);
    canvas_draw_dot(canvas, 127, 0);

    canvas_set_color(canvas, ColorBlack);
}

void archive_view_render(Canvas* canvas, void* model) {
    ArchiveBrowserViewModel* m = model;

    archive_render_status_bar(canvas, model);

    if(files_array_size(m->files)) {
        draw_list(canvas, m);
    } else {
        canvas_draw_str_aligned(
            canvas, GUI_DISPLAY_WIDTH / 2, 40, AlignCenter, AlignCenter, "Empty");
    }
}

View* archive_browser_get_view(ArchiveBrowserView* browser) {
    furi_assert(browser);
    return browser->view;
}

bool archive_view_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    ArchiveBrowserView* browser = context;

    bool in_menu;
    bool move_fav_mode;
    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            in_menu = model->menu;
            move_fav_mode = model->move_fav;
            return false;
        });

    if(in_menu) {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyUp || event->key == InputKeyDown) {
                with_view_model(
                    browser->view, (ArchiveBrowserViewModel * model) {
                        if(event->key == InputKeyUp) {
                            model->menu_idx = ((model->menu_idx - 1) + MENU_ITEMS) % MENU_ITEMS;
                        } else if(event->key == InputKeyDown) {
                            model->menu_idx = (model->menu_idx + 1) % MENU_ITEMS;
                        }
                        return true;
                    });
            }

            if(event->key == InputKeyOk) {
                uint8_t idx;
                with_view_model(
                    browser->view, (ArchiveBrowserViewModel * model) {
                        idx = model->menu_idx;
                        return false;
                    });
                browser->callback(file_menu_actions[idx], browser->context);
            } else if(event->key == InputKeyBack) {
                browser->callback(ArchiveBrowserEventFileMenuClose, browser->context);
            }
        }

    } else {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyLeft || event->key == InputKeyRight) {
                if(move_fav_mode) return false;
                archive_switch_tab(browser, event->key);
            } else if(event->key == InputKeyBack) {
                if(move_fav_mode) {
                    browser->callback(ArchiveBrowserEventExitFavMove, browser->context);
                } else {
                    browser->callback(ArchiveBrowserEventExit, browser->context);
                }
            }
        }

        if(event->key == InputKeyUp || event->key == InputKeyDown) {
            with_view_model(
                browser->view, (ArchiveBrowserViewModel * model) {
                    uint16_t num_elements = (uint16_t)files_array_size(model->files);
                    if((event->type == InputTypeShort || event->type == InputTypeRepeat)) {
                        if(event->key == InputKeyUp) {
                            model->idx = ((model->idx - 1) + num_elements) % num_elements;
                            if(move_fav_mode) {
                                browser->callback(ArchiveBrowserEventFavMoveUp, browser->context);
                            }
                        } else if(event->key == InputKeyDown) {
                            model->idx = (model->idx + 1) % num_elements;
                            if(move_fav_mode) {
                                browser->callback(
                                    ArchiveBrowserEventFavMoveDown, browser->context);
                            }
                        }
                    }

                    return true;
                });
            archive_update_offset(browser);
        }

        if(event->key == InputKeyOk) {
            ArchiveFile_t* selected = archive_get_current_file(browser);

            if(selected) {
                bool favorites = archive_get_tab(browser) == ArchiveTabFavorites;
                bool folder = selected->type == ArchiveFileTypeFolder;

                if(event->type == InputTypeShort) {
                    if(favorites) {
                        if(move_fav_mode) {
                            browser->callback(ArchiveBrowserEventSaveFavMove, browser->context);
                        } else {
                            browser->callback(ArchiveBrowserEventFileMenuRun, browser->context);
                        }
                    } else if(folder) {
                        browser->callback(ArchiveBrowserEventEnterDir, browser->context);
                    } else {
                        browser->callback(ArchiveBrowserEventFileMenuOpen, browser->context);
                    }
                } else if(event->type == InputTypeLong) {
                    if(move_fav_mode) {
                        browser->callback(ArchiveBrowserEventSaveFavMove, browser->context);
                    } else if(folder || favorites) {
                        browser->callback(ArchiveBrowserEventFileMenuOpen, browser->context);
                    }
                }
            }
        }
    }

    return true;
}

ArchiveBrowserView* browser_alloc() {
    ArchiveBrowserView* browser = furi_alloc(sizeof(ArchiveBrowserView));
    browser->view = view_alloc();
    view_allocate_model(browser->view, ViewModelTypeLocking, sizeof(ArchiveBrowserViewModel));
    view_set_context(browser->view, browser);
    view_set_draw_callback(browser->view, (ViewDrawCallback)archive_view_render);
    view_set_input_callback(browser->view, archive_view_input);

    string_init(browser->path);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            files_array_init(model->files);
            return true;
        });

    return browser;
}

void browser_free(ArchiveBrowserView* browser) {
    furi_assert(browser);

    with_view_model(
        browser->view, (ArchiveBrowserViewModel * model) {
            files_array_clear(model->files);
            return false;
        });

    string_clear(browser->path);

    view_free(browser->view);
    free(browser);
}
