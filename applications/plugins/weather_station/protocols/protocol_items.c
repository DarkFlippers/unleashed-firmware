#include "protocol_items.h"

const SubGhzProtocol* weather_station_protocol_registry_items[] = {
    &ws_protocol_infactory,
    &ws_protocol_thermopro_tx4,
    &ws_protocol_nexus_th,
    &ws_protocol_gt_wt_03,
    &ws_protocol_acurite_606tx,
    &ws_protocol_lacrosse_tx141thbv2,
};

const SubGhzProtocolRegistry weather_station_protocol_registry = {
    .items = weather_station_protocol_registry_items,
    .size = COUNT_OF(weather_station_protocol_registry_items)};