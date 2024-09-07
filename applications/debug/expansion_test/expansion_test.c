/**
 * @file expansion_test.c
 * @brief Expansion module support testing application.
 *
 * Before running, connect pins using the following scheme:
 * 13 -> 16 (USART TX to LPUART RX)
 * 14 -> 15 (USART RX to LPUART TX)
 *
 * Optional: Connect an LED with an appropriate series resistor
 * between pins 1 and 8. It will always be on if the device is
 * connected to USB power, so unplug it before running the app.
 *
 * What this application does:
 *
 * - Enables module support and emulates the module on a single device
 *   (hence the above connection),
 * - Connects to the expansion module service, sets baud rate,
 * - Enables OTG (5V) on GPIO via plain expansion protocol,
 * - Waits 5 cycles of idle loop (1 second),
 * - Starts the RPC session,
 * - Disables OTG (5V) on GPIO via RPC messages,
 * - Waits 5 cycles of idle loop (1 second),
 * - Creates a directory at `/ext/ExpansionTest` and writes a file
 *   named `test.txt` under it,
 * - Plays an audiovisual alert (sound and blinking display),
 * - Enables OTG (5V) on GPIO via RPC messages,
 * - Waits 5 cycles of idle loop (1 second),
 * - Stops the RPC session,
 * - Disables OTG (5V) on GPIO via plain expansion protocol,
 * - Exits (plays a sound if any of the above steps failed).
 */
#include <furi.h>

#include <furi_hal_resources.h>

#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>

#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include <flipper.pb.h>

#include <storage/storage.h>
#include <expansion/expansion.h>
#include <notification/notification_messages.h>
#include <expansion/expansion_protocol.h>

#define TAG "ExpansionTest"

#define TEST_DIR_PATH  EXT_PATH(TAG)
#define TEST_FILE_NAME "test.txt"
#define TEST_FILE_PATH EXT_PATH(TAG "/" TEST_FILE_NAME)

#define HOST_SERIAL_ID   (FuriHalSerialIdLpuart)
#define MODULE_SERIAL_ID (FuriHalSerialIdUsart)

#define RECEIVE_BUFFER_SIZE (sizeof(ExpansionFrame) + sizeof(ExpansionFrameChecksum))

typedef enum {
    ExpansionTestAppFlagData = 1U << 0,
    ExpansionTestAppFlagExit = 1U << 1,
} ExpansionTestAppFlag;

#define EXPANSION_TEST_APP_ALL_FLAGS (ExpansionTestAppFlagData | ExpansionTestAppFlagExit)

typedef struct {
    FuriThreadId thread_id;
    Expansion* expansion;
    FuriHalSerialHandle* handle;
    FuriStreamBuffer* buf;
    ExpansionFrame frame;
    PB_Main msg;
    Storage* storage;
} ExpansionTestApp;

static void expansion_test_app_serial_rx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context) {
    furi_assert(handle);
    furi_assert(context);
    ExpansionTestApp* app = context;

    if(event == FuriHalSerialRxEventData) {
        const uint8_t data = furi_hal_serial_async_rx(handle);
        furi_stream_buffer_send(app->buf, &data, sizeof(data), 0);
        furi_thread_flags_set(app->thread_id, ExpansionTestAppFlagData);
    }
}

static ExpansionTestApp* expansion_test_app_alloc(void) {
    ExpansionTestApp* instance = malloc(sizeof(ExpansionTestApp));
    instance->buf = furi_stream_buffer_alloc(RECEIVE_BUFFER_SIZE, 1);
    return instance;
}

static void expansion_test_app_free(ExpansionTestApp* instance) {
    furi_stream_buffer_free(instance->buf);
    free(instance);
}

static void expansion_test_app_start(ExpansionTestApp* instance) {
    instance->thread_id = furi_thread_get_current_id();
    instance->expansion = furi_record_open(RECORD_EXPANSION);
    instance->handle = furi_hal_serial_control_acquire(MODULE_SERIAL_ID);
    furi_check(instance->handle);
    // Configure the serial port
    furi_hal_serial_init(instance->handle, EXPANSION_PROTOCOL_DEFAULT_BAUD_RATE);
    // Start waiting for the initial pulse
    expansion_set_listen_serial(instance->expansion, HOST_SERIAL_ID);

    furi_hal_serial_async_rx_start(
        instance->handle, expansion_test_app_serial_rx_callback, instance, false);
}

static void expansion_test_app_stop(ExpansionTestApp* instance) {
    // Disable expansion module support
    expansion_disable(instance->expansion);
    // Give back the module handle
    furi_hal_serial_control_release(instance->handle);
    // Restore expansion user settings
    expansion_enable(instance->expansion);
    furi_record_close(RECORD_EXPANSION);
}

