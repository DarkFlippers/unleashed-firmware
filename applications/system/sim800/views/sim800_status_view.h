#pragma once

#include <gui/view.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Sim800StatusView Sim800StatusView;

typedef void (*Sim800StatusViewCallback)(void* context);

Sim800StatusView* sim800_status_view_alloc(void);
void sim800_status_view_free(Sim800StatusView* instance);
View* sim800_status_view_get_view(Sim800StatusView* instance);

/** Called on OK, to refresh now instead of waiting for the next poll. */
void sim800_status_view_set_callback(
    Sim800StatusView* instance,
    Sim800StatusViewCallback callback,
    void* context);

void sim800_status_view_set_operator(Sim800StatusView* instance, const char* name);
void sim800_status_view_set_registration(Sim800StatusView* instance, const char* text);
void sim800_status_view_set_sim(Sim800StatusView* instance, const char* text);
void sim800_status_view_set_power(Sim800StatusView* instance, const char* text);
/** @param rssi AT+CSQ units, 0..31 or 99 for none. */
void sim800_status_view_set_signal(Sim800StatusView* instance, uint8_t rssi);
void sim800_status_view_set_linked(Sim800StatusView* instance, bool linked);
