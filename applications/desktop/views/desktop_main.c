#include "gui/canvas.h"
#include "input/input.h"
#include <furi.h>
#include "../desktop_i.h"
#include "desktop_main.h"

void desktop_main_set_callback(
    DesktopMainView* main_view,
    DesktopMainViewCallback callback,
    void* context) {
    furi_assert(main_view);
    furi_assert(callback);
    main_view->callback = callback;
    main_view->context = context;
}

void desktop_main_reset_hint(DesktopMainView* main_view) {
    with_view_model(
        main_view->view, (DesktopMainViewModel * model) {
            model->hint_expire_at = 0;
            return true;
        });
}

void desktop_main_switch_dolphin_animation(
    DesktopMainView* main_view,
    const Icon* icon,
    bool status_bar_background_black) {
    with_view_model(
        main_view->view, (DesktopMainViewModel * model) {
            if(model->animation) icon_animation_free(model->animation);
            model->animation = icon_animation_alloc(icon);
            view_tie_icon_animation(main_view->view, model->animation);
            icon_animation_start(model->animation);
            model->icon = NULL;
            model->status_bar_background_black = status_bar_background_black;
            return true;
        });
}

void desktop_main_switch_dolphin_icon(DesktopMainView* main_view, const Icon* icon) {
    with_view_model(
        main_view->view, (DesktopMainViewModel * model) {
            if(model->animation) icon_animation_free(model->animation);
            model->animation = NULL;
            model->icon = icon;
            return true;
        });
}

void desktop_main_render(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    DesktopMainViewModel* m = model;
    uint32_t now = osKernelGetTickCount();

    if(m->status_bar_background_black) {
        canvas_draw_box(canvas, 0, 0, GUI_STATUS_BAR_WIDTH, GUI_STATUS_BAR_HEIGHT);
    }
    if(m->icon) {
        canvas_draw_icon(canvas, 0, 0 + STATUS_BAR_Y_SHIFT, m->icon);
    } else if(m->animation) {
        canvas_draw_icon_animation(canvas, 0, 0 + STATUS_BAR_Y_SHIFT, m->animation);
    }

    if(now < m->hint_expire_at) {
        canvas_set_font(canvas, FontPrimary);
        elements_multiline_text_framed(canvas, 42, 30 + STATUS_BAR_Y_SHIFT, "Unlocked");
    }
}

View* desktop_main_get_view(DesktopMainView* main_view) {
    furi_assert(main_view);
    return main_view->view;
}

bool desktop_main_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    DesktopMainView* main_view = context;

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
    }

    desktop_main_reset_hint(main_view);

    return true;
}

void desktop_main_enter(void* context) {
    DesktopMainView* main_view = context;

    with_view_model(
        main_view->view, (DesktopMainViewModel * model) {
            if(model->animation) icon_animation_start(model->animation);
            return false;
        });
}

void desktop_main_exit(void* context) {
    DesktopMainView* main_view = context;
    with_view_model(
        main_view->view, (DesktopMainViewModel * model) {
            if(model->animation) icon_animation_stop(model->animation);
            return false;
        });
}

DesktopMainView* desktop_main_alloc() {
    DesktopMainView* main_view = furi_alloc(sizeof(DesktopMainView));
    main_view->view = view_alloc();
    view_allocate_model(main_view->view, ViewModelTypeLocking, sizeof(DesktopMainViewModel));
    view_set_context(main_view->view, main_view);
    view_set_draw_callback(main_view->view, (ViewDrawCallback)desktop_main_render);
    view_set_input_callback(main_view->view, desktop_main_input);
    view_set_enter_callback(main_view->view, desktop_main_enter);
    view_set_exit_callback(main_view->view, desktop_main_exit);

    return main_view;
}

void desktop_main_free(DesktopMainView* main_view) {
    furi_assert(main_view);
    view_free(main_view->view);
    free(main_view);
}

void desktop_main_unlocked(DesktopMainView* main_view) {
    with_view_model(
        main_view->view, (DesktopMainViewModel * model) {
            model->hint_expire_at = osKernelGetTickCount() + osKernelGetTickFreq();
            return true;
        });
}
