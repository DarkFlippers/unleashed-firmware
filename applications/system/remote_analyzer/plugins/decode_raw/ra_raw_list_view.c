#include "ra_raw_list_view.h"

#include <furi.h>
#include <gui/elements.h>
#include <input/input.h>

/* The Sub-GHz list's own geometry, so the two screens read the same: rows of 12 from the top,
 * the icon inset 4 and the text at 15, and a scrollbar down the right once it is needed. */
#define RA_RAW_LIST_ROWS   4u
#define RA_RAW_LIST_FRAME  12
#define RA_RAW_LIST_TEXT_W 111
#define RA_RAW_LIST_SEP_Y  52
#define RA_RAW_LIST_FOOT_Y 62

struct RaRawListView {
    View* view;
    RaRawListCallback callback;
    void* context;
};

typedef struct {
    FuriString* label[RA_RAW_LIST_MAX];
    uint8_t kind[RA_RAW_LIST_MAX];
    const Icon* icons[RA_RAW_LIST_ICONS];
    FuriString* footer;
    size_t count;
    size_t index; // selected entry
    size_t offset; // first entry drawn
} RaRawListViewModel;

/** The selected row, inverted with the corners knocked out, as the Sub-GHz list draws it. */
static void ra_raw_list_view_draw_frame(Canvas* canvas, size_t row, bool scrollbar) {
    const uint8_t y = (uint8_t)(row * RA_RAW_LIST_FRAME);

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, y, scrollbar ? 122 : 127, RA_RAW_LIST_FRAME);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_dot(canvas, 0, y);
    canvas_draw_dot(canvas, 1, y);
    canvas_draw_dot(canvas, 0, y + 1);
    canvas_draw_dot(canvas, 0, y + 11);
    canvas_draw_dot(canvas, scrollbar ? 121 : 126, y);
    canvas_draw_dot(canvas, scrollbar ? 121 : 126, y + 11);

    // Left white, so the row's icon and text land on top of the box
}

static void ra_raw_list_view_draw(Canvas* canvas, RaRawListViewModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    const bool scrollbar = model->count > RA_RAW_LIST_ROWS;
    FuriString* line = furi_string_alloc();

    for(size_t row = 0; row < RA_RAW_LIST_ROWS; row++) {
        const size_t idx = row + model->offset;
        if(idx >= model->count) break;

        if(idx == model->index) {
            ra_raw_list_view_draw_frame(canvas, row, scrollbar);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        furi_string_set(line, model->label[idx]);
        elements_string_fit_width(
            canvas, line, scrollbar ? RA_RAW_LIST_TEXT_W - 7 : RA_RAW_LIST_TEXT_W);

        const Icon* icon = model->icons[model->kind[idx] % RA_RAW_LIST_ICONS];
        if(icon) canvas_draw_icon(canvas, 4, 2 + (uint8_t)(row * RA_RAW_LIST_FRAME), icon);
        canvas_draw_str(
            canvas, 15, 9 + (uint8_t)(row * RA_RAW_LIST_FRAME), furi_string_get_cstr(line));
    }
    furi_string_free(line);

    if(scrollbar) elements_scrollbar_pos(canvas, 128, 0, 49, model->index, model->count);

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_line(canvas, 2, RA_RAW_LIST_SEP_Y, 125, RA_RAW_LIST_SEP_Y);
    canvas_draw_str(canvas, 3, RA_RAW_LIST_FOOT_Y, furi_string_get_cstr(model->footer));
}

static bool ra_raw_list_view_input(InputEvent* event, void* context) {
    furi_assert(context);
    RaRawListView* instance = context;

    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return false;

    if(event->key == InputKeyOk) {
        uint32_t index = 0;
        bool any = false;
        with_view_model(
            instance->view,
            RaRawListViewModel * model,
            {
                index = model->index;
                any = model->count > 0;
            },
            false);
        if(any && instance->callback) instance->callback(instance->context, index);
        return true;
    }

    if(event->key != InputKeyUp && event->key != InputKeyDown) return false;

    const bool down = event->key == InputKeyDown;
    with_view_model(
        instance->view,
        RaRawListViewModel * model,
        {
            if(down) {
                if(model->index + 1 < model->count) model->index++;
            } else if(model->index > 0) {
                model->index--;
            }
            // Keep the selection on screen, scrolling by one as it reaches either edge
            if(model->index < model->offset) {
                model->offset = model->index;
            } else if(model->index >= model->offset + RA_RAW_LIST_ROWS) {
                model->offset = model->index - RA_RAW_LIST_ROWS + 1;
            }
        },
        true);
    return true;
}

RaRawListView* ra_raw_list_view_alloc(void) {
    RaRawListView* instance = malloc(sizeof(RaRawListView));

    instance->callback = NULL;
    instance->context = NULL;

    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(RaRawListViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)ra_raw_list_view_draw);
    view_set_input_callback(instance->view, ra_raw_list_view_input);

    with_view_model(
        instance->view,
        RaRawListViewModel * model,
        {
            model->footer = furi_string_alloc();
            model->count = 0;
            model->index = 0;
            model->offset = 0;
            for(size_t i = 0; i < RA_RAW_LIST_ICONS; i++) {
                model->icons[i] = NULL;
            }
        },
        false);

    return instance;
}

void ra_raw_list_view_free(RaRawListView* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        RaRawListViewModel * model,
        {
            for(size_t i = 0; i < model->count; i++) {
                furi_string_free(model->label[i]);
            }
            furi_string_free(model->footer);
        },
        false);
    view_free(instance->view);
    free(instance);
}

View* ra_raw_list_view_get_view(RaRawListView* instance) {
    furi_assert(instance);
    return instance->view;
}

void ra_raw_list_view_set_callback(
    RaRawListView* instance,
    RaRawListCallback callback,
    void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->context = context;
}

void ra_raw_list_view_set_icons(RaRawListView* instance, const Icon** icons) {
    furi_assert(instance);
    furi_assert(icons);
    with_view_model(
        instance->view,
        RaRawListViewModel * model,
        {
            for(size_t i = 0; i < RA_RAW_LIST_ICONS; i++) {
                model->icons[i] = icons[i];
            }
        },
        false);
}

bool ra_raw_list_view_add(RaRawListView* instance, const char* label, uint8_t kind) {
    furi_assert(instance);
    bool added = false;
    with_view_model(
        instance->view,
        RaRawListViewModel * model,
        {
            if(model->count < RA_RAW_LIST_MAX) {
                model->label[model->count] = furi_string_alloc_set_str(label);
                model->kind[model->count] = kind;
                model->count++;
                added = true;
            }
        },
        added);
    return added;
}

void ra_raw_list_view_set_footer(RaRawListView* instance, const char* text) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        RaRawListViewModel * model,
        { furi_string_set_str(model->footer, text); },
        true);
}
