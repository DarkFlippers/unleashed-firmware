#include "popup.h"
#include <gui/elements.h>
#include <furi.h>

struct Popup {
    View* view;
    void* context;
    PopupCallback callback;

    osTimerId_t timer;
    uint32_t timer_period_in_ms;
    bool timer_enabled;
};

typedef struct {
    const char* text;
    uint8_t x;
    uint8_t y;
    Align horizontal;
    Align vertical;
} TextElement;

typedef struct {
    int8_t x;
    int8_t y;
    IconName name;
} IconElement;

typedef struct {
    TextElement header;
    TextElement text;
    IconElement icon;
} PopupModel;

static void popup_view_draw_callback(Canvas* canvas, void* _model) {
    PopupModel* model = _model;

    // Prepare canvas
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // TODO other criteria for the draw
    if(model->icon.x >= 0 && model->icon.y >= 0) {
        canvas_draw_icon_name(canvas, model->icon.x, model->icon.y, model->icon.name);
    }

    // Draw header
    if(model->header.text != NULL) {
        canvas_set_font(canvas, FontPrimary);
        elements_multiline_text_aligned(
            canvas,
            model->header.x,
            model->header.y,
            model->header.horizontal,
            model->header.vertical,
            model->header.text);
    }

    // Draw text
    if(model->text.text != NULL) {
        canvas_set_font(canvas, FontSecondary);
        elements_multiline_text_aligned(
            canvas,
            model->text.x,
            model->text.y,
            model->text.horizontal,
            model->text.vertical,
            model->text.text);
    }
}

static void popup_timer_callback(void* context) {
    furi_assert(context);
    Popup* popup = context;

    if(popup->callback) {
        popup->callback(popup->context);
    }
}

static bool popup_view_input_callback(InputEvent* event, void* context) {
    Popup* popup = context;
    bool consumed = false;

    // Process key presses only
    if(event->type == InputTypeShort && popup->callback) {
        popup->callback(popup->context);
        consumed = true;
    }

    return consumed;
}

void popup_start_timer(void* context) {
    Popup* popup = context;
    if(popup->timer_enabled) {
        uint32_t timer_period = popup->timer_period_in_ms / (1000.0f / osKernelGetTickFreq());
        if(timer_period == 0) timer_period = 1;

        if(osTimerStart(popup->timer, timer_period) != osOK) {
            furi_assert(0);
        };
    }
}

void popup_stop_timer(void* context) {
    Popup* popup = context;
    osTimerStop(popup->timer);
}

Popup* popup_alloc() {
    Popup* popup = furi_alloc(sizeof(Popup));
    popup->view = view_alloc();
    popup->timer = osTimerNew(popup_timer_callback, osTimerOnce, popup, NULL);
    furi_assert(popup->timer);
    popup->timer_period_in_ms = 1000;
    popup->timer_enabled = false;

    view_set_context(popup->view, popup);
    view_allocate_model(popup->view, ViewModelTypeLockFree, sizeof(PopupModel));
    view_set_draw_callback(popup->view, popup_view_draw_callback);
    view_set_input_callback(popup->view, popup_view_input_callback);
    view_set_enter_callback(popup->view, popup_start_timer);
    view_set_exit_callback(popup->view, popup_stop_timer);

    with_view_model(
        popup->view, (PopupModel * model) {
            model->header.text = NULL;
            model->header.x = 0;
            model->header.y = 0;
            model->header.horizontal = AlignLeft;
            model->header.vertical = AlignBottom;

            model->text.text = NULL;
            model->text.x = 0;
            model->text.y = 0;
            model->text.horizontal = AlignLeft;
            model->text.vertical = AlignBottom;

            // TODO other criteria for the draw
            model->icon.x = -1;
            model->icon.y = -1;
            model->icon.name = I_ButtonCenter_7x7;
            return true;
        });
    return popup;
}

void popup_free(Popup* popup) {
    furi_assert(popup);
    osTimerDelete(popup->timer);
    view_free(popup->view);
    free(popup);
}

View* popup_get_view(Popup* popup) {
    furi_assert(popup);
    return popup->view;
}

void popup_set_callback(Popup* popup, PopupCallback callback) {
    furi_assert(popup);
    popup->callback = callback;
}

void popup_set_context(Popup* popup, void* context) {
    furi_assert(popup);
    popup->context = context;
}

void popup_set_header(
    Popup* popup,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical) {
    furi_assert(popup);
    with_view_model(
        popup->view, (PopupModel * model) {
            model->header.text = text;
            model->header.x = x;
            model->header.y = y;
            model->header.horizontal = horizontal;
            model->header.vertical = vertical;
            return true;
        });
}

void popup_set_text(
    Popup* popup,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical) {
    furi_assert(popup);
    with_view_model(
        popup->view, (PopupModel * model) {
            model->text.text = text;
            model->text.x = x;
            model->text.y = y;
            model->text.horizontal = horizontal;
            model->text.vertical = vertical;
            return true;
        });
}

void popup_set_icon(Popup* popup, int8_t x, int8_t y, IconName name) {
    furi_assert(popup);
    with_view_model(
        popup->view, (PopupModel * model) {
            model->icon.x = x;
            model->icon.y = y;
            model->icon.name = name;
            return true;
        });
}

void popup_set_timeout(Popup* popup, uint32_t timeout_in_ms) {
    furi_assert(popup);
    popup->timer_period_in_ms = timeout_in_ms;
}

void popup_enable_timeout(Popup* popup) {
    popup->timer_enabled = true;
}

void popup_disable_timeout(Popup* popup) {
    popup->timer_enabled = false;
}