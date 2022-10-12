#pragma once

#include <inttypes.h>

int32_t timezone_offset_from_hours(float hours);
uint64_t timezone_offset_apply(uint64_t time, int32_t offset);
