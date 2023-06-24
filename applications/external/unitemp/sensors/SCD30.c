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

typedef union {
    uint16_t array16[2];
    uint8_t array8[4];
    float value;
} ByteToFl;

bool unitemp_SCD30_alloc(Sensor* sensor, char* args);
bool unitemp_SCD30_init(Sensor* sensor);
bool unitemp_SCD30_deinit(Sensor* sensor);
UnitempStatus unitemp_SCD30_update(Sensor* sensor);
bool unitemp_SCD30_free(Sensor* sensor);

const SensorType SCD30 = {
    .typename = "SCD30",
    .interface = &I2C,
    .datatype = UT_DATA_TYPE_TEMP_HUM_CO2,
    .pollingInterval = 2000,
    .allocator = unitemp_SCD30_alloc,
    .mem_releaser = unitemp_SCD30_free,
    .initializer = unitemp_SCD30_init,
    .deinitializer = unitemp_SCD30_deinit,
    .updater = unitemp_SCD30_update};

#define SCD30_ID 0x61

#define COMMAND_CONTINUOUS_MEASUREMENT 0x0010
#define COMMAND_SET_MEASUREMENT_INTERVAL 0x4600
#define COMMAND_GET_DATA_READY 0x0202
#define COMMAND_READ_MEASUREMENT 0x0300
#define COMMAND_AUTOMATIC_SELF_CALIBRATION 0x5306
#define COMMAND_SET_FORCED_RECALIBRATION_FACTOR 0x5204
#define COMMAND_SET_TEMPERATURE_OFFSET 0x5403
#define COMMAND_SET_ALTITUDE_COMPENSATION 0x5102
#define COMMAND_RESET 0xD304 // Soft reset
#define COMMAND_STOP_MEAS 0x0104
#define COMMAND_READ_FW_VER 0xD100

static bool dataAvailable(Sensor* sensor) __attribute__((unused));
static bool readMeasurement(Sensor* sensor) __attribute__((unused));
static void reset(Sensor* sensor) __attribute__((unused));

static bool setAutoSelfCalibration(Sensor* sensor, bool enable) __attribute__((unused));
static bool getAutoSelfCalibration(Sensor* sensor) __attribute__((unused));

static bool getFirmwareVersion(Sensor* sensor, uint16_t* val) __attribute__((unused));

static bool setForcedRecalibrationFactor(Sensor* sensor, uint16_t concentration)
    __attribute__((unused));
static uint16_t getAltitudeCompensation(Sensor* sensor) __attribute__((unused));
static bool setAltitudeCompensation(Sensor* sensor, uint16_t altitude) __attribute__((unused));
static bool setAmbientPressure(Sensor* sensor, uint16_t pressure_mbar) __attribute__((unused));

static float getTemperatureOffset(Sensor* sensor) __attribute__((unused));
static bool setTemperatureOffset(Sensor* sensor, float tempOffset) __attribute__((unused));

static bool beginMeasuringWithSettings(Sensor* sensor, uint16_t pressureOffset)
    __attribute__((unused));
static bool beginMeasuring(Sensor* sensor) __attribute__((unused));
static bool stopMeasurement(Sensor* sensor) __attribute__((unused));

static bool setMeasurementInterval(Sensor* sensor, uint16_t interval) __attribute__((unused));
static uint16_t getMeasurementInterval(Sensor* sensor) __attribute__((unused));

bool unitemp_SCD30_alloc(Sensor* sensor, char* args) {
    UNUSED(args);
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    i2c_sensor->minI2CAdr = SCD30_ID << 1;
    i2c_sensor->maxI2CAdr = SCD30_ID << 1;
    return true;
}

bool unitemp_SCD30_free(Sensor* sensor) {
    //Нечего высвобождать, так как ничего не было выделено
    UNUSED(sensor);
    return true;
}

bool unitemp_SCD30_init(Sensor* sensor) {
    if(beginMeasuring(sensor) == true) { // Start continuous measurements
        setMeasurementInterval(sensor, SCD30.pollingInterval / 1000);
        setAutoSelfCalibration(sensor, true);
        setAmbientPressure(sensor, 0);
    } else
        return false;

    return true;
}

