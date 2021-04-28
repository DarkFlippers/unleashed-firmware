#pragma once
#include <stdint.h>

class AccessorEvent {
public:
    // events enum
    enum class Type : uint8_t {
        Tick,
        Back,
    };

    // payload
    union {
        uint32_t menu_index;
    } payload;

    // event type
    Type type;
};
