#pragma once

#include <gui/view.h>
#include "../helpers/subghz_types.h"
#include "../helpers/subghz_custom_event.h"

#define SUBGHZ_RAW_THRESHOLD_MIN -90.0f

typedef struct SubGhzReadRAW SubGhzReadRAW;

typedef void (*SubGhzReadRAWCallback)(SubGhzCustomEvent event, void* context);

typedef enum {
    SubGhzReadRAWStatusStart,
    SubGhzReadRAWStatusIDLE,
    SubGhzReadRAWStatusREC,
    SubGhzReadRAWStatusTX,
    SubGhzReadRAWStatusTXRepeat,

    SubGhzReadRAWStatusLoadKeyIDLE,
    SubGhzReadRAWStatusLoadKeyTX,
    SubGhzReadRAWStatusLoadKeyTXRepeat,
    SubGhzReadRAWStatusSaveKey,
} SubGhzReadRAWStatus;

void subghz_read_raw_set_callback(
    SubGhzReadRAW* subghz_read_raw,
    SubGhzReadRAWCallback callback,
    void* context);

SubGhzReadRAW* subghz_read_raw_alloc(void);

void subghz_read_raw_free(SubGhzReadRAW* subghz_static);

void subghz_read_raw_add_data_statusbar(
    SubGhzReadRAW* instance,
    const char* frequency_str,
    const char* preset_str);

void subghz_read_raw_set_radio_device_type(
    SubGhzReadRAW* instance,
    SubGhzRadioDeviceType device_type);

void subghz_read_raw_update_sample_write(SubGhzReadRAW* instance, size_t sample);

void subghz_read_raw_stop_send(SubGhzReadRAW* instance);

void subghz_read_raw_update_sin(SubGhzReadRAW* instance);

void subghz_read_raw_add_data_rssi(SubGhzReadRAW* instance, float rssi, bool trace);

void subghz_read_raw_set_status(
    SubGhzReadRAW* instance,
    SubGhzReadRAWStatus status,
    const char* file_name,
    float raw_threshold_rssi);

View* subghz_read_raw_get_view(SubGhzReadRAW* subghz_static);
