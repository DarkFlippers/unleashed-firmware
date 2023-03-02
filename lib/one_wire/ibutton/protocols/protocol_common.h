#pragma once

#include <stdint.h>
#include <stddef.h>

typedef int32_t iButtonProtocolId;

enum {
    iButtonProtocolIdInvalid = -1,
};

typedef enum {
    iButtonProtocolFeatureExtData = (1U << 0),
    iButtonProtocolFeatureWriteBlank = (1U << 1),
    iButtonProtocolFeatureWriteCopy = (1U << 2),
} iButtonProtocolFeature;

typedef struct {
    uint8_t* ptr;
    size_t size;
} iButtonEditableData;
