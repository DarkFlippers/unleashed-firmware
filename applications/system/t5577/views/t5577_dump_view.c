#include "t5577_dump_view.h"

#include <furi.h>
#include <gui/canvas.h>
#include <lfrfid/tools/t5577.h>

#define T5577_DUMP_VIEW_TABS 3

// Eight blocks have to fit under a title on a 64 pixel screen, which fixes the grid: a row every
// seven pixels from just below the title, and nothing drawn on it with a descender.
#define T5577_DUMP_VIEW_TITLE_Y 7
#define T5577_DUMP_VIEW_ROW_Y   14
#define T5577_DUMP_VIEW_PITCH   7
#define T5577_DUMP_VIEW_COL_HEX 18
#define T5577_DUMP_VIEW_COL_TAG 74
#define T5577_DUMP_VIEW_COL_VAL 54

typedef struct {
    T5577Dump dump;
    uint8_t tab;
} T5577DumpViewModel;

struct T5577DumpView {
    View* view;
};

static const char* t5577_dump_view_modulation(uint32_t block0) {
    // Compared as a field, not bit by bit: diphase is manchester and biphase together, so a
    // bitwise test for either of those admits it as well.
    switch(block0 & LFRFID_T5577_MODULATION_MASK) {
    case LFRFID_T5577_MODULATION_DIRECT:
        return "Direct";
    case LFRFID_T5577_MODULATION_PSK1:
        return "PSK1";
    case LFRFID_T5577_MODULATION_PSK2:
        return "PSK2";
    case LFRFID_T5577_MODULATION_PSK3:
        return "PSK3";
    case LFRFID_T5577_MODULATION_FSK1:
        return "FSK1";
    case LFRFID_T5577_MODULATION_FSK2:
        return "FSK2";
    case LFRFID_T5577_MODULATION_FSK1a:
        return "FSK1a";
    case LFRFID_T5577_MODULATION_FSK2a:
        return "FSK2a";
    case LFRFID_T5577_MODULATION_MANCHESTER:
        return "Manchester";
    case LFRFID_T5577_MODULATION_BIPHASE:
        return "Biphase";
    case LFRFID_T5577_MODULATION_DIPHASE:
        return "Diphase";
    default:
        return "reserved";
    }
}

static const char* t5577_dump_view_bitrate(uint32_t block0) {
    switch(block0 & LFRFID_T5577_BITRATE_MASK) {
    case LFRFID_T5577_BITRATE_RF_8:
        return "RF/8";
    case LFRFID_T5577_BITRATE_RF_16:
        return "RF/16";
    case LFRFID_T5577_BITRATE_RF_32:
        return "RF/32";
    case LFRFID_T5577_BITRATE_RF_40:
        return "RF/40";
    case LFRFID_T5577_BITRATE_RF_50:
        return "RF/50";
    case LFRFID_T5577_BITRATE_RF_64:
        return "RF/64";
    case LFRFID_T5577_BITRATE_RF_100:
        return "RF/100";
    default:
        return "RF/128";
    }
}

static const char* t5577_dump_view_pskcf(uint32_t block0) {
    switch(block0 & (LFRFID_T5577_PSKCF_RF_4 | LFRFID_T5577_PSKCF_RF_8)) {
    case LFRFID_T5577_PSKCF_RF_2:
        return "RF/2";
    case LFRFID_T5577_PSKCF_RF_4:
        return "RF/4";
    case LFRFID_T5577_PSKCF_RF_8:
        return "RF/8";
    default:
        return "reserved";
    }
}

static bool t5577_dump_view_is_psk(uint32_t block0) {
    const uint32_t modulation = block0 & LFRFID_T5577_MODULATION_MASK;

    return modulation == LFRFID_T5577_MODULATION_PSK1 ||
           modulation == LFRFID_T5577_MODULATION_PSK2 ||
           modulation == LFRFID_T5577_MODULATION_PSK3;
}

// What a block is for, where the chip gives it a job. In capitals because the grid leaves no room
// under a row for the tail of a lower-case letter.
static const char* t5577_dump_view_tag(uint8_t page, uint8_t block, bool read) {
    if(!read) return "NOT READ";
    if(page == 0 && block == 0) return "CONFIG";
    if(page == 0 && block == 7) return "PWD";
    if(page == 1 && (block == 1 || block == 2)) return "TRACE";
    return "";
}

static void t5577_dump_view_draw_title(Canvas* canvas, const char* title, uint8_t tab) {
    char counter[8];
    snprintf(counter, sizeof(counter), "%u/%u", tab + 1, T5577_DUMP_VIEW_TABS);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, T5577_DUMP_VIEW_TITLE_Y, title);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas, 126, T5577_DUMP_VIEW_TITLE_Y, AlignRight, AlignBottom, counter);
}

static void t5577_dump_view_draw_page(Canvas* canvas, const T5577Dump* dump, uint8_t page) {
    char text[16];

    canvas_set_font(canvas, FontSecondary);

    for(uint8_t block = 0; block < t5577_page_blocks(page); block++) {
        const int32_t y = T5577_DUMP_VIEW_ROW_Y + block * T5577_DUMP_VIEW_PITCH;
        const bool read = t5577_dump_was_read(dump, page, block);

        snprintf(text, sizeof(text), "%u", block);
        canvas_draw_str(canvas, 2, y, text);

        // Dashes, not zeros: a block that was never read is not a block holding zero, and the
        // difference is what decides whether a write would touch it.
        if(read) {
            snprintf(text, sizeof(text), "%08lX", dump->block[page][block]);
        } else {
            snprintf(text, sizeof(text), "--------");
        }
        canvas_draw_str(canvas, T5577_DUMP_VIEW_COL_HEX, y, text);

        canvas_draw_str(
            canvas, T5577_DUMP_VIEW_COL_TAG, y, t5577_dump_view_tag(page, block, read));
    }
}

