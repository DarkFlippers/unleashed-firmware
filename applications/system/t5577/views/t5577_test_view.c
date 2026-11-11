#include "t5577_test_view.h"

#include <furi.h>
#include <gui/canvas.h>
#include <lfrfid/tools/t5577.h>

#define T5577_TEST_VIEW_STATUS_MAX T5577_TEST_VIEW_LINE_MAX

// Blocks 1 to 7 of page 0, which is every block a test touches.
#define T5577_TEST_VIEW_FIRST 1

// Seven cells across the screen: 14 wide on an 18 pixel pitch leaves a 3 pixel margin each side.
#define T5577_TEST_VIEW_CELL_W     14
#define T5577_TEST_VIEW_CELL_H     14
#define T5577_TEST_VIEW_CELL_PITCH 18
#define T5577_TEST_VIEW_CELL_X     3
#define T5577_TEST_VIEW_CELL_Y     16

typedef struct {
    T5577TestCell cell[LFRFID_T5577_BLOCKS_IN_PAGE_0];
    char status[T5577_TEST_VIEW_STATUS_MAX];
    char hint[T5577_TEST_VIEW_STATUS_MAX];
} T5577TestViewModel;

struct T5577TestView {
    View* view;
};

// Indexed by block throughout, so the row's one conversion to a screen position happens here
// and nowhere else.
static void t5577_test_view_draw_cell(Canvas* canvas, uint8_t block, T5577TestCell cell) {
    const uint8_t x =
        T5577_TEST_VIEW_CELL_X + (block - T5577_TEST_VIEW_FIRST) * T5577_TEST_VIEW_CELL_PITCH;
    const uint8_t y = T5577_TEST_VIEW_CELL_Y;
    const uint8_t middle = x + T5577_TEST_VIEW_CELL_W / 2;
    const char* glyph = NULL;

    switch(cell) {
    case T5577TestCellTesting:
        // The block being worked on is the filled one, so the row reads as a cursor along it.
        canvas_draw_box(canvas, x, y, T5577_TEST_VIEW_CELL_W, T5577_TEST_VIEW_CELL_H);
        break;
    case T5577TestCellWriteable:
        glyph = "W";
        break;
    case T5577TestCellRefused:
        glyph = "-";
        break;
    case T5577TestCellUnknown:
        break;
    }

    if(cell != T5577TestCellTesting) {
        canvas_draw_frame(canvas, x, y, T5577_TEST_VIEW_CELL_W, T5577_TEST_VIEW_CELL_H);
    }

    if(glyph) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, middle, y + 11, AlignCenter, AlignBottom, glyph);
    }

    // The block number under its own cell, so a verdict never has to be counted out. One digit,
    // so it is built rather than formatted.
    const char number[2] = {(char)('0' + block), '\0'};
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas, middle, y + T5577_TEST_VIEW_CELL_H + 9, AlignCenter, AlignBottom, number);
}

static void t5577_test_view_draw_callback(Canvas* canvas, void* _model) {
    const T5577TestViewModel* model = _model;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignBottom, "Testing blocks");

    for(uint8_t block = T5577_TEST_VIEW_FIRST; block < LFRFID_T5577_BLOCKS_IN_PAGE_0; block++) {
        t5577_test_view_draw_cell(canvas, block, model->cell[block]);
    }

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 53, AlignCenter, AlignBottom, model->status);
    canvas_draw_str_aligned(canvas, 64, 63, AlignCenter, AlignBottom, model->hint);
}

T5577TestView* t5577_test_view_alloc(void) {
    T5577TestView* instance = malloc(sizeof(T5577TestView));

    instance->view = view_alloc();
    view_set_context(instance->view, instance);
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(T5577TestViewModel));
    view_set_draw_callback(instance->view, t5577_test_view_draw_callback);

    return instance;
}

void t5577_test_view_free(T5577TestView* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* t5577_test_view_get_view(T5577TestView* instance) {
    furi_assert(instance);

    return instance->view;
}

void t5577_test_view_reset(T5577TestView* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        T5577TestViewModel * model,
        {
            memset(model->cell, 0, sizeof(model->cell));
            model->status[0] = '\0';
            // The warning belongs on screen for the whole run, not only at the dialog.
            strlcpy(model->hint, "Do not move the tag", sizeof(model->hint));
        },
        true);
}

void t5577_test_view_set_cell(T5577TestView* instance, uint8_t block, T5577TestCell cell) {
    furi_check(instance);
    furi_check(block >= T5577_TEST_VIEW_FIRST);
    furi_check(block < LFRFID_T5577_BLOCKS_IN_PAGE_0);

    with_view_model(
        instance->view, T5577TestViewModel * model, { model->cell[block] = cell; }, true);
}

void t5577_test_view_set_status(T5577TestView* instance, const char* status) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        T5577TestViewModel * model,
        { strlcpy(model->status, status, sizeof(model->status)); },
        true);
}

void t5577_test_view_set_hint(T5577TestView* instance, const char* hint) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        T5577TestViewModel * model,
        { strlcpy(model->hint, hint, sizeof(model->hint)); },
        true);
}
