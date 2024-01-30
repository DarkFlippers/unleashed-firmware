#include "expansion.h"
#include "expansion_i.h"

#include <furi_hal_power.h>
#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>

#include <furi.h>

#include <rpc/rpc.h>

#include "expansion_settings.h"
#include "expansion_protocol.h"

#define TAG "ExpansionSrv"

#define EXPANSION_BUFFER_SIZE (sizeof(ExpansionFrame) + sizeof(ExpansionFrameChecksum))

typedef enum {
    ExpansionStateDisabled,
    ExpansionStateEnabled,
    ExpansionStateRunning,
} ExpansionState;

typedef enum {
    ExpansionSessionStateHandShake,
    ExpansionSessionStateConnected,
    ExpansionSessionStateRpcActive,
} ExpansionSessionState;

typedef enum {
    ExpansionSessionExitReasonUnknown,
    ExpansionSessionExitReasonUser,
    ExpansionSessionExitReasonError,
    ExpansionSessionExitReasonTimeout,
} ExpansionSessionExitReason;

typedef enum {
    ExpansionFlagStop = 1 << 0,
    ExpansionFlagData = 1 << 1,
    ExpansionFlagError = 1 << 2,
} ExpansionFlag;

#define EXPANSION_ALL_FLAGS (ExpansionFlagData | ExpansionFlagStop)

struct Expansion {
    ExpansionState state;
    ExpansionSessionState session_state;
    ExpansionSessionExitReason exit_reason;
    FuriStreamBuffer* rx_buf;
    FuriSemaphore* tx_semaphore;
    FuriMutex* state_mutex;
    FuriThread* worker_thread;
    FuriHalSerialId serial_id;
    FuriHalSerialHandle* serial_handle;
    RpcSession* rpc_session;

    ExpansionSettings settings;
};

static void expansion_detect_callback(void* context);

// Called in UART IRQ context
static void expansion_serial_rx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context) {
    furi_assert(handle);
    furi_assert(context);

    Expansion* instance = context;

    if(event == FuriHalSerialRxEventData) {
        const uint8_t data = furi_hal_serial_async_rx(handle);
        furi_stream_buffer_send(instance->rx_buf, &data, sizeof(data), 0);
        furi_thread_flags_set(furi_thread_get_id(instance->worker_thread), ExpansionFlagData);
    }
}

static size_t expansion_receive_callback(uint8_t* data, size_t data_size, void* context) {
    Expansion* instance = context;

    size_t received_size = 0;

    while(true) {
        received_size += furi_stream_buffer_receive(
            instance->rx_buf, data + received_size, data_size - received_size, 0);

        if(received_size == data_size) break;

        const uint32_t flags = furi_thread_flags_wait(
            EXPANSION_ALL_FLAGS, FuriFlagWaitAny, furi_ms_to_ticks(EXPANSION_PROTOCOL_TIMEOUT_MS));

        if(flags & FuriFlagError) {
            if(flags == (unsigned)FuriFlagErrorTimeout) {
                // Exiting due to timeout
                instance->exit_reason = ExpansionSessionExitReasonTimeout;
            } else {
                // Exiting due to an unspecified error
                instance->exit_reason = ExpansionSessionExitReasonError;
            }
            break;
        } else if(flags & ExpansionFlagStop) {
            // Exiting due to explicit request
            instance->exit_reason = ExpansionSessionExitReasonUser;
            break;
        } else if(flags & ExpansionFlagError) {
            // Exiting due to RPC error
            instance->exit_reason = ExpansionSessionExitReasonError;
            break;
        } else if(flags & ExpansionFlagData) {
            // Go to buffer reading
            continue;
        }
    }

    return received_size;
}

static inline bool expansion_receive_frame(Expansion* instance, ExpansionFrame* frame) {
    return expansion_protocol_decode(frame, expansion_receive_callback, instance) ==
           ExpansionProtocolStatusOk;
}

static size_t expansion_send_callback(const uint8_t* data, size_t data_size, void* context) {
    Expansion* instance = context;
    furi_hal_serial_tx(instance->serial_handle, data, data_size);
    furi_hal_serial_tx_wait_complete(instance->serial_handle);
    return data_size;
}

static inline bool expansion_send_frame(Expansion* instance, const ExpansionFrame* frame) {
    return expansion_protocol_encode(frame, expansion_send_callback, instance) ==
           ExpansionProtocolStatusOk;
}

static bool expansion_send_heartbeat(Expansion* instance) {
    const ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeHeartbeat,
        .content.heartbeat = {},
    };

    return expansion_send_frame(instance, &frame);
}

static bool expansion_send_status_response(Expansion* instance, ExpansionFrameError error) {
    const ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeStatus,
        .content.status.error = error,
    };

    return expansion_send_frame(instance, &frame);
}

static bool
    expansion_send_data_response(Expansion* instance, const uint8_t* data, size_t data_size) {
    furi_assert(data_size <= EXPANSION_PROTOCOL_MAX_DATA_SIZE);

    ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeData,
        .content.data.size = data_size,
    };

    memcpy(frame.content.data.bytes, data, data_size);
    return expansion_send_frame(instance, &frame);
}

