#pragma once

#include <gui/view.h>
#include "../helpers/subghz_custom_event.h"

typedef struct SubghzSaveRAW SubghzSaveRAW;

typedef void (*SubghzSaveRAWCallback)(SubghzCustomEvent event, void* context);

void subghz_save_raw_set_callback(
    SubghzSaveRAW* subghz_save_raw,
    SubghzSaveRAWCallback callback,
    void* context);

SubghzSaveRAW* subghz_save_raw_alloc();

void subghz_save_raw_free(SubghzSaveRAW* subghz_static);

void subghz_save_raw_add_data_statusbar(
    SubghzSaveRAW* instance,
    const char* frequency_str,
    const char* preset_str);

void subghz_save_raw_set_file_name(SubghzSaveRAW* instance, const char* file_name);

void subghz_save_raw_update_sample_write(SubghzSaveRAW* instance, size_t sample);

void subghz_save_raw_add_data_rssi(SubghzSaveRAW* instance, float rssi);

View* subghz_save_raw_get_view(SubghzSaveRAW* subghz_static);
