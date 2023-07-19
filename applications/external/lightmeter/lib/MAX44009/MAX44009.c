#include <MAX44009.h>
#include <math.h>
#include <furi.h>

void max44009_init() {
    furi_hal_i2c_acquire(I2C_BUS);
    furi_hal_i2c_write_reg_8(I2C_BUS, MAX44009_ADDR,
        MAX44009_REG_CONFIG, MAX44009_REG_CONFIG_CONT_MODE, I2C_TIMEOUT);
    furi_hal_i2c_release(I2C_BUS);
}

int max44009_read_light(float* result) {
    uint8_t data_one = 0;
    uint8_t exp, mantissa;
    int status;

    furi_hal_i2c_acquire(I2C_BUS);
    furi_hal_i2c_read_reg_8(I2C_BUS, MAX44009_ADDR, MAX44009_REG_LUX_HI, &data_one, I2C_TIMEOUT);
    exp = (data_one & MAX44009_REG_LUX_HI_EXP_MASK) >> 4;
    mantissa = (data_one & MAX44009_REG_LUX_HI_MANT_HI_MASK) << 4;
    status = furi_hal_i2c_read_reg_8(I2C_BUS, MAX44009_ADDR, MAX44009_REG_LUX_LO, &data_one, I2C_TIMEOUT);
    mantissa |= (data_one & MAX44009_REG_LUX_LO_MANT_LO_MASK);
    furi_hal_i2c_release(I2C_BUS);
    *result = (float)pow(2, exp) * mantissa * 0.045;
    FURI_LOG_D("MAX44009", "exp %d, mant %d, lux %f", exp, mantissa, (double)*result);
    return status;
}