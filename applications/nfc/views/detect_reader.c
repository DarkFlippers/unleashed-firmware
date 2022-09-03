#include "detect_reader.h"

#include <gui/elements.h>

struct DetectReader {
    View* view;
    DetectReaderDoneCallback callback;
    void* context;
};

typedef struct {
    uint16_t nonces;
} DetectReaderViewModel;

static void detect_reader_draw_callback(Canvas* canvas, void* model) {
    DetectReaderViewModel* m = model;
    char text[32] = {};

    snprintf(text, sizeof(text), "Tap the reader several times");
    canvas_draw_str_aligned(canvas, 64, 0, AlignCenter, AlignTop, "Tap the reader several times");

    if(m->nonces == 0) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 52, 22, AlignLeft, AlignTop, "Emulating...");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 52, 35, AlignLeft, AlignTop, "MIFARE Classic");
        canvas_draw_icon(canvas, 0, 13, &I_Tap_reader_36x38);
    } else {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 54, 22, AlignLeft, AlignTop, "Collecting...");
        canvas_set_font(canvas, FontSecondary);
        snprintf(text, sizeof(text), "Nonces: %d", m->nonces);
        canvas_draw_str_aligned(canvas, 54, 35, AlignLeft, AlignTop, text);
        elements_button_right(canvas, "Next");
        canvas_draw_icon(canvas, 6, 15, &I_ArrowC_1_36x36);
    }
}

static bool detect_reader_input_callback(InputEvent* event, void* context) {
    DetectReader* detect_reader = context;
    furi_assert(detect_reader->callback);
    bool consumed = false;

    uint8_t nonces = 0;
    with_view_model(
        detect_reader->view, (DetectReaderViewModel * model) {
            nonces = model->nonces;
            return false;
        });

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyRight) {
            if(nonces > 0) {
                detect_reader->callback(detect_reader->context);
                consumed = true;
            }
        }
    }

    return consumed;
}

DetectReader* detect_reader_alloc() {
    DetectReader* detect_reader = malloc(sizeof(DetectReader));
    detect_reader->view = view_alloc();
    view_allocate_model(detect_reader->view, ViewModelTypeLocking, sizeof(DetectReaderViewModel));
    view_set_draw_callback(detect_reader->view, detect_reader_draw_callback);
    view_set_input_callback(detect_reader->view, detect_reader_input_callback);
    view_set_context(detect_reader->view, detect_reader);

    return detect_reader;
}

void detect_reader_free(DetectReader* detect_reader) {
    furi_assert(detect_reader);

    view_free(detect_reader->view);
    free(detect_reader);
}

void detect_reader_reset(DetectReader* detect_reader) {
    furi_assert(detect_reader);

    with_view_model(
        detect_reader->view, (DetectReaderViewModel * model) {
            model->nonces = 0;
            return false;
        });
}

View* detect_reader_get_view(DetectReader* detect_reader) {
    furi_assert(detect_reader);

    return detect_reader->view;
}

void detect_reader_set_callback(
    DetectReader* detect_reader,
    DetectReaderDoneCallback callback,
    void* context) {
    furi_assert(detect_reader);
    furi_assert(callback);

    detect_reader->callback = callback;
    detect_reader->context = context;
}

void detect_reader_inc_nonce_cnt(DetectReader* detect_reader) {
    furi_assert(detect_reader);
    with_view_model(
        detect_reader->view, (DetectReaderViewModel * model) {
            model->nonces++;
            return false;
        });
}
