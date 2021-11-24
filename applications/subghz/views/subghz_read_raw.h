#pragma once

#include <gui/view.h>
#include "../helpers/subghz_custom_event.h"

typedef struct SubghzReadRAW SubghzReadRAW;

typedef void (*SubghzReadRAWCallback)(SubghzCustomEvent event, void* context);

typedef enum {
    SubghzReadRAWStatusStart,
    SubghzReadRAWStatusIDLE,
    SubghzReadRAWStatusREC,
    SubghzReadRAWStatusTX,
    SubghzReadRAWStatusTXRepeat,
} SubghzReadRAWStatus;

void subghz_read_raw_set_callback(
    SubghzReadRAW* subghz_read_raw,
    SubghzReadRAWCallback callback,
    void* context);

SubghzReadRAW* subghz_read_raw_alloc();

void subghz_read_raw_free(SubghzReadRAW* subghz_static);

void subghz_read_raw_add_data_statusbar(
    SubghzReadRAW* instance,
    const char* frequency_str,
    const char* preset_str);

void subghz_read_raw_update_sample_write(SubghzReadRAW* instance, size_t sample);

void subghz_read_raw_stop_send(SubghzReadRAW* instance);

void subghz_read_raw_update_sin(SubghzReadRAW* instance);

void subghz_read_raw_add_data_rssi(SubghzReadRAW* instance, float rssi);

void subghz_read_raw_set_status(SubghzReadRAW* instance, SubghzReadRAWStatus satus);

View* subghz_read_raw_get_view(SubghzReadRAW* subghz_static);
