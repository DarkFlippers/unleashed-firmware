#include <furi.h>
#include "../desktop_i.h"
#include "desktop_main.h"

static const Icon* idle_scenes[] = {&A_Wink_128x64, &A_WatchingTV_128x64};

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
            model->hint_timeout = 0;
            return true;
        });
}
// temporary main screen animation managment
void desktop_scene_handler_set_scene(DesktopMainView* main_view, const Icon* icon_data) {
    with_view_model(
        main_view->view, (DesktopMainViewModel * model) {
            if(model->animation) icon_animation_free(model->animation);
            model->animation = icon_animation_alloc(icon_data);
            icon_animation_start(model->animation);
            return true;
        });
}

void desktop_scene_handler_switch_scene(DesktopMainView* main_view) {
    with_view_model(
        main_view->view, (DesktopMainViewModel * model) {
            if(icon_animation_is_last_frame(model->animation)) {
                if(model->animation) icon_animation_free(model->animation);
                model->animation = icon_animation_alloc(idle_scenes[model->scene_num]);
                icon_animation_start(model->animation);
                model->scene_num = random() % COUNT_OF(idle_scenes);
            }
            return true;
        });
}

void desktop_main_render(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    DesktopMainViewModel* m = model;

    if(m->animation) {
        canvas_draw_icon_animation(canvas, 0, -3, m->animation);
    }

    if(m->unlocked && m->hint_timeout) {
        m->hint_timeout = CLAMP(m->hint_timeout - 1, 2, 0);
        canvas_set_font(canvas, FontPrimary);
        elements_multiline_text_framed(canvas, 42, 30, "Unlocked");
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
    }
    desktop_main_reset_hint(main_view);

    return true;
}

DesktopMainView* desktop_main_alloc() {
    DesktopMainView* main_view = furi_alloc(sizeof(DesktopMainView));
    main_view->view = view_alloc();
    view_allocate_model(main_view->view, ViewModelTypeLocking, sizeof(DesktopMainViewModel));
    view_set_context(main_view->view, main_view);
    view_set_draw_callback(main_view->view, (ViewDrawCallback)desktop_main_render);
    view_set_input_callback(main_view->view, desktop_main_input);

    desktop_scene_handler_set_scene(main_view, idle_scenes[random() % COUNT_OF(idle_scenes)]);

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
            model->unlocked = true;
            model->hint_timeout = 2;
            return true;
        });
}
