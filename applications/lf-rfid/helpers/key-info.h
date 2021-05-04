#pragma once
#include <stdint.h>

static const uint8_t LFRFID_KEY_SIZE = 8;

enum class LfrfidKeyType : uint8_t {
    KeyEmarine,
    KeyHID,
};