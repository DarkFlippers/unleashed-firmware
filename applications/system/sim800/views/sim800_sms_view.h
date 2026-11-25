#pragma once

#include <gui/view.h>
#include "../helpers/sim800_sms.h"
#include "../plugins/sim800_sms_plugin.h"

typedef struct Sim800SmsView Sim800SmsView;

typedef enum {
    Sim800SmsViewActionSelect, //!< OK pressed on inbox item
    Sim800SmsViewActionLeft, //!< Left button (Reply in reader)
    Sim800SmsViewActionRight, //!< Right button (Refresh in inbox, Delete in reader)
} Sim800SmsViewAction;

typedef void (*Sim800SmsViewCallback)(Sim800SmsViewAction action, size_t index, void* context);

Sim800SmsView* sim800_sms_view_alloc(void);
void sim800_sms_view_free(Sim800SmsView* instance);
View* sim800_sms_view_get_view(Sim800SmsView* instance);

void sim800_sms_view_set_plugin(Sim800SmsView* instance, const Sim800SmsPlugin* plugin);
void sim800_sms_view_set_callback(
    Sim800SmsView* instance,
    Sim800SmsViewCallback callback,
    void* context);

void sim800_sms_view_set_inbox(
    Sim800SmsView* instance,
    const Sim800Inbox* inbox,
    size_t selected,
    const char* status_text);

void sim800_sms_view_set_reader(Sim800SmsView* instance, const Sim800SmsEntry* entry);

void sim800_sms_view_set_text(
    Sim800SmsView* instance,
    const char* title,
    const char* text,
    bool focus_end);
