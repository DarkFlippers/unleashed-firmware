#include "ICM42688P_regs.h"
#include "ICM42688P.h"

#define TAG "ICM42688P"

#define ICM42688P_TIMEOUT 100

struct ICM42688P {
    FuriHalSpiBusHandle* spi_bus;
    const GpioPin* irq_pin;
    float accel_scale;
    float gyro_scale;
};

static const struct AccelFullScale {
    float value;
    uint8_t reg_mask;
} accel_fs_modes[] = {
    [AccelFullScale16G] = {16.f, ICM42688_AFS_16G},
    [AccelFullScale8G] = {8.f, ICM42688_AFS_8G},
    [AccelFullScale4G] = {4.f, ICM42688_AFS_4G},
    [AccelFullScale2G] = {2.f, ICM42688_AFS_2G},
};

static const struct GyroFullScale {
    float value;
    uint8_t reg_mask;
} gyro_fs_modes[] = {
    [GyroFullScale2000DPS] = {2000.f, ICM42688_GFS_2000DPS},
    [GyroFullScale1000DPS] = {1000.f, ICM42688_GFS_1000DPS},
    [GyroFullScale500DPS] = {500.f, ICM42688_GFS_500DPS},
    [GyroFullScale250DPS] = {250.f, ICM42688_GFS_250DPS},
    [GyroFullScale125DPS] = {125.f, ICM42688_GFS_125DPS},
    [GyroFullScale62_5DPS] = {62.5f, ICM42688_GFS_62_5DPS},
    [GyroFullScale31_25DPS] = {31.25f, ICM42688_GFS_31_25DPS},
    [GyroFullScale15_625DPS] = {15.625f, ICM42688_GFS_15_625DPS},
};

static bool icm42688p_write_reg(FuriHalSpiBusHandle* spi_bus, uint8_t addr, uint8_t value) {
    bool res = false;
    furi_hal_spi_acquire(spi_bus);
    do {
        uint8_t cmd_data[2] = {addr & 0x7F, value};
        if(!furi_hal_spi_bus_tx(spi_bus, cmd_data, 2, ICM42688P_TIMEOUT)) break;
        res = true;
    } while(0);
    furi_hal_spi_release(spi_bus);
    return res;
}

static bool icm42688p_read_reg(FuriHalSpiBusHandle* spi_bus, uint8_t addr, uint8_t* value) {
    bool res = false;
    furi_hal_spi_acquire(spi_bus);
    do {
        uint8_t cmd_byte = addr | (1 << 7);
        if(!furi_hal_spi_bus_tx(spi_bus, &cmd_byte, 1, ICM42688P_TIMEOUT)) break;
        if(!furi_hal_spi_bus_rx(spi_bus, value, 1, ICM42688P_TIMEOUT)) break;
        res = true;
    } while(0);
    furi_hal_spi_release(spi_bus);
    return res;
}

static bool
    icm42688p_read_mem(FuriHalSpiBusHandle* spi_bus, uint8_t addr, uint8_t* data, uint8_t len) {
    bool res = false;
    furi_hal_spi_acquire(spi_bus);
    do {
        uint8_t cmd_byte = addr | (1 << 7);
        if(!furi_hal_spi_bus_tx(spi_bus, &cmd_byte, 1, ICM42688P_TIMEOUT)) break;
        if(!furi_hal_spi_bus_rx(spi_bus, data, len, ICM42688P_TIMEOUT)) break;
        res = true;
    } while(0);
    furi_hal_spi_release(spi_bus);
    return res;
}

bool icm42688p_accel_config(
    ICM42688P* icm42688p,
    ICM42688PAccelFullScale full_scale,
    ICM42688PDataRate rate) {
    icm42688p->accel_scale = accel_fs_modes[full_scale].value;
    uint8_t reg_value = accel_fs_modes[full_scale].reg_mask | rate;
    return icm42688p_write_reg(icm42688p->spi_bus, ICM42688_ACCEL_CONFIG0, reg_value);
}

float icm42688p_accel_get_full_scale(ICM42688P* icm42688p) {
    return icm42688p->accel_scale;
}

bool icm42688p_gyro_config(
    ICM42688P* icm42688p,
    ICM42688PGyroFullScale full_scale,
    ICM42688PDataRate rate) {
    icm42688p->gyro_scale = gyro_fs_modes[full_scale].value;
    uint8_t reg_value = gyro_fs_modes[full_scale].reg_mask | rate;
    return icm42688p_write_reg(icm42688p->spi_bus, ICM42688_GYRO_CONFIG0, reg_value);
}

float icm42688p_gyro_get_full_scale(ICM42688P* icm42688p) {
    return icm42688p->gyro_scale;
}

