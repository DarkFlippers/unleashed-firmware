#include <furi.h>
#include "../desktop_i.h"
#include "desktop_debug.h"

#include "dolphin/helpers/dolphin_state.h"
#include "dolphin/dolphin.h"

void desktop_debug_set_callback(
    DesktopDebugView* debug_view,
    DesktopDebugViewCallback callback,
    void* context) {
    furi_assert(debug_view);
    furi_assert(callback);
    debug_view->callback = callback;
    debug_view->context = context;
}

void desktop_debug_render(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    DesktopDebugViewModel* m = model;
    const Version* ver;
    char buffer[64];

    static const char* headers[] = {"FW Version info:", "Boot Version info:", "Dolphin info:"};

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 9 + STATUS_BAR_Y_SHIFT, headers[m->screen]);
    canvas_set_font(canvas, FontSecondary);

    if(m->screen != DesktopViewStatsMeta) {
        // Hardware version
        const char* my_name = furi_hal_version_get_name_ptr();
        snprintf(
            buffer,
            sizeof(buffer),
            "HW: %d.F%dB%dC%d %s",
            furi_hal_version_get_hw_version(),
            furi_hal_version_get_hw_target(),
            furi_hal_version_get_hw_body(),
            furi_hal_version_get_hw_connect(),
            my_name ? my_name : "Unknown");
        canvas_draw_str(canvas, 5, 19 + STATUS_BAR_Y_SHIFT, buffer);

        ver = m->screen == DesktopViewStatsBoot ? furi_hal_version_get_bootloader_version() :
                                                  furi_hal_version_get_firmware_version();

        if(!ver) {
            canvas_draw_str(canvas, 5, 29 + STATUS_BAR_Y_SHIFT, "No info");
            return;
        }

        snprintf(
            buffer,
            sizeof(buffer),
            "%s [%s]",
            version_get_version(ver),
            version_get_builddate(ver));
        canvas_draw_str(canvas, 5, 28 + STATUS_BAR_Y_SHIFT, buffer);

        snprintf(
            buffer,
            sizeof(buffer),
            "%s [%s]",
            version_get_githash(ver),
            version_get_gitbranchnum(ver));
        canvas_draw_str(canvas, 5, 39 + STATUS_BAR_Y_SHIFT, buffer);

        snprintf(
            buffer, sizeof(buffer), "[%d] %s", version_get_target(ver), version_get_gitbranch(ver));
        canvas_draw_str(canvas, 5, 50 + STATUS_BAR_Y_SHIFT, buffer);

    } else {
        char buffer[64];
        Dolphin* dolphin = furi_record_open("dolphin");
        DolphinStats stats = dolphin_stats(dolphin);
        furi_record_close("dolphin");

        uint32_t current_lvl = stats.level;
        uint32_t remaining = dolphin_state_xp_to_levelup(m->icounter);

        canvas_set_font(canvas, FontSecondary);
        snprintf(buffer, 64, "Icounter: %ld  Butthurt %ld", m->icounter, m->butthurt);
        canvas_draw_str(canvas, 5, 19 + STATUS_BAR_Y_SHIFT, buffer);

        snprintf(
            buffer,
            64,
            "Level: %ld  To level up: %ld",
            current_lvl,
            (remaining == (uint32_t)(-1) ? remaining : 0));
        canvas_draw_str(canvas, 5, 29 + STATUS_BAR_Y_SHIFT, buffer);

        snprintf(buffer, 64, "%s", asctime(localtime((const time_t*)&m->timestamp)));
        canvas_draw_str(canvas, 5, 39 + STATUS_BAR_Y_SHIFT, buffer);
        canvas_draw_str(canvas, 0, 49 + STATUS_BAR_Y_SHIFT, "[< >] icounter value   [ok] save");
    }
}

View* desktop_debug_get_view(DesktopDebugView* debug_view) {
    furi_assert(debug_view);
    return debug_view->view;
}

bool desktop_debug_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    DesktopDebugView* debug_view = context;

    if(event->type != InputTypeShort) return false;
    DesktopViewStatsScreens current = 0;
    with_view_model(
        debug_view->view, (DesktopDebugViewModel * model) {
#if SRV_DOLPHIN_STATE_DEBUG == 1
            if(event->key == InputKeyDown) {
                model->screen = (model->screen + 1) % DesktopViewStatsTotalCount;
            } else if(event->key == InputKeyUp) {
                model->screen = ((model->screen - 1) + DesktopViewStatsTotalCount) %
                                DesktopViewStatsTotalCount;
            }
#else
            if((event->key == InputKeyDown) || (event->key == InputKeyUp)) {
                model->screen = !model->screen;
            }
#endif
            current = model->screen;
            return true;
        });

    if(current == DesktopViewStatsMeta) {
        if(event->key == InputKeyLeft) {
            debug_view->callback(DesktopDebugEventWrongDeed, debug_view->context);
        } else if(event->key == InputKeyRight) {
            debug_view->callback(DesktopDebugEventDeed, debug_view->context);
        } else if(event->key == InputKeyOk) {
            debug_view->callback(DesktopDebugEventSaveState, debug_view->context);
        } else {
            return false;
        }
    }

    if(event->key == InputKeyBack) {
        debug_view->callback(DesktopDebugEventExit, debug_view->context);
    }

    return true;
}

DesktopDebugView* desktop_debug_alloc() {
    DesktopDebugView* debug_view = furi_alloc(sizeof(DesktopDebugView));
    debug_view->view = view_alloc();
    view_allocate_model(debug_view->view, ViewModelTypeLocking, sizeof(DesktopDebugViewModel));
    view_set_context(debug_view->view, debug_view);
    view_set_draw_callback(debug_view->view, (ViewDrawCallback)desktop_debug_render);
    view_set_input_callback(debug_view->view, desktop_debug_input);

    return debug_view;
}

void desktop_debug_free(DesktopDebugView* debug_view) {
    furi_assert(debug_view);

    view_free(debug_view->view);
    free(debug_view);
}

void desktop_debug_get_dolphin_data(DesktopDebugView* debug_view) {
    Dolphin* dolphin = furi_record_open("dolphin");
    DolphinStats stats = dolphin_stats(dolphin);
    with_view_model(
        debug_view->view, (DesktopDebugViewModel * model) {
            model->icounter = stats.icounter;
            model->butthurt = stats.butthurt;
            model->timestamp = stats.timestamp;
            return true;
        });

    furi_record_close("dolphin");
}

void desktop_debug_reset_screen_idx(DesktopDebugView* debug_view) {
    with_view_model(
        debug_view->view, (DesktopDebugViewModel * model) {
            model->screen = 0;
            return true;
        });
}
