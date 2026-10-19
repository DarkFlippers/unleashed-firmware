#include "t5577_dump_view.h"

#include <furi.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <lfrfid/tools/t5577.h>

#define T5577_DUMP_VIEW_TABS 3

// A title with a rule under it, then rows 12 pixels apart, which is what fits four rows of
// blocks under the rule - the menus are on 16. Page 0 is the one page that does not fit a
// column of its own: eight blocks go side by side, four and four.
#define T5577_DUMP_VIEW_TITLE_Y   10
#define T5577_DUMP_VIEW_RULE_Y    13
#define T5577_DUMP_VIEW_ROW_Y     25
#define T5577_DUMP_VIEW_PITCH     12
#define T5577_DUMP_VIEW_ROWS      4
#define T5577_DUMP_VIEW_COL_LEFT  2
#define T5577_DUMP_VIEW_COL_RIGHT 66
#define T5577_DUMP_VIEW_HEX       10

// The configuration is a list of its own: five lines, closer together than the block rows.
#define T5577_DUMP_VIEW_CFG_Y     23
#define T5577_DUMP_VIEW_CFG_PITCH 10
#define T5577_DUMP_VIEW_CFG_VALUE 56

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
    switch(t5577_config_modulation(block0)) {
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
    switch(t5577_config_bitrate(block0)) {
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
    // RF/2 is the field's zero, and both bits set is reserved, so this reads as a field.
    switch(t5577_config_psk_carrier(block0)) {
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
    const uint32_t modulation = t5577_config_modulation(block0);

    return modulation == LFRFID_T5577_MODULATION_PSK1 ||
           modulation == LFRFID_T5577_MODULATION_PSK2 ||
           modulation == LFRFID_T5577_MODULATION_PSK3;
}

static void t5577_dump_view_draw_title(Canvas* canvas, const char* title, uint8_t tab) {
    char counter[8];
    snprintf(counter, sizeof(counter), "%u/%u", tab + 1, T5577_DUMP_VIEW_TABS);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, T5577_DUMP_VIEW_COL_LEFT, T5577_DUMP_VIEW_TITLE_Y, title);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas, 126, T5577_DUMP_VIEW_TITLE_Y, AlignRight, AlignBottom, counter);

    canvas_draw_line(canvas, 0, T5577_DUMP_VIEW_RULE_Y, 127, T5577_DUMP_VIEW_RULE_Y);
}

// One block: its number, and what it holds. Dashes, not zeros, for a block that was never read -
// a block nobody has seen is not a block holding zero, and the difference is what decides whether
// a write would touch it.
static void t5577_dump_view_draw_block(
    Canvas* canvas,
    const T5577Dump* dump,
    uint8_t page,
    uint8_t block,
    int32_t x,
    int32_t y) {
    char text[16];

    snprintf(text, sizeof(text), "%u", block);
    canvas_draw_str(canvas, x, y, text);

    if(t5577_dump_was_read(dump, page, block)) {
        snprintf(text, sizeof(text), "%08lX", dump->block[page][block]);
    } else {
        snprintf(text, sizeof(text), "--------");
    }
    canvas_draw_str(canvas, x + T5577_DUMP_VIEW_HEX, y, text);
}

static void t5577_dump_view_draw_page_0(Canvas* canvas, const T5577Dump* dump) {
    canvas_set_font(canvas, FontSecondary);

    for(uint8_t row = 0; row < T5577_DUMP_VIEW_ROWS; row++) {
        const int32_t y = T5577_DUMP_VIEW_ROW_Y + row * T5577_DUMP_VIEW_PITCH;

        t5577_dump_view_draw_block(canvas, dump, 0, row, T5577_DUMP_VIEW_COL_LEFT, y);
        t5577_dump_view_draw_block(
            canvas, dump, 0, row + T5577_DUMP_VIEW_ROWS, T5577_DUMP_VIEW_COL_RIGHT, y);
    }
}

static void t5577_dump_view_draw_page_1(Canvas* canvas, const T5577Dump* dump) {
    canvas_set_font(canvas, FontSecondary);

    for(uint8_t block = 0; block < LFRFID_T5577_BLOCKS_IN_PAGE_1; block++) {
        const int32_t y = T5577_DUMP_VIEW_ROW_Y + block * T5577_DUMP_VIEW_PITCH;

        t5577_dump_view_draw_block(canvas, dump, 1, block, T5577_DUMP_VIEW_COL_LEFT, y);
    }
}

