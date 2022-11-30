/* Flipper App to read the values from a HTU2XD, SHT2X, SI702X, SI700X, SI701X or AM2320 Sensor  */
/* Created by Mywk - https://github.com/Mywk - https://mywk.net */
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_i2c.h>
#include <math.h>

#include <gui/gui.h>
#include <input/input.h>

#include <notification/notification_messages.h>

#include <string.h>

#define TS_DEFAULT_VALUE 0xFFFF

#define TS_AVAILABLE_SENSORS 2

// HTU2XD, SHT2X, SI702X, SI700X address
#define HTU2XD_SHT2X_SI702X_SI700X_ADDRESS (0x40 << 1)
// SI701X ADDRESS
#define SI701X_ADDRESS (0x41 << 1)

// HTU2XD, SHT2X, SI702X, SI700X commands
#define HTU21D_CMD_TEMPERATURE 0xE3
#define HTU21D_CMD_HUMIDITY 0xE5

// AM2320 address
#define AM2320_ADDRESS (0x5C << 1)

// Used for the temperature and humidity buffers
#define DATA_BUFFER_SIZE 8

// External I2C BUS
#define I2C_BUS &furi_hal_i2c_handle_external

// Typedef enums to make everything easier to read

typedef enum { TSSCmdNone, TSSCmdTemperature, TSSCmdHumidity } TSSCmdType;

typedef enum {
    TSSInitializing,
    TSSNoSensor,
    TSSPendingUpdate,
} TSStatus;

typedef enum {
    TSEventTypeTick,
    TSEventTypeInput,
} TSEventType;

typedef struct {
    TSEventType type;
    InputEvent input;
} TSEvent;

// Possible return values for sensor_cmd
typedef enum {
    TSCmdRet_Error,
    TSCmdRet_HTU2XD_SHT2X_SI702X_SI700X,
    TSCmdRet_SI701X,
    TSCmdRet_AM2320,
} TSCmdRet;

// External NotificationSequence RGB
extern const NotificationSequence sequence_blink_red_100;
extern const NotificationSequence sequence_blink_green_100;
extern const NotificationSequence sequence_blink_blue_100;

// Current status of the temperature sensor app
static TSStatus temperature_sensor_current_status = TSSInitializing;

// We keep track of the last cmd return
static TSCmdRet temperature_sensor_last_cmd_ret = TSCmdRet_Error;

// Temperature and Humidity data buffers, ready to print
char ts_data_buffer_temperature_c[DATA_BUFFER_SIZE];
char ts_data_buffer_temperature_f[DATA_BUFFER_SIZE];
char ts_data_buffer_relative_humidity[DATA_BUFFER_SIZE];
char ts_data_buffer_absolute_humidity[DATA_BUFFER_SIZE];

// <sumary>
// Executes an I2C cmd (trx)
// </sumary>
// <TODO>
// CRC
// </TODO>
// <returns>
// true if fetch was successful, false otherwise
// </returns>
static TSCmdRet temperature_sensor_cmd(TSSCmdType cmd, uint8_t* buffer) {
    uint32_t timeout = furi_ms_to_ticks(100);
    TSCmdRet ret = TSCmdRet_Error;

    // Aquire I2C and check if device is ready, then release
    furi_hal_i2c_acquire(I2C_BUS);

    // Check if HTU2XD, SHT2X, SI702X, SI700X sensor is available
    uint8_t isAddress40 =
        furi_hal_i2c_is_device_ready(I2C_BUS, HTU2XD_SHT2X_SI702X_SI700X_ADDRESS, timeout);
    uint8_t isAddress41 = 0;

    // Check if SI701X sensor is available if necessary
    if(!isAddress40) isAddress41 = furi_hal_i2c_is_device_ready(I2C_BUS, SI701X_ADDRESS, timeout);

    if(isAddress40 || isAddress41) {
        uint8_t address = isAddress40 ? HTU2XD_SHT2X_SI702X_SI700X_ADDRESS : SI701X_ADDRESS;

        // Better safe than sorry delay
        furi_delay_ms(15);

        // Extra delay for the SI70XX
        if(isAddress41) furi_delay_ms(50);

        // Transmit either the temperature or the humidity command depending on TSSCmdType
        uint8_t c = (cmd == TSSCmdTemperature) ? HTU21D_CMD_TEMPERATURE : HTU21D_CMD_HUMIDITY;
        if(furi_hal_i2c_tx(I2C_BUS, address, &c, 1, timeout)) {
            // Receive data (2 bytes)
            if(furi_hal_i2c_rx(I2C_BUS, address, buffer, 2, timeout + 50))
                ret = isAddress40 ? TSCmdRet_HTU2XD_SHT2X_SI702X_SI700X : TSCmdRet_SI701X;
        }
    } else {
        // The AM2320 goes to sleep after a period of inactivity, wake it up (check AM2320 datasheet for more info)
        furi_hal_i2c_is_device_ready(I2C_BUS, AM2320_ADDRESS, timeout);
        furi_delay_ms(30);

        // Check if it's really available
        if(furi_hal_i2c_is_device_ready(I2C_BUS, AM2320_ADDRESS, timeout)) {
            // {Address, Register, Len}
            const uint8_t request[3] = {0x03, 0x00, 0x04};

            if(furi_hal_i2c_tx(I2C_BUS, AM2320_ADDRESS, request, 3, timeout)) {
                // 6 bytes - usually 8 but we currently don't check the CRC
                if(furi_hal_i2c_rx(I2C_BUS, (uint8_t)AM2320_ADDRESS, buffer, 6, timeout))
                    ret = TSCmdRet_AM2320;
            }
        }
    }

    furi_hal_i2c_release(I2C_BUS);

    temperature_sensor_last_cmd_ret = ret;
    return ret;
}

