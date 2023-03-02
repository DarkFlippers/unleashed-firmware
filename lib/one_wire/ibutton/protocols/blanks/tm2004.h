#pragma once

#include <stddef.h>

#include <one_wire/one_wire_host.h>

bool tm2004_write(OneWireHost* host, const uint8_t* data, size_t data_size);
