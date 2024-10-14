#pragma once

#include <gui/view.h>
#include "../helpers/ducky_script.h"

typedef struct BadBle BadBle;
typedef void (*BadBleButtonCallback)(InputKey key, void* context);

BadBle* bad_ble_view_alloc(void);

void bad_ble_view_free(BadBle* bad_ble);

View* bad_ble_view_get_view(BadBle* bad_ble);

void bad_ble_view_set_button_callback(
    BadBle* bad_ble,
    BadBleButtonCallback callback,
    void* context);

void bad_ble_view_set_file_name(BadBle* bad_ble, const char* name);

void bad_ble_view_set_layout(BadBle* bad_ble, const char* layout);

void bad_ble_view_set_state(BadBle* bad_ble, BadBleState* st);

bool bad_ble_view_is_idle_state(BadBle* bad_ble);
