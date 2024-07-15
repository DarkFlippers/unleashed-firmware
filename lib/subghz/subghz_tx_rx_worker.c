#include "subghz_tx_rx_worker.h"

#include <furi.h>

#define TAG "SubGhzTxRxWorker"

#define SUBGHZ_TXRX_WORKER_BUF_SIZE      2048
//you can not set more than 62 because it will not fit into the FIFO CC1101
#define SUBGHZ_TXRX_WORKER_MAX_TXRX_SIZE 60

#define SUBGHZ_TXRX_WORKER_TIMEOUT_READ_WRITE_BUF 40

struct SubGhzTxRxWorker {
    FuriThread* thread;
    FuriStreamBuffer* stream_tx;
    FuriStreamBuffer* stream_rx;

    volatile bool worker_running;
    volatile bool worker_stoping;

    SubGhzTxRxWorkerStatus status;

    uint32_t frequency;
    const SubGhzDevice* device;
    const GpioPin* device_data_gpio;

    SubGhzTxRxWorkerCallbackHaveRead callback_have_read;
    void* context_have_read;
};

bool subghz_tx_rx_worker_write(SubGhzTxRxWorker* instance, uint8_t* data, size_t size) {
    furi_check(instance);
    bool ret = false;
    size_t stream_tx_free_byte = furi_stream_buffer_spaces_available(instance->stream_tx);
    if(size && (stream_tx_free_byte >= size)) {
        if(furi_stream_buffer_send(
               instance->stream_tx, data, size, SUBGHZ_TXRX_WORKER_TIMEOUT_READ_WRITE_BUF) ==
           size) {
            ret = true;
        }
    }
    return ret;
}

size_t subghz_tx_rx_worker_available(SubGhzTxRxWorker* instance) {
    furi_check(instance);
    return furi_stream_buffer_bytes_available(instance->stream_rx);
}

size_t subghz_tx_rx_worker_read(SubGhzTxRxWorker* instance, uint8_t* data, size_t size) {
    furi_check(instance);
    return furi_stream_buffer_receive(instance->stream_rx, data, size, 0);
}

void subghz_tx_rx_worker_set_callback_have_read(
    SubGhzTxRxWorker* instance,
    SubGhzTxRxWorkerCallbackHaveRead callback,
    void* context) {
    furi_check(instance);
    furi_check(callback);
    furi_check(context);
    instance->callback_have_read = callback;
    instance->context_have_read = context;
}

bool subghz_tx_rx_worker_rx(SubGhzTxRxWorker* instance, uint8_t* data, uint8_t* size) {
    uint8_t timeout = 100;
    bool ret = false;
    if(instance->status != SubGhzTxRxWorkerStatusRx) {
        subghz_devices_set_rx(instance->device);
        instance->status = SubGhzTxRxWorkerStatusRx;
        furi_delay_tick(1);
    }
    //waiting for reception to complete
    while(furi_hal_gpio_read(instance->device_data_gpio)) {
        furi_delay_tick(1);
        if(!--timeout) {
            FURI_LOG_W(TAG, "RX cc1101_g0 timeout");
            subghz_devices_flush_rx(instance->device);
            subghz_devices_set_rx(instance->device);
            break;
        }
    }

    if(subghz_devices_rx_pipe_not_empty(instance->device)) {
        FURI_LOG_I(
            TAG,
            "RSSI: %03.1fdbm LQI: %d",
            (double)subghz_devices_get_rssi(instance->device),
            subghz_devices_get_lqi(instance->device));
        if(subghz_devices_is_rx_data_crc_valid(instance->device)) {
            subghz_devices_read_packet(instance->device, data, size);
            ret = true;
        }
        subghz_devices_flush_rx(instance->device);
        subghz_devices_set_rx(instance->device);
    }
    return ret;
}

void subghz_tx_rx_worker_tx(SubGhzTxRxWorker* instance, uint8_t* data, size_t size) {
    uint8_t timeout = 200;
    if(instance->status != SubGhzTxRxWorkerStatusIDLE) {
        subghz_devices_idle(instance->device);
    }
    subghz_devices_write_packet(instance->device, data, size);
    subghz_devices_set_tx(instance->device); //start send
    instance->status = SubGhzTxRxWorkerStatusTx;
    while(!furi_hal_gpio_read(
        instance->device_data_gpio)) { // Wait for GDO0 to be set -> sync transmitted
        furi_delay_tick(1);
        if(!--timeout) {
            FURI_LOG_W(TAG, "TX !cc1101_g0 timeout");
            break;
        }
    }
    while(furi_hal_gpio_read(
        instance->device_data_gpio)) { // Wait for GDO0 to be cleared -> end of packet
        furi_delay_tick(1);
        if(!--timeout) {
            FURI_LOG_W(TAG, "TX cc1101_g0 timeout");
            break;
        }
    }
    subghz_devices_idle(instance->device);
    instance->status = SubGhzTxRxWorkerStatusIDLE;
}
/** Worker thread
 * 
 * @param context 
 * @return exit code 
 */
