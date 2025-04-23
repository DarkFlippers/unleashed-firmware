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

/**
 * @brief Actual data bits, i.e. not including start/stop and parity bits
 * @note 6 data bits are only permitted when parity is enabled
 * @note 9 data bits are only permitted when parity is disabled
 */
typedef enum {
    FuriHalSerialDataBits6,
    FuriHalSerialDataBits7,
    FuriHalSerialDataBits8,
    FuriHalSerialDataBits9,

    FuriHalSerialDataBitsMax,
} FuriHalSerialDataBits;

typedef enum {
    FuriHalSerialParityNone,
    FuriHalSerialParityEven,
    FuriHalSerialParityOdd,

    FuriHalSerialParityMax,
} FuriHalSerialParity;

/**
 * @brief Stop bit length
 * @note LPUART only supports whole stop bit lengths (i.e. 1 and 2, but not 0.5 and 1.5)
 */
typedef enum {
    FuriHalSerialStopBits0_5,
    FuriHalSerialStopBits1,
    FuriHalSerialStopBits1_5,
    FuriHalSerialStopBits2,

    FuriHalSerialStopBits2Max,
} FuriHalSerialStopBits;

typedef struct FuriHalSerialHandle FuriHalSerialHandle;
