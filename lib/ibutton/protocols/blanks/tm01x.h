#pragma once

#include <core/kernel.h>
#include <one_wire/one_wire_host.h>

// Function to write Dallas protocol to TM01x
bool tm01x_write_dallas(OneWireHost* host, const uint8_t* data, size_t data_size);
