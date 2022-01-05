#pragma once
#include <stdint.h>

static const uint8_t IBUTTON_KEY_DATA_SIZE = 8;
static const uint8_t IBUTTON_KEY_NAME_SIZE = 22;

enum class iButtonKeyType : uint8_t {
    KeyDallas,
    KeyCyfral,
    KeyMetakom,
};