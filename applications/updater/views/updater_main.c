#include <gui/gui_i.h>
#include <gui/view.h>
#include <gui/elements.h>
#include <gui/canvas.h>
#include <furi.h>
#include <input/input.h>

#include "../updater_i.h"
#include "updater_main.h"

struct UpdaterMainView {
    View* view;
    ViewDispatcher* view_dispatcher;
    FuriPubSubSubscription* subscription;
    void* context;
};

static const uint8_t PROGRESS_RENDER_STEP = 3; /* percent, to limit rendering rate */

typedef struct {
    string_t status;
    uint8_t progress, rendered_progress;
    uint8_t idx_stage, total_stages;
    bool failed;
} UpdaterProgressModel;

void updater_main_model_set_state(
    UpdaterMainView* main_view,
    const char* message,
    uint8_t progress,
    uint8_t idx_stage,
    uint8_t total_stages,
    bool failed) {
    with_view_model(
        main_view->view, (UpdaterProgressModel * model) {
            model->failed = failed;
            model->idx_stage = idx_stage;
            model->total_stages = total_stages;
            model->progress = progress;
            if(string_cmp_str(model->status, message)) {
                string_set(model->status, message);
                model->rendered_progress = progress;
                return true;
            }
            if((model->rendered_progress > progress) ||
               ((progress - model->rendered_progress) > PROGRESS_RENDER_STEP)) {
                model->rendered_progress = progress;
                return true;
            }
            return false;
        });
}

View* updater_main_get_view(UpdaterMainView* main_view) {
    furi_assert(main_view);
    return main_view->view;
}

bool updater_main_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    UpdaterMainView* main_view = context;
    if(!main_view->view_dispatcher) {
        return true;
    }

    if(event->type != InputTypeShort) {
        return true;
    }

    if(event->key == InputKeyOk) {
        view_dispatcher_send_custom_event(
            main_view->view_dispatcher, UpdaterCustomEventRetryUpdate);
    } else if(event->key == InputKeyBack) {
        view_dispatcher_send_custom_event(
            main_view->view_dispatcher, UpdaterCustomEventCancelUpdate);
    }

    return true;
}

static void updater_main_draw_callback(Canvas* canvas, void* _model) {
    UpdaterProgressModel* model = _model;

    canvas_set_font(canvas, FontPrimary);

    uint16_t y_offset = model->failed ? 5 : 13;
    string_t status_text;
    if(!model->failed && (model->idx_stage != 0) && (model->idx_stage <= model->total_stages)) {
        string_init_printf(
            status_text,
            "[%d/%d] %s",
            model->idx_stage,
            model->total_stages,
            string_get_cstr(model->status));
    } else {
        string_init_set(status_text, model->status);
    }
    canvas_draw_str_aligned(
        canvas, 128 / 2, y_offset, AlignCenter, AlignTop, string_get_cstr(status_text));
    string_clear(status_text);
    if(model->failed) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(
            canvas, 128 / 2, 20, AlignCenter, AlignTop, "[OK] to retry, [Back] to abort");
    }
    elements_progress_bar(canvas, 14, 35, 100, (float)model->progress / 100);
}

UpdaterMainView* updater_main_alloc() {
    UpdaterMainView* main_view = malloc(sizeof(UpdaterMainView));

    main_view->view = view_alloc();
    view_allocate_model(main_view->view, ViewModelTypeLocking, sizeof(UpdaterProgressModel));

    with_view_model(
        main_view->view, (UpdaterProgressModel * model) {
            string_init_set(model->status, "Waiting for storage");
            return true;
        });

    view_set_context(main_view->view, main_view);
    view_set_input_callback(main_view->view, updater_main_input);
    view_set_draw_callback(main_view->view, updater_main_draw_callback);

    return main_view;
}

void updater_main_free(UpdaterMainView* main_view) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, (UpdaterProgressModel * model) {
            string_clear(model->status);
            return false;
        });
    view_free(main_view->view);
    free(main_view);
}

void updater_main_set_storage_pubsub(UpdaterMainView* main_view, FuriPubSubSubscription* sub) {
    main_view->subscription = sub;
}

FuriPubSubSubscription* updater_main_get_storage_pubsub(UpdaterMainView* main_view) {
    return main_view->subscription;
}

void updater_main_set_view_dispatcher(UpdaterMainView* main_view, ViewDispatcher* view_dispatcher) {
    main_view->view_dispatcher = view_dispatcher;
}
