#pragma once

#include <furi_hal.h>
#include <notification/notification_messages.h>

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
    int time_hours;
    int time_minutes;
    int time_seconds;
} GpsStatus;

typedef struct {
    FuriMutex* mutex;
    FuriThread* thread;
    FuriStreamBuffer* rx_stream;
    uint8_t rx_buf[RX_BUF_SIZE];

    NotificationApp* notifications;

    GpsStatus status;
} GpsUart;

GpsUart* gps_uart_enable();

void gps_uart_disable(GpsUart* gps_uart);
