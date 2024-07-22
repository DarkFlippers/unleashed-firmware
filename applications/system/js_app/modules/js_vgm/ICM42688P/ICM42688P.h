#pragma once

#include <furi.h>
#include <furi_hal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DataRate32kHz = 0x01,
    DataRate16kHz = 0x02,
    DataRate8kHz = 0x03,
    DataRate4kHz = 0x04,
    DataRate2kHz = 0x05,
    DataRate1kHz = 0x06,
    DataRate200Hz = 0x07,
    DataRate100Hz = 0x08,
    DataRate50Hz = 0x09,
    DataRate25Hz = 0x0A,
    DataRate12_5Hz = 0x0B,
    DataRate6_25Hz = 0x0C, // Accelerometer only
    DataRate3_125Hz = 0x0D, // Accelerometer only
    DataRate1_5625Hz = 0x0E, // Accelerometer only
    DataRate500Hz = 0x0F,
} ICM42688PDataRate;

typedef enum {
    AccelFullScale16G = 0,
    AccelFullScale8G,
    AccelFullScale4G,
    AccelFullScale2G,
    AccelFullScaleTotal,
} ICM42688PAccelFullScale;

typedef enum {
    GyroFullScale2000DPS = 0,
    GyroFullScale1000DPS,
    GyroFullScale500DPS,
    GyroFullScale250DPS,
    GyroFullScale125DPS,
    GyroFullScale62_5DPS,
    GyroFullScale31_25DPS,
    GyroFullScale15_625DPS,
    GyroFullScaleTotal,
} ICM42688PGyroFullScale;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} __attribute__((packed)) ICM42688PRawData;

typedef struct {
    uint8_t header;
    int16_t a_x;
    int16_t a_y;
    int16_t a_z;
    int16_t g_x;
    int16_t g_y;
    int16_t g_z;
    uint8_t temp;
    uint16_t ts;
} __attribute__((packed)) ICM42688PFifoPacket;

typedef struct {
    float x;
    float y;
    float z;
} ICM42688PScaledData;

typedef struct ICM42688P ICM42688P;

typedef void (*ICM42688PIrqCallback)(void* ctx);

ICM42688P* icm42688p_alloc(FuriHalSpiBusHandle* spi_bus, const GpioPin* irq_pin);

bool icm42688p_init(ICM42688P* icm42688p);

bool icm42688p_deinit(ICM42688P* icm42688p);

void icm42688p_free(ICM42688P* icm42688p);

bool icm42688p_accel_config(
    ICM42688P* icm42688p,
    ICM42688PAccelFullScale full_scale,
    ICM42688PDataRate rate);

float icm42688p_accel_get_full_scale(ICM42688P* icm42688p);

bool icm42688p_gyro_config(
    ICM42688P* icm42688p,
    ICM42688PGyroFullScale full_scale,
    ICM42688PDataRate rate);

float icm42688p_gyro_get_full_scale(ICM42688P* icm42688p);

bool icm42688p_read_accel_raw(ICM42688P* icm42688p, ICM42688PRawData* data);

bool icm42688p_read_gyro_raw(ICM42688P* icm42688p, ICM42688PRawData* data);

bool icm42688p_write_gyro_offset(ICM42688P* icm42688p, ICM42688PScaledData* scaled_data);

void icm42688p_apply_scale(ICM42688PRawData* raw_data, float full_scale, ICM42688PScaledData* data);

void icm42688p_apply_scale_fifo(
    ICM42688P* icm42688p,
    ICM42688PFifoPacket* fifo_data,
    ICM42688PScaledData* accel_data,
    ICM42688PScaledData* gyro_data);

float icm42688p_read_temp(ICM42688P* icm42688p);

void icm42688_fifo_enable(
    ICM42688P* icm42688p,
    ICM42688PIrqCallback irq_callback,
    void* irq_context);

void icm42688_fifo_disable(ICM42688P* icm42688p);

uint16_t icm42688_fifo_get_count(ICM42688P* icm42688p);

bool icm42688_fifo_read(ICM42688P* icm42688p, ICM42688PFifoPacket* data);

#ifdef __cplusplus
}
#endif
