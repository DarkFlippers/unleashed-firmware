#include "lp5562.h"
#include "lp5562_reg.h"

#include <api-hal-i2c.h>
#include <stdio.h>

bool lp5562_write_reg(uint8_t address, uint8_t* data) {
    uint8_t buffer[2] = {address, *data};
    bool ret;
    with_api_hal_i2c(
        bool, &ret, () {
            return api_hal_i2c_tx(POWER_I2C, LP5562_ADDRESS, buffer, 2, LP5562_I2C_TIMEOUT);
        });
    return ret;
}

void lp5562_reset() {
    Reg0D_Reset reg = {.value = 0xFF};
    lp5562_write_reg(0x0D, (uint8_t*)&reg);
}

void lp5562_configure() {
    Reg08_Config config = {.INT_CLK_EN = true, .PS_EN = true, .PWM_HF = true};
    lp5562_write_reg(0x08, (uint8_t*)&config);

    Reg70_LedMap map = {
        .red = EngSelectI2C,
        .green = EngSelectI2C,
        .blue = EngSelectI2C,
        .white = EngSelectI2C,
    };
    lp5562_write_reg(0x70, (uint8_t*)&map);
}

void lp5562_enable() {
    Reg00_Enable reg = {.CHIP_EN = true, .LOG_EN = true};
    lp5562_write_reg(0x00, (uint8_t*)&reg);
}

void lp5562_set_channel_current(LP5562Channel channel, uint8_t value) {
    uint8_t reg_no;
    if(channel == LP5562ChannelRed) {
        reg_no = 0x07;
    } else if(channel == LP5562ChannelGreen) {
        reg_no = 0x06;
    } else if(channel == LP5562ChannelBlue) {
        reg_no = 0x05;
    } else if(channel == LP5562ChannelWhite) {
        reg_no = 0x0F;
    } else {
        return;
    }
    lp5562_write_reg(reg_no, &value);
}

void lp5562_set_channel_value(LP5562Channel channel, uint8_t value) {
    uint8_t reg_no;
    if(channel == LP5562ChannelRed) {
        reg_no = 0x04;
    } else if(channel == LP5562ChannelGreen) {
        reg_no = 0x03;
    } else if(channel == LP5562ChannelBlue) {
        reg_no = 0x02;
    } else if(channel == LP5562ChannelWhite) {
        reg_no = 0x0E;
    } else {
        return;
    }
    lp5562_write_reg(reg_no, &value);
}
