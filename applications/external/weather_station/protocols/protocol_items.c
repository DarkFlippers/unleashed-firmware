#include "protocol_items.h"

const SubGhzProtocol* weather_station_protocol_registry_items[] = {
    &ws_protocol_infactory,
    &ws_protocol_thermopro_tx4,
    &ws_protocol_nexus_th,
    &ws_protocol_gt_wt_02,
    &ws_protocol_gt_wt_03,
    &ws_protocol_acurite_606tx,
    &ws_protocol_acurite_609txc,
    &ws_protocol_lacrosse_tx,
    &ws_protocol_lacrosse_tx141thbv2,
    &ws_protocol_oregon2,
    &ws_protocol_oregon3,
    &ws_protocol_acurite_592txr,
    &ws_protocol_ambient_weather,
    &ws_protocol_auriol_th,
    &ws_protocol_oregon_v1,
    &ws_protocol_tx_8300,
    &ws_protocol_wendox_w6726,
};

const SubGhzProtocolRegistry weather_station_protocol_registry = {
    .items = weather_station_protocol_registry_items,
    .size = COUNT_OF(weather_station_protocol_registry_items)};