static void
    t5577_dump_view_draw_row(Canvas* canvas, uint8_t row, const char* label, const char* value) {
    const int32_t y = T5577_DUMP_VIEW_ROW_Y + row * T5577_DUMP_VIEW_PITCH;

    canvas_draw_str(canvas, 2, y, label);
    canvas_draw_str(canvas, T5577_DUMP_VIEW_COL_VAL, y, value);
}

// Block 0 in the terms the chip reads it in. It is the one block whose hex says nothing on its
// own, and the one that decides whether the rest of the dump can be read back at all.
static void t5577_dump_view_draw_config(Canvas* canvas, const T5577Dump* dump) {
    canvas_set_font(canvas, FontSecondary);

    if(!t5577_dump_was_read(dump, 0, 0)) {
        canvas_draw_str(canvas, 2, T5577_DUMP_VIEW_ROW_Y, "Block 0 was not read,");
        canvas_draw_str(
            canvas, 2, T5577_DUMP_VIEW_ROW_Y + T5577_DUMP_VIEW_PITCH, "so there is nothing to say");
        return;
    }

    const uint32_t block0 = dump->block[0][0];
    char text[16];
    uint8_t row = 0;

    snprintf(text, sizeof(text), "%08lX", block0);
    t5577_dump_view_draw_row(canvas, row++, "Block 0", text);
    t5577_dump_view_draw_row(canvas, row++, "Mod", t5577_dump_view_modulation(block0));
    t5577_dump_view_draw_row(canvas, row++, "Bit rate", t5577_dump_view_bitrate(block0));

    if(t5577_dump_view_is_psk(block0)) {
        t5577_dump_view_draw_row(canvas, row++, "PSK rate", t5577_dump_view_pskcf(block0));
    }

    // How far the chip answers: blocks above this one are there but stay silent.
    snprintf(text, sizeof(text), "%lu", (block0 >> LFRFID_T5577_MAXBLOCK_SHIFT) & 0x7);
    t5577_dump_view_draw_row(canvas, row++, "Max block", text);

    FuriString* flags = furi_string_alloc();
    if(block0 & LFRFID_T5577_PWD) furi_string_cat(flags, "PWD ");
    if(block0 & LFRFID_T5577_AOR) furi_string_cat(flags, "AOR ");
    if(block0 & LFRFID_T5577_ST_TERMINATOR) furi_string_cat(flags, "ST ");
    if(block0 & LFRFID_T5577_X_MODE) furi_string_cat(flags, "XM ");
    if(block0 & LFRFID_T5577_POR_DELAY) furi_string_cat(flags, "POR");
    furi_string_trim(flags);
    if(furi_string_empty(flags)) furi_string_set(flags, "none");
    t5577_dump_view_draw_row(canvas, row++, "Flags", furi_string_get_cstr(flags));
    furi_string_free(flags);

    // The password the chip holds is block 7 of page 0, whether or not it is being asked for. A
    // password the dump was read with is a different thing, and gets its own line.
    if(t5577_dump_was_read(dump, 0, 7)) {
        snprintf(text, sizeof(text), "%08lX", dump->block[0][7]);
        t5577_dump_view_draw_row(canvas, row++, "Password", text);
    } else {
        t5577_dump_view_draw_row(canvas, row++, "Password", "not read");
    }

    if(dump->has_password) {
        snprintf(text, sizeof(text), "%08lX", dump->password);
        t5577_dump_view_draw_row(canvas, row++, "Read with", text);
    }
}

static void t5577_dump_view_draw_callback(Canvas* canvas, void* _model) {
    const T5577DumpViewModel* model = _model;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    if(model->tab < 2) {
        t5577_dump_view_draw_title(canvas, model->tab == 0 ? "PAGE 0" : "PAGE 1", model->tab);
        t5577_dump_view_draw_page(canvas, &model->dump, model->tab);
    } else {
        t5577_dump_view_draw_title(canvas, "CONFIG", model->tab);
        t5577_dump_view_draw_config(canvas, &model->dump);
    }
}

static bool t5577_dump_view_input_callback(InputEvent* event, void* context) {
    T5577DumpView* instance = context;

    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return false;
    if(event->key != InputKeyLeft && event->key != InputKeyRight) return false;

    // Stops at the ends rather than wrapping around, so the counter means what it says.
    with_view_model(
        instance->view,
        T5577DumpViewModel * model,
        {
            if(event->key == InputKeyRight) {
                if(model->tab + 1 < T5577_DUMP_VIEW_TABS) model->tab++;
            } else if(model->tab > 0) {
                model->tab--;
            }
        },
        true);

    return true;
}

T5577DumpView* t5577_dump_view_alloc(void) {
    T5577DumpView* instance = malloc(sizeof(T5577DumpView));

    instance->view = view_alloc();
    view_set_context(instance->view, instance);
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(T5577DumpViewModel));
    view_set_draw_callback(instance->view, t5577_dump_view_draw_callback);
    view_set_input_callback(instance->view, t5577_dump_view_input_callback);

    return instance;
}

void t5577_dump_view_free(T5577DumpView* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* t5577_dump_view_get_view(T5577DumpView* instance) {
    furi_assert(instance);

    return instance->view;
}

void t5577_dump_view_set_dump(T5577DumpView* instance, const T5577Dump* dump) {
    furi_assert(instance);
    furi_assert(dump);

    with_view_model(
        instance->view,
        T5577DumpViewModel * model,
        {
            model->dump = *dump;
            model->tab = 0;
        },
        true);
}