bool icm42688p_read_accel_raw(ICM42688P* icm42688p, ICM42688PRawData* data) {
    bool ret = icm42688p_read_mem(
        icm42688p->spi_bus, ICM42688_ACCEL_DATA_X1, (uint8_t*)data, sizeof(ICM42688PRawData));
    return ret;
}

bool icm42688p_read_gyro_raw(ICM42688P* icm42688p, ICM42688PRawData* data) {
    bool ret = icm42688p_read_mem(
        icm42688p->spi_bus, ICM42688_GYRO_DATA_X1, (uint8_t*)data, sizeof(ICM42688PRawData));
    return ret;
}

bool icm42688p_write_gyro_offset(ICM42688P* icm42688p, ICM42688PScaledData* scaled_data) {
    if((fabsf(scaled_data->x) > 64.f) || (fabsf(scaled_data->y) > 64.f) ||
       (fabsf(scaled_data->z) > 64.f)) {
        return false;
    }

    uint16_t offset_x = (uint16_t)(-(int16_t)(scaled_data->x * 32.f) * 16) >> 4;
    uint16_t offset_y = (uint16_t)(-(int16_t)(scaled_data->y * 32.f) * 16) >> 4;
    uint16_t offset_z = (uint16_t)(-(int16_t)(scaled_data->z * 32.f) * 16) >> 4;

    uint8_t offset_regs[9];
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_REG_BANK_SEL, 4);
    icm42688p_read_mem(icm42688p->spi_bus, ICM42688_OFFSET_USER0, offset_regs, 5);

    offset_regs[0] = offset_x & 0xFF;
    offset_regs[1] = (offset_x & 0xF00) >> 8;
    offset_regs[1] |= (offset_y & 0xF00) >> 4;
    offset_regs[2] = offset_y & 0xFF;
    offset_regs[3] = offset_z & 0xFF;
    offset_regs[4] &= 0xF0;
    offset_regs[4] |= (offset_z & 0x0F00) >> 8;

    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_OFFSET_USER0, offset_regs[0]);
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_OFFSET_USER1, offset_regs[1]);
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_OFFSET_USER2, offset_regs[2]);
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_OFFSET_USER3, offset_regs[3]);
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_OFFSET_USER4, offset_regs[4]);

    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_REG_BANK_SEL, 0);
    return true;
}

void icm42688p_apply_scale(ICM42688PRawData* raw_data, float full_scale, ICM42688PScaledData* data) {
    data->x = ((float)(raw_data->x)) / 32768.f * full_scale;
    data->y = ((float)(raw_data->y)) / 32768.f * full_scale;
    data->z = ((float)(raw_data->z)) / 32768.f * full_scale;
}

void icm42688p_apply_scale_fifo(
    ICM42688P* icm42688p,
    ICM42688PFifoPacket* fifo_data,
    ICM42688PScaledData* accel_data,
    ICM42688PScaledData* gyro_data) {
    float full_scale = icm42688p->accel_scale;
    accel_data->x = ((float)(fifo_data->a_x)) / 32768.f * full_scale;
    accel_data->y = ((float)(fifo_data->a_y)) / 32768.f * full_scale;
    accel_data->z = ((float)(fifo_data->a_z)) / 32768.f * full_scale;

    full_scale = icm42688p->gyro_scale;
    gyro_data->x = ((float)(fifo_data->g_x)) / 32768.f * full_scale;
    gyro_data->y = ((float)(fifo_data->g_y)) / 32768.f * full_scale;
    gyro_data->z = ((float)(fifo_data->g_z)) / 32768.f * full_scale;
}

float icm42688p_read_temp(ICM42688P* icm42688p) {
    uint8_t reg_val[2];

    icm42688p_read_mem(icm42688p->spi_bus, ICM42688_TEMP_DATA1, reg_val, 2);
    int16_t temp_int = (reg_val[0] << 8) | reg_val[1];
    return ((float)temp_int / 132.48f) + 25.f;
}