// Called in Rpc session thread context
static void expansion_rpc_send_callback(void* context, uint8_t* data, size_t data_size) {
    Expansion* instance = context;

    for(size_t sent_data_size = 0; sent_data_size < data_size;) {
        if(furi_semaphore_acquire(
               instance->tx_semaphore, furi_ms_to_ticks(EXPANSION_PROTOCOL_TIMEOUT_MS)) !=
           FuriStatusOk) {
            furi_thread_flags_set(furi_thread_get_id(instance->worker_thread), ExpansionFlagError);
            break;
        }

        const size_t current_data_size =
            MIN(data_size - sent_data_size, EXPANSION_PROTOCOL_MAX_DATA_SIZE);
        if(!expansion_send_data_response(instance, data + sent_data_size, current_data_size))
            break;
        sent_data_size += current_data_size;
    }
}

static bool expansion_rpc_session_open(Expansion* instance) {
    Rpc* rpc = furi_record_open(RECORD_RPC);
    instance->rpc_session = rpc_session_open(rpc, RpcOwnerUart);

    if(instance->rpc_session) {
        instance->tx_semaphore = furi_semaphore_alloc(1, 1);
        rpc_session_set_context(instance->rpc_session, instance);
        rpc_session_set_send_bytes_callback(instance->rpc_session, expansion_rpc_send_callback);
    }

    return instance->rpc_session != NULL;
}

static void expansion_rpc_session_close(Expansion* instance) {
    if(instance->rpc_session) {
        rpc_session_close(instance->rpc_session);
        furi_semaphore_free(instance->tx_semaphore);
    }

    furi_record_close(RECORD_RPC);
}

static bool
    expansion_handle_session_state_handshake(Expansion* instance, const ExpansionFrame* rx_frame) {
    bool success = false;

    do {
        if(rx_frame->header.type != ExpansionFrameTypeBaudRate) break;
        const uint32_t baud_rate = rx_frame->content.baud_rate.baud;

        FURI_LOG_D(TAG, "Proposed baud rate: %lu", baud_rate);

        if(furi_hal_serial_is_baud_rate_supported(instance->serial_handle, baud_rate)) {
            instance->session_state = ExpansionSessionStateConnected;
            // Send response at previous baud rate
            if(!expansion_send_status_response(instance, ExpansionFrameErrorNone)) break;
            furi_hal_serial_set_br(instance->serial_handle, baud_rate);

        } else {
            if(!expansion_send_status_response(instance, ExpansionFrameErrorBaudRate)) break;
            FURI_LOG_E(TAG, "Bad baud rate");
        }
        success = true;
    } while(false);

    return success;
}

static bool
    expansion_handle_session_state_connected(Expansion* instance, const ExpansionFrame* rx_frame) {
    bool success = false;

    do {
        if(rx_frame->header.type == ExpansionFrameTypeControl) {
            if(rx_frame->content.control.command != ExpansionFrameControlCommandStartRpc) break;
            instance->session_state = ExpansionSessionStateRpcActive;
            if(!expansion_rpc_session_open(instance)) break;
            if(!expansion_send_status_response(instance, ExpansionFrameErrorNone)) break;

        } else if(rx_frame->header.type == ExpansionFrameTypeHeartbeat) {
            if(!expansion_send_heartbeat(instance)) break;

        } else {
            break;
        }
        success = true;
    } while(false);

    return success;
}

static bool
    expansion_handle_session_state_rpc_active(Expansion* instance, const ExpansionFrame* rx_frame) {
    bool success = false;

    do {
        if(rx_frame->header.type == ExpansionFrameTypeData) {
            if(!expansion_send_status_response(instance, ExpansionFrameErrorNone)) break;

            const size_t size_consumed = rpc_session_feed(
                instance->rpc_session,
                rx_frame->content.data.bytes,
                rx_frame->content.data.size,
                EXPANSION_PROTOCOL_TIMEOUT_MS);
            if(size_consumed != rx_frame->content.data.size) break;

        } else if(rx_frame->header.type == ExpansionFrameTypeControl) {
            if(rx_frame->content.control.command != ExpansionFrameControlCommandStopRpc) break;
            instance->session_state = ExpansionSessionStateConnected;
            expansion_rpc_session_close(instance);
            if(!expansion_send_status_response(instance, ExpansionFrameErrorNone)) break;

        } else if(rx_frame->header.type == ExpansionFrameTypeStatus) {
            if(rx_frame->content.status.error != ExpansionFrameErrorNone) break;
            furi_semaphore_release(instance->tx_semaphore);

        } else if(rx_frame->header.type == ExpansionFrameTypeHeartbeat) {
            if(!expansion_send_heartbeat(instance)) break;

        } else {
            break;
        }
        success = true;
    } while(false);

    return success;
}

