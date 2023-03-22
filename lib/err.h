#pragma once
#include <furi.h>

#define err(...) FURI_LOG_E("Heatshrink", "Error: %d-%s", __VA_ARGS__)