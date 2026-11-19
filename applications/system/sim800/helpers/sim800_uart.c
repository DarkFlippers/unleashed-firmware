#include "sim800_uart.h"

#include <furi_hal_serial_control.h>

#define TAG "Sim800Uart"

#define SIM800_RX_STREAM_SIZE 1024
#define SIM800_LINE_MAX       512
#define SIM800_PROMPT_TIMEOUT 5000
#define SIM800_CTRL_Z         0x1A
#define SIM800_ESC            0x1B

typedef enum {
    WorkerFlagRx = (1 << 0),
    WorkerFlagStop = (1 << 1),
} WorkerFlag;

#define WORKER_ALL_FLAGS (WorkerFlagRx | WorkerFlagStop)

struct Sim800Uart {
    FuriHalSerialHandle* handle;
    FuriStreamBuffer* rx_stream;
    FuriThread* thread;
    bool open;

    FuriMutex* mutex; //!< guards everything in the command block below
    FuriSemaphore* cmd_sem;
    FuriSemaphore* prompt_sem;
    bool cmd_pending;
    bool want_prompt;
    bool cmd_call_progress; //!< this command may end on NO CARRIER and friends
    Sim800AtResult cmd_result;
    FuriString* cmd_response;

    Sim800UartLineCallback line_callback;
    void* line_context;

    FuriString* line; //!< assembly buffer, worker thread only
};

/** Final result codes.
 *
 * The call-progress codes are only an answer to ATD and ATA. At any other moment they are
 * unsolicited - a call the far end dropped - and taking one as the result of whatever happened
 * to be in flight ends that command early, leaving its real reply to be read as the next
 * command's. From there every response is one behind until something times out.
 */
static bool sim800_is_final(const char* line, bool call_progress, Sim800AtResult* result) {
    if(strcmp(line, "OK") == 0 || strcmp(line, "SEND OK") == 0) {
        *result = Sim800AtResultOk;
        return true;
    }
    if(strcmp(line, "ERROR") == 0 || strcmp(line, "SEND FAIL") == 0 ||
       strncmp(line, "+CME ERROR", 10) == 0 || strncmp(line, "+CMS ERROR", 10) == 0) {
        *result = Sim800AtResultError;
        return true;
    }
    if(call_progress && (strcmp(line, "NO CARRIER") == 0 || strcmp(line, "BUSY") == 0 ||
                         strcmp(line, "NO ANSWER") == 0 || strcmp(line, "NO DIALTONE") == 0)) {
        *result = Sim800AtResultError;
        return true;
    }
    return false;
}

static void sim800_uart_process_line(Sim800Uart* uart, const char* line) {
    Sim800AtResult result = Sim800AtResultOk;

    furi_mutex_acquire(uart->mutex, FuriWaitForever);
    if(uart->cmd_pending) {
        if(sim800_is_final(line, uart->cmd_call_progress, &result)) {
            uart->cmd_result = result;
            uart->cmd_pending = false;
            furi_semaphore_release(uart->cmd_sem);
        } else if(furi_string_size(uart->cmd_response) < SIM800_LINE_MAX * 8) {
            if(furi_string_size(uart->cmd_response) > 0) {
                furi_string_cat_str(uart->cmd_response, "\n");
            }
            furi_string_cat_str(uart->cmd_response, line);
        }
    }
    furi_mutex_release(uart->mutex);

    // Outside the mutex: the callback may take locks of its own
    Sim800UartLineCallback callback = uart->line_callback;
    if(callback) callback(line, uart->line_context);
}

static void sim800_uart_rx_byte(Sim800Uart* uart, uint8_t data) {
    if(data == '\n') {
        if(furi_string_size(uart->line) > 0) {
            sim800_uart_process_line(uart, furi_string_get_cstr(uart->line));
            furi_string_reset(uart->line);
        }
        return;
    }
    if(data == '\r') return;

    if(furi_string_size(uart->line) < SIM800_LINE_MAX) {
        furi_string_push_back(uart->line, (char)data);
    }

    // The "> " prompt has no terminator, so it can only be seen mid-line
    if(data == '>' && furi_string_size(uart->line) == 1) {
        furi_mutex_acquire(uart->mutex, FuriWaitForever);
        bool wanted = uart->want_prompt;
        if(wanted) uart->want_prompt = false;
        furi_mutex_release(uart->mutex);
        if(wanted) {
            furi_string_reset(uart->line);
            furi_semaphore_release(uart->prompt_sem);
        }
    }
}

static int32_t sim800_uart_worker(void* context) {
    Sim800Uart* uart = context;
    uint8_t buffer[64];

    while(true) {
        uint32_t flags =
            furi_thread_flags_wait(WORKER_ALL_FLAGS, FuriFlagWaitAny, FuriWaitForever);
        if(flags & WorkerFlagStop) break;
        if(!(flags & WorkerFlagRx)) continue;

        size_t len;
        while((len = furi_stream_buffer_receive(uart->rx_stream, buffer, sizeof(buffer), 0)) > 0) {
            for(size_t i = 0; i < len; i++) {
                sim800_uart_rx_byte(uart, buffer[i]);
            }
        }
    }

    return 0;
}

