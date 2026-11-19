#pragma once

#include <gui/view.h>

typedef struct Sim800DtmfView Sim800DtmfView;

/** @param digit one of 0-9, * or #. */
typedef void (*Sim800DtmfViewCallback)(char digit, void* context);

Sim800DtmfView* sim800_dtmf_view_alloc(void);
void sim800_dtmf_view_free(Sim800DtmfView* instance);
View* sim800_dtmf_view_get_view(Sim800DtmfView* instance);

void sim800_dtmf_view_set_callback(
    Sim800DtmfView* instance,
    Sim800DtmfViewCallback callback,
    void* context);

/** Digits sent so far, shown back so the user can see what the far end got. */
void sim800_dtmf_view_set_sent(Sim800DtmfView* instance, const char* sent);
