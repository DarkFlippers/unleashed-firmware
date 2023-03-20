#pragma once

#include "ibutton_key.h"

#include "protocols/protocol_common_i.h"

iButtonProtocolData* ibutton_key_get_protocol_data(const iButtonKey* key);

size_t ibutton_key_get_protocol_data_size(const iButtonKey* key);