static void
    t5577_dump_view_draw_row(Canvas* canvas, uint8_t row, const char* label, const char* value) {
    const int32_t y = T5577_DUMP_VIEW_CFG_Y + row * T5577_DUMP_VIEW_CFG_PITCH;

    canvas_draw_str(canvas, T5577_DUMP_VIEW_COL_LEFT, y, label);
    canvas_draw_str(canvas, T5577_DUMP_VIEW_CFG_VALUE, y, value);
}

// Block 0 in the terms the chip reads it in. It is the one block whose hex says nothing on its
// own, and the one that decides whether the rest of the dump can be read back at all.
static void t5577_dump_view_draw_config(Canvas* canvas, const T5577Dump* dump) {
    canvas_set_font(canvas, FontSecondary);

    if(!t5577_dump_was_read(dump, 0, 0)) {
        elements_multiline_text(
            canvas,
            T5577_DUMP_VIEW_COL_LEFT,
            T5577_DUMP_VIEW_CFG_Y,
            "Block 0 was not\nread, so there is\nnothing to say");
        return;
    }

    const uint32_t block0 = dump->block[0][0];
    char text[24];

    // The clock a PSK answer is keyed at belongs with the modulation that uses it, and means
    // nothing next to any other.
    if(t5577_dump_view_is_psk(block0)) {
        snprintf(
            text,
            sizeof(text),
            "%s %s",
            t5577_dump_view_modulation(block0),
            t5577_dump_view_pskcf(block0));
    } else {
        snprintf(text, sizeof(text), "%s", t5577_dump_view_modulation(block0));
    }
    t5577_dump_view_draw_row(canvas, 0, "Modulation", text);

    t5577_dump_view_draw_row(canvas, 1, "Bit rate", t5577_dump_view_bitrate(block0));

    // How far the chip answers: blocks above this one are there but stay silent.
    snprintf(text, sizeof(text), "%u", t5577_config_max_block(block0));
    t5577_dump_view_draw_row(canvas, 2, "Max block", text);

    char flags[24] = "";
    // The bit, not the question: this row names what is set, and AOR has its own entry.
    if(block0 & LFRFID_T5577_PWD) strlcat(flags, "PWD ", sizeof(flags));
    if(block0 & LFRFID_T5577_AOR) strlcat(flags, "AOR ", sizeof(flags));
    if(block0 & LFRFID_T5577_ST_TERMINATOR) strlcat(flags, "ST ", sizeof(flags));
    if(block0 & LFRFID_T5577_X_MODE) strlcat(flags, "XM ", sizeof(flags));
    if(block0 & LFRFID_T5577_POR_DELAY) strlcat(flags, "POR", sizeof(flags));
    t5577_dump_view_draw_row(canvas, 3, "Flags", flags[0] ? flags : "none");

    // Whether the chip asks for a password comes first, because block 7 holds a word either way
    // and printing it where none is asked for reads as a password that is in force. Where one is
    // asked for and block 7 was not read, the password the dump was read with is the next best
    // thing, and is said to be that rather than passed off as the block.
    if(!t5577_config_needs_password(block0)) {
        snprintf(text, sizeof(text), "not required");
    } else if(t5577_dump_was_read(dump, 0, 7)) {
        snprintf(text, sizeof(text), "%08lX", dump->block[0][7]);
    } else if(dump->has_password) {
        snprintf(text, sizeof(text), "%08lX used", dump->password);
    } else {
        snprintf(text, sizeof(text), "set, unknown");
    }
    t5577_dump_view_draw_row(canvas, 4, "Password", text);
}

static void t5577_dump_view_draw_callback(Canvas* canvas, void* _model) {
    const T5577DumpViewModel* model = _model;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    static const char* const titles[T5577_DUMP_VIEW_TABS] = {"Page 0", "Page 1", "Config"};
    t5577_dump_view_draw_title(canvas, titles[model->tab], model->tab);

    if(model->tab == 0) {
        t5577_dump_view_draw_page_0(canvas, &model->dump);
    } else if(model->tab == 1) {
        t5577_dump_view_draw_page_1(canvas, &model->dump);
    } else {
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