static inline bool expansion_test_app_is_success_response(const ExpansionFrame* response) {
    return response->header.type == ExpansionFrameTypeStatus &&
           response->content.status.error == ExpansionFrameErrorNone;
}

static inline bool expansion_test_app_is_success_rpc_message(const PB_Main* message) {
    return (message->command_status == PB_CommandStatus_OK ||
            message->command_status == PB_CommandStatus_ERROR_STORAGE_EXIST) &&
           (message->which_content == PB_Main_empty_tag);
}

static size_t expansion_test_app_receive_callback(uint8_t* data, size_t data_size, void* context) {
    ExpansionTestApp* instance = context;

    size_t received_size = 0;

    while(true) {
        received_size += furi_stream_buffer_receive(
            instance->buf, data + received_size, data_size - received_size, 0);
        if(received_size == data_size) break;

        const uint32_t flags = furi_thread_flags_wait(
            EXPANSION_TEST_APP_ALL_FLAGS, FuriFlagWaitAny, EXPANSION_PROTOCOL_TIMEOUT_MS);

        // Exit on any error
        if(flags & FuriFlagError) break;
    }

    return received_size;
}

static size_t
    expansion_test_app_send_callback(const uint8_t* data, size_t data_size, void* context) {
    ExpansionTestApp* instance = context;

    furi_hal_serial_tx(instance->handle, data, data_size);
    furi_hal_serial_tx_wait_complete(instance->handle);

    return data_size;
}

static bool expansion_test_app_receive_frame(ExpansionTestApp* instance, ExpansionFrame* frame) {
    return expansion_protocol_decode(frame, expansion_test_app_receive_callback, instance) ==
           ExpansionProtocolStatusOk;
}

static bool
    expansion_test_app_send_status_response(ExpansionTestApp* instance, ExpansionFrameError error) {
    ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeStatus,
        .content.status.error = error,
    };
    return expansion_protocol_encode(&frame, expansion_test_app_send_callback, instance) ==
           ExpansionProtocolStatusOk;
}

static bool expansion_test_app_send_heartbeat(ExpansionTestApp* instance) {
    ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeHeartbeat,
        .content.heartbeat = {},
    };
    return expansion_protocol_encode(&frame, expansion_test_app_send_callback, instance) ==
           ExpansionProtocolStatusOk;
}

static bool
    expansion_test_app_send_baud_rate_request(ExpansionTestApp* instance, uint32_t baud_rate) {
    ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeBaudRate,
        .content.baud_rate.baud = baud_rate,
    };
    return expansion_protocol_encode(&frame, expansion_test_app_send_callback, instance) ==
           ExpansionProtocolStatusOk;
}

static bool expansion_test_app_send_control_request(
    ExpansionTestApp* instance,
    ExpansionFrameControlCommand command) {
    ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeControl,
        .content.control.command = command,
    };
    return expansion_protocol_encode(&frame, expansion_test_app_send_callback, instance) ==
           ExpansionProtocolStatusOk;
}

static bool expansion_test_app_send_data_request(
    ExpansionTestApp* instance,
    const uint8_t* data,
    size_t data_size) {
    furi_assert(data_size <= EXPANSION_PROTOCOL_MAX_DATA_SIZE);

    ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeData,
        .content.data.size = data_size,
    };

    memcpy(frame.content.data.bytes, data, data_size);
    return expansion_protocol_encode(&frame, expansion_test_app_send_callback, instance) ==
           ExpansionProtocolStatusOk;
}

static bool expansion_test_app_rpc_encode_callback(
    pb_ostream_t* stream,
    const pb_byte_t* data,
    size_t data_size) {
    ExpansionTestApp* instance = stream->state;

    size_t size_sent = 0;

    while(size_sent < data_size) {
        const size_t current_size = MIN(data_size - size_sent, EXPANSION_PROTOCOL_MAX_DATA_SIZE);
        if(!expansion_test_app_send_data_request(instance, data + size_sent, current_size)) break;
        if(!expansion_test_app_receive_frame(instance, &instance->frame)) break;
        if(!expansion_test_app_is_success_response(&instance->frame)) break;
        size_sent += current_size;
    }

    return size_sent == data_size;
}

static bool expansion_test_app_send_rpc_request(ExpansionTestApp* instance, PB_Main* message) {
    pb_ostream_t stream = {
        .callback = expansion_test_app_rpc_encode_callback,
        .state = instance,
        .max_size = SIZE_MAX,
        .bytes_written = 0,
        .errmsg = NULL,
    };

    const bool success = pb_encode_ex(&stream, &PB_Main_msg, message, PB_ENCODE_DELIMITED);
    pb_release(&PB_Main_msg, message);
    return success;
}

