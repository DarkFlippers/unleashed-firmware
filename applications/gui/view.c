#include "view_i.h"

View* view_alloc() {
    View* view = furi_alloc(sizeof(View));
    return view;
}

void view_free(View* view) {
    furi_assert(view);
    view_free_model(view);
    free(view);
}

void view_set_dispatcher(View* view, ViewDispatcher* view_dispatcher) {
    furi_assert(view);
    furi_assert(view_dispatcher);
    furi_assert(view->dispatcher == NULL);
    view->dispatcher = view_dispatcher;
}

void view_set_draw_callback(View* view, ViewDrawCallback callback) {
    furi_assert(view);
    furi_assert(view->draw_callback == NULL);
    view->draw_callback = callback;
}

void view_set_input_callback(View* view, ViewInputCallback callback) {
    furi_assert(view);
    furi_assert(view->input_callback == NULL);
    view->input_callback = callback;
}

void view_set_previous_callback(View* view, ViewNavigationCallback callback) {
    furi_assert(view);
    view->previous_callback = callback;
}

void view_set_next_callback(View* view, ViewNavigationCallback callback) {
    furi_assert(view);
    view->next_callback = callback;
}

void view_set_context(View* view, void* context) {
    furi_assert(view);
    furi_assert(context);
    view->context = context;
}

void view_allocate_model(View* view, ViewModelType type, size_t size) {
    furi_assert(view);
    furi_assert(size > 0);
    furi_assert(view->model_type == ViewModelTypeNone);
    furi_assert(view->model == NULL);
    view->model_type = type;
    if(view->model_type == ViewModelTypeLockFree) {
        view->model = furi_alloc(size);
    } else if(view->model_type == ViewModelTypeLocking) {
        ViewModelLocking* model = furi_alloc(sizeof(ViewModelLocking));
        model->mutex = osMutexNew(NULL);
        furi_check(model->mutex);
        model->data = furi_alloc(size);
        view->model = model;
    } else {
        furi_assert(false);
    }
}

void view_free_model(View* view) {
    furi_assert(view);
    if(view->model_type == ViewModelTypeNone) {
        return;
    } else if(view->model_type == ViewModelTypeLockFree) {
        free(view->model);
    } else if(view->model_type == ViewModelTypeLocking) {
        ViewModelLocking* model = view->model;
        furi_check(osMutexDelete(model->mutex) == osOK);
        free(model->data);
        free(model);
        view->model = NULL;
    } else {
        furi_assert(false);
    }
}

void* view_get_model(View* view) {
    furi_assert(view);
    if(view->model_type == ViewModelTypeLocking) {
        ViewModelLocking* model = (ViewModelLocking*)(view->model);
        furi_check(osMutexAcquire(model->mutex, osWaitForever) == osOK);
        return model->data;
    }
    return view->model;
}

void view_commit_model(View* view) {
    furi_assert(view);
    view_unlock_model(view);
    if(view->dispatcher) {
        view_dispatcher_update(view->dispatcher, view);
    }
}

void view_unlock_model(View* view) {
    furi_assert(view);
    if(view->model_type == ViewModelTypeLocking) {
        ViewModelLocking* model = (ViewModelLocking*)(view->model);
        furi_check(osMutexRelease(model->mutex) == osOK);
    }
}

void view_draw(View* view, Canvas* canvas) {
    furi_assert(view);
    if(view->draw_callback) {
        void* data = view_get_model(view);
        view->draw_callback(canvas, data);
        view_unlock_model(view);
    }
}

bool view_input(View* view, InputEvent* event) {
    furi_assert(view);
    if(view->input_callback) {
        return view->input_callback(event, view->context);
    } else {
        return false;
    }
}

uint32_t view_previous(View* view) {
    furi_assert(view);
    if(view->previous_callback) {
        return view->previous_callback(view->context);
    } else {
        return VIEW_IGNORE;
    }
}

uint32_t view_next(View* view) {
    furi_assert(view);
    if(view->next_callback) {
        return view->next_callback(view->context);
    } else {
        return VIEW_IGNORE;
    }
}
