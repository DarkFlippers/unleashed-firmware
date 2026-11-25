#include "sim800_sms_view.h"

#include <furi.h>
#include <string.h>

#define SMS_READER_VISIBLE_LINES 4
#define SMS_TEXT_VISIBLE_LINES   5

typedef enum {
    Sim800SmsViewModeInbox,
    Sim800SmsViewModeReader,
    Sim800SmsViewModeText,
} Sim800SmsViewMode;

typedef struct {
    Sim800SmsViewMode mode;
    const Sim800SmsPlugin* plugin;
    const Sim800Inbox* inbox;
    const Sim800SmsEntry* entry;
    const char* text;
    char title[32];
    size_t selected;
    uint16_t scroll_line;
    uint16_t total_lines;
    char status_text[64];
} Sim800SmsViewModel;

struct Sim800SmsView {
    View* view;
    Sim800SmsViewCallback callback;
    void* context;
};

static void sim800_sms_view_draw(Canvas* canvas, void* _model) {
    Sim800SmsViewModel* model = _model;
    if(!model->plugin) {
        canvas_clear(canvas);
        return;
    }

    if(model->mode == Sim800SmsViewModeInbox) {
        model->plugin->draw_inbox(canvas, model->inbox, model->selected, model->status_text);
    } else if(model->mode == Sim800SmsViewModeReader && model->entry) {
        model->total_lines = model->plugin->draw_reader(
            canvas,
            furi_string_get_cstr(model->entry->from),
            furi_string_get_cstr(model->entry->date),
            furi_string_get_cstr(model->entry->body),
            model->scroll_line);
    } else if(model->mode == Sim800SmsViewModeText) {
        model->total_lines =
            model->plugin->draw_text(canvas, model->title, model->text, model->scroll_line);
    }
}

static bool sim800_sms_view_input(InputEvent* event, void* context) {
    Sim800SmsView* instance = context;

    bool is_nav = (event->key == InputKeyUp || event->key == InputKeyDown);
    if(is_nav) {
        if(event->type != InputTypePress && event->type != InputTypeRepeat) return false;
    } else {
        if(event->type != InputTypeShort) return false;
    }

    bool consumed = false;
    bool fire = false;
    Sim800SmsViewAction action = Sim800SmsViewActionSelect;
    size_t selected = 0;

    with_view_model(
        instance->view,
        Sim800SmsViewModel * model,
        {
            if(model->mode == Sim800SmsViewModeInbox) {
                size_t count = model->inbox ? model->inbox->count : 0;
                if(event->key == InputKeyUp && count > 0) {
                    model->selected = (model->selected == 0) ? (count - 1) : (model->selected - 1);
                    consumed = true;
                } else if(event->key == InputKeyDown && count > 0) {
                    model->selected = (model->selected + 1 >= count) ? 0 : (model->selected + 1);
                    consumed = true;
                } else if(event->key == InputKeyOk && count > 0) {
                    action = Sim800SmsViewActionSelect;
                    selected = model->selected;
                    fire = true;
                    consumed = true;
                } else if(event->key == InputKeyRight) {
                    action = Sim800SmsViewActionRight;
                    fire = true;
                    consumed = true;
                }
            } else if(model->mode == Sim800SmsViewModeReader) {
                uint16_t max_scroll = (model->total_lines > SMS_READER_VISIBLE_LINES) ?
                                          (model->total_lines - SMS_READER_VISIBLE_LINES) :
                                          0;
                if(event->key == InputKeyUp) {
                    if(model->scroll_line > 0) model->scroll_line--;
                    consumed = true;
                } else if(event->key == InputKeyDown) {
                    if(model->scroll_line < max_scroll) model->scroll_line++;
                    consumed = true;
                } else if(event->key == InputKeyLeft) {
                    action = Sim800SmsViewActionLeft;
                    fire = true;
                    consumed = true;
                } else if(event->key == InputKeyRight) {
                    action = Sim800SmsViewActionRight;
                    fire = true;
                    consumed = true;
                }
            } else if(model->mode == Sim800SmsViewModeText) {
                uint16_t max_scroll = (model->total_lines > SMS_TEXT_VISIBLE_LINES) ?
                                          (model->total_lines - SMS_TEXT_VISIBLE_LINES) :
                                          0;
                if(event->key == InputKeyUp) {
                    if(model->scroll_line > 0) model->scroll_line--;
                    consumed = true;
                } else if(event->key == InputKeyDown) {
                    if(model->scroll_line < max_scroll) model->scroll_line++;
                    consumed = true;
                }
            }
        },
        consumed);

    if(fire && instance->callback) {
        instance->callback(action, selected, instance->context);
    }

    return consumed;
}

