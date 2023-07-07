/*
    Unitemp - Universal temperature reader
    Copyright (C) 2022-2023  Victor Nikitchuk (https://github.com/quen0n)
    Contributed by divinebird (https://github.com/divinebird)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Some information may be seen on https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library

#include "SCD30.h"
#include "../interfaces/I2CSensor.h"
#include "../interfaces/endianness.h"
//#include <3rdparty/everest/include/everest/kremlin/c_endianness.h>

bool unitemp_SCD40_alloc(Sensor* sensor, char* args);
bool unitemp_SCD40_init(Sensor* sensor);
bool unitemp_SCD40_deinit(Sensor* sensor);
UnitempStatus unitemp_SCD40_update(Sensor* sensor);
bool unitemp_SCD40_free(Sensor* sensor);

const SensorType SCD40 = {
    .typename = "SCD40",
    .interface = &I2C,
    .datatype = UT_DATA_TYPE_TEMP_HUM_CO2,
    .pollingInterval = 5000,
    .allocator = unitemp_SCD40_alloc,
    .mem_releaser = unitemp_SCD40_free,
    .initializer = unitemp_SCD40_init,
    .deinitializer = unitemp_SCD40_deinit,
    .updater = unitemp_SCD40_update};

#define SCD40_ID 0x62

#define COMMAND_START_PERIODIC_MEASUREMENT 0X21B1
#define COMMAND_READ_MEASUREMENT 0XEC05
#define COMMAND_STOP_PERIODIC_MEASUREMENT 0X3F86

#define COMMAND_PERSIST_SETTINGS 0X3615
#define COMMAND_GET_SERIAL_NUMBER 0X3682
#define COMMAND_PERFORM_SELF_TEST 0X3639
#define COMMAND_PERFORM_FACTORY_RESET 0X3632
#define COMMAND_REINIT 0X3646

#define COMMAND_SET_TEMPERATURE_OFFSET 0X241D
#define COMMAND_GET_TEMPERATURE_OFFSET 0X2318
#define COMMAND_SET_SENSOR_ALTITUDE 0X2427
#define COMMAND_GET_SENSOR_ALTITUDE 0X2322
#define COMMAND_SET_AMBIENT_PRESSURE 0XE000
#define COMMAND_PERFORM_FORCED_RECALIBRATION 0X362F
#define COMMAND_SET_AUTOMATIC_SELF_CALIBRATION_ENABLED 0X2416
#define COMMAND_GET_AUTOMATIC_SELF_CALIBRATION_ENABLED 0X2313

static bool readMeasurement(Sensor* sensor) __attribute__((unused));
static void reset(Sensor* sensor) __attribute__((unused));

static bool setAutoSelfCalibration(Sensor* sensor, bool enable) __attribute__((unused));
static bool getAutoSelfCalibration(Sensor* sensor) __attribute__((unused));

static bool getFirmwareVersion(Sensor* sensor, uint16_t* val) __attribute__((unused));

static float getTemperatureOffset(Sensor* sensor) __attribute__((unused));
static bool setTemperatureOffset(Sensor* sensor, float tempOffset) __attribute__((unused));

static bool beginMeasuring(Sensor* sensor) __attribute__((unused));
static bool stopMeasurement(Sensor* sensor) __attribute__((unused));

bool unitemp_SCD40_alloc(Sensor* sensor, char* args) {
    UNUSED(args);
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    i2c_sensor->minI2CAdr = SCD40_ID << 1;
    i2c_sensor->maxI2CAdr = SCD40_ID << 1;
    return true;
}

bool unitemp_SCD40_free(Sensor* sensor) {
    //Нечего высвобождать, так как ничего не было выделено
    UNUSED(sensor);
    return true;
}

bool unitemp_SCD40_init(Sensor* sensor) {
    return beginMeasuring(sensor);
}

bool unitemp_SCD40_deinit(Sensor* sensor) {
    return stopMeasurement(sensor);
}

UnitempStatus unitemp_SCD40_update(Sensor* sensor) {
    readMeasurement(sensor);
    return UT_SENSORSTATUS_OK;
}

#define CRC8_POLYNOMIAL 0x31
#define CRC8_INIT 0xFF

static uint8_t computeCRC8(uint8_t* message, uint8_t len) {
    uint8_t crc = CRC8_INIT; // Init with 0xFF
    for(uint8_t x = 0; x < len; x++) {
        crc ^= message[x]; // XOR-in the next input byte
        for(uint8_t i = 0; i < 8; i++) {
            if((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ CRC8_POLYNOMIAL);
            else
                crc <<= 1;
        }
    }
    return crc; // No output reflection
}

// Sends a command along with arguments and CRC
static bool sendCommandWithCRC(Sensor* sensor, uint16_t command, uint16_t arguments) {
    static const uint8_t cmdSize = 5;

    uint8_t bytes[cmdSize];
    uint8_t* pointer = bytes;
    store16_be(pointer, command);
    pointer += 2;
    uint8_t* argPos = pointer;
    store16_be(pointer, arguments);
    pointer += 2;
    *pointer = computeCRC8(argPos, pointer - argPos);

    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    return unitemp_i2c_writeArray(i2c_sensor, cmdSize, bytes);
}

// Sends just a command, no arguments, no CRC
static bool sendCommand(Sensor* sensor, uint16_t command) {
    static const uint8_t cmdSize = 2;

    uint8_t bytes[cmdSize];
    store16_be(bytes, command);

    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    return unitemp_i2c_writeArray(i2c_sensor, cmdSize, bytes);
}

static uint16_t readRegister(Sensor* sensor, uint16_t registerAddress) {
    static const uint8_t regSize = 2;

    if(!sendCommand(sensor, registerAddress)) return 0; // Sensor did not ACK

    furi_delay_ms(3);

    uint8_t bytes[regSize];
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    if(!unitemp_i2c_readArray(i2c_sensor, regSize, bytes)) return 0;

    return load16_be(bytes);
}

static bool loadWord(uint8_t* buff, uint16_t* val) {
    uint16_t tmp = load16_be(buff);
    uint8_t expectedCRC = computeCRC8(buff, 2);
    if(buff[2] != expectedCRC) return false;
    *val = tmp;
    return true;
}

static bool getSettingValue(Sensor* sensor, uint16_t registerAddress, uint16_t* val) {
    static const uint8_t respSize = 3;

    if(!sendCommand(sensor, registerAddress)) return false; // Sensor did not ACK

    furi_delay_ms(3);

    uint8_t bytes[respSize];
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    if(!unitemp_i2c_readArray(i2c_sensor, respSize, bytes)) return false;

    return loadWord(bytes, val);
}

// Get 18 bytes from SCD30
// Updates global variables with floats
// Returns true if success
static bool readMeasurement(Sensor* sensor) {
    if(!sendCommand(sensor, COMMAND_READ_MEASUREMENT)) {
        FURI_LOG_E(APP_NAME, "Sensor did not ACK");
        return false; // Sensor did not ACK
    }

    furi_delay_ms(3);

    static const uint8_t respSize = 9;
    uint8_t buff[respSize];
    uint8_t* bytes = buff;
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    if(!unitemp_i2c_readArray(i2c_sensor, respSize, bytes)) {
        FURI_LOG_E(APP_NAME, "Error while read measures");
        return false;
    }

    uint16_t tmpValue;

    bool error = false;
    if(loadWord(bytes, &tmpValue)) {
        sensor->co2 = tmpValue;
    } else {
        FURI_LOG_E(APP_NAME, "Error while parsing CO2");
        error = true;
    }

    bytes += 3;
    if(loadWord(bytes, &tmpValue)) {
        sensor->temp = (float)tmpValue * 175.0f / 65535.0f - 45.0f;
    } else {
        FURI_LOG_E(APP_NAME, "Error while parsing temp");
        error = true;
    }

    bytes += 3;
    if(loadWord(bytes, &tmpValue)) {
        sensor->hum = (float)tmpValue * 100.0f / 65535.0f;
    } else {
        FURI_LOG_E(APP_NAME, "Error while parsing humidity");
        error = true;
    }

    return !error;
}

static void reset(Sensor* sensor) {
    sendCommand(sensor, COMMAND_REINIT);
}

static bool setAutoSelfCalibration(Sensor* sensor, bool enable) {
    return sendCommandWithCRC(
        sensor, COMMAND_SET_AUTOMATIC_SELF_CALIBRATION_ENABLED, enable); // Activate continuous ASC
}

// Get the current ASC setting
static bool getAutoSelfCalibration(Sensor* sensor) {
    return 1 == readRegister(sensor, COMMAND_GET_AUTOMATIC_SELF_CALIBRATION_ENABLED);
}

// Unfinished
static bool getFirmwareVersion(Sensor* sensor, uint16_t* val) {
    if(!sendCommand(sensor, COMMAND_READ_MEASUREMENT)) {
        FURI_LOG_E(APP_NAME, "Sensor did not ACK");
        return false; // Sensor did not ACK
    }

    static const uint8_t respSize = 9;
    uint8_t buff[respSize];
    uint8_t* bytes = buff;
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    if(!unitemp_i2c_readArray(i2c_sensor, respSize, bytes)) {
        FURI_LOG_E(APP_NAME, "Error while read measures");
        return false;
    }

    *val = 0;

    return true;
}

static bool beginMeasuring(Sensor* sensor) {
    return sendCommand(sensor, COMMAND_START_PERIODIC_MEASUREMENT);
}

// Stop continuous measurement
static bool stopMeasurement(Sensor* sensor) {
    return sendCommand(sensor, COMMAND_READ_MEASUREMENT);
}

static float getTemperatureOffset(Sensor* sensor) {
    uint16_t curOffset;
    if(!getSettingValue(sensor, COMMAND_GET_TEMPERATURE_OFFSET, &curOffset)) return 0.0;
    return (float)curOffset * 175.0f / 65536.0f;
}

static bool setTemperatureOffset(Sensor* sensor, float tempOffset) {
    uint16_t newOffset = tempOffset * 65536.0 / 175.0 + 0.5f;
    return sendCommandWithCRC(
        sensor, COMMAND_SET_TEMPERATURE_OFFSET, newOffset); // Activate continuous ASC
}
