#include "gui/view.h"
#include <core/memmgr.h>
#include "view_stack.h"
#include "view_i.h"

#define MAX_VIEWS 3

typedef struct {
    View* views[MAX_VIEWS];
} ViewStackModel;

struct ViewStack {
    View* view;
};

static void view_stack_draw(Canvas* canvas, void* model);
static bool view_stack_input(InputEvent* event, void* context);

static void view_stack_update_callback(View* view_top_or_bottom, void* context) {
    furi_assert(view_top_or_bottom);
    furi_assert(context);

    View* view_stack_view = context;
    if(view_stack_view->update_callback) {
        view_stack_view->update_callback(
            view_stack_view, view_stack_view->update_callback_context);
    }
}

static void view_stack_enter(void* context) {
    furi_assert(context);

    ViewStack* view_stack = context;
    ViewStackModel* model = view_get_model(view_stack->view);

    /* if more than 1 Stack View hold same view they have to reassign update_callback_context */
    for(int i = 0; i < MAX_VIEWS; ++i) {
        if(model->views[i]) {
            view_set_update_callback_context(model->views[i], view_stack->view);
            if(model->views[i]->enter_callback) {
                model->views[i]->enter_callback(model->views[i]->context);
            }
        }
    }

    view_commit_model(view_stack->view, false);
}

static void view_stack_exit(void* context) {
    furi_assert(context);

    ViewStack* view_stack = context;
    ViewStackModel* model = view_get_model(view_stack->view);

    for(int i = 0; i < MAX_VIEWS; ++i) {
        if(model->views[i] && model->views[i]->exit_callback) {
            model->views[i]->exit_callback(model->views[i]->context);
        }
    }

    view_commit_model(view_stack->view, false);
}

ViewStack* view_stack_alloc(void) {
    ViewStack* view_stack = malloc(sizeof(ViewStack));
    view_stack->view = view_alloc();

    view_allocate_model(view_stack->view, ViewModelTypeLocking, sizeof(ViewStackModel));
    view_set_draw_callback(view_stack->view, view_stack_draw);
    view_set_input_callback(view_stack->view, view_stack_input);
    view_set_context(view_stack->view, view_stack);
    view_set_enter_callback(view_stack->view, view_stack_enter);
    view_set_exit_callback(view_stack->view, view_stack_exit);
    return view_stack;
}

void view_stack_free(ViewStack* view_stack) {
    furi_assert(view_stack);

    ViewStackModel* model = view_get_model(view_stack->view);
    for(int i = 0; i < MAX_VIEWS; ++i) {
        if(model->views[i]) {
            view_set_update_callback(model->views[i], NULL);
            view_set_update_callback_context(model->views[i], NULL);
        }
    }
    view_commit_model(view_stack->view, false);

    view_free(view_stack->view);
    free(view_stack);
}

static void view_stack_draw(Canvas* canvas, void* _model) {
    furi_assert(_model);

    ViewStackModel* model = _model;
    for(int i = 0; i < MAX_VIEWS; ++i) {
        if(model->views[i]) {
            view_draw(model->views[i], canvas);
        }
    }
}

static bool view_stack_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    ViewStack* view_stack = context;

    bool consumed = false;
    ViewStackModel* model = view_get_model(view_stack->view);
    for(int i = MAX_VIEWS - 1; i >= 0; i--) {
        if(model->views[i] && view_input(model->views[i], event)) {
            consumed = true;
            break;
        }
    }
    view_commit_model(view_stack->view, false);

    return consumed;
}

void view_stack_add_view(ViewStack* view_stack, View* view) {
    furi_assert(view_stack);
    furi_assert(view);

    bool result = false;
    ViewStackModel* model = view_get_model(view_stack->view);
    for(int i = 0; i < MAX_VIEWS; ++i) {
        if(!model->views[i]) {
            model->views[i] = view;
            view_set_update_callback(model->views[i], view_stack_update_callback);
            view_set_update_callback_context(model->views[i], view_stack->view);
            if(view->enter_callback) {
                view->enter_callback(view->context);
            }
            result = true;
            break;
        }
    }
    view_commit_model(view_stack->view, result);
    furi_assert(result);
}

void view_stack_remove_view(ViewStack* view_stack, View* view) {
    furi_assert(view_stack);
    furi_assert(view);

    /* Removing view on-the-go is dangerous, but it is protected with
     * Locking model, so system is consistent at any time. */
    bool result = false;
    ViewStackModel* model = view_get_model(view_stack->view);
    for(int i = 0; i < MAX_VIEWS; ++i) {
        if(model->views[i] == view) {
            if(view->exit_callback) {
                view->exit_callback(view->context);
            }
            view_set_update_callback(model->views[i], NULL);
            view_set_update_callback_context(model->views[i], NULL);
            model->views[i] = NULL;
            result = true;
            break;
        }
    }
    view_commit_model(view_stack->view, result);
    furi_assert(result);
}

View* view_stack_get_view(ViewStack* view_stack) {
    furi_assert(view_stack);
    return view_stack->view;
}
