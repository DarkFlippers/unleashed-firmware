#pragma once

#include <stddef.h>

#include <one_wire/one_wire_host.h>

bool rw1990_write_v1(OneWireHost* host, const uint8_t* data, size_t data_size);

bool rw1990_write_v2(OneWireHost* host, const uint8_t* data, size_t data_size);
