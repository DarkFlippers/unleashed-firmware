#pragma once

#include <gui/view.h>

typedef void (*BadBtButtonCallback)(InputKey key, void* context);

typedef struct {
    View* view;
    BadBtButtonCallback callback;
    void* context;
} BadBt;

typedef struct BadBtState BadBtState;

BadBt* bad_bt_alloc();

void bad_bt_free(BadBt* bad_bt);

View* bad_bt_get_view(BadBt* bad_bt);

void bad_bt_set_button_callback(BadBt* bad_bt, BadBtButtonCallback callback, void* context);

void bad_bt_set_file_name(BadBt* bad_bt, const char* name);

void bad_bt_set_layout(BadBt* bad_bt, const char* layout);

void bad_bt_set_state(BadBt* bad_bt, BadBtState* st);

bool bad_bt_is_idle_state(BadBt* bad_bt);