static int32_t subghz_tx_rx_worker_thread(void* context) {
    SubGhzTxRxWorker* instance = context;
    furi_check(instance->device);
    FURI_LOG_I(TAG, "Worker start");

    subghz_devices_begin(instance->device);
    instance->device_data_gpio = subghz_devices_get_data_gpio(instance->device);
    subghz_devices_reset(instance->device);
    subghz_devices_idle(instance->device);
    subghz_devices_load_preset(instance->device, FuriHalSubGhzPresetGFSK9_99KbAsync, NULL);

    furi_hal_gpio_init(instance->device_data_gpio, GpioModeInput, GpioPullNo, GpioSpeedLow);

    subghz_devices_set_frequency(instance->device, instance->frequency);
    subghz_devices_flush_rx(instance->device);

    uint8_t data[SUBGHZ_TXRX_WORKER_MAX_TXRX_SIZE + 1] = {0};
    size_t size_tx = 0;
    uint8_t size_rx[1] = {0};
    uint8_t timeout_tx = 0;
    bool callback_rx = false;

    while(instance->worker_running) {
        //transmit
        size_tx = furi_stream_buffer_bytes_available(instance->stream_tx);
        if(size_tx > 0 && !timeout_tx) {
            timeout_tx = 10; //20ms
            if(size_tx > SUBGHZ_TXRX_WORKER_MAX_TXRX_SIZE) {
                furi_stream_buffer_receive(
                    instance->stream_tx,
                    &data,
                    SUBGHZ_TXRX_WORKER_MAX_TXRX_SIZE,
                    SUBGHZ_TXRX_WORKER_TIMEOUT_READ_WRITE_BUF);
                subghz_tx_rx_worker_tx(instance, data, SUBGHZ_TXRX_WORKER_MAX_TXRX_SIZE);
            } else {
                furi_stream_buffer_receive(
                    instance->stream_tx, &data, size_tx, SUBGHZ_TXRX_WORKER_TIMEOUT_READ_WRITE_BUF);
                subghz_tx_rx_worker_tx(instance, data, size_tx);
            }
        } else {
            //recive
            if(subghz_tx_rx_worker_rx(instance, data, size_rx)) {
                if(furi_stream_buffer_spaces_available(instance->stream_rx) >= size_rx[0]) {
                    if(instance->callback_have_read &&
                       furi_stream_buffer_bytes_available(instance->stream_rx) == 0) {
                        callback_rx = true;
                    }
                    furi_stream_buffer_send(
                        instance->stream_rx,
                        &data,
                        size_rx[0],
                        SUBGHZ_TXRX_WORKER_TIMEOUT_READ_WRITE_BUF);
                    if(callback_rx) {
                        instance->callback_have_read(instance->context_have_read);
                        callback_rx = false;
                    }
                } else {
                    FURI_LOG_E(TAG, "Receive buffer overflow, over-the-air transmission too fast");
                }
            }
        }

        if(timeout_tx) timeout_tx--;
        furi_delay_tick(1);
    }

    subghz_devices_sleep(instance->device);
    subghz_devices_end(instance->device);

    FURI_LOG_I(TAG, "Worker stop");
    return 0;
}

SubGhzTxRxWorker* subghz_tx_rx_worker_alloc(void) {
    SubGhzTxRxWorker* instance = malloc(sizeof(SubGhzTxRxWorker));

    instance->thread =
        furi_thread_alloc_ex("SubGhzTxRxWorker", 2048, subghz_tx_rx_worker_thread, instance);
    instance->stream_tx =
        furi_stream_buffer_alloc(sizeof(uint8_t) * SUBGHZ_TXRX_WORKER_BUF_SIZE, sizeof(uint8_t));
    instance->stream_rx =
        furi_stream_buffer_alloc(sizeof(uint8_t) * SUBGHZ_TXRX_WORKER_BUF_SIZE, sizeof(uint8_t));

    instance->status = SubGhzTxRxWorkerStatusIDLE;
    instance->worker_stoping = true;

    return instance;
}

void subghz_tx_rx_worker_free(SubGhzTxRxWorker* instance) {
    furi_check(instance);
    furi_check(!instance->worker_running);
    furi_stream_buffer_free(instance->stream_tx);
    furi_stream_buffer_free(instance->stream_rx);
    furi_thread_free(instance->thread);

    free(instance);
}

bool subghz_tx_rx_worker_start(
    SubGhzTxRxWorker* instance,
    const SubGhzDevice* device,
    uint32_t frequency) {
    furi_check(instance);
    furi_check(!instance->worker_running);
    bool res = false;
    furi_stream_buffer_reset(instance->stream_tx);
    furi_stream_buffer_reset(instance->stream_rx);

    instance->worker_running = true;

    if(furi_hal_region_is_frequency_allowed(frequency)) {
        instance->frequency = frequency;
        instance->device = device;
        res = true;
    }

    furi_thread_start(instance->thread);

    return res;
}

void subghz_tx_rx_worker_stop(SubGhzTxRxWorker* instance) {
    furi_check(instance);
    furi_check(instance->worker_running);

    instance->worker_running = false;

    furi_thread_join(instance->thread);
}

bool subghz_tx_rx_worker_is_running(SubGhzTxRxWorker* instance) {
    furi_check(instance);
    return instance->worker_running;
}
