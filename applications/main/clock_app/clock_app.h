#pragma once

#include <input/input.h>
#include <locale/locale.h>

#define TAG "Clock"

#define CLOCK_ISO_DATE_FORMAT "%.4d-%.2d-%.2d"
#define CLOCK_RFC_DATE_FORMAT "%.2d-%.2d-%.4d"
#define CLOCK_TIME_FORMAT "%.2d:%.2d:%.2d"

#define MERIDIAN_FORMAT "%s"
#define MERIDIAN_STRING_AM "AM"
#define MERIDIAN_STRING_PM "PM"

#define TIME_LEN 12
#define DATE_LEN 14
#define MERIDIAN_LEN 3

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    LocaleDateFormat date_format;
    LocaleTimeFormat time_format;
    FuriHalRtcDateTime datetime;
    FuriMutex* mutex;
    FuriMessageQueue* event_queue;
    uint32_t timer_start_timestamp;
    uint32_t timer_stopped_seconds;
    bool timer_running;
} ClockState;
