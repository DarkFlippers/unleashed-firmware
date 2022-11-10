/* Flipper Plugin to read the values from a AM2320/AM2321 Sensor */
/* Created by @xMasterX, original app (was used as template) by Mywk - https://github.com/Mywk */
/* Lib used as reference: https://github.com/Gozem/am2320/blob/master/am2321.c*/
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_i2c.h>
#include <math.h>

#include <gui/gui.h>
#include <input/input.h>

#include <notification/notification_messages.h>

#include <string.h>

#define TS_DEFAULT_VALUE 0xFFFF

#define AM2320_ADDRESS (0x5C << 1)

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
char ts_data_buffer_relative_humidity[DATA_BUFFER_SIZE];
char ts_data_buffer_absolute_humidity[DATA_BUFFER_SIZE];

// CRC16 calculation
static uint16_t get_crc16(const uint8_t* buf, size_t len) {
    uint16_t crc = 0xFFFF;

    while(len--) {
        crc ^= (uint16_t)*buf++;
        for(unsigned i = 0; i < 8; i++) {
            if(crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}
// Combine bytes
static uint16_t combine_bytes(uint8_t msb, uint8_t lsb) {
    return ((uint16_t)msb << 8) | (uint16_t)lsb;
}

// Executes an I2C wake up, sends command and reads result
// true if fetch was successful, false otherwise
static bool temperature_sensor_get_data(uint8_t* buffer, uint8_t size) {
    uint32_t timeout = furi_ms_to_ticks(100);
    uint8_t cmdbuffer[3] = {0, 0, 0};
    bool ret = false;

    // Aquire I2C bus
    furi_hal_i2c_acquire(I2C_BUS);

    // Wake UP AM2320 (sensor goes to sleep to not warm up and affect the humidity sensor)
    furi_hal_i2c_is_device_ready(I2C_BUS, (uint8_t)AM2320_ADDRESS, timeout);
    // Check if device woken up then we do next stuff
    if(furi_hal_i2c_is_device_ready(I2C_BUS, (uint8_t)AM2320_ADDRESS, timeout)) {
        // Wait a bit
        furi_delay_us(1000);

        // Prepare command: Addr 0x03, start register = 0x00, number of registers to read = 0x04
        cmdbuffer[0] = 0x03;
        cmdbuffer[1] = 0x00;
        cmdbuffer[2] = 0x04;

        // Transmit command to read registers
        ret = furi_hal_i2c_tx(I2C_BUS, (uint8_t)AM2320_ADDRESS, cmdbuffer, 3, timeout);

        // Wait a bit
        furi_delay_us(1600);
        if(ret) {
            /*
             * Read out 8 bytes of data
             * Byte 0: Should be Modbus function code 0x03
             * Byte 1: Should be number of registers to read (0x04)
             * Byte 2: Humidity msb
             * Byte 3: Humidity lsb
             * Byte 4: Temperature msb
             * Byte 5: Temperature lsb
             * Byte 6: CRC lsb byte
             * Byte 7: CRC msb byte
             */
            ret = furi_hal_i2c_rx(I2C_BUS, (uint8_t)AM2320_ADDRESS, buffer, size, timeout);
        }
    }
    // Release i2c bus
    furi_hal_i2c_release(I2C_BUS);

    return ret;
}

// Fetches temperature and humidity from sensor
// Temperature and humidity must be preallocated
// true if fetch was successful, false otherwise
static bool temperature_sensor_fetch_info(double* temperature, double* humidity) {
    *humidity = (float)0;
    bool ret = false;

    uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    // Fetch data from sensor
    ret = temperature_sensor_get_data(buffer, 8);

    // If we got no result
    if(!ret) return false;

    if(buffer[0] != 0x03) return false; // must be 0x03 modbus reply
    if(buffer[1] != 0x04) return false; // must be 0x04 number of registers reply

    // Check CRC16 sum, if not correct - return false
    uint16_t crcdata = get_crc16(buffer, 6);
    uint16_t crcread = combine_bytes(buffer[7], buffer[6]);
    if(crcdata != crcread) return false;

    // Combine bytes for temp and humidity
    uint16_t temp16 = combine_bytes(buffer[4], buffer[5]);
    uint16_t humi16 = combine_bytes(buffer[2], buffer[3]);

    /* Temperature resolution is 16Bit, 
   * temperature highest bit (Bit15) is equal to 1 indicates a
   * negative temperature, the temperature highest bit (Bit15)
   * is equal to 0 indicates a positive temperature; 
   * temperature in addition to the most significant bit (Bit14 ~ Bit0)
   *  indicates the temperature sensor string value.
   * Temperature sensor value is a string of 10 times the
   * actual temperature value.
   */
    if(temp16 & 0x8000) {
        temp16 = -(temp16 & 0x7FFF);
    }

    // Prepare output data
    *temperature = (float)temp16 / 10.0;
    *humidity = (float)humi16 / 10.0;

    return true;
}

// Draw callback

static void temperature_sensor_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "AM2320/AM2321 Sensor");

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

// Input callback

static void temperature_sensor_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;

    TSEvent event = {.type = TSEventTypeInput, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

// Timer callback

static void temperature_sensor_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    TSEvent event = {.type = TSEventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

// App entry point

int32_t am_temperature_sensor_app(void* p) {
    UNUSED(p);

    furi_hal_power_suppress_charge_enter();
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
            sensorFound = temperature_sensor_fetch_info(&celsius, &rel_humidity);
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
