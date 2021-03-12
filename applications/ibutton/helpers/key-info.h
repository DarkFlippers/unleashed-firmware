#pragma once
#include <stdint.h>

static const uint8_t IBUTTON_KEY_SIZE = 8;

enum class iButtonKeyType : uint8_t {
    KeyDallas,
    KeyCyfral,
    KeyMetakom,
};