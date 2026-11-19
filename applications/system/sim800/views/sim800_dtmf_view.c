#include "sim800_dtmf_view.h"

#include <gui/elements.h>
#include <furi.h>

#define DTMF_ROWS 4
#define DTMF_COLS 3

#define DTMF_SENT_MAX 20

static const char dtmf_keys[DTMF_ROWS][DTMF_COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'},
};

typedef struct {
    uint8_t row;
    uint8_t col;
    char sent[DTMF_SENT_MAX + 1];
} Sim800DtmfViewModel;

struct Sim800DtmfView {
    View* view;
    Sim800DtmfViewCallback callback;
    void* context;
};

static void sim800_dtmf_view_draw(Canvas* canvas, void* ctx) {
    Sim800DtmfViewModel* m = ctx;
    char key[2] = {0, 0};

    canvas_clear(canvas);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 9, "Tones");
    if(m->sent[0] != '\0') {
        canvas_draw_str_aligned(canvas, 126, 9, AlignRight, AlignBottom, m->sent);
    }
    canvas_draw_line(canvas, 0, 12, 127, 12);

    canvas_set_font(canvas, FontPrimary);
    for(uint8_t row = 0; row < DTMF_ROWS; row++) {
        for(uint8_t col = 0; col < DTMF_COLS; col++) {
            int32_t cx = 22 + col * 34;
            int32_t cy = 24 + row * 12;
            bool picked = (row == m->row) && (col == m->col);

            if(picked) {
                canvas_draw_rbox(canvas, cx - 12, cy - 9, 25, 12, 2);
                canvas_set_color(canvas, ColorWhite);
            }

            key[0] = dtmf_keys[row][col];
            canvas_draw_str_aligned(canvas, cx, cy, AlignCenter, AlignBottom, key);

            if(picked) canvas_set_color(canvas, ColorBlack);
        }
    }
}

static bool sim800_dtmf_view_input(InputEvent* event, void* ctx) {
    Sim800DtmfView* instance = ctx;

    if((event->type != InputTypeShort) && (event->type != InputTypeRepeat)) return false;

    char digit = 0;
    bool consumed = true;

    with_view_model(
        instance->view,
        Sim800DtmfViewModel * model,
        {
            switch(event->key) {
            case InputKeyUp:
                model->row = (model->row + DTMF_ROWS - 1) % DTMF_ROWS;
                break;
            case InputKeyDown:
                model->row = (model->row + 1) % DTMF_ROWS;
                break;
            case InputKeyLeft:
                model->col = (model->col + DTMF_COLS - 1) % DTMF_COLS;
                break;
            case InputKeyRight:
                model->col = (model->col + 1) % DTMF_COLS;
                break;
            case InputKeyOk:
                digit = dtmf_keys[model->row][model->col];
                break;
            default:
                consumed = false;
                break;
            }
        },
        true);

    if((digit != 0) && (instance->callback != NULL)) {
        instance->callback(digit, instance->context);
    }

    return consumed;
}

Sim800DtmfView* sim800_dtmf_view_alloc(void) {
    Sim800DtmfView* instance = malloc(sizeof(Sim800DtmfView));

    instance->view = view_alloc();
    instance->callback = NULL;
    instance->context = NULL;

    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(Sim800DtmfViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, sim800_dtmf_view_draw);
    view_set_input_callback(instance->view, sim800_dtmf_view_input);

    return instance;
}

void sim800_dtmf_view_free(Sim800DtmfView* instance) {
    furi_assert(instance);
    view_free(instance->view);
    free(instance);
}

View* sim800_dtmf_view_get_view(Sim800DtmfView* instance) {
    furi_assert(instance);
    return instance->view;
}

void sim800_dtmf_view_set_callback(
    Sim800DtmfView* instance,
    Sim800DtmfViewCallback callback,
    void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->context = context;
}

void sim800_dtmf_view_set_sent(Sim800DtmfView* instance, const char* sent) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        Sim800DtmfViewModel * model,
        {
            // Only the tail fits on the header line, and it is the recent digits that matter
            size_t len = strlen(sent);
            const char* from = (len > DTMF_SENT_MAX) ? (sent + len - DTMF_SENT_MAX) : sent;
            strlcpy(model->sent, from, sizeof(model->sent));
        },
        true);
}
