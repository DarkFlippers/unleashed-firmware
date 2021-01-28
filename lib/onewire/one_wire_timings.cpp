#include "one_wire_timings.h"

// fix pre C++17 "undefined reference" errors
constexpr const OneWiteTimeType OneWireEmulateTiming::RESET_MIN;
constexpr const OneWiteTimeType OneWireEmulateTiming::RESET_MAX;

constexpr const OneWiteTimeType OneWireEmulateTiming::PRESENCE_TIMEOUT;
constexpr const OneWiteTimeType OneWireEmulateTiming::PRESENCE_MIN;
constexpr const OneWiteTimeType OneWireEmulateTiming::PRESENCE_MAX;

constexpr const OneWiteTimeType OneWireEmulateTiming::MSG_HIGH_TIMEOUT;
constexpr const OneWiteTimeType OneWireEmulateTiming::SLOT_MAX;

constexpr const OneWiteTimeType OneWireEmulateTiming::READ_MIN;
constexpr const OneWiteTimeType OneWireEmulateTiming::READ_MAX;
constexpr const OneWiteTimeType OneWireEmulateTiming::WRITE_ZERO;
