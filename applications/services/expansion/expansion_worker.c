#include "expansion_worker.h"

#include <furi_hal_power.h>
#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>

#include <furi.h>
#include <rpc/rpc.h>

#include "expansion_protocol.h"

#define TAG "ExpansionSrv"

#define EXPANSION_WORKER_STACK_SZIE  (768UL)
#define EXPANSION_WORKER_BUFFER_SIZE (sizeof(ExpansionFrame) + sizeof(ExpansionFrameChecksum))

typedef enum {
    ExpansionWorkerStateHandShake,
    ExpansionWorkerStateConnected,
    ExpansionWorkerStateRpcActive,
} ExpansionWorkerState;

typedef enum {
    ExpansionWorkerExitReasonUnknown,
    ExpansionWorkerExitReasonUser,
    ExpansionWorkerExitReasonError,
    ExpansionWorkerExitReasonTimeout,
} ExpansionWorkerExitReason;

typedef enum {
    ExpansionWorkerFlagStop = 1 << 0,
    ExpansionWorkerFlagData = 1 << 1,
    ExpansionWorkerFlagError = 1 << 2,
} ExpansionWorkerFlag;

#define EXPANSION_ALL_FLAGS (ExpansionWorkerFlagData | ExpansionWorkerFlagStop)

struct ExpansionWorker {
    FuriThread* thread;
    FuriStreamBuffer* rx_buf;
    FuriSemaphore* tx_semaphore;

    FuriHalSerialId serial_id;
    FuriHalSerialHandle* serial_handle;

    RpcSession* rpc_session;

    ExpansionWorkerState state;
    ExpansionWorkerExitReason exit_reason;
    ExpansionWorkerCallback callback;
    void* cb_context;
};

// Called in UART IRQ context
static void expansion_worker_serial_rx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context) {
    furi_assert(handle);
    furi_assert(context);

    ExpansionWorker* instance = context;

    if(event & (FuriHalSerialRxEventNoiseError | FuriHalSerialRxEventFrameError |
                FuriHalSerialRxEventOverrunError)) {
        furi_thread_flags_set(furi_thread_get_id(instance->thread), ExpansionWorkerFlagError);
    } else if(event & FuriHalSerialRxEventData) {
        while(furi_hal_serial_async_rx_available(handle)) {
            const uint8_t data = furi_hal_serial_async_rx(handle);
            furi_stream_buffer_send(instance->rx_buf, &data, sizeof(data), 0);
        }
        furi_thread_flags_set(furi_thread_get_id(instance->thread), ExpansionWorkerFlagData);
    }
}

static size_t expansion_worker_receive_callback(uint8_t* data, size_t data_size, void* context) {
    ExpansionWorker* instance = context;

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
                instance->exit_reason = ExpansionWorkerExitReasonTimeout;
            } else {
                // Exiting due to an unspecified error
                instance->exit_reason = ExpansionWorkerExitReasonError;
            }
            break;
        } else if(flags & ExpansionWorkerFlagStop) {
            // Exiting due to explicit request
            instance->exit_reason = ExpansionWorkerExitReasonUser;
            break;
        } else if(flags & ExpansionWorkerFlagError) {
            // Exiting due to RPC error
            instance->exit_reason = ExpansionWorkerExitReasonError;
            break;
        } else if(flags & ExpansionWorkerFlagData) {
            // Go to buffer reading
            continue;
        }
    }

    return received_size;
}

static inline bool
    expansion_worker_receive_frame(ExpansionWorker* instance, ExpansionFrame* frame) {
    return expansion_protocol_decode(frame, expansion_worker_receive_callback, instance) ==
           ExpansionProtocolStatusOk;
}

static size_t
    expansion_worker_send_callback(const uint8_t* data, size_t data_size, void* context) {
    ExpansionWorker* instance = context;
    furi_hal_serial_tx(instance->serial_handle, data, data_size);
    furi_hal_serial_tx_wait_complete(instance->serial_handle);
    return data_size;
}

static inline bool
    expansion_worker_send_frame(ExpansionWorker* instance, const ExpansionFrame* frame) {
    return expansion_protocol_encode(frame, expansion_worker_send_callback, instance) ==
           ExpansionProtocolStatusOk;
}

static bool expansion_worker_send_heartbeat(ExpansionWorker* instance) {
    const ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeHeartbeat,
        .content.heartbeat = {},
    };

    return expansion_worker_send_frame(instance, &frame);
}

static bool
    expansion_worker_send_status_response(ExpansionWorker* instance, ExpansionFrameError error) {
    const ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeStatus,
        .content.status.error = error,
    };

    return expansion_worker_send_frame(instance, &frame);
}

