#include "protocol_items.h"

const SubGhzProtocol* pocsag_pager_protocol_registry_items[] = {
    &subghz_protocol_pocsag,
};

const SubGhzProtocolRegistry pocsag_pager_protocol_registry = {
    .items = pocsag_pager_protocol_registry_items,
    .size = COUNT_OF(pocsag_pager_protocol_registry_items)};