#include "detect_reader.h"
#include <assets_icons.h>
#include <gui/elements.h>

#define DETECT_READER_UID_MAX_LEN (10)

struct DetectReader {
    View* view;
    DetectReaderDoneCallback callback;
    void* context;
};

typedef struct {
    uint16_t nonces;
    uint16_t nonces_max;
    DetectReaderState state;
    FuriString* uid_str;
} DetectReaderViewModel;

static void detect_reader_draw_callback(Canvas* canvas, void* model) {
    DetectReaderViewModel* m = model;
    char text[32] = {};

    // Draw header and icon
    canvas_draw_icon(canvas, 0, 16, &I_Modern_reader_18x34);
    if(m->state == DetectReaderStateStart) {
        snprintf(text, sizeof(text), "Touch the reader");
        canvas_draw_icon(canvas, 21, 13, &I_Move_flipper_26x39);
        if(furi_string_size(m->uid_str)) {
            elements_multiline_text_aligned(
                canvas, 64, 64, AlignCenter, AlignBottom, furi_string_get_cstr(m->uid_str));
        }
    } else if(m->state == DetectReaderStateReaderDetected) {
        snprintf(text, sizeof(text), "Move the Flipper away");
        canvas_draw_icon(canvas, 24, 25, &I_Release_arrow_18x15);
    } else if(m->state == DetectReaderStateReaderLost) {
        snprintf(text, sizeof(text), "Touch the reader again");
        canvas_draw_icon(canvas, 21, 13, &I_Move_flipper_26x39);
    }

    canvas_draw_str_aligned(canvas, 64, 0, AlignCenter, AlignTop, text);

    // Draw collected nonces
    if(m->state == DetectReaderStateStart) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 51, 22, AlignLeft, AlignTop, "Emulating...");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 51, 35, AlignLeft, AlignTop, "MIFARE MFkey32");
    } else {
        if(m->state == DetectReaderStateDone) {
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str_aligned(canvas, 51, 22, AlignLeft, AlignTop, "Completed!");
            canvas_draw_icon(canvas, 24, 23, &I_check_big_20x17);
        } else {
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str_aligned(canvas, 51, 22, AlignLeft, AlignTop, "Collecting...");
        }
        canvas_set_font(canvas, FontSecondary);
        snprintf(text, sizeof(text), "Nonce pairs: %d/%d", m->nonces, m->nonces_max);
        canvas_draw_str_aligned(canvas, 51, 35, AlignLeft, AlignTop, text);
    }
    // Draw button
    if(m->nonces > 0) {
        elements_button_center(canvas, "Done");
    }
}

static bool detect_reader_input_callback(InputEvent* event, void* context) {
    DetectReader* detect_reader = context;
    furi_assert(detect_reader->callback);
    bool consumed = false;

    uint8_t nonces = 0;
    with_view_model(
        detect_reader->view, DetectReaderViewModel * model, { nonces = model->nonces; }, false);

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyOk) {
            if(nonces > 0) {
                detect_reader->callback(detect_reader->context);
                consumed = true;
            }
        }
    }

    return consumed;
}

DetectReader* detect_reader_alloc(void) {
    DetectReader* detect_reader = malloc(sizeof(DetectReader));
    detect_reader->view = view_alloc();
    view_allocate_model(detect_reader->view, ViewModelTypeLocking, sizeof(DetectReaderViewModel));
    view_set_draw_callback(detect_reader->view, detect_reader_draw_callback);
    view_set_input_callback(detect_reader->view, detect_reader_input_callback);
    view_set_context(detect_reader->view, detect_reader);

    with_view_model(
        detect_reader->view,
        DetectReaderViewModel * model,
        { model->uid_str = furi_string_alloc(); },
        false);

    return detect_reader;
}

void detect_reader_free(DetectReader* detect_reader) {
    furi_assert(detect_reader);

    with_view_model(
        detect_reader->view,
        DetectReaderViewModel * model,
        { furi_string_free(model->uid_str); },
        false);

    view_free(detect_reader->view);
    free(detect_reader);
}

void detect_reader_reset(DetectReader* detect_reader) {
    furi_assert(detect_reader);

    with_view_model(
        detect_reader->view,
        DetectReaderViewModel * model,
        {
            model->nonces = 0;
            model->nonces_max = 0;
            model->state = DetectReaderStateStart;
            furi_string_reset(model->uid_str);
        },
        false);
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

void detect_reader_set_nonces_max(DetectReader* detect_reader, uint16_t nonces_max) {
    furi_assert(detect_reader);

    with_view_model(
        detect_reader->view,
        DetectReaderViewModel * model,
        { model->nonces_max = nonces_max; },
        false);
}

void detect_reader_set_nonces_collected(DetectReader* detect_reader, uint16_t nonces_collected) {
    furi_assert(detect_reader);

    with_view_model(
        detect_reader->view,
        DetectReaderViewModel * model,
        {
            model->nonces = nonces_collected;
            model->state = DetectReaderStateReaderDetected;
        },
        false);
}

void detect_reader_set_state(DetectReader* detect_reader, DetectReaderState state) {
    furi_assert(detect_reader);
    with_view_model(
        detect_reader->view, DetectReaderViewModel * model, { model->state = state; }, true);
}

void detect_reader_set_uid(DetectReader* detect_reader, uint8_t* uid, uint8_t uid_len) {
    furi_assert(detect_reader);
    furi_assert(uid);
    furi_assert(uid_len < DETECT_READER_UID_MAX_LEN);
    with_view_model(
        detect_reader->view,
        DetectReaderViewModel * model,
        {
            furi_string_set_str(model->uid_str, "UID:");
            for(size_t i = 0; i < uid_len; i++) {
                furi_string_cat_printf(model->uid_str, " %02X", uid[i]);
            }
        },
        true);
}
