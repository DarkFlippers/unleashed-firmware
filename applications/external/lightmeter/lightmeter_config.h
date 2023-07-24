#pragma once

#define LM_VERSION_APP "1.2"
#define LM_DEVELOPED "Oleksii Kutuzov"
#define LM_GITHUB "https://github.com/oleksiikutuzov/flipperzero-lightmeter"

#define DOME_COEFFICIENT 2.3
#define DEFAULT_ISO ISO_100
#define DEFAULT_ND ND_0
#define DEFAULT_APERTURE AP_2_8
#define DEFAULT_SPEED SPEED_125
#define DEFAULT_DOME WITHOUT_DOME
#define DEFAULT_BACKLIGHT BACKLIGHT_AUTO

typedef enum {
    ISO_6,
    ISO_12,
    ISO_25,
    ISO_50,
    ISO_100,
    ISO_200,
    ISO_400,
    ISO_800,
    ISO_1600,
    ISO_3200,
    ISO_6400,
    ISO_12800,
    ISO_25600,
    ISO_51200,
    ISO_102400,

    ISO_NUM,
} LightMeterISONumbers;

typedef enum {
    ND_0,
    ND_2,
    ND_4,
    ND_8,
    ND_16,
    ND_32,
    ND_64,
    ND_128,
    ND_256,
    ND_512,
    ND_1024,
    ND_2048,
    ND_4096,

    ND_NUM,
} LightMeterNDNumbers;

typedef enum {
    AP_1,
    AP_1_4,
    AP_2,
    AP_2_8,
    AP_4,
    AP_5_6,
    AP_8,
    AP_11,
    AP_16,
    AP_22,
    AP_32,
    AP_45,
    AP_64,
    AP_90,
    AP_128,

    AP_NUM,
} LightMeterApertureNumbers;

typedef enum {
    SPEED_8000,
    SPEED_4000,
    SPEED_2000,
    SPEED_1000,
    SPEED_500,
    SPEED_250,
    SPEED_125,
    SPEED_60,
    SPEED_48,
    SPEED_30,
    SPEED_15,
    SPEED_8,
    SPEED_4,
    SPEED_2,
    SPEED_1S,
    SPEED_2S,
    SPEED_4S,
    SPEED_8S,
    SPEED_15S,
    SPEED_30S,

    SPEED_NUM,
} LightMeterSpeedNumbers;

typedef enum {
    WITHOUT_DOME,
    WITH_DOME,
} LightMeterDomePresence;

typedef enum {
    LUX_ONLY_OFF,
    LUX_ONLY_ON,
} LightMeterLuxOnlyMode;

typedef enum {
    LOW_RES,
    HIGH_RES,
    HIGH_RES2,
} LightMeterMeterMode;

typedef enum {
    ADDR_LOW,
    ADDR_HIGH,
} LightMeterMeterAddr;

typedef enum {
    SENSOR_BH1750,
    SENSOR_MAX44009,
} LightMeterSensorType;

typedef enum { BACKLIGHT_AUTO, BACKLIGHT_ON } LightMeterBacklight;