Sim800SmsView* sim800_sms_view_alloc(void) {
    Sim800SmsView* instance = malloc(sizeof(Sim800SmsView));
    instance->view = view_alloc();
    instance->callback = NULL;
    instance->context = NULL;

    view_set_context(instance->view, instance);
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(Sim800SmsViewModel));
    view_set_draw_callback(instance->view, sim800_sms_view_draw);
    view_set_input_callback(instance->view, sim800_sms_view_input);

    with_view_model(
        instance->view,
        Sim800SmsViewModel * model,
        {
            memset(model, 0, sizeof(Sim800SmsViewModel));
            model->mode = Sim800SmsViewModeInbox;
        },
        false);

    return instance;
}

void sim800_sms_view_free(Sim800SmsView* instance) {
    furi_assert(instance);
    view_free(instance->view);
    free(instance);
}

View* sim800_sms_view_get_view(Sim800SmsView* instance) {
    furi_assert(instance);
    return instance->view;
}

void sim800_sms_view_set_plugin(Sim800SmsView* instance, const Sim800SmsPlugin* plugin) {
    furi_assert(instance);
    with_view_model(
        instance->view, Sim800SmsViewModel * model, { model->plugin = plugin; }, false);
}

void sim800_sms_view_set_callback(
    Sim800SmsView* instance,
    Sim800SmsViewCallback callback,
    void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->context = context;
}

void sim800_sms_view_set_inbox(
    Sim800SmsView* instance,
    const Sim800Inbox* inbox,
    size_t selected,
    const char* status_text) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        Sim800SmsViewModel * model,
        {
            model->mode = Sim800SmsViewModeInbox;
            model->inbox = inbox;
            model->entry = NULL;
            model->text = NULL;
            model->selected = (inbox && inbox->count > 0 && selected < inbox->count) ? selected : 0;
            model->scroll_line = 0;
            model->total_lines = 0;
            strlcpy(
                model->status_text,
                status_text ? status_text : "",
                sizeof(model->status_text));
        },
        true);
}

void sim800_sms_view_set_reader(Sim800SmsView* instance, const Sim800SmsEntry* entry) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        Sim800SmsViewModel * model,
        {
            model->mode = Sim800SmsViewModeReader;
            model->entry = entry;
            model->text = NULL;
            model->scroll_line = 0;
            model->total_lines = (model->plugin && entry) ?
                                     model->plugin->draw_reader(
                                         NULL,
                                         furi_string_get_cstr(entry->from),
                                         furi_string_get_cstr(entry->date),
                                         furi_string_get_cstr(entry->body),
                                         0) :
                                     1;
        },
        true);
}

void sim800_sms_view_set_text(
    Sim800SmsView* instance,
    const char* title,
    const char* text,
    bool focus_end) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        Sim800SmsViewModel * model,
        {
            model->mode = Sim800SmsViewModeText;
            model->entry = NULL;
            model->text = text;
            strlcpy(model->title, title ? title : "", sizeof(model->title));
            model->total_lines =
                model->plugin ? model->plugin->draw_text(NULL, model->title, text, 0) : 1;
            if(focus_end && model->total_lines > SMS_TEXT_VISIBLE_LINES) {
                model->scroll_line = model->total_lines - SMS_TEXT_VISIBLE_LINES;
            } else {
                model->scroll_line = 0;
            }
        },
        true);
}
