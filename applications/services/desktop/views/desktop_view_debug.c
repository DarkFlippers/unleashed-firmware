#include <toolbox/version.h>
#include <furi.h>
#include <furi_hal.h>
#include <dolphin/helpers/dolphin_state.h>
#include <dolphin/dolphin.h>

#include "../desktop_i.h"
#include "desktop_view_debug.h"

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
    UNUSED(model);
    canvas_clear(canvas);
    const Version* ver;
    char buffer[64];

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);

    uint32_t uptime = furi_get_tick() / furi_kernel_get_tick_frequency();
    snprintf(
        buffer,
        sizeof(buffer),
        "Uptime: %luh%lum%lus",
        uptime / 60 / 60,
        uptime / 60 % 60,
        uptime % 60);
    canvas_draw_str_aligned(canvas, 64, 1 + STATUS_BAR_Y_SHIFT, AlignCenter, AlignTop, buffer);

    canvas_set_font(canvas, FontSecondary);

    // Hardware version
    const char* my_name = furi_hal_version_get_name_ptr();
    snprintf(
        buffer,
        sizeof(buffer),
        "%d.F%dB%dC%d %s:%s %s",
        furi_hal_version_get_hw_version(),
        furi_hal_version_get_hw_target(),
        furi_hal_version_get_hw_body(),
        furi_hal_version_get_hw_connect(),
        furi_hal_version_get_hw_region_name(),
        furi_hal_region_get_name(),
        my_name ? my_name : "Unknown");
    canvas_draw_str(canvas, 0, 19 + STATUS_BAR_Y_SHIFT, buffer);

    ver = furi_hal_version_get_firmware_version();
    const BleGlueC2Info* c2_ver = NULL;
#ifdef SRV_BT
    c2_ver = ble_glue_get_c2_info();
#endif
    if(!ver) { //-V1051
        canvas_draw_str(canvas, 0, 30 + STATUS_BAR_Y_SHIFT, "No info");
        return;
    }

    snprintf(
        buffer, sizeof(buffer), "%s [%s]", version_get_version(ver), version_get_builddate(ver));
    canvas_draw_str(canvas, 0, 30 + STATUS_BAR_Y_SHIFT, buffer);

    uint16_t api_major, api_minor;
    furi_hal_info_get_api_version(&api_major, &api_minor);
    snprintf(
        buffer,
        sizeof(buffer),
        "%s%s [%d.%d] %s",
        version_get_dirty_flag(ver) ? "[!] " : "",
        version_get_githash(ver),
        api_major,
        api_minor,
        c2_ver ? c2_ver->StackTypeString : "<none>");
    canvas_draw_str(canvas, 0, 40 + STATUS_BAR_Y_SHIFT, buffer);

    snprintf(
        buffer, sizeof(buffer), "[%d] %s", version_get_target(ver), version_get_gitbranch(ver));
    canvas_draw_str(canvas, 0, 50 + STATUS_BAR_Y_SHIFT, buffer);
}

View* desktop_debug_get_view(DesktopDebugView* debug_view) {
    furi_assert(debug_view);
    return debug_view->view;
}

static bool desktop_debug_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    DesktopDebugView* debug_view = context;

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        debug_view->callback(DesktopDebugEventExit, debug_view->context);
    }

    return true;
}

static void desktop_debug_enter(void* context) {
    DesktopDebugView* debug_view = context;
    furi_timer_start(debug_view->timer, furi_ms_to_ticks(1000));
}

static void desktop_debug_exit(void* context) {
    DesktopDebugView* debug_view = context;
    furi_timer_stop(debug_view->timer);
}
void desktop_debug_timer(void* context) {
    DesktopDebugView* debug_view = context;
    view_get_model(debug_view->view);
    view_commit_model(debug_view->view, true);
}

DesktopDebugView* desktop_debug_alloc(void) {
    DesktopDebugView* debug_view = malloc(sizeof(DesktopDebugView));
    debug_view->view = view_alloc();
    debug_view->timer = furi_timer_alloc(desktop_debug_timer, FuriTimerTypePeriodic, debug_view);
    view_set_context(debug_view->view, debug_view);
    view_set_draw_callback(debug_view->view, (ViewDrawCallback)desktop_debug_render);
    view_set_input_callback(debug_view->view, desktop_debug_input);
    view_set_enter_callback(debug_view->view, desktop_debug_enter);
    view_set_exit_callback(debug_view->view, desktop_debug_exit);

    return debug_view;
}

void desktop_debug_free(DesktopDebugView* debug_view) {
    furi_assert(debug_view);

    furi_timer_free(debug_view->timer);
    view_free(debug_view->view);
    free(debug_view);
}
