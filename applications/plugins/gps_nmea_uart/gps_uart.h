#pragma once

#include <furi_hal.h>

#define GPS_BAUDRATE 9600
#define RX_BUF_SIZE 1024

typedef struct {
    bool valid;
    float latitude;
    float longitude;
    float speed;
    float course;
    float altitude;
    char altitude_units;
    int fix_quality;
    int satellites_tracked;
} GpsStatus;

typedef struct {
    FuriThread* thread;
    FuriStreamBuffer* rx_stream;
    uint8_t rx_buf[RX_BUF_SIZE];

    GpsStatus status;
} GpsUart;

GpsUart* gps_uart_enable();

void gps_uart_disable(GpsUart* gps_uart);
