#include <string.h>

#include "minmea.h"
#include "gps_uart.h"

typedef enum {
    WorkerEvtStop = (1 << 0),
    WorkerEvtRxDone = (1 << 1),
} WorkerEvtFlags;

#define WORKER_ALL_RX_EVENTS (WorkerEvtStop | WorkerEvtRxDone)

static void gps_uart_on_irq_cb(UartIrqEvent ev, uint8_t data, void* context) {
    GpsUart* gps_uart = (GpsUart*)context;

    if(ev == UartIrqEventRXNE) {
        furi_stream_buffer_send(gps_uart->rx_stream, &data, 1, 0);
        furi_thread_flags_set(furi_thread_get_id(gps_uart->thread), WorkerEvtRxDone);
    }
}

static void gps_uart_serial_init(GpsUart* gps_uart) {
    furi_hal_console_disable();
    furi_hal_uart_set_irq_cb(FuriHalUartIdUSART1, gps_uart_on_irq_cb, gps_uart);
    furi_hal_uart_set_br(FuriHalUartIdUSART1, gps_uart->baudrate);
}

static void gps_uart_serial_deinit(GpsUart* gps_uart) {
    UNUSED(gps_uart);
    furi_hal_uart_set_irq_cb(FuriHalUartIdUSART1, NULL, NULL);
    furi_hal_console_enable();
}

static void gps_uart_parse_nmea(GpsUart* gps_uart, char* line) {
    switch(minmea_sentence_id(line, false)) {
    case MINMEA_SENTENCE_RMC: {
        struct minmea_sentence_rmc frame;
        if(minmea_parse_rmc(&frame, line)) {
            gps_uart->status.valid = frame.valid;
            gps_uart->status.latitude = minmea_tocoord(&frame.latitude);
            gps_uart->status.longitude = minmea_tocoord(&frame.longitude);
            gps_uart->status.speed = minmea_tofloat(&frame.speed);
            gps_uart->status.course = minmea_tofloat(&frame.course);
            gps_uart->status.time_hours = frame.time.hours;
            gps_uart->status.time_minutes = frame.time.minutes;
            gps_uart->status.time_seconds = frame.time.seconds;

            notification_message_block(gps_uart->notifications, &sequence_blink_green_10);
        }
    } break;

    case MINMEA_SENTENCE_GGA: {
        struct minmea_sentence_gga frame;
        if(minmea_parse_gga(&frame, line)) {
            gps_uart->status.latitude = minmea_tocoord(&frame.latitude);
            gps_uart->status.longitude = minmea_tocoord(&frame.longitude);
            gps_uart->status.altitude = minmea_tofloat(&frame.altitude);
            gps_uart->status.altitude_units = frame.altitude_units;
            gps_uart->status.fix_quality = frame.fix_quality;
            gps_uart->status.satellites_tracked = frame.satellites_tracked;
            gps_uart->status.time_hours = frame.time.hours;
            gps_uart->status.time_minutes = frame.time.minutes;
            gps_uart->status.time_seconds = frame.time.seconds;

            notification_message_block(gps_uart->notifications, &sequence_blink_magenta_10);
        }
    } break;

    case MINMEA_SENTENCE_GLL: {
        struct minmea_sentence_gll frame;
        if(minmea_parse_gll(&frame, line)) {
            gps_uart->status.latitude = minmea_tocoord(&frame.latitude);
            gps_uart->status.longitude = minmea_tocoord(&frame.longitude);
            gps_uart->status.time_hours = frame.time.hours;
            gps_uart->status.time_minutes = frame.time.minutes;
            gps_uart->status.time_seconds = frame.time.seconds;

            notification_message_block(gps_uart->notifications, &sequence_blink_red_10);
        }
    } break;

    default:
        break;
    }
}