static bool expansion_test_app_receive_rpc_request(ExpansionTestApp* instance, PB_Main* message) {
    bool success = false;

    do {
        if(!expansion_test_app_receive_frame(instance, &instance->frame)) break;
        if(!expansion_test_app_send_status_response(instance, ExpansionFrameErrorNone)) break;
        if(instance->frame.header.type != ExpansionFrameTypeData) break;
        pb_istream_t stream = pb_istream_from_buffer(
            instance->frame.content.data.bytes, instance->frame.content.data.size);
        if(!pb_decode_ex(&stream, &PB_Main_msg, message, PB_DECODE_DELIMITED)) break;
        success = true;
    } while(false);

    return success;
}

static bool expansion_test_app_send_presence(ExpansionTestApp* instance) {
    // Send pulses to emulate module insertion
    const uint8_t init = 0xAA;
    furi_hal_serial_tx(instance->handle, &init, sizeof(init));
    furi_hal_serial_tx_wait_complete(instance->handle);
    return true;
}

static bool expansion_test_app_wait_ready(ExpansionTestApp* instance) {
    bool success = false;

    do {
        if(!expansion_test_app_receive_frame(instance, &instance->frame)) break;
        if(instance->frame.header.type != ExpansionFrameTypeHeartbeat) break;
        success = true;
    } while(false);

    return success;
}

static bool expansion_test_app_handshake(ExpansionTestApp* instance) {
    bool success = false;

    do {
        if(!expansion_test_app_send_baud_rate_request(instance, 230400)) break;
        if(!expansion_test_app_receive_frame(instance, &instance->frame)) break;
        if(!expansion_test_app_is_success_response(&instance->frame)) break;
        furi_hal_serial_set_br(instance->handle, 230400);
        furi_delay_ms(EXPANSION_PROTOCOL_BAUD_CHANGE_DT_MS);
        success = true;
    } while(false);

    return success;
}

static bool expansion_test_app_enable_otg(ExpansionTestApp* instance, bool enable) {
    bool success = false;

    do {
        const ExpansionFrameControlCommand command = enable ?
                                                         ExpansionFrameControlCommandEnableOtg :
                                                         ExpansionFrameControlCommandDisableOtg;
        if(!expansion_test_app_send_control_request(instance, command)) break;
        if(!expansion_test_app_receive_frame(instance, &instance->frame)) break;
        if(!expansion_test_app_is_success_response(&instance->frame)) break;
        success = true;
    } while(false);

    return success;
}

static bool expansion_test_app_start_rpc(ExpansionTestApp* instance) {
    bool success = false;

    do {
        if(!expansion_test_app_send_control_request(instance, ExpansionFrameControlCommandStartRpc))
            break;
        if(!expansion_test_app_receive_frame(instance, &instance->frame)) break;
        if(!expansion_test_app_is_success_response(&instance->frame)) break;
        success = true;
    } while(false);

    return success;
}

static bool expansion_test_app_rpc_mkdir(ExpansionTestApp* instance) {
    bool success = false;

    instance->msg.command_id++;
    instance->msg.command_status = PB_CommandStatus_OK;
    instance->msg.which_content = PB_Main_storage_mkdir_request_tag;
    instance->msg.has_next = false;
    instance->msg.content.storage_mkdir_request.path = TEST_DIR_PATH;

    do {
        if(!expansion_test_app_send_rpc_request(instance, &instance->msg)) break;
        if(!expansion_test_app_receive_rpc_request(instance, &instance->msg)) break;
        if(!expansion_test_app_is_success_rpc_message(&instance->msg)) break;
        success = true;
    } while(false);

    return success;
}

static bool expansion_test_app_rpc_write(ExpansionTestApp* instance) {
    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, APP_ASSETS_PATH(TEST_FILE_NAME), FSAM_READ, FSOM_OPEN_EXISTING))
            break;

        const uint64_t file_size = storage_file_size(file);

        instance->msg.command_id++;
        instance->msg.command_status = PB_CommandStatus_OK;
        instance->msg.which_content = PB_Main_storage_write_request_tag;
        instance->msg.has_next = false;
        instance->msg.content.storage_write_request.path = TEST_FILE_PATH;
        instance->msg.content.storage_write_request.has_file = true;
        instance->msg.content.storage_write_request.file.data =
            malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(file_size));
        instance->msg.content.storage_write_request.file.data->size = file_size;

        const size_t bytes_read = storage_file_read(
            file, instance->msg.content.storage_write_request.file.data->bytes, file_size);

        if(bytes_read != file_size) {
            pb_release(&PB_Main_msg, &instance->msg);
            break;
        }

        if(!expansion_test_app_send_rpc_request(instance, &instance->msg)) break;
        if(!expansion_test_app_receive_rpc_request(instance, &instance->msg)) break;
        if(!expansion_test_app_is_success_rpc_message(&instance->msg)) break;
        success = true;
    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}

