#pragma once

#include <stdint.h>
#include <stddef.h>

enum InfraredCustomEventType {
    // Reserve first 100 events for button types and indexes, starting from 0
    InfraredCustomEventTypeReserved = 100,
    InfraredCustomEventTypeMenuSelected,
    InfraredCustomEventTypeTransmitStarted,
    InfraredCustomEventTypeTransmitStopped,
    InfraredCustomEventTypeSignalReceived,
    InfraredCustomEventTypeTextEditDone,
    InfraredCustomEventTypePopupClosed,
    InfraredCustomEventTypeButtonSelected,
    InfraredCustomEventTypeBackPressed,

    InfraredCustomEventTypeRpcLoad,
    InfraredCustomEventTypeRpcExit,
    InfraredCustomEventTypeRpcButtonPress,
    InfraredCustomEventTypeRpcButtonRelease,
    InfraredCustomEventTypeRpcSessionClose,
};

#pragma pack(push, 1)
typedef union {
    uint32_t packed_value;
    struct {
        uint16_t type;
        int16_t value;
    } content;
} InfraredCustomEvent;
#pragma pack(pop)

static inline uint32_t infrared_custom_event_pack(uint16_t type, int16_t value) {
    InfraredCustomEvent event = {.content = {.type = type, .value = value}};
    return event.packed_value;
}

static inline void
    infrared_custom_event_unpack(uint32_t packed_value, uint16_t* type, int16_t* value) {
    InfraredCustomEvent event = {.packed_value = packed_value};
    if(type) *type = event.content.type;
    if(value) *value = event.content.value;
}

static inline uint16_t infrared_custom_event_get_type(uint32_t packed_value) {
    uint16_t type;
    infrared_custom_event_unpack(packed_value, &type, NULL);
    return type;
}

static inline int16_t infrared_custom_event_get_value(uint32_t packed_value) {
    int16_t value;
    infrared_custom_event_unpack(packed_value, NULL, &value);
    return value;
}
