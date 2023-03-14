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

#include <furi.h>
#include <furi_hal.h>
#include "SPISensor.h"

static uint8_t sensors_count = 0;

bool unitemp_spi_sensor_alloc(Sensor* sensor, char* args) {
    if(args == NULL) return false;

    //Создание инстанса датчика SPI
    SPISensor* instance = malloc(sizeof(SPISensor));
    if(instance == NULL) {
        FURI_LOG_E(APP_NAME, "Sensor %s instance allocation error", sensor->name);
        return false;
    }
    sensor->instance = instance;

    //Определение GPIO chip select
    int gpio = 255;
    sscanf(args, "%d", &gpio);
    instance->CS_pin = unitemp_gpio_getFromInt(gpio);
    if(instance->CS_pin == NULL) {
        FURI_LOG_E(APP_NAME, "Sensor %s GPIO setting error", sensor->name);
        free(instance);
        return false;
    }

    instance->spi = malloc(sizeof(FuriHalSpiBusHandle));
    memcpy(instance->spi, &furi_hal_spi_bus_handle_external, sizeof(FuriHalSpiBusHandle));

    instance->spi->cs = instance->CS_pin->pin;

    bool status = sensor->type->allocator(sensor, args);

    //Блокировка портов GPIO
    sensors_count++;
    unitemp_gpio_lock(unitemp_gpio_getFromInt(2), &SPI);
    unitemp_gpio_lock(unitemp_gpio_getFromInt(3), &SPI);
    unitemp_gpio_lock(unitemp_gpio_getFromInt(5), &SPI);
    unitemp_gpio_lock(instance->CS_pin, &SPI);
    return status;
}

bool unitemp_spi_sensor_free(Sensor* sensor) {
    bool status = sensor->type->mem_releaser(sensor);
    unitemp_gpio_unlock(((SPISensor*)sensor->instance)->CS_pin);
    free(((SPISensor*)(sensor->instance))->spi);
    free(sensor->instance);

    if(--sensors_count == 0) {
        unitemp_gpio_unlock(unitemp_gpio_getFromInt(2));
        unitemp_gpio_unlock(unitemp_gpio_getFromInt(3));
        unitemp_gpio_unlock(unitemp_gpio_getFromInt(5));
    }

    return status;
}

bool unitemp_spi_sensor_init(Sensor* sensor) {
    return sensor->type->initializer(sensor);
}

bool unitemp_spi_sensor_deinit(Sensor* sensor) {
    UNUSED(sensor);

    return true;
}

UnitempStatus unitemp_spi_sensor_update(Sensor* sensor) {
    return sensor->type->updater(sensor);
}