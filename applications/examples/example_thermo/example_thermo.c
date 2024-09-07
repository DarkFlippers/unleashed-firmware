/**
 * @file example_thermo.c
 * @brief 1-Wire thermometer example.
 *
 * This file contains an example application that reads and displays
 * the temperature from a DS18B20 1-wire thermometer.
 *
 * It also covers basic GUI, input handling, threads and localisation.
 *
 * References:
 * [1] DS18B20 Datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/DS18B20.pdf
 */

#include <gui/gui.h>
#include <gui/view_port.h>

#include <core/thread.h>
#include <core/kernel.h>

#include <locale/locale.h>

#include <one_wire/maxim_crc.h>
#include <one_wire/one_wire_host.h>

#include <furi_hal_power.h>

#define UPDATE_PERIOD_MS 1000UL
#define TEXT_STORE_SIZE  64U

#define DS18B20_CMD_SKIP_ROM        0xccU
#define DS18B20_CMD_CONVERT         0x44U
#define DS18B20_CMD_READ_SCRATCHPAD 0xbeU

#define DS18B20_CFG_RESOLUTION_POS  5U
#define DS18B20_CFG_RESOLUTION_MASK 0x03U
#define DS18B20_DECIMAL_PART_MASK   0x0fU

#define DS18B20_SIGN_MASK 0xf0U

/* Possible GPIO pin choices:
 - gpio_ext_pc0
 - gpio_ext_pc1
 - gpio_ext_pc3
 - gpio_ext_pb2
 - gpio_ext_pb3
 - gpio_ext_pa4
 - gpio_ext_pa6
 - gpio_ext_pa7
 - gpio_ibutton
*/

#define THERMO_GPIO_PIN (gpio_ibutton)

/* Flags which the reader thread responds to */
typedef enum {
    ReaderThreadFlagExit = 1,
} ReaderThreadFlag;

typedef union {
    struct {
        uint8_t temp_lsb; /* Least significant byte of the temperature */
        uint8_t temp_msb; /* Most significant byte of the temperature */
        uint8_t user_alarm_high; /* User register 1 (Temp high alarm) */
        uint8_t user_alarm_low; /* User register 2 (Temp low alarm) */
        uint8_t config; /* Configuration register */
        uint8_t reserved[3]; /* Not used */
        uint8_t crc; /* CRC checksum for error detection */
    } fields;
    uint8_t bytes[9];
} DS18B20Scratchpad;

/* Application context structure */
typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriThread* reader_thread;
    FuriMessageQueue* event_queue;
    OneWireHost* onewire;
    float temp_celsius;
    bool has_device;
} ExampleThermoContext;

/*************** 1-Wire Communication and Processing *****************/

/* Commands the thermometer to begin measuring the temperature. */
static void example_thermo_request_temperature(ExampleThermoContext* context) {
    OneWireHost* onewire = context->onewire;

    /* All 1-wire transactions must happen in a critical section, i.e
       not interrupted by other threads. */
    FURI_CRITICAL_ENTER();

    bool success = false;
    do {
        /* Each communication with a 1-wire device starts by a reset.
           The function will return true if a device responded with a presence pulse. */
        if(!onewire_host_reset(onewire)) break;
        /* After the reset, a ROM operation must follow.
           If there is only one device connected, the "Skip ROM" command is most appropriate
           (it can also be used to address all of the connected devices in some cases).*/
        onewire_host_write(onewire, DS18B20_CMD_SKIP_ROM);
        /* After the ROM operation, a device-specific command is issued.
           In this case, it's a request to start measuring the temperature. */
        onewire_host_write(onewire, DS18B20_CMD_CONVERT);

        success = true;
    } while(false);

    context->has_device = success;

    FURI_CRITICAL_EXIT();
}

