#include "one_wire_timings.h"

// fix pre C++17 "undefined reference" errors
constexpr const OneWiteTimeType OneWireEmulateTiming::RESET_TIMEOUT;
constexpr const OneWiteTimeType OneWireEmulateTiming::RESET_MIN[2];
constexpr const OneWiteTimeType OneWireEmulateTiming::RESET_MAX[2];

constexpr const OneWiteTimeType OneWireEmulateTiming::PRESENCE_TIMEOUT;
constexpr const OneWiteTimeType OneWireEmulateTiming::PRESENCE_MIN[2];
constexpr const OneWiteTimeType OneWireEmulateTiming::PRESENCE_MAX[2];

constexpr const OneWiteTimeType OneWireEmulateTiming::MSG_HIGH_TIMEOUT;
constexpr const OneWiteTimeType OneWireEmulateTiming::SLOT_MAX[2];

constexpr const OneWiteTimeType OneWireEmulateTiming::READ_MIN[2];
constexpr const OneWiteTimeType OneWireEmulateTiming::READ_MAX[2];
constexpr const OneWiteTimeType OneWireEmulateTiming::WRITE_ZERO[2];
