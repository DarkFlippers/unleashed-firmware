
#include <bq25896.h>
#include <bq25896_reg.h>
#include <i2c.h>

uint8_t bit_reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

bool bq25896_read(uint8_t address, uint8_t* data, size_t size) {
    if (HAL_I2C_Master_Transmit(&POWER_I2C, BQ25896_ADDRESS, &address, 1, 2000) != HAL_OK) {
        return false;
    }
    if (HAL_I2C_Master_Receive(&POWER_I2C, BQ25896_ADDRESS, data, size, 2000) != HAL_OK) {
        return false;
    }
    return true;
}

bool bq25896_read_reg(uint8_t address, uint8_t* data) {
    bq25896_read(address, data, 1);
    return true;
}

bool bq25896_write_reg(uint8_t address, uint8_t* data) {
    uint8_t buffer[2] = { address, *data };

    if (HAL_I2C_Master_Transmit(&POWER_I2C, BQ25896_ADDRESS, buffer, 2, 2000) != HAL_OK) {
        return false;
    }
    return true;
}

typedef struct {
    REG00 r00;
    REG01 r01;
    REG02 r02;
    REG03 r03;
    REG04 r04;
    REG05 r05;
    REG06 r06;
    REG07 r07;
    REG08 r08;
    REG09 r09;
    REG0A r0A;
    REG0B r0B;
    REG0C r0C;
    REG0D r0D;
    REG0E r0E;
    REG0F r0F;
    REG10 r10;
    REG11 r11;
    REG12 r12;
    REG13 r13;
    REG14 r14;
} bq25896_regs_t;

static bq25896_regs_t bq25896_regs;

void bq25896_init() {
    bq25896_read(0x00, (uint8_t*)&bq25896_regs, sizeof(bq25896_regs));

    bq25896_regs.r09.BATFET_DIS = 0;
    bq25896_write_reg(0x09, (uint8_t*)&bq25896_regs.r09);

    bq25896_regs.r14.REG_RST = 1;
    bq25896_write_reg(0x14, (uint8_t*)&bq25896_regs.r14);

    bq25896_read(0x00, (uint8_t*)&bq25896_regs, sizeof(bq25896_regs));
}

void bq25896_poweroff() {
    bq25896_regs.r09.BATFET_DIS=1;
    bq25896_write_reg(0x09, (uint8_t*)&bq25896_regs.r09);
}

bool bq25896_is_charging() {
    bq25896_read_reg(0x0B, (uint8_t*)&bq25896_regs.r0B);
    return bq25896_regs.r0B.CHRG_STAT != ChrgStatNo;
}

void bq25896_enable_otg() {
    bq25896_regs.r03.OTG_CONFIG = 1;
    bq25896_write_reg(0x03, (uint8_t*)&bq25896_regs.r03);
}

void bq25896_disable_otg() {
    bq25896_regs.r03.OTG_CONFIG = 0;
    bq25896_write_reg(0x03, (uint8_t*)&bq25896_regs.r03);
}

void bq25896_adc_sample() {
    bq25896_regs.r02.CONV_START = 1;
    bq25896_write_reg(0x02, (uint8_t*)&bq25896_regs.r02);
    while(bq25896_regs.r02.CONV_START == 1) {
        bq25896_read_reg(0x02, (uint8_t*)&bq25896_regs.r02);
    }
}

uint16_t bq25896_get_vbus_voltage() {
    bq25896_adc_sample();
    bq25896_read_reg(0x11, (uint8_t*)&bq25896_regs.r11);
    if (bq25896_regs.r11.VBUS_GD) {
        return (uint16_t)bq25896_regs.r11.VBUSV * 100 + 2600;
    } else {
        return 0;
    }
}

uint16_t bq25896_get_vsys_voltage() {
    bq25896_adc_sample();
    bq25896_read_reg(0x0F, (uint8_t*)&bq25896_regs.r0F);
    return (uint16_t)bq25896_regs.r0F.SYSV * 20 + 2304;
}

uint16_t bq25896_get_vbat_voltage() {
    bq25896_adc_sample();
    bq25896_read_reg(0x0E, (uint8_t*)&bq25896_regs.r0E);
    return (uint16_t)bq25896_regs.r0E.BATV * 20 + 2304;
}

uint16_t bq25896_get_vbat_current() {
    bq25896_adc_sample();
    bq25896_read_reg(0x12, (uint8_t*)&bq25896_regs.r12);
    return (uint16_t)bq25896_regs.r12.ICHGR * 50;
}

uint32_t bq25896_get_ntc_mpct() {
    bq25896_adc_sample();
    bq25896_read_reg(0x10, (uint8_t*)&bq25896_regs.r10);
    return (uint32_t)bq25896_regs.r10.TSPCT * 465+21000;
}