// <sumary>
// Fetches temperature and humidity from sensor
// </sumary>
// <params>
// temperature in C
// humidity in relative humidity
// </params>
// <remarks>
// Temperature and humidity must be preallocated
// </remarks>
// <TODO>
// CRC
// </TODO>
// <returns>
// true if fetch was successful, false otherwise
// </returns>
static bool temperature_sensor_fetch_data(double* temperature, double* humidity) {
    bool ret = false;

    uint16_t adc_raw;

    uint8_t buffer[DATA_BUFFER_SIZE] = {0x00};

    // Check if the sensor is the HTU21D by attempting to fetch the temperature
    TSCmdRet cmdRet = temperature_sensor_cmd(TSSCmdTemperature, buffer);
    if(cmdRet == TSCmdRet_HTU2XD_SHT2X_SI702X_SI700X || cmdRet == TSCmdRet_SI701X) {
        // Calculate temperature
        adc_raw = ((uint16_t)(buffer[0] << 8) | (buffer[1]));
        *temperature = (float)(adc_raw * 175.72 / 65536.00) - 46.85;

        // Fetch humidity
        if(temperature_sensor_cmd(TSSCmdHumidity, buffer)) {
            // Calculate humidity
            adc_raw = ((uint16_t)(buffer[0] << 8) | (buffer[1]));
            *humidity = (float)(adc_raw * 125.0 / 65536.00) - 6.0;

            ret = true;
        }
    } else if(cmdRet == TSCmdRet_AM2320) {
        // The AM2320 returns all the data immediately so we just process it all
        // Note: CRC isn't currently present in the buffer

        // Temperature
        float temp = (((buffer[4] & 0x7F) << 8) + buffer[5]) / 10;
        *temperature = ((buffer[4] & 0x80) >> 7) == 1 ? temp * (-1) : temp;

        // Humidity
        temp = ((buffer[2] << 8) + buffer[3]) / 10;
        *humidity = temp;

        ret = true;
    }

    return ret;
}

// <sumary>
// Draw callback
// </sumary>
static void temperature_sensor_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    // Update title accordingly (this could be improved by checking the hardware id)
    switch(temperature_sensor_last_cmd_ret) {
    case TSCmdRet_Error:
        canvas_draw_str(canvas, 2, 10, "Temperature Sensor");
        break;

    case TSCmdRet_HTU2XD_SHT2X_SI702X_SI700X:
        canvas_draw_str(canvas, 2, 10, "HTU/SHT/SI70 Sensor");
        break;

    case TSCmdRet_SI701X:
        canvas_draw_str(canvas, 2, 10, "SI701X Sensor");
        break;

    case TSCmdRet_AM2320:
        canvas_draw_str(canvas, 2, 10, "AM2320 Sensor");
        break;

    default:
        break;
    }

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 62, "Press back to exit.");

    switch(temperature_sensor_current_status) {
    case TSSInitializing:
        canvas_draw_str(canvas, 2, 30, "Initializing..");
        break;
    case TSSNoSensor:
        canvas_draw_str(canvas, 2, 30, "No sensor found!");
        break;
    case TSSPendingUpdate: {
        canvas_draw_str(canvas, 3, 24, "Temperature");
        canvas_draw_str(canvas, 68, 24, "Humidity");

        // Draw vertical lines
        canvas_draw_line(canvas, 61, 16, 61, 50);
        canvas_draw_line(canvas, 62, 16, 62, 50);

        // Draw horizontal line
        canvas_draw_line(canvas, 2, 27, 122, 27);

        // Draw temperature and humidity values
        canvas_draw_str(canvas, 8, 38, ts_data_buffer_temperature_c);
        canvas_draw_str(canvas, 42, 38, "C");
        canvas_draw_str(canvas, 8, 48, ts_data_buffer_temperature_f);
        canvas_draw_str(canvas, 42, 48, "F");
        canvas_draw_str(canvas, 68, 38, ts_data_buffer_relative_humidity);
        canvas_draw_str(canvas, 100, 38, "%");
        canvas_draw_str(canvas, 68, 48, ts_data_buffer_absolute_humidity);
        canvas_draw_str(canvas, 100, 48, "g/m3");
    } break;
    default:
        break;
    }
}

