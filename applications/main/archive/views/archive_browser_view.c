#include "assets_icons.h"
#include "toolbox/path.h"
#include <furi.h>
#include "archive_browser_view.h"
#include "../helpers/archive_browser.h"

#define SCROLL_INTERVAL (333)
#define SCROLL_DELAY    (2)

static const char* ArchiveTabNames[] = {
    [ArchiveTabFavorites] = "Favorites",
    [ArchiveTabIButton] = "iButton",
    [ArchiveTabNFC] = "NFC",
    [ArchiveTabSubGhz] = "Sub-GHz",
    [ArchiveTabLFRFID] = "RFID LF",
    [ArchiveTabInfrared] = "Infrared",
    [ArchiveTabBadUsb] = "Bad USB",
    [ArchiveTabU2f] = "U2F",
    [ArchiveTabApplications] = "Apps",
    [ArchiveTabBrowser] = "Browser",
};

static const Icon* ArchiveItemIcons[] = {
    [ArchiveFileTypeIButton] = &I_ibutt_10px,
    [ArchiveFileTypeNFC] = &I_Nfc_10px,
    [ArchiveFileTypeSubGhz] = &I_sub1_10px,
    [ArchiveFileTypeLFRFID] = &I_125_10px,
    [ArchiveFileTypeInfrared] = &I_ir_10px,
    [ArchiveFileTypeBadUsb] = &I_badusb_10px,
    [ArchiveFileTypeU2f] = &I_u2f_10px,
    [ArchiveFileTypeUpdateManifest] = &I_update_10px,
    [ArchiveFileTypeFolder] = &I_dir_10px,
    [ArchiveFileTypeUnknown] = &I_unknown_10px,
    [ArchiveFileTypeLoading] = &I_loading_10px,
    [ArchiveFileTypeApplication] = &I_unknown_10px,
    [ArchiveFileTypeJS] = &I_js_script_10px,
    [ArchiveFileTypeAppOrJs] = &I_unknown_10px,
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

    FuriString* menu[MENU_ITEMS];

    menu[0] = furi_string_alloc_set("Run in app");
    menu[1] = furi_string_alloc_set("Pin");
    menu[2] = furi_string_alloc_set("Rename");
    menu[3] = furi_string_alloc_set("Delete");

    ArchiveFile_t* selected = files_array_get(model->files, model->item_idx - model->array_offset);

    if((selected->fav) || (model->tab_idx == ArchiveTabFavorites)) {
        furi_string_set(menu[1], "Unpin");
    }

    if(!archive_is_known_app(selected->type)) {
        furi_string_set(menu[0], "---");
        furi_string_set(menu[1], "---");
    } else {
        if(model->tab_idx == ArchiveTabFavorites) {
            furi_string_set(menu[2], "Move");
            furi_string_set(menu[3], "---");
        } else if(selected->is_app) {
            furi_string_set(menu[2], "---");
        }
    }

    for(size_t i = 0; i < MENU_ITEMS; i++) {
        canvas_draw_str(canvas, 82, 27 + i * 11, furi_string_get_cstr(menu[i]));
        furi_string_free(menu[i]);
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

static void archive_draw_loading(Canvas* canvas, ArchiveBrowserViewModel* model) {
    furi_assert(model);

    uint8_t x = 128 / 2 - 24 / 2;
    uint8_t y = 64 / 2 - 24 / 2;

    canvas_draw_icon(canvas, x, y, &A_Loading_24);
}

static void draw_list(Canvas* canvas, ArchiveBrowserViewModel* model) {
    furi_assert(model);

    size_t array_size = files_array_size(model->files);
    bool scrollbar = model->item_cnt > 4;

    for(uint32_t i = 0; i < MIN(model->item_cnt, MENU_ITEMS); ++i) {
        FuriString* str_buf;
        str_buf = furi_string_alloc();
        int32_t idx = CLAMP((uint32_t)(i + model->list_offset), model->item_cnt, 0u);
        uint8_t x_offset = (model->move_fav && model->item_idx == idx) ? MOVE_OFFSET : 0;

        ArchiveFileTypeEnum file_type = ArchiveFileTypeLoading;
        uint8_t* custom_icon_data = NULL;

        if(archive_is_item_in_array(model, idx)) {
            ArchiveFile_t* file = files_array_get(
                model->files, CLAMP(idx - model->array_offset, (int32_t)(array_size - 1), 0));
            file_type = file->type;
            if(file_type == ArchiveFileTypeApplication) {
                if(file->custom_icon_data) {
                    custom_icon_data = file->custom_icon_data;
                    furi_string_set(str_buf, file->custom_name);
                } else {
                    file_type = ArchiveFileTypeUnknown;
                    path_extract_filename(file->path, str_buf, archive_is_known_app(file->type));
                }
            } else {
                path_extract_filename(file->path, str_buf, archive_is_known_app(file->type));
            }
        } else {
            furi_string_set(str_buf, "---");
        }

        size_t scroll_counter = model->scroll_counter;

        if(model->item_idx == idx) {
            archive_draw_frame(canvas, i, scrollbar, model->move_fav);
            if(scroll_counter < SCROLL_DELAY) {
                scroll_counter = 0;
            } else {
                scroll_counter -= SCROLL_DELAY;
            }
        } else {
            canvas_set_color(canvas, ColorBlack);
            scroll_counter = 0;
        }

        if(custom_icon_data) {
            canvas_draw_bitmap(
                canvas, 2 + x_offset, 16 + i * FRAME_HEIGHT, 11, 10, custom_icon_data);
        } else {
            canvas_draw_icon(
                canvas, 2 + x_offset, 16 + i * FRAME_HEIGHT, ArchiveItemIcons[file_type]);
        }

        elements_scrollable_text_line(
            canvas,
            15 + x_offset,
            24 + i * FRAME_HEIGHT,
            ((scrollbar ? MAX_LEN_PX - 6 : MAX_LEN_PX) - x_offset),
            str_buf,
            scroll_counter,
            (model->item_idx != idx));

        furi_string_free(str_buf);
    }

    if(scrollbar) {
        elements_scrollbar_pos(canvas, 126, 15, 49, model->item_idx, model->item_cnt);
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
    canvas_draw_rframe(canvas, 0, 0, 51, 13, 1); // frame
    canvas_draw_line(canvas, 49, 1, 49, 11); // shadow right
    canvas_draw_line(canvas, 1, 11, 49, 11); // shadow bottom
    canvas_draw_str_aligned(canvas, 25, 9, AlignCenter, AlignBottom, tab_name);

    canvas_draw_rframe(canvas, 107, 0, 21, 13, 1);
    canvas_draw_line(canvas, 126, 1, 126, 11);
    canvas_draw_line(canvas, 108, 11, 126, 11);

    if(model->move_fav) {
        canvas_draw_icon(canvas, 110, 4, &I_ButtonUp_7x4);
        canvas_draw_icon(canvas, 117, 4, &I_ButtonDown_7x4);
    } else {
        canvas_draw_icon(canvas, 111, 2, &I_ButtonLeft_4x7);
        canvas_draw_icon(canvas, 119, 2, &I_ButtonRight_4x7);
    }

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_dot(canvas, 50, 0);
    canvas_draw_dot(canvas, 127, 0);

    canvas_set_color(canvas, ColorBlack);
}

static void archive_view_render(Canvas* canvas, void* mdl) {
    ArchiveBrowserViewModel* model = mdl;

    archive_render_status_bar(canvas, mdl);

    if(model->folder_loading) {
        archive_draw_loading(canvas, model);
    } else if(model->item_cnt > 0) {
        draw_list(canvas, model);
    } else {
        canvas_draw_str_aligned(
            canvas, GUI_DISPLAY_WIDTH / 2, 40, AlignCenter, AlignCenter, "Empty");
    }
}

View* archive_browser_get_view(ArchiveBrowserView* browser) {
    furi_assert(browser);
    return browser->view;
}

static void file_list_rollover(ArchiveBrowserViewModel* model) {
    if(!model->list_loading && files_array_size(model->files) < model->item_cnt) {
        files_array_reset(model->files);
    }
}

static bool archive_view_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    ArchiveBrowserView* browser = context;

    bool in_menu;
    bool move_fav_mode;
    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        {
            in_menu = model->menu;
            move_fav_mode = model->move_fav;
        },
        false);

    if(in_menu) {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyUp || event->key == InputKeyDown) {
                with_view_model(
                    browser->view,
                    ArchiveBrowserViewModel * model,
                    {
                        if(event->key == InputKeyUp) {
                            model->menu_idx = ((model->menu_idx - 1) + MENU_ITEMS) % MENU_ITEMS;
                        } else if(event->key == InputKeyDown) {
                            model->menu_idx = (model->menu_idx + 1) % MENU_ITEMS;
                        }
                    },
                    true);
            }

            if(event->key == InputKeyOk) {
                uint8_t idx;
                with_view_model(
                    browser->view,
                    ArchiveBrowserViewModel * model,
                    { idx = model->menu_idx; },
                    false);
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

        if((event->key == InputKeyUp || event->key == InputKeyDown) &&
           (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
            with_view_model(
                browser->view,
                ArchiveBrowserViewModel * model,
                {
                    int32_t scroll_speed = 1;
                    if(model->button_held_for_ticks > 5) {
                        if(model->button_held_for_ticks % 2) {
                            scroll_speed = 0;
                        } else {
                            scroll_speed = model->button_held_for_ticks > 9 ? 4 : 2;
                        }
                    }

                    if(event->key == InputKeyUp) {
                        if(model->item_idx < scroll_speed) {
                            model->button_held_for_ticks = 0;
                            model->item_idx = model->item_cnt - 1;
                            file_list_rollover(model);
                        } else {
                            model->item_idx =
                                ((model->item_idx - scroll_speed) + model->item_cnt) %
                                model->item_cnt;
                        }
                        if(archive_is_file_list_load_required(model)) {
                            model->list_loading = true;
                            browser->callback(ArchiveBrowserEventLoadPrevItems, browser->context);
                        }
                        if(move_fav_mode) {
                            browser->callback(ArchiveBrowserEventFavMoveUp, browser->context);
                        }
                        model->scroll_counter = 0;
                        model->button_held_for_ticks += 1;
                    } else if(event->key == InputKeyDown) {
                        int32_t count = model->item_cnt;
                        if(model->item_idx + scroll_speed >= count) {
                            model->button_held_for_ticks = 0;
                            model->item_idx = 0;
                            file_list_rollover(model);
                        } else {
                            model->item_idx = (model->item_idx + scroll_speed) % model->item_cnt;
                        }
                        if(archive_is_file_list_load_required(model)) {
                            model->list_loading = true;
                            browser->callback(ArchiveBrowserEventLoadNextItems, browser->context);
                        }
                        if(move_fav_mode) {
                            browser->callback(ArchiveBrowserEventFavMoveDown, browser->context);
                        }
                        model->scroll_counter = 0;
                        model->button_held_for_ticks += 1;
                    }
                },
                true);
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

    if(event->type == InputTypeRelease) {
        with_view_model(
            browser->view,
            ArchiveBrowserViewModel * model,
            { model->button_held_for_ticks = 0; },
            true);
    }

    return true;
}

static void browser_scroll_timer(void* context) {
    furi_assert(context);
    ArchiveBrowserView* browser = context;
    with_view_model(
        browser->view, ArchiveBrowserViewModel * model, { model->scroll_counter++; }, true);
}

static void browser_view_enter(void* context) {
    furi_assert(context);
    ArchiveBrowserView* browser = context;
    with_view_model(
        browser->view, ArchiveBrowserViewModel * model, { model->scroll_counter = 0; }, true);
    furi_timer_start(browser->scroll_timer, SCROLL_INTERVAL);
}

static void browser_view_exit(void* context) {
    furi_assert(context);
    ArchiveBrowserView* browser = context;
    furi_timer_stop(browser->scroll_timer);
}

ArchiveBrowserView* browser_alloc(void) {
    ArchiveBrowserView* browser = malloc(sizeof(ArchiveBrowserView));
    browser->view = view_alloc();
    view_allocate_model(browser->view, ViewModelTypeLocking, sizeof(ArchiveBrowserViewModel));
    view_set_context(browser->view, browser);
    view_set_draw_callback(browser->view, archive_view_render);
    view_set_input_callback(browser->view, archive_view_input);
    view_set_enter_callback(browser->view, browser_view_enter);
    view_set_exit_callback(browser->view, browser_view_exit);

    browser->scroll_timer = furi_timer_alloc(browser_scroll_timer, FuriTimerTypePeriodic, browser);

    browser->path = furi_string_alloc_set(archive_get_default_path(TAB_DEFAULT));

    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        {
            files_array_init(model->files);
            model->tab_idx = TAB_DEFAULT;
        },
        true);

    return browser;
}

void browser_free(ArchiveBrowserView* browser) {
    furi_assert(browser);

    furi_timer_free(browser->scroll_timer);

    if(browser->worker_running) {
        file_browser_worker_free(browser->worker);
    }

    with_view_model(
        browser->view,
        ArchiveBrowserViewModel * model,
        { files_array_clear(model->files); },
        false);

    furi_string_free(browser->path);

    view_free(browser->view);
    free(browser);
}