static bool expansion_worker_send_data_response(
    ExpansionWorker* instance,
    const uint8_t* data,
    size_t data_size) {
    furi_assert(data_size <= EXPANSION_PROTOCOL_MAX_DATA_SIZE);

    ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeData,
        .content.data.size = data_size,
    };

    memcpy(frame.content.data.bytes, data, data_size);
    return expansion_worker_send_frame(instance, &frame);
}

// Called in Rpc session thread context
static void expansion_worker_rpc_send_callback(void* context, uint8_t* data, size_t data_size) {
    ExpansionWorker* instance = context;

    for(size_t sent_data_size = 0; sent_data_size < data_size;) {
        if(furi_semaphore_acquire(
               instance->tx_semaphore, furi_ms_to_ticks(EXPANSION_PROTOCOL_TIMEOUT_MS)) !=
           FuriStatusOk) {
            furi_thread_flags_set(furi_thread_get_id(instance->thread), ExpansionWorkerFlagError);
            break;
        }

        const size_t current_data_size =
            MIN(data_size - sent_data_size, EXPANSION_PROTOCOL_MAX_DATA_SIZE);
        if(!expansion_worker_send_data_response(instance, data + sent_data_size, current_data_size))
            break;
        sent_data_size += current_data_size;
    }
}

static bool expansion_worker_rpc_session_open(ExpansionWorker* instance) {
    Rpc* rpc = furi_record_open(RECORD_RPC);
    instance->rpc_session = rpc_session_open(rpc, RpcOwnerUart);

    if(instance->rpc_session) {
        instance->tx_semaphore = furi_semaphore_alloc(1, 1);
        rpc_session_set_context(instance->rpc_session, instance);
        rpc_session_set_send_bytes_callback(
            instance->rpc_session, expansion_worker_rpc_send_callback);
    }

    return instance->rpc_session != NULL;
}

static void expansion_worker_rpc_session_close(ExpansionWorker* instance) {
    if(instance->rpc_session) {
        rpc_session_close(instance->rpc_session);
        furi_semaphore_free(instance->tx_semaphore);
    }

    furi_record_close(RECORD_RPC);
}

static bool expansion_worker_handle_state_handshake(
    ExpansionWorker* instance,
    const ExpansionFrame* rx_frame) {
    bool success = false;

    do {
        if(rx_frame->header.type != ExpansionFrameTypeBaudRate) break;
        const uint32_t baud_rate = rx_frame->content.baud_rate.baud;

        FURI_LOG_D(TAG, "Proposed baud rate: %lu", baud_rate);

        if(furi_hal_serial_is_baud_rate_supported(instance->serial_handle, baud_rate)) {
            instance->state = ExpansionWorkerStateConnected;
            // Send response at previous baud rate
            if(!expansion_worker_send_status_response(instance, ExpansionFrameErrorNone)) break;
            furi_hal_serial_set_br(instance->serial_handle, baud_rate);

        } else {
            if(!expansion_worker_send_status_response(instance, ExpansionFrameErrorBaudRate))
                break;
            FURI_LOG_E(TAG, "Bad baud rate");
        }
        success = true;
    } while(false);

    return success;
}

static bool expansion_worker_handle_state_connected(
    ExpansionWorker* instance,
    const ExpansionFrame* rx_frame) {
    bool success = false;

    do {
        if(rx_frame->header.type == ExpansionFrameTypeControl) {
            const uint8_t command = rx_frame->content.control.command;
            if(command == ExpansionFrameControlCommandStartRpc) {
                if(!expansion_worker_rpc_session_open(instance)) break;
                instance->state = ExpansionWorkerStateRpcActive;
            } else if(command == ExpansionFrameControlCommandEnableOtg) {
                furi_hal_power_enable_otg();
            } else if(command == ExpansionFrameControlCommandDisableOtg) {
                furi_hal_power_disable_otg();
            } else {
                break;
            }

            if(!expansion_worker_send_status_response(instance, ExpansionFrameErrorNone)) break;

        } else if(rx_frame->header.type == ExpansionFrameTypeHeartbeat) {
            if(!expansion_worker_send_heartbeat(instance)) break;

        } else {
            break;
        }
        success = true;
    } while(false);

    return success;
}