static inline void expansion_state_machine(Expansion* instance) {
    typedef bool (*ExpansionSessionStateHandler)(Expansion*, const ExpansionFrame*);

    static const ExpansionSessionStateHandler expansion_handlers[] = {
        [ExpansionSessionStateHandShake] = expansion_handle_session_state_handshake,
        [ExpansionSessionStateConnected] = expansion_handle_session_state_connected,
        [ExpansionSessionStateRpcActive] = expansion_handle_session_state_rpc_active,
    };

    ExpansionFrame rx_frame;

    while(true) {
        if(!expansion_receive_frame(instance, &rx_frame)) break;
        if(!expansion_handlers[instance->session_state](instance, &rx_frame)) break;
    }
}

static void expansion_worker_pending_callback(void* context, uint32_t arg) {
    furi_assert(context);
    UNUSED(arg);

    Expansion* instance = context;
    furi_thread_join(instance->worker_thread);

    // Do not re-enable detection interrupt on user-requested exit
    if(instance->exit_reason != ExpansionSessionExitReasonUser) {
        furi_check(furi_mutex_acquire(instance->state_mutex, FuriWaitForever) == FuriStatusOk);
        instance->state = ExpansionStateEnabled;
        furi_hal_serial_control_set_expansion_callback(
            instance->serial_id, expansion_detect_callback, instance);
        furi_mutex_release(instance->state_mutex);
    }
}

static int32_t expansion_worker(void* context) {
    furi_assert(context);
    Expansion* instance = context;

    furi_hal_power_insomnia_enter();
    furi_hal_serial_control_set_expansion_callback(instance->serial_id, NULL, NULL);

    instance->serial_handle = furi_hal_serial_control_acquire(instance->serial_id);
    furi_check(instance->serial_handle);

    FURI_LOG_D(TAG, "Service started");

    instance->rx_buf = furi_stream_buffer_alloc(EXPANSION_BUFFER_SIZE, 1);
    instance->session_state = ExpansionSessionStateHandShake;
    instance->exit_reason = ExpansionSessionExitReasonUnknown;

    furi_hal_serial_init(instance->serial_handle, EXPANSION_PROTOCOL_DEFAULT_BAUD_RATE);

    furi_hal_serial_async_rx_start(
        instance->serial_handle, expansion_serial_rx_callback, instance, false);

    if(expansion_send_heartbeat(instance)) {
        expansion_state_machine(instance);
    }

    if(instance->session_state == ExpansionSessionStateRpcActive) {
        expansion_rpc_session_close(instance);
    }

    FURI_LOG_D(TAG, "Service stopped");

    furi_hal_serial_control_release(instance->serial_handle);
    furi_stream_buffer_free(instance->rx_buf);

    furi_hal_power_insomnia_exit();
    furi_timer_pending_callback(expansion_worker_pending_callback, instance, 0);

    return 0;
}

// Called from the serial control thread
static void expansion_detect_callback(void* context) {
    furi_assert(context);
    Expansion* instance = context;

    furi_check(furi_mutex_acquire(instance->state_mutex, FuriWaitForever) == FuriStatusOk);

    if(instance->state == ExpansionStateEnabled) {
        instance->state = ExpansionStateRunning;
        furi_thread_start(instance->worker_thread);
    }

    furi_mutex_release(instance->state_mutex);
}

static Expansion* expansion_alloc() {
    Expansion* instance = malloc(sizeof(Expansion));

    instance->state_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->worker_thread = furi_thread_alloc_ex(TAG, 768, expansion_worker, instance);

    return instance;
}

void expansion_on_system_start(void* arg) {
    UNUSED(arg);

    Expansion* instance = expansion_alloc();
    furi_record_create(RECORD_EXPANSION, instance);

    expansion_settings_load(&instance->settings);
    expansion_enable(instance);
}

// Public API functions

void expansion_enable(Expansion* instance) {
    if(instance->settings.uart_index < FuriHalSerialIdMax) {
        expansion_set_listen_serial(instance, instance->settings.uart_index);
    }
}

void expansion_disable(Expansion* instance) {
    furi_check(furi_mutex_acquire(instance->state_mutex, FuriWaitForever) == FuriStatusOk);

    if(instance->state == ExpansionStateRunning) {
        furi_thread_flags_set(furi_thread_get_id(instance->worker_thread), ExpansionFlagStop);
        furi_thread_join(instance->worker_thread);
    } else if(instance->state == ExpansionStateEnabled) {
        FURI_LOG_D(TAG, "Detection disabled");
        furi_hal_serial_control_set_expansion_callback(instance->serial_id, NULL, NULL);
    }

    instance->state = ExpansionStateDisabled;

    furi_mutex_release(instance->state_mutex);
}

void expansion_set_listen_serial(Expansion* instance, FuriHalSerialId serial_id) {
    expansion_disable(instance);

    furi_check(furi_mutex_acquire(instance->state_mutex, FuriWaitForever) == FuriStatusOk);

    instance->serial_id = serial_id;
    instance->state = ExpansionStateEnabled;

    furi_hal_serial_control_set_expansion_callback(
        instance->serial_id, expansion_detect_callback, instance);

    furi_mutex_release(instance->state_mutex);

    FURI_LOG_D(TAG, "Detection enabled");
}

ExpansionSettings* expansion_get_settings(Expansion* instance) {
    return &instance->settings;
}