void icm42688_fifo_enable(
    ICM42688P* icm42688p,
    ICM42688PIrqCallback irq_callback,
    void* irq_context) {
    // FIFO mode: stream
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_FIFO_CONFIG, (1 << 6));
    // Little-endian data, FIFO count in records
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INTF_CONFIG0, (1 << 7) | (1 << 6));
    // FIFO partial read, FIFO packet: gyro + accel TODO: 20bit
    icm42688p_write_reg(
        icm42688p->spi_bus, ICM42688_FIFO_CONFIG1, (1 << 6) | (1 << 5) | (1 << 1) | (1 << 0));
    // FIFO irq watermark
    uint16_t fifo_watermark = 1;
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_FIFO_CONFIG2, fifo_watermark & 0xFF);
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_FIFO_CONFIG3, fifo_watermark >> 8);

    // IRQ1: push-pull, active high
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INT_CONFIG, (1 << 1) | (1 << 0));
    // Clear IRQ on status read
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INT_CONFIG0, 0);
    // IRQ pulse duration
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INT_CONFIG1, (1 << 6) | (1 << 5));

    uint8_t reg_data = 0;
    icm42688p_read_reg(icm42688p->spi_bus, ICM42688_INT_STATUS, &reg_data);

    furi_hal_gpio_init(icm42688p->irq_pin, GpioModeInterruptRise, GpioPullDown, GpioSpeedVeryHigh);
    furi_hal_gpio_remove_int_callback(icm42688p->irq_pin);
    furi_hal_gpio_add_int_callback(icm42688p->irq_pin, irq_callback, irq_context);

    // IRQ1 source: FIFO threshold
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INT_SOURCE0, (1 << 2));
}

void icm42688_fifo_disable(ICM42688P* icm42688p) {
    furi_hal_gpio_remove_int_callback(icm42688p->irq_pin);
    furi_hal_gpio_init(icm42688p->irq_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INT_SOURCE0, 0);

    // FIFO mode: bypass
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_FIFO_CONFIG, 0);
}

uint16_t icm42688_fifo_get_count(ICM42688P* icm42688p) {
    uint16_t reg_val = 0;
    icm42688p_read_mem(icm42688p->spi_bus, ICM42688_FIFO_COUNTH, (uint8_t*)&reg_val, 2);
    return reg_val;
}

bool icm42688_fifo_read(ICM42688P* icm42688p, ICM42688PFifoPacket* data) {
    icm42688p_read_mem(
        icm42688p->spi_bus, ICM42688_FIFO_DATA, (uint8_t*)data, sizeof(ICM42688PFifoPacket));
    return (data->header) & (1 << 7);
}

ICM42688P* icm42688p_alloc(FuriHalSpiBusHandle* spi_bus, const GpioPin* irq_pin) {
    ICM42688P* icm42688p = malloc(sizeof(ICM42688P));
    icm42688p->spi_bus = spi_bus;
    icm42688p->irq_pin = irq_pin;
    return icm42688p;
}

void icm42688p_free(ICM42688P* icm42688p) {
    free(icm42688p);
}

bool icm42688p_init(ICM42688P* icm42688p) {
    furi_hal_spi_bus_handle_init(icm42688p->spi_bus);

    // Software reset
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_REG_BANK_SEL, 0); // Set reg bank to 0
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_DEVICE_CONFIG, 0x01); // SPI Mode 0, SW reset
    furi_delay_ms(1);

    uint8_t reg_value = 0;
    bool read_ok = icm42688p_read_reg(icm42688p->spi_bus, ICM42688_WHO_AM_I, &reg_value);
    if(!read_ok) {
        FURI_LOG_E(TAG, "Chip ID read failed");
        return false;
    } else if(reg_value != ICM42688_WHOAMI) {
        FURI_LOG_E(
            TAG, "Sensor returned wrong ID 0x%02X, expected 0x%02X", reg_value, ICM42688_WHOAMI);
        return false;
    }

    // Disable all interrupts
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INT_SOURCE0, 0);
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INT_SOURCE1, 0);
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INT_SOURCE3, 0);
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INT_SOURCE4, 0);
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_REG_BANK_SEL, 4);
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INT_SOURCE6, 0);
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INT_SOURCE7, 0);
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_REG_BANK_SEL, 0);

    // Data format: little endian
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_INTF_CONFIG0, 0);

    // Enable all sensors
    icm42688p_write_reg(
        icm42688p->spi_bus,
        ICM42688_PWR_MGMT0,
        ICM42688_PWR_TEMP_ON | ICM42688_PWR_GYRO_MODE_LN | ICM42688_PWR_ACCEL_MODE_LN);
    furi_delay_ms(45);

    icm42688p_accel_config(icm42688p, AccelFullScale16G, DataRate1kHz);
    icm42688p_gyro_config(icm42688p, GyroFullScale2000DPS, DataRate1kHz);

    return true;
}

bool icm42688p_deinit(ICM42688P* icm42688p) {
    // Software reset
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_REG_BANK_SEL, 0); // Set reg bank to 0
    icm42688p_write_reg(icm42688p->spi_bus, ICM42688_DEVICE_CONFIG, 0x01); // SPI Mode 0, SW reset

    furi_hal_spi_bus_handle_deinit(icm42688p->spi_bus);
    return true;
}