static bool expansion_test_app_rpc_alert(ExpansionTestApp* instance) {
    bool success = false;

    instance->msg.command_id++;
    instance->msg.command_status = PB_CommandStatus_OK;
    instance->msg.which_content = PB_Main_system_play_audiovisual_alert_request_tag;
    instance->msg.has_next = false;

    do {
        if(!expansion_test_app_send_rpc_request(instance, &instance->msg)) break;
        if(!expansion_test_app_receive_rpc_request(instance, &instance->msg)) break;
        if(instance->msg.which_content != PB_Main_empty_tag) break;
        if(instance->msg.command_status != PB_CommandStatus_OK) break;
        success = true;
    } while(false);

    return success;
}

static bool expansion_test_app_rpc_enable_otg(ExpansionTestApp* instance, bool enable) {
    bool success = false;

    instance->msg.command_id++;
    instance->msg.command_status = PB_CommandStatus_OK;
    instance->msg.which_content = PB_Main_gpio_set_otg_mode_tag;
    instance->msg.content.gpio_set_otg_mode.mode = enable ? PB_Gpio_GpioOtgMode_ON :
                                                            PB_Gpio_GpioOtgMode_OFF;
    instance->msg.has_next = false;

    do {
        if(!expansion_test_app_send_rpc_request(instance, &instance->msg)) break;
        if(!expansion_test_app_receive_rpc_request(instance, &instance->msg)) break;
        if(instance->msg.which_content != PB_Main_empty_tag) break;
        if(instance->msg.command_status != PB_CommandStatus_OK) break;
        success = true;
    } while(false);

    return success;
}

static bool expansion_test_app_idle(ExpansionTestApp* instance, uint32_t num_cycles) {
    uint32_t num_cycles_done;
    for(num_cycles_done = 0; num_cycles_done < num_cycles; ++num_cycles_done) {
        if(!expansion_test_app_send_heartbeat(instance)) break;
        if(!expansion_test_app_receive_frame(instance, &instance->frame)) break;
        if(instance->frame.header.type != ExpansionFrameTypeHeartbeat) break;
        furi_delay_ms(EXPANSION_PROTOCOL_TIMEOUT_MS - 50);
    }

    return num_cycles_done == num_cycles;
}

static bool expansion_test_app_stop_rpc(ExpansionTestApp* instance) {
    bool success = false;

    do {
        if(!expansion_test_app_send_control_request(instance, ExpansionFrameControlCommandStopRpc))
            break;
        if(!expansion_test_app_receive_frame(instance, &instance->frame)) break;
        if(!expansion_test_app_is_success_response(&instance->frame)) break;
        success = true;
    } while(false);

    return success;
}

int32_t expansion_test_app(void* p) {
    UNUSED(p);

    ExpansionTestApp* instance = expansion_test_app_alloc();
    expansion_test_app_start(instance);

    bool success = false;

    do {
        if(!expansion_test_app_send_presence(instance)) break;
        if(!expansion_test_app_wait_ready(instance)) break;
        if(!expansion_test_app_handshake(instance)) break;
        if(!expansion_test_app_enable_otg(instance, true)) break;
        if(!expansion_test_app_idle(instance, 5)) break;
        if(!expansion_test_app_start_rpc(instance)) break;
        if(!expansion_test_app_rpc_enable_otg(instance, false)) break;
        if(!expansion_test_app_idle(instance, 5)) break;
        if(!expansion_test_app_rpc_mkdir(instance)) break;
        if(!expansion_test_app_rpc_write(instance)) break;
        if(!expansion_test_app_rpc_alert(instance)) break;
        if(!expansion_test_app_rpc_enable_otg(instance, true)) break;
        if(!expansion_test_app_idle(instance, 5)) break;
        if(!expansion_test_app_stop_rpc(instance)) break;
        if(!expansion_test_app_enable_otg(instance, false)) break;
        success = true;
    } while(false);

    expansion_test_app_stop(instance);
    expansion_test_app_free(instance);

    if(!success) {
        NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
        notification_message(notification, &sequence_error);
        furi_record_close(RECORD_NOTIFICATION);
    }

    return 0;
}