bool unitemp_SCD30_deinit(Sensor* sensor) {
    return stopMeasurement(sensor);
}

UnitempStatus unitemp_SCD30_update(Sensor* sensor) {
    readMeasurement(sensor);
    return UT_SENSORSTATUS_OK;
}

static uint8_t computeCRC8(uint8_t* message, uint8_t len) {
    uint8_t crc = 0xFF; // Init with 0xFF
    for(uint8_t x = 0; x < len; x++) {
        crc ^= message[x]; // XOR-in the next input byte
        for(uint8_t i = 0; i < 8; i++) {
            if((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ 0x31);
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

static bool loadFloat(uint8_t* buff, float* val) {
    //    ByteToFl tmp;
    size_t cntr = 0;
    uint8_t floatBuff[4];
    for(size_t i = 0; i < 2; i++) {
        floatBuff[cntr++] = buff[0];
        floatBuff[cntr++] = buff[1];
        uint8_t expectedCRC = computeCRC8(buff, 2);
        if(buff[2] != expectedCRC) return false;
        buff += 3;
    }
    uint32_t tmpVal = load32_be(floatBuff);
    memcpy(val, &tmpVal, sizeof(float));
    return true;
}

// Get 18 bytes from SCD30
// Updates global variables with floats
// Returns true if success
static bool readMeasurement(Sensor* sensor) {
    // Verify we have data from the sensor
    if(!dataAvailable(sensor)) {
        return false;
    }

    if(!sendCommand(sensor, COMMAND_READ_MEASUREMENT)) {
        FURI_LOG_E(APP_NAME, "Sensor did not ACK");
        return false; // Sensor did not ACK
    }

    float tempCO2 = 0;
    float tempHumidity = 0;
    float tempTemperature = 0;

    furi_delay_ms(3);

    static const uint8_t respSize = 18;
    uint8_t buff[respSize];
    uint8_t* bytes = buff;
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    if(!unitemp_i2c_readArray(i2c_sensor, respSize, bytes)) {
        FURI_LOG_E(APP_NAME, "Error while read measures");
        return false;
    }

    bool error = false;
    if(loadFloat(bytes, &tempCO2)) {
        sensor->co2 = tempCO2;
    } else {
        FURI_LOG_E(APP_NAME, "Error while parsing CO2");
        error = true;
    }

    bytes += 6;
    if(loadFloat(bytes, &tempTemperature)) {
        sensor->temp = tempTemperature;
    } else {
        FURI_LOG_E(APP_NAME, "Error while parsing temp");
        error = true;
    }

    bytes += 6;
    if(loadFloat(bytes, &tempHumidity)) {
        sensor->hum = tempHumidity;
    } else {
        FURI_LOG_E(APP_NAME, "Error while parsing humidity");
        error = true;
    }

    return !error;
}

static void reset(Sensor* sensor) {
    sendCommand(sensor, COMMAND_RESET);
}

static bool setAutoSelfCalibration(Sensor* sensor, bool enable) {
    return sendCommandWithCRC(
        sensor, COMMAND_AUTOMATIC_SELF_CALIBRATION, enable); // Activate continuous ASC
}

// Get the current ASC setting
static bool getAutoSelfCalibration(Sensor* sensor) {
    return 1 == readRegister(sensor, COMMAND_AUTOMATIC_SELF_CALIBRATION);
}

static bool getFirmwareVersion(Sensor* sensor, uint16_t* val) {
    return getSettingValue(sensor, COMMAND_READ_FW_VER, val);
}

// Set the forced recalibration factor. See 1.3.7.
// The reference CO2 concentration has to be within the range 400 ppm ≤ cref(CO2) ≤ 2000 ppm.
static bool setForcedRecalibrationFactor(Sensor* sensor, uint16_t concentration) {
    if(concentration < 400 || concentration > 2000) {
        return false; // Error check.
    }
    return sendCommandWithCRC(sensor, COMMAND_SET_FORCED_RECALIBRATION_FACTOR, concentration);
}

// Get the temperature offset. See 1.3.8.
static float getTemperatureOffset(Sensor* sensor) {
    union {
        int16_t signed16;
        uint16_t unsigned16;
    } signedUnsigned; // Avoid any ambiguity casting int16_t to uint16_t
    signedUnsigned.unsigned16 = readRegister(sensor, COMMAND_SET_TEMPERATURE_OFFSET);

    return ((float)signedUnsigned.signed16) / 100.0;
}

static bool setTemperatureOffset(Sensor* sensor, float tempOffset) {
    // Temp offset is only positive. See: https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library/issues/27#issuecomment-971986826
    //"The SCD30 offset temperature is obtained by subtracting the reference temperature from the SCD30 output temperature"
    // https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/9.5_CO2/Sensirion_CO2_Sensors_SCD30_Low_Power_Mode.pdf

    if(tempOffset < 0.0) return false;

    uint16_t value = tempOffset * 100;

    return sendCommandWithCRC(sensor, COMMAND_SET_TEMPERATURE_OFFSET, value);
}

// Get the altitude compenstation. See 1.3.9.
static uint16_t getAltitudeCompensation(Sensor* sensor) {
    return readRegister(sensor, COMMAND_SET_ALTITUDE_COMPENSATION);
}

// Set the altitude compenstation. See 1.3.9.
static bool setAltitudeCompensation(Sensor* sensor, uint16_t altitude) {
    return sendCommandWithCRC(sensor, COMMAND_SET_ALTITUDE_COMPENSATION, altitude);
}

// Set the pressure compenstation. This is passed during measurement startup.
// mbar can be 700 to 1200
static bool setAmbientPressure(Sensor* sensor, uint16_t pressure_mbar) {
    if(pressure_mbar != 0 || pressure_mbar < 700 || pressure_mbar > 1200) {
        return false;
    }
    return sendCommandWithCRC(sensor, COMMAND_CONTINUOUS_MEASUREMENT, pressure_mbar);
}

// Begins continuous measurements
// Continuous measurement status is saved in non-volatile memory. When the sensor
// is powered down while continuous measurement mode is active SCD30 will measure
// continuously after repowering without sending the measurement command.
// Returns true if successful
static bool beginMeasuringWithSettings(Sensor* sensor, uint16_t pressureOffset) {
    return sendCommandWithCRC(sensor, COMMAND_CONTINUOUS_MEASUREMENT, pressureOffset);
}

// Overload - no pressureOffset
static bool beginMeasuring(Sensor* sensor) {
    return beginMeasuringWithSettings(sensor, 0);
}

// Stop continuous measurement
static bool stopMeasurement(Sensor* sensor) {
    return sendCommand(sensor, COMMAND_STOP_MEAS);
}

// Sets interval between measurements
// 2 seconds to 1800 seconds (30 minutes)
static bool setMeasurementInterval(Sensor* sensor, uint16_t interval) {
    if(interval < 2 || interval > 1800) return false;
    if(!sendCommandWithCRC(sensor, COMMAND_SET_MEASUREMENT_INTERVAL, interval)) return false;
    uint16_t verInterval = readRegister(sensor, COMMAND_SET_MEASUREMENT_INTERVAL);
    if(verInterval != interval) {
        FURI_LOG_E(APP_NAME, "Measure interval wrong! Val: %02x", verInterval);
        return false;
    }
    return true;
}

// Gets interval between measurements
// 2 seconds to 1800 seconds (30 minutes)
static uint16_t getMeasurementInterval(Sensor* sensor) {
    uint16_t interval = 0;
    getSettingValue(sensor, COMMAND_SET_MEASUREMENT_INTERVAL, &interval);
    return interval;
}

// Returns true when data is available
static bool dataAvailable(Sensor* sensor) {
    return 1 == readRegister(sensor, COMMAND_GET_DATA_READY);
}