static void
    sim800_uart_irq_cb(FuriHalSerialHandle* handle, FuriHalSerialRxEvent event, void* context) {
    Sim800Uart* uart = context;

    if(event & FuriHalSerialRxEventData) {
        while(furi_hal_serial_async_rx_available(handle)) {
            uint8_t data = furi_hal_serial_async_rx(handle);
            furi_stream_buffer_send(uart->rx_stream, &data, 1, 0);
        }
        furi_thread_flags_set(furi_thread_get_id(uart->thread), WorkerFlagRx);
    }
}

Sim800Uart* sim800_uart_alloc(void) {
    Sim800Uart* uart = malloc(sizeof(Sim800Uart));

    uart->handle = NULL;
    uart->open = false;
    uart->rx_stream = furi_stream_buffer_alloc(SIM800_RX_STREAM_SIZE, 1);
    uart->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    uart->cmd_sem = furi_semaphore_alloc(1, 0);
    uart->prompt_sem = furi_semaphore_alloc(1, 0);
    uart->cmd_pending = false;
    uart->want_prompt = false;
    uart->cmd_call_progress = false;
    uart->cmd_result = Sim800AtResultTimeout;
    uart->cmd_response = furi_string_alloc();
    uart->line = furi_string_alloc();
    uart->line_callback = NULL;
    uart->line_context = NULL;

    uart->thread = furi_thread_alloc_ex("Sim800UartWorker", 1024, sim800_uart_worker, uart);
    furi_thread_start(uart->thread);

    return uart;
}

void sim800_uart_free(Sim800Uart* uart) {
    furi_assert(uart);

    sim800_uart_close(uart);

    furi_thread_flags_set(furi_thread_get_id(uart->thread), WorkerFlagStop);
    furi_thread_join(uart->thread);
    furi_thread_free(uart->thread);

    furi_stream_buffer_free(uart->rx_stream);
    furi_mutex_free(uart->mutex);
    furi_semaphore_free(uart->cmd_sem);
    furi_semaphore_free(uart->prompt_sem);
    furi_string_free(uart->cmd_response);
    furi_string_free(uart->line);
    free(uart);
}

void sim800_uart_set_line_callback(Sim800Uart* uart, Sim800UartLineCallback callback, void* ctx) {
    furi_assert(uart);
    uart->line_context = ctx;
    uart->line_callback = callback;
}

bool sim800_uart_open(Sim800Uart* uart, FuriHalSerialId serial_id, uint32_t baudrate) {
    furi_assert(uart);

    sim800_uart_close(uart);

    uart->handle = furi_hal_serial_control_acquire(serial_id);
    if(uart->handle == NULL) {
        FURI_LOG_E(TAG, "Serial %d is busy", serial_id);
        return false;
    }

    furi_hal_serial_init(uart->handle, baudrate);
    furi_hal_serial_configure_framing(
        uart->handle, FuriHalSerialDataBits8, FuriHalSerialParityNone, FuriHalSerialStopBits1);
    furi_hal_serial_async_rx_start(uart->handle, sim800_uart_irq_cb, uart, false);

    uart->open = true;
    return true;
}

void sim800_uart_close(Sim800Uart* uart) {
    furi_assert(uart);
    if(!uart->open) return;

    uart->open = false;
    furi_hal_serial_async_rx_stop(uart->handle);
    furi_hal_serial_deinit(uart->handle);
    furi_hal_serial_control_release(uart->handle);
    uart->handle = NULL;

    furi_stream_buffer_reset(uart->rx_stream);
    furi_string_reset(uart->line);
}

bool sim800_uart_is_open(const Sim800Uart* uart) {
    furi_assert(uart);
    return uart->open;
}

void sim800_uart_tx(Sim800Uart* uart, const uint8_t* data, size_t len) {
    furi_assert(uart);
    if(!uart->open || len == 0) return;
    furi_hal_serial_tx(uart->handle, data, len);
    furi_hal_serial_tx_wait_complete(uart->handle);
}

void sim800_uart_tx_str(Sim800Uart* uart, const char* str) {
    sim800_uart_tx(uart, (const uint8_t*)str, strlen(str));
}

/** Arm the command block, draining a token left by a line that landed after a timeout. */
static void sim800_uart_arm(Sim800Uart* uart, bool prompt, bool call_progress) {
    while(furi_semaphore_acquire(uart->cmd_sem, 0) == FuriStatusOk) {
    }
    while(furi_semaphore_acquire(uart->prompt_sem, 0) == FuriStatusOk) {
    }

    furi_mutex_acquire(uart->mutex, FuriWaitForever);
    furi_string_reset(uart->cmd_response);
    uart->cmd_result = Sim800AtResultTimeout;
    uart->cmd_pending = true;
    uart->want_prompt = prompt;
    uart->cmd_call_progress = call_progress;
    furi_mutex_release(uart->mutex);
}

