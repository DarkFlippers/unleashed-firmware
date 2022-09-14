#pragma once

#include <gui/view.h>

typedef struct U2fView U2fView;
typedef void (*U2fOkCallback)(InputType type, void* context);

typedef enum {
    U2fMsgNotConnected,
    U2fMsgIdle,
    U2fMsgRegister,
    U2fMsgAuth,
    U2fMsgSuccess,
    U2fMsgError,
} U2fViewMsg;

U2fView* u2f_view_alloc();

void u2f_view_free(U2fView* u2f);

View* u2f_view_get_view(U2fView* u2f);

void u2f_view_set_ok_callback(U2fView* u2f, U2fOkCallback callback, void* context);

void u2f_view_set_state(U2fView* u2f, U2fViewMsg msg);
