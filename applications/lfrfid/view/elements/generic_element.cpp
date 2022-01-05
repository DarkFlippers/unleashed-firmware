#include "generic_element.h"

void GenericElement::lock_model() {
    furi_assert(view != nullptr);
    view_get_model(view);
}

void GenericElement::unlock_model(bool need_redraw) {
    furi_assert(view != nullptr);
    view_commit_model(view, need_redraw);
}

void GenericElement::set_parent_view(View* _view) {
    view = _view;
}
