/*
    Unitemp - Universal temperature reader
    Copyright (C) 2022-2023  Victor Nikitchuk (https://github.com/quen0n)

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
#include "MAX31855.h"

const SensorType MAX31855 = {
    .typename = "MAX31855",
    .altname = "MAX31855 (Thermocouple)",
    .interface = &SPI,
    .datatype = UT_TEMPERATURE,
    .pollingInterval = 500,
    .allocator = unitemp_MAX31855_alloc,
    .mem_releaser = unitemp_MAX31855_free,
    .initializer = unitemp_MAX31855_init,
    .deinitializer = unitemp_MAX31855_deinit,
    .updater = unitemp_MAX31855_update};

bool unitemp_MAX31855_alloc(Sensor* sensor, char* args) {
    UNUSED(sensor);
    UNUSED(args);
    return true;
}

bool unitemp_MAX31855_free(Sensor* sensor) {
    UNUSED(sensor);
    return true;
}

bool unitemp_MAX31855_init(Sensor* sensor) {
    SPISensor* instance = sensor->instance;
    furi_hal_spi_bus_handle_init(instance->spi);
    UNUSED(instance);
    return true;
}

bool unitemp_MAX31855_deinit(Sensor* sensor) {
    UNUSED(sensor);
    return true;
}

UnitempStatus unitemp_MAX31855_update(Sensor* sensor) {
    SPISensor* instance = sensor->instance;

    furi_hal_spi_acquire(instance->spi);
    furi_hal_gpio_write(instance->CS_pin->pin, false);

    uint8_t buff[4] = {0};

    furi_hal_spi_bus_rx(instance->spi, buff, 4, 0xFF);
    furi_hal_spi_release(instance->spi);

    uint32_t raw = (buff[0] << 24) | (buff[1] << 16) | (buff[2] << 8) | buff[3];

    if(raw == 0xFFFFFFFF || raw == 0) return UT_SENSORSTATUS_TIMEOUT;

    //Определение состояния термопары
    uint8_t state = raw & 0b111;
    //Обрыв
    if(state == 0x01) {
        UNITEMP_DEBUG("%s has thermocouple open circuit", sensor->name);
        return UT_SENSORSTATUS_ERROR;
    }
    //Короткое замыкание к земле
    if(state == 0x02) {
        UNITEMP_DEBUG("%s has thermocouple short to GND", sensor->name);
        return UT_SENSORSTATUS_ERROR;
    }
    //Короткое замыкание к питанию
    if(state == 0x04) {
        UNITEMP_DEBUG("%s has thermocouple short to VCC", sensor->name);
        return UT_SENSORSTATUS_ERROR;
    }

    raw = (raw >> 16) & 0xFFFC;

    sensor->temp = (int16_t)(raw) / 16.0f;

    return UT_SENSORSTATUS_OK;
}
