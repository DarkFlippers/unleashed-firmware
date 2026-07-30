#pragma once

#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/variable_item_list.h>
#include <input/input.h>
#include <locale/locale.h>
#include <datetime/datetime.h>
#include <storage/storage.h> // for APP_DATA_PATH (reverted to EXT_PATH at the moment due to app type change)

#define TAG "Clock"

#define CLOCK_ISO_DATE_FORMAT "%.4d-%.2d-%.2d"
#define CLOCK_RFC_DATE_FORMAT "%.2d-%.2d-%.4d"
#define CLOCK_TIME_FORMAT     "%.2d:%.2d:%.2d"

#define MERIDIAN_FORMAT    "%s"
#define MERIDIAN_STRING_AM "AM"
#define MERIDIAN_STRING_PM "PM"

#define TIME_LEN     12
#define DATE_LEN     14
#define MERIDIAN_LEN 3
#define BATTERY_LEN  4
#define DATE_PCT_LEN 21

// Settings persist so the alarm and brightness survive closing the app.
// NS_SETTINGS_DIR is created at startup so the save always has somewhere to go.
#define NS_SETTINGS_PATH    EXT_PATH("apps_data/clock/settings.save")
#define NS_SETTINGS_DIR     EXT_PATH("apps_data/clock")
#define NS_SETTINGS_MAGIC   0x4E // 'N'
#define NS_SETTINGS_VERSION 2 // bumped: struct gained the brightness field

typedef enum {
    NsViewClock,
    NsViewAlarmMenu,
    NsViewAlarmTime,
} NsViewId;

// The periodic timer runs on its own task; it only pokes the dispatcher with
// this custom event so all the state work happens on the GUI thread.
typedef enum {
    NsCustomTick = 100,
} NsCustomEvent;

typedef struct {
    // Persisted config (alarm + last brightness).
    bool alarm_enabled;
    uint8_t alarm_hour;
    uint8_t alarm_minute;
    uint8_t brightness; // 0..100, restored and applied on next launch
} NsSettings;

typedef struct {
    // GUI infra
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    View* clock_view;
    VariableItemList* alarm_menu;
    View* alarm_time_view; // custom HH:MM picker (time only, no date)
    VariableItem* alarm_toggle_item;
    VariableItem* alarm_time_item;
    FuriTimer* timer;
    NsViewId current_view;

    // Clock display
    LocaleDateFormat date_format;
    LocaleTimeFormat time_format;
    uint8_t battery_pct;

    // Stopwatch (unchanged behaviour from the original app)
    uint32_t timer_start_timestamp;
    uint32_t timer_stopped_seconds;
    bool timer_running;

    // Brightness on-screen display: shown until this tick, then hidden (3s).
    uint32_t brightness_shown_until;

    // Alarm
    NsSettings settings;
    bool alarm_firing; // currently ringing
    bool alarm_flash_on; // flash toggle, advanced each tick while firing
    bool alarm_consumed; // fired for the current matching minute already
    uint8_t melody_phase; // replay the melody every other tick, not every tick
    uint8_t edit_hour; // scratch values the time picker edits
    uint8_t edit_minute;
    uint8_t edit_field; // 0 = hour, 1 = minute
} AppState;

// The clock view's model just points back at the app; the draw reads through it,
// and committing it (even unchanged) is what forces a redraw from the timer tick.
typedef struct {
    AppState* app;
} ClockModel;
