#pragma once

typedef enum {
    GpioStartEventOtgOff = 0,
    GpioStartEventOtgOn,
    GpioStartEventManualConrol,
    GpioStartEventUsbUart,

    GpioCustomEventErrorBack,

    GpioUsbUartEventConfig,
} GpioCustomEvent;