/* Reads the measured temperature from the thermometer. */
static void example_thermo_read_temperature(ExampleThermoContext* context) {
    /* If there was no device detected, don't try to read the temperature */
    if(!context->has_device) {
        return;
    }

    OneWireHost* onewire = context->onewire;

    /* All 1-wire transactions must happen in a critical section, i.e
       not interrupted by other threads. */
    FURI_CRITICAL_ENTER();

    bool success = false;

    do {
        DS18B20Scratchpad buf;

        /* Attempt reading the temperature 10 times before giving up */
        size_t attempts_left = 10;
        do {
            /* Each communication with a 1-wire device starts by a reset.
            The function will return true if a device responded with a presence pulse. */
            if(!onewire_host_reset(onewire)) continue;

            /* After the reset, a ROM operation must follow.
            If there is only one device connected, the "Skip ROM" command is most appropriate
            (it can also be used to address all of the connected devices in some cases).*/
            onewire_host_write(onewire, DS18B20_CMD_SKIP_ROM);

            /* After the ROM operation, a device-specific command is issued.
            This time, it will be the "Read Scratchpad" command which will
            prepare the device's internal buffer memory for reading. */
            onewire_host_write(onewire, DS18B20_CMD_READ_SCRATCHPAD);

            /* The actual reading happens here. A total of 9 bytes is read. */
            onewire_host_read_bytes(onewire, buf.bytes, sizeof(buf.bytes));

            /* Calculate the checksum and compare it with one provided by the device. */
            const uint8_t crc = maxim_crc8(buf.bytes, sizeof(buf.bytes) - 1, MAXIM_CRC8_INIT);

            /* Checksums match, exit the loop */
            if(crc == buf.fields.crc) break;

        } while(--attempts_left);

        if(attempts_left == 0) break;

        /* Get the measurement resolution from the configuration register. (See [1] page 9) */
        const uint8_t resolution_mode = (buf.fields.config >> DS18B20_CFG_RESOLUTION_POS) &
                                        DS18B20_CFG_RESOLUTION_MASK;

        /* Generate a mask for undefined bits in the decimal part. (See [1] page 6) */
        const uint8_t decimal_mask =
            (DS18B20_DECIMAL_PART_MASK << (DS18B20_CFG_RESOLUTION_MASK - resolution_mode)) &
            DS18B20_DECIMAL_PART_MASK;

        /* Get the integer and decimal part of the temperature (See [1] page 6) */
        const uint8_t integer_part = (buf.fields.temp_msb << 4U) | (buf.fields.temp_lsb >> 4U);
        const uint8_t decimal_part = buf.fields.temp_lsb & decimal_mask;

        /* Calculate the sign of the temperature (See [1] page 6) */
        const bool is_negative = (buf.fields.temp_msb & DS18B20_SIGN_MASK) != 0;

        /* Combine the integer and decimal part together */
        const float temp_celsius_abs = integer_part + decimal_part / 16.f;

        /* Set the appropriate sign */
        context->temp_celsius = is_negative ? -temp_celsius_abs : temp_celsius_abs;

        success = true;
    } while(false);

    context->has_device = success;

    FURI_CRITICAL_EXIT();
}

/* Periodically requests measurements and reads temperature. This function runs in a separare thread. */
static int32_t example_thermo_reader_thread_callback(void* ctx) {
    ExampleThermoContext* context = ctx;

    for(;;) {
        /* Tell the termometer to start measuring the temperature. The process may take up to 750ms. */
        example_thermo_request_temperature(context);

        /* Wait for the measurement to finish. At the same time wait for an exit signal. */
        const uint32_t flags =
            furi_thread_flags_wait(ReaderThreadFlagExit, FuriFlagWaitAny, UPDATE_PERIOD_MS);

        /* If an exit signal was received, return from this thread. */
        if(flags != (unsigned)FuriFlagErrorTimeout) break;

        /* The measurement is now ready, read it from the termometer. */
        example_thermo_read_temperature(context);
    }

    return 0;
}

/*************** GUI, Input and Main Loop *****************/

