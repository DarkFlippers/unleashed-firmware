#pragma once
#include <stdint.h>

class RW1990_1 {
public:
    constexpr static const uint8_t CMD_WRITE_RECORD_FLAG = 0xD1;
    constexpr static const uint8_t CMD_READ_RECORD_FLAG = 0xB5;
    constexpr static const uint8_t CMD_WRITE_ROM = 0xD5;
};

class RW1990_2 {
public:
    constexpr static const uint8_t CMD_WRITE_RECORD_FLAG = 0x1D;
    constexpr static const uint8_t CMD_READ_RECORD_FLAG = 0x1E;
    constexpr static const uint8_t CMD_WRITE_ROM = 0xD5;
};

class TM2004 {
public:
    constexpr static const uint8_t CMD_READ_STATUS = 0xAA;
    constexpr static const uint8_t CMD_READ_MEMORY = 0xF0;
    constexpr static const uint8_t CMD_WRITE_ROM = 0x3C;
    constexpr static const uint8_t CMD_FINALIZATION = 0x35;

    constexpr static const uint8_t ANSWER_READ_MEMORY = 0xF5;
};

class TM01 {
public:
    constexpr static const uint8_t CMD_WRITE_RECORD_FLAG = 0xC1;
    constexpr static const uint8_t CMD_WRITE_ROM = 0xC5;
    constexpr static const uint8_t CMD_SWITCH_TO_CYFRAL = 0xCA;
    constexpr static const uint8_t CMD_SWITCH_TO_METAKOM = 0xCB;
};

class DS1990 {
public:
    constexpr static const uint8_t CMD_READ_ROM = 0x33;
};