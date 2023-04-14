#pragma once

#include <furi_hal.h>
#include <notification/notification_messages.h>

#define RX_BUF_SIZE 1024

static const int gps_baudrates[5] = {9600, 19200, 38400, 57600, 115200};
static int current_gps_baudrate = 3;

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
    uint32_t baudrate;
    bool changing_baudrate;
    bool backlight_on;
    bool speed_in_kms;

    GpsStatus status;
} GpsUart;

void gps_uart_init_thread(GpsUart* gps_uart);
void gps_uart_deinit_thread(GpsUart* gps_uart);

GpsUart* gps_uart_enable();

void gps_uart_disable(GpsUart* gps_uart);
