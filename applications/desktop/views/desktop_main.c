#include <gui/gui_i.h>
#include <gui/view.h>
#include <gui/elements.h>
#include <gui/canvas.h>
#include <furi.h>
#include <input/input.h>
#include <dolphin/dolphin.h>

#include "../desktop_i.h"
#include "desktop_main.h"

struct DesktopMainView {
    View* view;
    DesktopMainViewCallback callback;
    void* context;
};

void desktop_main_set_callback(
    DesktopMainView* main_view,
    DesktopMainViewCallback callback,
    void* context) {
    furi_assert(main_view);
    furi_assert(callback);
    main_view->callback = callback;
    main_view->context = context;
}

View* desktop_main_get_view(DesktopMainView* main_view) {
    furi_assert(main_view);
    return main_view->view;
}

bool desktop_main_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    DesktopMainView* main_view = context;
    bool consumed = false;

    if(event->key == InputKeyOk && event->type == InputTypeShort) {
        main_view->callback(DesktopMainEventOpenMenu, main_view->context);
    } else if(event->key == InputKeyDown && event->type == InputTypeLong) {
        main_view->callback(DesktopMainEventOpenDebug, main_view->context);
    } else if(event->key == InputKeyUp && event->type == InputTypeShort) {
        main_view->callback(DesktopMainEventOpenLockMenu, main_view->context);
    } else if(event->key == InputKeyDown && event->type == InputTypeShort) {
        main_view->callback(DesktopMainEventOpenArchive, main_view->context);
    } else if(event->key == InputKeyLeft && event->type == InputTypeShort) {
        main_view->callback(DesktopMainEventOpenFavorite, main_view->context);
    } else if(event->key == InputKeyRight && event->type == InputTypeShort) {
        main_view->callback(DesktopMainEventRightShort, main_view->context);
    } else if(event->key == InputKeyBack && event->type == InputTypeShort) {
        consumed = true;
    }

    return consumed;
}

DesktopMainView* desktop_main_alloc() {
    DesktopMainView* main_view = furi_alloc(sizeof(DesktopMainView));

    main_view->view = view_alloc();
    view_allocate_model(main_view->view, ViewModelTypeLockFree, 1);
    view_set_context(main_view->view, main_view);
    view_set_input_callback(main_view->view, desktop_main_input);

    return main_view;
}

void desktop_main_free(DesktopMainView* main_view) {
    furi_assert(main_view);
    view_free(main_view->view);
    free(main_view);
}
