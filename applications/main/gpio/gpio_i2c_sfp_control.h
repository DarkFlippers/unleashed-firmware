#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <furi_hal_i2c.h>

#define FIRST_NON_RESERVED_I2C_ADDRESS 8
#define HIGHEST_I2C_ADDRESS 127
#define AVAILABLE_NONRESVERED_I2C_ADDRESSES 120
#define SFP_I2C_ADDRESS 0x50

typedef struct {
    char vendor[32];
    char oui[32];
    char rev[32];
    char pn[32];
    char sn[32];
    char dc[32];
    uint8_t type;
    char connector[32];
    int wavelength;
    int sm_reach;
    int mm_reach_om3;
    int bitrate;
} I2CSfpState;

/** Reads data from a connected SFP on I2C-Bus (SDA: Pin 15, SCL: Pin 16). Saves data from SFP.
 *
 * @param      i2c_sfp_state  Data collected from SFP.
 */
void gpio_i2c_sfp_run_once(I2CSfpState* st);
