/**
 * @file BH1750.h
 * @author Oleksii Kutuzov (oleksii.kutuzov@icloud.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-06
 * 
 * @copyright Copyright (c) 2022
 * 
 * Ported from:
 * https://github.com/lamik/Light_Sensors_STM32
 */

#include "BH1750.h"

BH1750_mode bh1750_mode = BH1750_DEFAULT_MODE; // Current sensor mode
uint8_t bh1750_mt_reg = BH1750_DEFAULT_MTREG; // Current MT register value
uint8_t bh1750_addr = BH1750_ADDRESS;

BH1750_STATUS bh1750_init() {
    if(BH1750_OK == bh1750_reset()) {
        if(BH1750_OK == bh1750_set_mt_reg(BH1750_DEFAULT_MTREG)) {
            return BH1750_OK;
        }
    }
    return BH1750_ERROR;
}

BH1750_STATUS bh1750_init_with_addr(uint8_t addr) {
    bh1750_addr = (addr << 1);
    return bh1750_init();
}

BH1750_STATUS bh1750_reset() {
    uint8_t command = 0x07;
    bool status;

    furi_hal_i2c_acquire(I2C_BUS);
    status = furi_hal_i2c_tx(I2C_BUS, bh1750_addr, &command, 1, I2C_TIMEOUT);
    furi_hal_i2c_release(I2C_BUS);

    if(status) {
        return BH1750_OK;
    }

    return BH1750_ERROR;
}

BH1750_STATUS bh1750_set_power_state(uint8_t PowerOn) {
    PowerOn = (PowerOn ? 1 : 0);
    bool status;

    furi_hal_i2c_acquire(I2C_BUS);
    status = furi_hal_i2c_tx(I2C_BUS, bh1750_addr, &PowerOn, 1, I2C_TIMEOUT);
    furi_hal_i2c_release(I2C_BUS);

    if(status) {
        return BH1750_OK;
    }

    return BH1750_ERROR;
}

BH1750_STATUS bh1750_set_mode(BH1750_mode mode) {
    if(!((mode >> 4) || (mode >> 5))) {
        return BH1750_ERROR;
    }

    if((mode & 0x0F) > 3) {
        return BH1750_ERROR;
    }

    bool status;

    bh1750_mode = mode;

    furi_hal_i2c_acquire(I2C_BUS);
    status = furi_hal_i2c_tx(I2C_BUS, bh1750_addr, &mode, 1, I2C_TIMEOUT);
    furi_hal_i2c_release(I2C_BUS);

    if(status) {
        return BH1750_OK;
    }

    return BH1750_ERROR;
}

BH1750_STATUS bh1750_set_mt_reg(uint8_t mt_reg) {
    if(mt_reg < 31 || mt_reg > 254) {
        return BH1750_ERROR;
    }

    bh1750_mt_reg = mt_reg;

    uint8_t tmp[2];
    bool status;

    tmp[0] = (0x40 | (mt_reg >> 5));
    tmp[1] = (0x60 | (mt_reg & 0x1F));

    furi_hal_i2c_acquire(I2C_BUS);
    status = furi_hal_i2c_tx(I2C_BUS, bh1750_addr, &tmp[0], 1, I2C_TIMEOUT);
    furi_hal_i2c_release(I2C_BUS);
    if(!status) {
        return BH1750_ERROR;
    }

    furi_hal_i2c_acquire(I2C_BUS);
    status = furi_hal_i2c_tx(I2C_BUS, bh1750_addr, &tmp[1], 1, I2C_TIMEOUT);
    furi_hal_i2c_release(I2C_BUS);
    if(status) {
        return BH1750_OK;
    }

    return BH1750_ERROR;
}

BH1750_STATUS bh1750_trigger_manual_conversion() {
    if(BH1750_OK == bh1750_set_mode(bh1750_mode)) {
        return BH1750_OK;
    }
    return BH1750_ERROR;
}

BH1750_STATUS bh1750_read_light(float* result) {
    float result_tmp;
    uint8_t rcv[2];
    bool status;

    furi_hal_i2c_acquire(I2C_BUS);
    status = furi_hal_i2c_rx(I2C_BUS, bh1750_addr, rcv, 2, I2C_TIMEOUT);
    furi_hal_i2c_release(I2C_BUS);

    if(status) {
        result_tmp = (rcv[0] << 8) | (rcv[1]);

        if(bh1750_mt_reg != BH1750_DEFAULT_MTREG) {
            result_tmp *= (float)((uint8_t)BH1750_DEFAULT_MTREG / (float)bh1750_mt_reg);
        }

        if(bh1750_mode == ONETIME_HIGH_RES_MODE_2 || bh1750_mode == CONTINUOUS_HIGH_RES_MODE_2) {
            result_tmp /= 2.0;
        }

        *result = result_tmp / BH1750_CONVERSION_FACTOR;

        return BH1750_OK;
    }
    return BH1750_ERROR;
}
