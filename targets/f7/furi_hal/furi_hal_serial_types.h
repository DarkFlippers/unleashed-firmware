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

typedef enum {
    FuriHalSerialDirectionTx,
    FuriHalSerialDirectionRx,

    FuriHalSerialDirectionMax,
} FuriHalSerialDirection;

typedef struct FuriHalSerialHandle FuriHalSerialHandle;
