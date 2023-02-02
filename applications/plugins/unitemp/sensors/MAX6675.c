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
#include "MAX6675.h"

const SensorType MAX6675 = {
    .typename = "MAX6675",
    .altname = "MAX6675 (Thermocouple)",
    .interface = &SPI,
    .datatype = UT_TEMPERATURE,
    .pollingInterval = 500,
    .allocator = unitemp_MAX6675_alloc,
    .mem_releaser = unitemp_MAX6675_free,
    .initializer = unitemp_MAX6675_init,
    .deinitializer = unitemp_MAX6675_deinit,
    .updater = unitemp_MAX6675_update};

bool unitemp_MAX6675_alloc(Sensor* sensor, char* args) {
    UNUSED(sensor);
    UNUSED(args);
    return true;
}

bool unitemp_MAX6675_free(Sensor* sensor) {
    UNUSED(sensor);
    return true;
}

bool unitemp_MAX6675_init(Sensor* sensor) {
    SPISensor* instance = sensor->instance;
    furi_hal_spi_bus_handle_init(instance->spi);
    UNUSED(instance);
    return true;
}

bool unitemp_MAX6675_deinit(Sensor* sensor) {
    UNUSED(sensor);
    return true;
}

UnitempStatus unitemp_MAX6675_update(Sensor* sensor) {
    SPISensor* instance = sensor->instance;

    furi_hal_spi_acquire(instance->spi);
    furi_hal_gpio_write(instance->CS_pin->pin, false);

    uint8_t buff[2] = {0};

    furi_hal_spi_bus_rx(instance->spi, buff, 2, 0xFF);
    furi_hal_spi_release(instance->spi);

    uint32_t raw = (buff[0] << 8) | buff[1];

    if(raw == 0xFFFFFFFF || raw == 0) return UT_SENSORSTATUS_TIMEOUT;

    //Определение состояния термопары
    uint8_t state = raw & 0b100;
    //Обрыв
    if(state == 0b100) {
        UNITEMP_DEBUG("%s has thermocouple open circuit", sensor->name);
        return UT_SENSORSTATUS_ERROR;
    }

    sensor->temp = (int16_t)(raw) / 32.0f;

    return UT_SENSORSTATUS_OK;
}
