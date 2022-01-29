#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include "../desktop_i.h"
#include "desktop_first_start.h"

#define DESKTOP_FIRST_START_POWEROFF_SHORT 5000
#define DESKTOP_FIRST_START_POWEROFF_LONG (60 * 60 * 1000)

struct DesktopFirstStartView {
    View* view;
    DesktopFirstStartViewCallback callback;
    void* context;
    osTimerId_t timer;
};

typedef struct {
    uint8_t page;
} DesktopFirstStartViewModel;

static void desktop_first_start_draw(Canvas* canvas, void* model) {
    DesktopFirstStartViewModel* m = model;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    uint8_t width = canvas_width(canvas);
    uint8_t height = canvas_height(canvas);
    const char* my_name = furi_hal_version_get_name_ptr();
    if(m->page == 0) {
        canvas_draw_icon(canvas, 0, height - 51, &I_DolphinFirstStart0_70x53);
        elements_multiline_text_framed(
            canvas, 75, 16 + STATUS_BAR_Y_SHIFT, "Hey m8,\npress > to\ncontinue");
    } else if(m->page == 1) {
        canvas_draw_icon(canvas, 0, height - 51, &I_DolphinFirstStart1_59x53);
        elements_multiline_text_framed(
            canvas, 64, 16 + STATUS_BAR_Y_SHIFT, "First Of All,\n...      >");
    } else if(m->page == 2) {
        canvas_draw_icon(canvas, 0, height - 51, &I_DolphinFirstStart2_59x51);
        elements_multiline_text_framed(
            canvas, 64, 16 + STATUS_BAR_Y_SHIFT, "Thank you\nfor your\nsupport! >");
    } else if(m->page == 3) {
        canvas_draw_icon(canvas, width - 57, height - 45, &I_DolphinFirstStart3_57x48);
        elements_multiline_text_framed(
            canvas, 0, 16 + STATUS_BAR_Y_SHIFT, "Kickstarter\ncampaign\nwas INSANE! >");
    } else if(m->page == 4) {
        canvas_draw_icon(canvas, width - 67, height - 51, &I_DolphinFirstStart4_67x53);
        elements_multiline_text_framed(
            canvas, 0, 13 + STATUS_BAR_Y_SHIFT, "Now\nallow me\nto introduce\nmyself >");
    } else if(m->page == 5) {
        char buf[64];
        snprintf(
            buf,
            64,
            "%s %s%s",
            "I am",
            my_name ? my_name : "Unknown",
            ",\ncyberdolphin\nliving in your\npocket >");
        canvas_draw_icon(canvas, 0, height - 49, &I_DolphinFirstStart5_54x49);
        elements_multiline_text_framed(canvas, 60, 13 + STATUS_BAR_Y_SHIFT, buf);
    } else if(m->page == 6) {
        canvas_draw_icon(canvas, 0, height - 51, &I_DolphinFirstStart6_58x54);
        elements_multiline_text_framed(
            canvas,
            63,
            13 + STATUS_BAR_Y_SHIFT,
            "I can grow\nsmart'n'cool\nif you use me\noften >");
    } else if(m->page == 7) {
        canvas_draw_icon(canvas, width - 61, height - 51, &I_DolphinFirstStart7_61x51);
        elements_multiline_text_framed(
            canvas, 0, 13 + STATUS_BAR_Y_SHIFT, "As long as\nyou read, write\nand emulate >");
    } else if(m->page == 8) {
        canvas_draw_icon(canvas, width - 56, height - 51, &I_DolphinFirstStart8_56x51);
        elements_multiline_text_framed(
            canvas,
            0,
            13 + STATUS_BAR_Y_SHIFT,
            "You can check\nmy level and\nmood in the\nPassport menu");
    }
}

static bool desktop_first_start_input(InputEvent* event, void* context) {
    furi_assert(event);
    DesktopFirstStartView* instance = context;

    if(event->type == InputTypeShort) {
        DesktopFirstStartViewModel* model = view_get_model(instance->view);
        if(event->key == InputKeyLeft) {
            if(model->page > 0) model->page--;
        } else if(event->key == InputKeyRight) {
            uint32_t page = ++model->page;
            if(page > 8) {
                instance->callback(DesktopFirstStartCompleted, instance->context);
            }
        }
        view_commit_model(instance->view, true);
    }

    if(event->key == InputKeyOk) {
        if(event->type == InputTypePress) {
            osTimerStart(instance->timer, DESKTOP_FIRST_START_POWEROFF_SHORT);
        } else if(event->type == InputTypeRelease) {
            osTimerStop(instance->timer);
        }
    }

    return true;
}

static void desktop_first_start_timer_callback(void* context) {
    DesktopFirstStartView* instance = context;
    instance->callback(DesktopFirstStartPoweroff, instance->context);
}

static void desktop_first_start_enter(void* context) {
    DesktopFirstStartView* instance = context;

    furi_assert(instance->timer == NULL);
    instance->timer = osTimerNew(desktop_first_start_timer_callback, osTimerOnce, instance, NULL);

    osTimerStart(instance->timer, DESKTOP_FIRST_START_POWEROFF_LONG);
}

static void desktop_first_start_exit(void* context) {
    DesktopFirstStartView* instance = context;

    osTimerStop(instance->timer);
    osTimerDelete(instance->timer);
    instance->timer = NULL;
}

DesktopFirstStartView* desktop_first_start_alloc() {
    DesktopFirstStartView* instance = furi_alloc(sizeof(DesktopFirstStartView));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(DesktopFirstStartViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)desktop_first_start_draw);
    view_set_input_callback(instance->view, desktop_first_start_input);
    view_set_enter_callback(instance->view, desktop_first_start_enter);
    view_set_exit_callback(instance->view, desktop_first_start_exit);

    return instance;
}

void desktop_first_start_free(DesktopFirstStartView* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* desktop_first_start_get_view(DesktopFirstStartView* instance) {
    furi_assert(instance);
    return instance->view;
}

void desktop_first_start_set_callback(
    DesktopFirstStartView* instance,
    DesktopFirstStartViewCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);
    instance->callback = callback;
    instance->context = context;
}
