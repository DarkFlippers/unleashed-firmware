#include "gui/view.h"
#include "furi/memmgr.h"
#include "view_composed.h"
#include "view_i.h"

typedef struct {
    View* bottom;
    View* top;
    bool top_enabled;
} ViewComposedModel;

struct ViewComposed {
    View* view;
};

static void view_composed_draw(Canvas* canvas, void* model);
static bool view_composed_input(InputEvent* event, void* context);

static void view_composed_update_callback(View* view_top_or_bottom, void* context) {
    furi_assert(view_top_or_bottom);
    furi_assert(context);

    View* view_composed_view = context;
    view_composed_view->update_callback(
        view_composed_view, view_composed_view->update_callback_context);
}

static void view_composed_enter(void* context) {
    furi_assert(context);

    ViewComposed* view_composed = context;
    ViewComposedModel* model = view_get_model(view_composed->view);

    /* if more than 1 composed views hold same view it has to reassign update_callback_context */
    if(model->bottom) {
        view_set_update_callback_context(model->bottom, view_composed->view);
        if(model->bottom->enter_callback) {
            model->bottom->enter_callback(model->bottom->context);
        }
    }
    if(model->top) {
        view_set_update_callback_context(model->top, view_composed->view);
        if(model->top->enter_callback) {
            model->top->enter_callback(model->top->context);
        }
    }

    view_commit_model(view_composed->view, false);
}

static void view_composed_exit(void* context) {
    furi_assert(context);

    ViewComposed* view_composed = context;
    ViewComposedModel* model = view_get_model(view_composed->view);

    if(model->bottom) {
        if(model->bottom->exit_callback) {
            model->bottom->exit_callback(model->bottom->context);
        }
    }
    if(model->top) {
        if(model->top->exit_callback) {
            model->top->exit_callback(model->top->context);
        }
    }

    view_commit_model(view_composed->view, false);
}

ViewComposed* view_composed_alloc(void) {
    ViewComposed* view_composed = furi_alloc(sizeof(ViewComposed));
    view_composed->view = view_alloc();

    view_allocate_model(view_composed->view, ViewModelTypeLocking, sizeof(ViewComposedModel));
    view_set_draw_callback(view_composed->view, view_composed_draw);
    view_set_input_callback(view_composed->view, view_composed_input);
    view_set_context(view_composed->view, view_composed);
    view_set_enter_callback(view_composed->view, view_composed_enter);
    view_set_exit_callback(view_composed->view, view_composed_exit);
    return view_composed;
}

void view_composed_free(ViewComposed* view_composed) {
    furi_assert(view_composed);

    ViewComposedModel* view_composed_model = view_get_model(view_composed->view);
    view_set_update_callback(view_composed_model->bottom, NULL);
    view_set_update_callback_context(view_composed_model->bottom, NULL);
    view_set_update_callback(view_composed_model->top, NULL);
    view_set_update_callback_context(view_composed_model->top, NULL);
    view_commit_model(view_composed->view, true);

    view_free(view_composed->view);
    free(view_composed);
}

static void view_composed_draw(Canvas* canvas, void* model) {
    furi_assert(model);

    ViewComposedModel* view_composed_model = model;

    view_draw(view_composed_model->bottom, canvas);
    if(view_composed_model->top_enabled && view_composed_model->top) {
        view_draw(view_composed_model->top, canvas);
    }
}

static bool view_composed_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    ViewComposed* view_composed = context;
    ViewComposedModel* view_composed_model = view_get_model(view_composed->view);
    bool consumed = false;

    if(view_composed_model->top_enabled && view_composed_model->top) {
        consumed = view_input(view_composed_model->top, event);
    }
    if(!consumed) {
        consumed = view_input(view_composed_model->bottom, event);
    }

    view_commit_model(view_composed->view, false);

    return consumed;
}

void view_composed_top_enable(ViewComposed* view_composed, bool enable) {
    furi_assert(view_composed);

    ViewComposedModel* view_composed_model = view_get_model(view_composed->view);
    bool update = (view_composed_model->top_enabled != enable);
    view_composed_model->top_enabled = enable;
    view_commit_model(view_composed->view, update);
}

void view_composed_tie_views(ViewComposed* view_composed, View* view_bottom, View* view_top) {
    furi_assert(view_composed);
    furi_assert(view_bottom);

    ViewComposedModel* view_composed_model = view_get_model(view_composed->view);

    if(view_composed_model->bottom) {
        view_set_update_callback(view_composed_model->bottom, NULL);
        view_set_update_callback_context(view_composed_model->bottom, NULL);
    }
    if(view_composed_model->top) {
        view_set_update_callback(view_composed_model->top, NULL);
        view_set_update_callback_context(view_composed_model->top, NULL);
    }

    view_composed_model->bottom = view_bottom;
    view_set_update_callback(view_bottom, view_composed_update_callback);
    view_set_update_callback_context(view_bottom, view_composed->view);
    view_composed_model->top = view_top;
    view_set_update_callback(view_top, view_composed_update_callback);
    view_set_update_callback_context(view_top, view_composed->view);

    view_commit_model(view_composed->view, true);
}

View* view_composed_get_view(ViewComposed* view_composed) {
    furi_assert(view_composed);
    return view_composed->view;
}