static bool expansion_worker_handle_state_rpc_active(
    ExpansionWorker* instance,
    const ExpansionFrame* rx_frame) {
    bool success = false;

    do {
        if(rx_frame->header.type == ExpansionFrameTypeData) {
            if(!expansion_worker_send_status_response(instance, ExpansionFrameErrorNone)) break;

            const size_t size_consumed = rpc_session_feed(
                instance->rpc_session,
                rx_frame->content.data.bytes,
                rx_frame->content.data.size,
                EXPANSION_PROTOCOL_TIMEOUT_MS);
            if(size_consumed != rx_frame->content.data.size) break;

        } else if(rx_frame->header.type == ExpansionFrameTypeControl) {
            const uint8_t command = rx_frame->content.control.command;
            if(command == ExpansionFrameControlCommandStopRpc) {
                instance->state = ExpansionWorkerStateConnected;
                expansion_worker_rpc_session_close(instance);
            } else {
                break;
            }

            if(!expansion_worker_send_status_response(instance, ExpansionFrameErrorNone)) break;

        } else if(rx_frame->header.type == ExpansionFrameTypeStatus) {
            if(rx_frame->content.status.error != ExpansionFrameErrorNone) break;
            furi_semaphore_release(instance->tx_semaphore);

        } else if(rx_frame->header.type == ExpansionFrameTypeHeartbeat) {
            if(!expansion_worker_send_heartbeat(instance)) break;

        } else {
            break;
        }
        success = true;
    } while(false);

    return success;
}

typedef bool (*ExpansionWorkerStateHandler)(ExpansionWorker*, const ExpansionFrame*);

static const ExpansionWorkerStateHandler expansion_handlers[] = {
    [ExpansionWorkerStateHandShake] = expansion_worker_handle_state_handshake,
    [ExpansionWorkerStateConnected] = expansion_worker_handle_state_connected,
    [ExpansionWorkerStateRpcActive] = expansion_worker_handle_state_rpc_active,
};

static inline void expansion_worker_state_machine(ExpansionWorker* instance) {
    ExpansionFrame rx_frame;

    while(true) {
        if(!expansion_worker_receive_frame(instance, &rx_frame)) break;
        if(!expansion_handlers[instance->state](instance, &rx_frame)) break;
    }
}

static int32_t expansion_worker(void* context) {
    furi_assert(context);
    ExpansionWorker* instance = context;

    furi_hal_power_insomnia_enter();

    instance->serial_handle = furi_hal_serial_control_acquire(instance->serial_id);
    furi_check(instance->serial_handle);

    FURI_LOG_D(TAG, "Worker started");

    instance->state = ExpansionWorkerStateHandShake;
    instance->exit_reason = ExpansionWorkerExitReasonUnknown;

    furi_hal_serial_init(instance->serial_handle, EXPANSION_PROTOCOL_DEFAULT_BAUD_RATE);

    furi_hal_serial_async_rx_start(
        instance->serial_handle, expansion_worker_serial_rx_callback, instance, true);

    if(expansion_worker_send_heartbeat(instance)) {
        expansion_worker_state_machine(instance);
    }

    if(instance->state == ExpansionWorkerStateRpcActive) {
        expansion_worker_rpc_session_close(instance);
    }

    FURI_LOG_D(TAG, "Worker stopped");

    furi_hal_serial_control_release(instance->serial_handle);
    furi_hal_power_insomnia_exit();

    // Do not invoke worker callback on user-requested exit
    if((instance->exit_reason != ExpansionWorkerExitReasonUser) && (instance->callback != NULL)) {
        instance->callback(instance->cb_context);
    }

    return 0;
}

ExpansionWorker* expansion_worker_alloc(FuriHalSerialId serial_id) {
    ExpansionWorker* instance = malloc(sizeof(ExpansionWorker));

    instance->thread = furi_thread_alloc_ex(
        TAG "Worker", EXPANSION_WORKER_STACK_SZIE, expansion_worker, instance);
    instance->rx_buf = furi_stream_buffer_alloc(EXPANSION_WORKER_BUFFER_SIZE, 1);
    instance->serial_id = serial_id;

    // Improves responsiveness in heavy games at the expense of dropped frames
    furi_thread_set_priority(instance->thread, FuriThreadPriorityLow);

    return instance;
}

void expansion_worker_free(ExpansionWorker* instance) {
    furi_stream_buffer_free(instance->rx_buf);
    furi_thread_join(instance->thread);
    furi_thread_free(instance->thread);
    free(instance);
}

void expansion_worker_set_callback(
    ExpansionWorker* instance,
    ExpansionWorkerCallback callback,
    void* context) {
    instance->callback = callback;
    instance->cb_context = context;
}

void expansion_worker_start(ExpansionWorker* instance) {
    furi_thread_start(instance->thread);
}

void expansion_worker_stop(ExpansionWorker* instance) {
    furi_thread_flags_set(furi_thread_get_id(instance->thread), ExpansionWorkerFlagStop);
    furi_thread_join(instance->thread);
}
