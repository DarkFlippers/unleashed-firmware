/* Flipper App to read the values from a HTU21D Sensor  */
/* Created by Mywk - https://github.com/Mywk - https://mywk.net */
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_i2c.h>

#include <gui/gui.h>
#include <input/input.h>

#include <notification/notification_messages.h>

#include <string.h>

#define TS_DEFAULT_VALUE 0xFFFF

#define HTU21D_ADDRESS (0x40 << 1)

#define HTU21D_CMD_TEMPERATURE 0xE3
#define HTU21D_CMD_HUMIDITY 0xE5

#define DATA_BUFFER_SIZE 8

// External I2C BUS
#define I2C_BUS &furi_hal_i2c_handle_external

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

extern const NotificationSequence sequence_blink_red_100;
extern const NotificationSequence sequence_blink_blue_100;

static TSStatus temperature_sensor_current_status = TSSInitializing;

// Temperature and Humidity data buffers, ready to print
char ts_data_buffer_temperature_c[DATA_BUFFER_SIZE];
char ts_data_buffer_temperature_f[DATA_BUFFER_SIZE];
char ts_data_buffer_humidity[DATA_BUFFER_SIZE];

// <sumary>
// Executes an I2C cmd (trx)
// </sumary>
// <returns>
// true if fetch was successful, false otherwise
// </returns>
static bool temperature_sensor_cmd(uint8_t cmd, uint8_t* buffer, uint8_t size) {
    uint32_t timeout = furi_ms_to_ticks(100);
    bool ret = false;

    // Aquire I2C and check if device is ready, then release
    furi_hal_i2c_acquire(I2C_BUS);
    if(furi_hal_i2c_is_device_ready(I2C_BUS, HTU21D_ADDRESS, timeout)) {
        furi_hal_i2c_release(I2C_BUS);

        furi_hal_i2c_acquire(I2C_BUS);
        // Transmit given command
        ret = furi_hal_i2c_tx(I2C_BUS, HTU21D_ADDRESS, &cmd, 1, timeout);
        furi_hal_i2c_release(I2C_BUS);

        if(ret) {
            uint32_t wait_ticks = furi_ms_to_ticks(50);
            furi_delay_tick(wait_ticks);

            furi_hal_i2c_acquire(I2C_BUS);
            // Receive data
            ret = furi_hal_i2c_rx(I2C_BUS, HTU21D_ADDRESS, buffer, size, timeout);
            furi_hal_i2c_release(I2C_BUS);
        }
    } else
        furi_hal_i2c_release(I2C_BUS);

    return ret;
}

// <sumary>
// Fetches temperature and humidity from sensor
// </sumary>
// <remarks>
// Temperature and humidity must be preallocated
// Note: CRC is not checked (3rd byte)
// </remarks>
// <returns>
// true if fetch was successful, false otherwise
// </returns>
static bool temperature_sensor_fetch_data(double* temperature, double* humidity) {
    bool ret = false;

    uint16_t adc_raw;

    uint8_t buffer[2] = {0x00};

    // Fetch temperature
    ret = temperature_sensor_cmd((uint8_t)HTU21D_CMD_TEMPERATURE, buffer, 2);

    if(ret) {
        // Calculate temperature
        adc_raw = ((uint16_t)(buffer[0] << 8) | (buffer[1]));
        *temperature = (float)(adc_raw * 175.72 / 65536.00) - 46.85;

        // Fetch humidity
        ret = temperature_sensor_cmd((uint8_t)HTU21D_CMD_TEMPERATURE, buffer, 2);

        if(ret) {
            // Calculate humidity
            adc_raw = ((uint16_t)(buffer[0] << 8) | (buffer[1]));
            *humidity = (float)(adc_raw * 125.0 / 65536.00) - 6.0;
        }
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
    canvas_draw_str(canvas, 2, 10, "HTU21D Sensor");

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
        canvas_draw_str(canvas, 6, 24, "Temperature");
        canvas_draw_str(canvas, 80, 24, "Humidity");

        // Draw vertical lines
        canvas_draw_line(canvas, 68, 16, 68, 50);
        canvas_draw_line(canvas, 69, 16, 69, 50);

        // Draw horizontal line
        canvas_draw_line(canvas, 3, 27, 144, 27);

        // Draw temperature and humidity values
        canvas_draw_str(canvas, 14, 38, ts_data_buffer_temperature_c);
        canvas_draw_str(canvas, 48, 38, "C");
        canvas_draw_str(canvas, 14, 48, ts_data_buffer_temperature_f);
        canvas_draw_str(canvas, 48, 48, "F");
        canvas_draw_str(canvas, 78, 42, ts_data_buffer_humidity);
        canvas_draw_str(canvas, 112, 42, "%");

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

    furi_hal_power_suppress_charge_enter();
    // Declare our variables
    TSEvent tsEvent;
    bool sensorFound = false;
    double celsius, fahrenheit, humidity;

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

    // Assign variables a default value
    celsius = fahrenheit = humidity = TS_DEFAULT_VALUE;

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
            sensorFound = temperature_sensor_fetch_data(&celsius, &humidity);
            temperature_sensor_current_status = (sensorFound ? TSSPendingUpdate : TSSNoSensor);

            if(sensorFound) {
                // Blink blue
                notification_message(notifications, &sequence_blink_blue_100);

                if(celsius != TS_DEFAULT_VALUE && humidity != TS_DEFAULT_VALUE) {
                    // Convert celsius to fahrenheit
                    fahrenheit = (celsius * 9 / 5) + 32;

                    // Fill our buffers here, not on the canvas draw callback
                    snprintf(ts_data_buffer_temperature_c, DATA_BUFFER_SIZE, "%.2f", celsius);
                    snprintf(ts_data_buffer_temperature_f, DATA_BUFFER_SIZE, "%.2f", fahrenheit);
                    snprintf(ts_data_buffer_humidity, DATA_BUFFER_SIZE, "%.2f", humidity);
                }

            } else {
                // Reset our variables to their default values
                celsius = fahrenheit = humidity = TS_DEFAULT_VALUE;

                // Blink red
                notification_message(notifications, &sequence_blink_red_100);
            }
        }

        uint32_t wait_ticks = furi_ms_to_ticks(!sensorFound ? 100 : 500);
        furi_delay_tick(wait_ticks);
    }

    furi_hal_power_suppress_charge_exit();
    // Dobby is freee (free our variables, Flipper will crash if we don't do this!)
    furi_timer_free(timer);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    return 0;
}
