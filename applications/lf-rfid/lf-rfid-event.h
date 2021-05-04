#pragma once
#include <stdint.h>

class LfrfidEvent {
public:
    // events enum
    enum class Type : uint8_t {
        Tick,
        Back,
        MenuSelected,
        NextScene,
    };

    // payload
    union {
        uint32_t menu_index;
    } payload;

    // event type
    Type type;
};