static int32_t gps_uart_worker(void* context) {
    GpsUart* gps_uart = (GpsUart*)context;

    size_t rx_offset = 0;

    while(1) {
        uint32_t events =
            furi_thread_flags_wait(WORKER_ALL_RX_EVENTS, FuriFlagWaitAny, FuriWaitForever);
        furi_check((events & FuriFlagError) == 0);

        if(events & WorkerEvtStop) {
            break;
        }

        if(events & WorkerEvtRxDone) {
            size_t len = 0;
            do {
                // receive serial bytes into rx_buf, starting at rx_offset from the start of the buffer
                // the maximum we can receive is RX_BUF_SIZE - 1 - rx_offset
                len = furi_stream_buffer_receive(
                    gps_uart->rx_stream,
                    gps_uart->rx_buf + rx_offset,
                    RX_BUF_SIZE - 1 - rx_offset,
                    0);
                if(len > 0) {
                    // increase rx_offset by the number of bytes received, and null-terminate rx_buf
                    rx_offset += len;
                    gps_uart->rx_buf[rx_offset] = '\0';

                    // look for strings ending in newlines, starting at the start of rx_buf
                    char* line_current = (char*)gps_uart->rx_buf;
                    while(1) {
                        // skip null characters
                        while(*line_current == '\0' &&
                              line_current < (char*)gps_uart->rx_buf + rx_offset - 1) {
                            line_current++;
                        }

                        // find the next newline
                        char* newline = strchr(line_current, '\n');
                        if(newline) // newline found
                        {
                            // put a null terminator in place of the newline, to delimit the line string
                            *newline = '\0';

                            // attempt to parse the line as a NMEA sentence
                            gps_uart_parse_nmea(gps_uart, line_current);

                            // move the cursor to the character after the newline
                            line_current = newline + 1;
                        } else // no more newlines found
                        {
                            if(line_current >
                               (char*)gps_uart->rx_buf) // at least one line was found
                            {
                                // clear parsed lines, and move any leftover bytes to the start of rx_buf
                                rx_offset = 0;
                                while(
                                    *line_current) // stop when the original rx_offset terminator is reached
                                {
                                    gps_uart->rx_buf[rx_offset++] = *(line_current++);
                                }
                            }
                            break; // go back to receiving bytes from the serial stream
                        }
                    }
                }
            } while(len > 0);
        }
    }

    gps_uart_serial_deinit(gps_uart);
    furi_stream_buffer_free(gps_uart->rx_stream);

    return 0;
}

void gps_uart_init_thread(GpsUart* gps_uart) {
    furi_assert(gps_uart);
    gps_uart->status.valid = false;
    gps_uart->status.latitude = 0.0;
    gps_uart->status.longitude = 0.0;
    gps_uart->status.speed = 0.0;
    gps_uart->status.course = 0.0;
    gps_uart->status.altitude = 0.0;
    gps_uart->status.altitude_units = ' ';
    gps_uart->status.fix_quality = 0;
    gps_uart->status.satellites_tracked = 0;
    gps_uart->status.time_hours = 0;
    gps_uart->status.time_minutes = 0;
    gps_uart->status.time_seconds = 0;

    gps_uart->rx_stream = furi_stream_buffer_alloc(RX_BUF_SIZE * 5, 1);

    gps_uart->thread = furi_thread_alloc();
    furi_thread_set_name(gps_uart->thread, "GpsUartWorker");
    furi_thread_set_stack_size(gps_uart->thread, 1024);
    furi_thread_set_context(gps_uart->thread, gps_uart);
    furi_thread_set_callback(gps_uart->thread, gps_uart_worker);

    furi_thread_start(gps_uart->thread);

    gps_uart_serial_init(gps_uart);
}

void gps_uart_deinit_thread(GpsUart* gps_uart) {
    furi_assert(gps_uart);
    furi_thread_flags_set(furi_thread_get_id(gps_uart->thread), WorkerEvtStop);
    furi_thread_join(gps_uart->thread);
    furi_thread_free(gps_uart->thread);
}

GpsUart* gps_uart_enable() {
    GpsUart* gps_uart = malloc(sizeof(GpsUart));

    gps_uart->notifications = furi_record_open(RECORD_NOTIFICATION);

    gps_uart->baudrate = gps_baudrates[current_gps_baudrate];

    gps_uart_init_thread(gps_uart);

    return gps_uart;
}

void gps_uart_disable(GpsUart* gps_uart) {
    furi_assert(gps_uart);
    gps_uart_deinit_thread(gps_uart);
    furi_record_close(RECORD_NOTIFICATION);

    free(gps_uart);
}
