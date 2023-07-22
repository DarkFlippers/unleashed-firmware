#pragma once

#include <gui/view.h>
#include "lightmeter_icons.h"
#include "../../lightmeter_config.h"

/* log base 1.4 and 12 pixels cut off
   makes it show values approx 65-65k
   with reasonable resolution in 1-10k range
   on 20px of screen height  */
#define LUX_HISTORGRAM_LOGBASE 1.4
#define LUX_HISTORGRAM_BOTTOM 64 + 12

/* 40 pixels between 45th and 85th
   between left and right button labels */
#define LUX_HISTORGRAM_LEFT 45
#define LUX_HISTORGRAM_LENGTH 40

typedef struct MainView MainView;

typedef enum {
    FIXED_APERTURE,
    FIXED_SPEED,

    MODES_SIZE
} MainViewMode;

typedef struct {
    uint8_t recv[2];
    MainViewMode current_mode;
    float lux;
    float peakLux;
    float EV;
    float aperture_val;
    float speed_val;
    int iso_val;
    bool response;
    int iso;
    int nd;
    int aperture;
    int speed;
    bool dome;
    bool lux_only;
    int measurement_resolution;
    int device_addr;
    int sensor_type;

    float luxHistogram[LUX_HISTORGRAM_LENGTH];
    int luxHistogramIndex;
} MainViewModel;

typedef void (*LightMeterMainViewButtonCallback)(void* context);

void lightmeter_main_view_set_left_callback(
    MainView* lightmeter_main_view,
    LightMeterMainViewButtonCallback callback,
    void* context);

void lightmeter_main_view_set_right_callback(
    MainView* lightmeter_main_view,
    LightMeterMainViewButtonCallback callback,
    void* context);

MainView* main_view_alloc();

void main_view_free(MainView* main_view);

View* main_view_get_view(MainView* main_view);

void main_view_set_lux(MainView* main_view, float val);

void main_view_reset_lux(MainView* main_view);

void main_view_set_EV(MainView* main_view_, float val);

void main_view_set_response(MainView* main_view_, bool val);

void main_view_set_iso(MainView* main_view, int val);

void main_view_set_nd(MainView* main_view, int val);

void main_view_set_aperture(MainView* main_view, int val);

void main_view_set_speed(MainView* main_view, int val);

void main_view_set_dome(MainView* main_view, bool val);

void main_view_set_lux_only(MainView* main_view, bool val);

void main_view_set_measurement_resolution(MainView* main_view, int val);

void main_view_set_device_addr(MainView* main_view, int addr);

void main_view_set_sensor_type(MainView* main_view, int sensor_type);

bool main_view_get_dome(MainView* main_view);

void draw_top_row(Canvas* canvas, MainViewModel* context);

void draw_aperture(Canvas* canvas, MainViewModel* context);

void draw_speed(Canvas* canvas, MainViewModel* context);

void draw_mode_indicator(Canvas* canvas, MainViewModel* context);

void draw_nd_number(Canvas* canvas, MainViewModel* context);

void draw_EV_number(Canvas* canvas, MainViewModel* context);

void draw_lux_only_mode(Canvas* canvas, MainViewModel* context);