static Sim800AtResult
    sim800_uart_collect(Sim800Uart* uart, uint32_t timeout_ms, FuriString* response) {
    FuriStatus status = furi_semaphore_acquire(uart->cmd_sem, timeout_ms);

    furi_mutex_acquire(uart->mutex, FuriWaitForever);
    uart->cmd_pending = false;
    uart->want_prompt = false;
    uart->cmd_call_progress = false;
    Sim800AtResult result = (status == FuriStatusOk) ? uart->cmd_result : Sim800AtResultTimeout;
    if(response) furi_string_set(response, uart->cmd_response);
    furi_mutex_release(uart->mutex);

    return result;
}

Sim800AtResult sim800_uart_command(
    Sim800Uart* uart,
    const char* cmd,
    uint32_t timeout_ms,
    FuriString* response) {
    furi_assert(uart);
    if(!uart->open) return Sim800AtResultNotOpen;

    sim800_uart_arm(uart, false, false);
    sim800_uart_tx_str(uart, cmd);
    sim800_uart_tx_str(uart, "\r");

    return sim800_uart_collect(uart, timeout_ms, response);
}

Sim800AtResult sim800_uart_command_call(
    Sim800Uart* uart,
    const char* cmd,
    uint32_t timeout_ms,
    FuriString* response) {
    furi_assert(uart);
    if(!uart->open) return Sim800AtResultNotOpen;

    sim800_uart_arm(uart, false, true);
    sim800_uart_tx_str(uart, cmd);
    sim800_uart_tx_str(uart, "\r");

    return sim800_uart_collect(uart, timeout_ms, response);
}

Sim800AtResult sim800_uart_command_prompt(
    Sim800Uart* uart,
    const char* cmd,
    const char* body,
    uint32_t timeout_ms,
    FuriString* response) {
    furi_assert(uart);
    if(!uart->open) return Sim800AtResultNotOpen;

    sim800_uart_arm(uart, true, false);
    sim800_uart_tx_str(uart, cmd);
    sim800_uart_tx_str(uart, "\r");

    if(furi_semaphore_acquire(uart->prompt_sem, SIM800_PROMPT_TIMEOUT) != FuriStatusOk) {
        // Leave the module out of body-entry mode, or every later command is swallowed as text
        const uint8_t esc = SIM800_ESC;
        sim800_uart_tx(uart, &esc, 1);
        return sim800_uart_collect(uart, 200, response);
    }

    sim800_uart_tx_str(uart, body);
    const uint8_t ctrl_z = SIM800_CTRL_Z;
    sim800_uart_tx(uart, &ctrl_z, 1);

    return sim800_uart_collect(uart, timeout_ms, response);
}

bool sim800_uart_sync(Sim800Uart* uart, size_t attempts) {
    // A module left on autobaud only locks onto the rate after it has seen a few "AT"s, and one
    // that was idling eats the first byte regardless. Either way a single try proves nothing.
    for(size_t i = 0; i < attempts; i++) {
        if(sim800_uart_command(uart, "AT", 300, NULL) == Sim800AtResultOk) return true;
        furi_delay_ms(50);
    }
    return false;
}

bool sim800_uart_probe(Sim800Uart* uart) {
    return sim800_uart_sync(uart, 3);
}

bool sim800_response_field(const FuriString* response, const char* prefix, FuriString* out) {
    furi_assert(response);
    furi_assert(prefix);
    furi_assert(out);

    const char* text = furi_string_get_cstr(response);
    size_t prefix_len = strlen(prefix);

    while(text != NULL) {
        if(strncmp(text, prefix, prefix_len) == 0) {
            const char* value = text + prefix_len;
            while(*value == ':' || *value == ' ')
                value++;
            const char* end = strchr(value, '\n');
            if(end) {
                furi_string_set_strn(out, value, (size_t)(end - value));
            } else {
                furi_string_set_str(out, value);
            }
            furi_string_trim(out);
            return true;
        }
        text = strchr(text, '\n');
        if(text) text++;
    }
    return false;
}

void sim800_unquote(FuriString* str) {
    furi_assert(str);
    furi_string_trim(str);
    if(furi_string_size(str) >= 2 && furi_string_get_char(str, 0) == '"' &&
       furi_string_get_char(str, furi_string_size(str) - 1) == '"') {
        furi_string_right(str, 1);
        furi_string_left(str, furi_string_size(str) - 1);
    }
}

bool sim800_csv_field(const char* list, size_t index, FuriString* out) {
    furi_assert(list);
    furi_assert(out);

    const char* start = list;
    size_t current = 0;
    bool in_quotes = false;

    for(const char* p = list;; p++) {
        if(*p == '"') in_quotes = !in_quotes;
        if((*p == ',' && !in_quotes) || *p == '\0' || *p == '\n') {
            if(current == index) {
                furi_string_set_strn(out, start, (size_t)(p - start));
                sim800_unquote(out);
                return true;
            }
            if(*p == '\0' || *p == '\n') return false;
            current++;
            start = p + 1;
        }
    }
}