// <sumary>
// Input callback
// </sumary>
static void temperature_sensor_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;

    TSEvent event = {.type = TSEventTypeInput, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

// <sumary>
// Timer callback
// </sumary>
static void temperature_sensor_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    TSEvent event = {.type = TSEventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

// <sumary>
// App entry point
// </sumary>
int32_t temperature_sensor_app(void* p) {
    UNUSED(p);

    // Declare our variables and assign variables a default value
    TSEvent tsEvent;
    bool sensorFound = false;
    double celsius, fahrenheit, rel_humidity, abs_humidity = TS_DEFAULT_VALUE;

    // Used for absolute humidity calculation
    double vapour_pressure = 0;

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(TSEvent));

    // Register callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, temperature_sensor_draw_callback, NULL);
    view_port_input_callback_set(view_port, temperature_sensor_input_callback, event_queue);

    // Create timer and register its callback
    FuriTimer* timer =
        furi_timer_alloc(temperature_sensor_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency());

    // Register viewport
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Used to notify the user by blinking red (error) or blue (fetch successful)
    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);

    while(1) {
        furi_check(furi_message_queue_get(event_queue, &tsEvent, FuriWaitForever) == FuriStatusOk);

        // Handle events
        if(tsEvent.type == TSEventTypeInput) {
            // Exit on back key
            if(tsEvent.input.key ==
               InputKeyBack) // We dont check for type here, we can check the type of keypress like: (event.input.type == InputTypeShort)
                break;
        } else if(tsEvent.type == TSEventTypeTick) {
            // Update sensor data
            // Fetch data and set the sensor current status accordingly
            sensorFound = temperature_sensor_fetch_data(&celsius, &rel_humidity);
            temperature_sensor_current_status = (sensorFound ? TSSPendingUpdate : TSSNoSensor);

            if(sensorFound) {
                // Blink blue
                notification_message(notifications, &sequence_blink_blue_100);

                if(celsius != TS_DEFAULT_VALUE && rel_humidity != TS_DEFAULT_VALUE) {
                    // Convert celsius to fahrenheit
                    fahrenheit = (celsius * 9 / 5) + 32;

                    // Calculate absolute humidity - For more info refer to https://github.com/Mywk/FlipperTemperatureSensor/issues/1
                    // Calculate saturation vapour pressure first
                    vapour_pressure =
                        (double)6.11 *
                        pow(10, (double)(((double)7.5 * celsius) / ((double)237.3 + celsius)));
                    // Then the vapour pressure in Pa
                    vapour_pressure = vapour_pressure * rel_humidity;
                    // Calculate absolute humidity
                    abs_humidity =
                        (double)2.16679 * (double)(vapour_pressure / ((double)273.15 + celsius));

                    // Fill our buffers here, not on the canvas draw callback
                    snprintf(ts_data_buffer_temperature_c, DATA_BUFFER_SIZE, "%.2f", celsius);
                    snprintf(ts_data_buffer_temperature_f, DATA_BUFFER_SIZE, "%.2f", fahrenheit);
                    snprintf(
                        ts_data_buffer_relative_humidity, DATA_BUFFER_SIZE, "%.2f", rel_humidity);
                    snprintf(
                        ts_data_buffer_absolute_humidity, DATA_BUFFER_SIZE, "%.2f", abs_humidity);
                }
            } else {
                // Reset our variables to their default values
                celsius = fahrenheit = rel_humidity = abs_humidity = TS_DEFAULT_VALUE;

                // Blink red
                notification_message(notifications, &sequence_blink_red_100);
            }
        }

        furi_delay_ms(!sensorFound ? 100 : 500);
    }

    // Dobby is freee (free our variables, Flipper will crash if we don't do this!)
    furi_timer_free(timer);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    return 0;
}