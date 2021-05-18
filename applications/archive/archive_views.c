#include "archive_views.h"

static const char* ArchiveTabNames[] =
    {"Favorites", "iButton", "NFC", "SubOne", "Rfid", "Infared", "Browser"};

static const IconName ArchiveItemIcons[] = {
    [ArchiveFileTypeIButton] = I_ibutt_10px,
    [ArchiveFileTypeNFC] = I_Nfc_10px,
    [ArchiveFileTypeSubOne] = I_sub1_10px,
    [ArchiveFileTypeLFRFID] = I_125_10px,
    [ArchiveFileTypeIrda] = I_ir_10px,
    [ArchiveFileTypeFolder] = I_dir_10px,
    [ArchiveFileTypeUnknown] = I_unknown_10px,
};

static inline bool is_known_app(ArchiveFileTypeEnum type) {
    return (type != ArchiveFileTypeFolder && type != ArchiveFileTypeUnknown);
}

static void render_item_menu(Canvas* canvas, ArchiveViewModel* model) {
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 61, 17, 62, 46);
    canvas_set_color(canvas, ColorBlack);
    elements_slightly_rounded_frame(canvas, 60, 16, 64, 48);

    string_t menu[MENU_ITEMS];

    string_init_set_str(menu[0], "Open in app");
    string_init_set_str(menu[1], "Pin");
    string_init_set_str(menu[2], "Rename");
    string_init_set_str(menu[3], "Delete");

    ArchiveFile_t* selected = files_array_get(model->files, model->idx);

    if(!is_known_app(selected->type)) {
        string_set_str(menu[0], "---");
        string_set_str(menu[1], "---");
    } else if(model->tab_idx == 0) {
        string_set_str(menu[1], "Move");
    }

    for(size_t i = 0; i < MENU_ITEMS; i++) {
        canvas_draw_str(canvas, 72, 27 + i * 11, string_get_cstr(menu[i]));
        string_clear(menu[i]);
    }

    canvas_draw_icon_name(canvas, 64, 20 + model->menu_idx * 11, I_ButtonRight_4x7);
}

static void trim_file_ext(string_t name) {
    size_t str_len = strlen(string_get_cstr(name));
    char* buff_ptr = stringi_get_cstr(name);
    char* end = buff_ptr + str_len;
    while(end > buff_ptr && *end != '.' && *end != '\\' && *end != '/') {
        --end;
    }
    if((end > buff_ptr && *end == '.') && (*(end - 1) != '\\' && *(end - 1) != '/')) {
        *end = '\0';
    }
}

static void format_filename_buffer(Canvas* canvas, string_t name, ArchiveFileTypeEnum type) {
    furi_assert(name);

    size_t s_len = strlen(string_get_cstr(name));
    uint16_t len_px = canvas_string_width(canvas, string_get_cstr(name));

    if(is_known_app(type)) trim_file_ext(name);

    if(len_px > MAX_LEN_PX) {
        string_mid(name, 0, s_len - (size_t)((len_px - MAX_LEN_PX) / ((len_px / s_len) + 2) + 2));
        string_cat(name, "...");
    }
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

static void draw_list(Canvas* canvas, ArchiveViewModel* model) {
    furi_assert(model);

    size_t array_size = files_array_size(model->files);
    bool scrollbar = array_size > 4;

    string_t str_buff;
    string_init(str_buff);

    for(size_t i = 0; i < MIN(MENU_ITEMS, array_size); ++i) {
        size_t idx = CLAMP(i + model->list_offset, array_size, 0);
        ArchiveFile_t* file = files_array_get(model->files, CLAMP(idx, array_size - 1, 0));

        string_set(str_buff, file->name);
        format_filename_buffer(canvas, str_buff, file->type);

        if(model->idx == idx) {
            archive_draw_frame(canvas, i, scrollbar);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        canvas_draw_icon_name(canvas, 2, 16 + i * FRAME_HEIGHT, ArchiveItemIcons[file->type]);
        canvas_draw_str(canvas, 15, 24 + i * FRAME_HEIGHT, stringi_get_cstr(str_buff));
        string_clean(str_buff);
    }

    if(scrollbar) {
        elements_scrollbar_pos(canvas, 126, 16, 48, model->idx, array_size);
    }

    if(model->menu) {
        render_item_menu(canvas, model);
    }

    string_clear(str_buff);
}

static void archive_render_status_bar(Canvas* canvas, ArchiveViewModel* model) {
    furi_assert(model);

    const char* tab_name = ArchiveTabNames[model->tab_idx];

    canvas_draw_icon_name(canvas, 0, 0, I_Background_128x11);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 0, 0, 50, 13);
    canvas_draw_box(canvas, 100, 0, 28, 13);

    canvas_set_color(canvas, ColorBlack);
    elements_frame(canvas, 0, 0, 50, 13);
    canvas_draw_str_aligned(canvas, 25, 10, AlignCenter, AlignBottom, tab_name);

    elements_frame(canvas, 100, 0, 24, 13);

    if(model->tab_idx > 0) {
        canvas_draw_icon_name(canvas, 106, 3, I_ButtonLeft_4x7);
    }
    if(model->tab_idx < SIZEOF_ARRAY(ArchiveTabNames) - 1) {
        canvas_draw_icon_name(canvas, 114, 3, I_ButtonRight_4x7);
    }
}

void archive_view_render(Canvas* canvas, void* model) {
    ArchiveViewModel* m = model;

    archive_render_status_bar(canvas, model);

    if(files_array_size(m->files) > 0) {
        draw_list(canvas, m);
    } else {
        canvas_draw_str_aligned(
            canvas, GUI_DISPLAY_WIDTH / 2, 40, AlignCenter, AlignCenter, "Empty");
    }
}