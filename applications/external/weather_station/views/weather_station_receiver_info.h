#pragma once

#include <gui/view.h>
#include "../helpers/weather_station_types.h"
#include "../helpers/weather_station_event.h"
#include <lib/flipper_format/flipper_format.h>

typedef struct WSReceiverInfo WSReceiverInfo;

void ws_view_receiver_info_update(WSReceiverInfo* ws_receiver_info, FlipperFormat* fff);

WSReceiverInfo* ws_view_receiver_info_alloc();

void ws_view_receiver_info_free(WSReceiverInfo* ws_receiver_info);

View* ws_view_receiver_info_get_view(WSReceiverInfo* ws_receiver_info);
