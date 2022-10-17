#pragma once

#include <gui/view.h>
#include "dtmf_dolphin_common.h"

typedef struct DTMFDolphinDialer DTMFDolphinDialer;
typedef void (*DTMFDolphinDialerOkCallback)(InputType type, void* context);

DTMFDolphinDialer* dtmf_dolphin_dialer_alloc();

void dtmf_dolphin_dialer_free(DTMFDolphinDialer* dtmf_dolphin_dialer);

View* dtmf_dolphin_dialer_get_view(DTMFDolphinDialer* dtmf_dolphin_dialer);

void dtmf_dolphin_dialer_set_ok_callback(
    DTMFDolphinDialer* dtmf_dolphin_dialer,
    DTMFDolphinDialerOkCallback callback,
    void* context);
