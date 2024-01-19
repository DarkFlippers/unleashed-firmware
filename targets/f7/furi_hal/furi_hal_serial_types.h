#pragma once

#include <furi.h>

/**
 * UART channels
 */
typedef enum {
    FuriHalSerialIdUsart,
    FuriHalSerialIdLpuart,

    FuriHalSerialIdMax,
} FuriHalSerialId;

typedef struct FuriHalSerialHandle FuriHalSerialHandle;
