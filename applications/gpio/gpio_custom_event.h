#pragma once

typedef enum {
    GpioStartEventOtgOff = 0,
    GpioStartEventOtgOn,
    GpioStartEventManualControl,
    GpioStartEventUsbUart,

    GpioCustomEventErrorBack,

    GpioUsbUartEventConfig,
} GpioCustomEvent;
