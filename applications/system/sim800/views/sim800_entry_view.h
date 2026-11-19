#pragma once

#include <gui/view.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Sim800EntryView Sim800EntryView;

typedef enum {
    Sim800EntryButtonSms,
    Sim800EntryButtonCall,
    Sim800EntryButtonEdit,
} Sim800EntryButton;

typedef void (*Sim800EntryViewCallback)(Sim800EntryButton button, void* context);

Sim800EntryView* sim800_entry_view_alloc(void);
void sim800_entry_view_free(Sim800EntryView* instance);
View* sim800_entry_view_get_view(Sim800EntryView* instance);

void sim800_entry_view_set_callback(
    Sim800EntryView* instance,
    Sim800EntryViewCallback callback,
    void* context);

void sim800_entry_view_set_entry(Sim800EntryView* instance, const char* name, const char* number);
void sim800_entry_view_set_operator(Sim800EntryView* instance, const char* name);
/** AT+CSQ units, 0..31 or 99 for none. */
void sim800_entry_view_set_rssi(Sim800EntryView* instance, uint8_t rssi);
/** Center button becomes Re-dial once a call to this number has ended. */
void sim800_entry_view_set_redial(Sim800EntryView* instance, bool redial);
