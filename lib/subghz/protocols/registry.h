#pragma once

#include "../types.h"

const SubGhzProtocol* subghz_protocol_registry_get_by_name(const char* name);

const SubGhzProtocol* subghz_protocol_registry_get_by_index(size_t index);

size_t subghz_protocol_registry_count();