/* Draw the GUI of the application. The screen is completely redrawn during each call. */
static void example_thermo_draw_callback(Canvas* canvas, void* ctx) {
    ExampleThermoContext* context = ctx;
    char text_store[TEXT_STORE_SIZE];
    const size_t middle_x = canvas_width(canvas) / 2U;

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, middle_x, 12, AlignCenter, AlignBottom, "Thermometer Demo");
    canvas_draw_line(canvas, 0, 16, 128, 16);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, middle_x, 30, AlignCenter, AlignBottom, "Connect thermometer");

    snprintf(
        text_store,
        TEXT_STORE_SIZE,
        "to GPIO pin %ld",
        furi_hal_resources_get_ext_pin_number(&THERMO_GPIO_PIN));
    canvas_draw_str_aligned(canvas, middle_x, 42, AlignCenter, AlignBottom, text_store);

    canvas_set_font(canvas, FontKeyboard);

    if(context->has_device) {
        float temp;
        char temp_units;

        /* The application is locale-aware.
           Change Settings->System->Units to check it out. */
        switch(locale_get_measurement_unit()) {
        case LocaleMeasurementUnitsMetric:
            temp = context->temp_celsius;
            temp_units = 'C';
            break;
        case LocaleMeasurementUnitsImperial:
            temp = locale_celsius_to_fahrenheit(context->temp_celsius);
            temp_units = 'F';
            break;
        default:
            furi_crash("Illegal measurement units");
        }
        /* If a reading is available, display it */
        snprintf(text_store, TEXT_STORE_SIZE, "Temperature: %+.1f%c", (double)temp, temp_units);
    } else {
        /* Or show a message that no data is available */
        strlcpy(text_store, "-- No data --", TEXT_STORE_SIZE);
    }

    canvas_draw_str_aligned(canvas, middle_x, 58, AlignCenter, AlignBottom, text_store);
}

/* This function is called from the GUI thread. All it does is put the event
   into the application's queue so it can be processed later. */
static void example_thermo_input_callback(InputEvent* event, void* ctx) {
    ExampleThermoContext* context = ctx;
    furi_message_queue_put(context->event_queue, event, FuriWaitForever);
}

/* Starts the reader thread and handles the input */
static void example_thermo_run(ExampleThermoContext* context) {
    /* Enable power on external pins */
    furi_hal_power_enable_otg();

    /* Configure the hardware in host mode */
    onewire_host_start(context->onewire);

    /* Start the reader thread. It will talk to the thermometer in the background. */
    furi_thread_start(context->reader_thread);

    /* An endless loop which handles the input*/
    for(bool is_running = true; is_running;) {
        InputEvent event;
        /* Wait for an input event. Input events come from the GUI thread via a callback. */
        const FuriStatus status =
            furi_message_queue_get(context->event_queue, &event, FuriWaitForever);

        /* This application is only interested in short button presses. */
        if((status != FuriStatusOk) || (event.type != InputTypeShort)) {
            continue;
        }

        /* When the user presses the "Back" button, break the loop and exit the application. */
        if(event.key == InputKeyBack) {
            is_running = false;
        }
    }

    /* Signal the reader thread to cease operation and exit */
    furi_thread_flags_set(furi_thread_get_id(context->reader_thread), ReaderThreadFlagExit);

    /* Wait for the reader thread to finish */
    furi_thread_join(context->reader_thread);

    /* Reset the hardware */
    onewire_host_stop(context->onewire);

    /* Disable power on external pins */
    furi_hal_power_disable_otg();
}

/******************** Initialisation & startup *****************************/

/* Allocate the memory and initialise the variables */
static ExampleThermoContext* example_thermo_context_alloc(void) {
    ExampleThermoContext* context = malloc(sizeof(ExampleThermoContext));

    context->view_port = view_port_alloc();
    view_port_draw_callback_set(context->view_port, example_thermo_draw_callback, context);
    view_port_input_callback_set(context->view_port, example_thermo_input_callback, context);

    context->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    context->reader_thread = furi_thread_alloc();
    furi_thread_set_stack_size(context->reader_thread, 1024U);
    furi_thread_set_context(context->reader_thread, context);
    furi_thread_set_callback(context->reader_thread, example_thermo_reader_thread_callback);

    context->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(context->gui, context->view_port, GuiLayerFullscreen);

    context->onewire = onewire_host_alloc(&THERMO_GPIO_PIN);

    return context;
}

/* Release the unused resources and deallocate memory */
static void example_thermo_context_free(ExampleThermoContext* context) {
    view_port_enabled_set(context->view_port, false);
    gui_remove_view_port(context->gui, context->view_port);

    onewire_host_free(context->onewire);
    furi_thread_free(context->reader_thread);
    furi_message_queue_free(context->event_queue);
    view_port_free(context->view_port);

    furi_record_close(RECORD_GUI);
}

/* The application's entry point. Execution starts from here. */
int32_t example_thermo_main(void* p) {
    UNUSED(p);

    /* Allocate all of the necessary structures */
    ExampleThermoContext* context = example_thermo_context_alloc();

    /* Start the application's main loop. It won't return until the application was requested to exit. */
    example_thermo_run(context);

    /* Release all unneeded resources */
    example_thermo_context_free(context);

    return 0;
}
