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

#include <furi.h>
#include <furi_hal.h>

#ifndef BH1750_H_
#define BH1750_H_

// I2C BUS
#define I2C_BUS &furi_hal_i2c_handle_external
#define I2C_TIMEOUT 10

#define BH1750_ADDRESS (0x23 << 1)

#define BH1750_POWER_DOWN 0x00
#define BH1750_POWER_ON 0x01
#define BH1750_RESET 0x07
#define BH1750_DEFAULT_MTREG 69
#define BH1750_DEFAULT_MODE ONETIME_HIGH_RES_MODE

#define BH1750_CONVERSION_FACTOR 1.2

typedef enum { BH1750_OK = 0, BH1750_ERROR = 1 } BH1750_STATUS;

typedef enum {
    CONTINUOUS_HIGH_RES_MODE = 0x10,
    CONTINUOUS_HIGH_RES_MODE_2 = 0x11,
    CONTINUOUS_LOW_RES_MODE = 0x13,
    ONETIME_HIGH_RES_MODE = 0x20,
    ONETIME_HIGH_RES_MODE_2 = 0x21,
    ONETIME_LOW_RES_MODE = 0x23
} BH1750_mode;

/**
 * @brief Initialize the sensor. Sends the reset command and sets the measurement register to the default value.
 * 
 * @return BH1750_STATUS
 */
BH1750_STATUS bh1750_init();

/**
 * @brief Change the I2C device address and then initialize the sensor.
 * 
 * @return BH1750_STATUS
 */
BH1750_STATUS bh1750_init_with_addr(uint8_t addr);

/**
 * @brief Reset all registers to the default value.
 * 
 * @return BH1750_STATUS 
 */
BH1750_STATUS bh1750_reset();

/**
 * @brief Sets the power state. 1 - running; 0 - sleep, low power. 
 * 
 * @param PowerOn sensor state.
 * @return BH1750_STATUS 
 */
BH1750_STATUS bh1750_set_power_state(uint8_t PowerOn);

/**
 * @brief Set the Measurement Time register. It allows to increase or decrease the sensitivity.
 * 
 * @param MTreg value from 31 to 254, defaults to 69.
 * 
 * @return BH1750_STATUS 
 */
BH1750_STATUS bh1750_set_mt_reg(uint8_t MTreg);

/**
 * @brief Set the mode of converting. Look into the bh1750_mode enum.
 * 
 * @param Mode mode enumerator
 * @return BH1750_STATUS 
 */
BH1750_STATUS bh1750_set_mode(BH1750_mode Mode);

/**
 * @brief Trigger the conversion in manual modes. 
 * 
 * @details a low-resolution mode, the conversion time is typically 16 ms, and for a high-resolution 
 * mode is 120 ms. You need to wait until reading the measurement value. There is no need 
 * to exit low-power mode for manual conversion. It makes automatically.
 * 
 * @return BH1750_STATUS 
 */
BH1750_STATUS bh1750_trigger_manual_conversion();

/**
 * @brief Read the converted value and calculate the result.
 * 
 * @param Result stores received value to this variable.
 * @return BH1750_STATUS 
 */
BH1750_STATUS bh1750_read_light(float* Result);

#endif /* BH1750_H_ */
