#include <furi.h>
#include <furi_hal.h>

#pragma once

// I2C BUS
#define I2C_BUS &furi_hal_i2c_handle_external
#define I2C_TIMEOUT 10

#define MAX44009_ADDR (0x4A << 1)

#define MAX44009_REG_INT_STATUS 0x00
#define MAX44009_REG_INT_EN 0x01
#define MAX44009_REG_CONFIG 0x02
#define MAX44009_REG_CONFIG_CONT_MODE (1 << 7)
#define MAX44009_REG_LUX_HI 0x03
#define MAX44009_REG_LUX_HI_EXP_MASK 0xF0
#define MAX44009_REG_LUX_HI_MANT_HI_MASK 0x0F
#define MAX44009_REG_LUX_LO 0x04
#define MAX44009_REG_LUX_LO_MANT_LO_MASK 0x0F
#define MAX44009_REG_THRESH_HI 0x05
#define MAX44009_REG_THRESH_LO 0x06
#define MAX44009_REG_INT_TIME 0x07

void max44009_init();
void max44009_init_with_addr(uint8_t addr);
int max44009_read_light(float* result);
