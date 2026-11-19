#pragma once

#include <gui/view.h>
#include "../helpers/sim800_types.h"

typedef struct Sim800CallView Sim800CallView;

typedef enum {
    Sim800CallStateDialing,
    Sim800CallStateRinging, //!< the far end is alerting
    Sim800CallStateActive,
    Sim800CallStateIncoming,
    Sim800CallStateEnded,
    Sim800CallStateFailed, //!< busy, no answer, no network, module error
} Sim800CallState;

typedef enum {
    Sim800CallButtonLeft,
    Sim800CallButtonCenter,
    Sim800CallButtonRight,
} Sim800CallButton;

typedef void (*Sim800CallViewCallback)(Sim800CallButton button, void* context);

Sim800CallView* sim800_call_view_alloc(void);
void sim800_call_view_free(Sim800CallView* instance);
View* sim800_call_view_get_view(Sim800CallView* instance);

void sim800_call_view_set_callback(
    Sim800CallView* instance,
    Sim800CallViewCallback callback,
    void* context);

void sim800_call_view_set_number(Sim800CallView* instance, const char* number);
void sim800_call_view_set_state(Sim800CallView* instance, Sim800CallState state);
void sim800_call_view_set_detail(Sim800CallView* instance, const char* detail);
void sim800_call_view_set_operator(Sim800CallView* instance, const char* name);
void sim800_call_view_set_duration(Sim800CallView* instance, uint32_t seconds);
/** @param rssi AT+CSQ units, 0..31 or 99 for none. */
void sim800_call_view_set_rssi(Sim800CallView* instance, uint8_t rssi);
/** Hides the Save button once the number is saved. */
void sim800_call_view_set_saved(Sim800CallView* instance, bool saved);

Sim800CallState sim800_call_view_get_state(Sim800CallView* instance);
