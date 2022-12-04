#pragma once

typedef enum {
    GpioStartEventOtgOff = 0,
    GpioStartEventOtgOn,
    GpioStartEventManualControl,
    GpioStartEventUsbUart,
    GpioStartEventI2CScanner,
    GpioStartEventI2CSfp,

    GpioCustomEventErrorBack,

    GpioUsbUartEventConfig,
    GpioUsbUartEventConfigSet,
} GpioCustomEvent;
