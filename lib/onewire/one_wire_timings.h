#pragma once
#include <stdint.h>

class __OneWireTiming {
public:
    constexpr static const uint16_t TIMING_A = 9;
    constexpr static const uint16_t TIMING_B = 64;
    constexpr static const uint16_t TIMING_C = 64;
    constexpr static const uint16_t TIMING_D = 14;
    constexpr static const uint16_t TIMING_E = 9;
    constexpr static const uint16_t TIMING_F = 55;
    constexpr static const uint16_t TIMING_G = 0;
    constexpr static const uint16_t TIMING_H = 480;
    constexpr static const uint16_t TIMING_I = 70;
    constexpr static const uint16_t TIMING_J = 410;
};

class OneWireTiming {
public:
    constexpr static const uint16_t WRITE_1_DRIVE = __OneWireTiming::TIMING_A;
    constexpr static const uint16_t WRITE_1_RELEASE = __OneWireTiming::TIMING_B;

    constexpr static const uint16_t WRITE_0_DRIVE = __OneWireTiming::TIMING_C;
    constexpr static const uint16_t WRITE_0_RELEASE = __OneWireTiming::TIMING_D;

    constexpr static const uint16_t READ_DRIVE = 3;
    constexpr static const uint16_t READ_RELEASE = __OneWireTiming::TIMING_E;
    constexpr static const uint16_t READ_DELAY_POST = __OneWireTiming::TIMING_F;

    constexpr static const uint16_t RESET_DELAY_PRE = __OneWireTiming::TIMING_G;
    constexpr static const uint16_t RESET_DRIVE = __OneWireTiming::TIMING_H;
    constexpr static const uint16_t RESET_RELEASE = __OneWireTiming::TIMING_I;
    constexpr static const uint16_t RESET_DELAY_POST = __OneWireTiming::TIMING_J;
};

typedef uint32_t OneWiteTimeType;

class OneWireEmulateTiming {
public:
    constexpr static const OneWiteTimeType RESET_MIN = 270;
    constexpr static const OneWiteTimeType RESET_MAX = 960;

    constexpr static const OneWiteTimeType PRESENCE_TIMEOUT = 20;
    constexpr static const OneWiteTimeType PRESENCE_MIN = 100;
    constexpr static const OneWiteTimeType PRESENCE_MAX = 480;

    constexpr static const OneWiteTimeType MSG_HIGH_TIMEOUT = 15000;
    constexpr static const OneWiteTimeType SLOT_MAX = 135;

    constexpr static const OneWiteTimeType READ_MIN = 20;
    constexpr static const OneWiteTimeType READ_MAX = 60;
    constexpr static const OneWiteTimeType WRITE_ZERO = 30;
};