#include "subghz_tx_rx_worker.h"

#include <stream_buffer.h>
#include <furi.h>

#define TAG "SubGhzTxRxWorker"

#define SUBGHZ_TXRX_WORKER_BUF_SIZE 2048
//you can not set more than 62 because it will not fit into the FIFO CC1101
#define SUBGHZ_TXRX_WORKER_MAX_TXRX_SIZE 60

#define SUBGHZ_TXRX_WORKER_TIMEOUT_READ_WRITE_BUF 40

struct SubGhzTxRxWorker {
    FuriThread* thread;
    StreamBufferHandle_t stream_tx;
    StreamBufferHandle_t stream_rx;

    volatile bool worker_running;
    volatile bool worker_stoping;

    SubGhzTxRxWorkerStatus status;

    uint32_t frequency;

    SubGhzTxRxWorkerCallbackHaveRead callback_have_read;
    void* context_have_read;
};

bool subghz_tx_rx_worker_write(SubGhzTxRxWorker* instance, uint8_t* data, size_t size) {
    furi_assert(instance);
    bool ret = false;
    size_t stream_tx_free_byte = xStreamBufferSpacesAvailable(instance->stream_tx);
    if(size && (stream_tx_free_byte >= size)) {
        if(xStreamBufferSend(
               instance->stream_tx, data, size, SUBGHZ_TXRX_WORKER_TIMEOUT_READ_WRITE_BUF) ==
           size) {
            ret = true;
        }
    }
    return ret;
}

size_t subghz_tx_rx_worker_available(SubGhzTxRxWorker* instance) {
    furi_assert(instance);
    return xStreamBufferBytesAvailable(instance->stream_rx);
}

size_t subghz_tx_rx_worker_read(SubGhzTxRxWorker* instance, uint8_t* data, size_t size) {
    furi_assert(instance);
    return xStreamBufferReceive(instance->stream_rx, data, size, 0);
}

void subghz_tx_rx_worker_set_callback_have_read(
    SubGhzTxRxWorker* instance,
    SubGhzTxRxWorkerCallbackHaveRead callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);
    furi_assert(context);
    instance->callback_have_read = callback;
    instance->context_have_read = context;
}

bool subghz_tx_rx_worker_rx(SubGhzTxRxWorker* instance, uint8_t* data, uint8_t* size) {
    uint8_t timeout = 100;
    bool ret = false;
    if(instance->status != SubGhzTxRxWorkerStatusRx) {
        furi_hal_subghz_rx();
        instance->status = SubGhzTxRxWorkerStatusRx;
        furi_delay_tick(1);
    }
    //waiting for reception to complete
    while(furi_hal_gpio_read(&gpio_cc1101_g0)) {
        furi_delay_tick(1);
        if(!--timeout) {
            FURI_LOG_W(TAG, "RX cc1101_g0 timeout");
            furi_hal_subghz_flush_rx();
            furi_hal_subghz_rx();
            break;
        }
    }

    if(furi_hal_subghz_rx_pipe_not_empty()) {
        FURI_LOG_I(
            TAG,
            "RSSI: %03.1fdbm LQI: %d",
            (double)furi_hal_subghz_get_rssi(),
            furi_hal_subghz_get_lqi());
        if(furi_hal_subghz_is_rx_data_crc_valid()) {
            furi_hal_subghz_read_packet(data, size);
            ret = true;
        }
        furi_hal_subghz_flush_rx();
        furi_hal_subghz_rx();
    }
    return ret;
}

void subghz_tx_rx_worker_tx(SubGhzTxRxWorker* instance, uint8_t* data, size_t size) {
    uint8_t timeout = 200;
    if(instance->status != SubGhzTxRxWorkerStatusIDLE) {
        furi_hal_subghz_idle();
    }
    furi_hal_subghz_write_packet(data, size);
    furi_hal_subghz_tx(); //start send
    instance->status = SubGhzTxRxWorkerStatusTx;
    while(!furi_hal_gpio_read(&gpio_cc1101_g0)) { // Wait for GDO0 to be set -> sync transmitted
        furi_delay_tick(1);
        if(!--timeout) {
            FURI_LOG_W(TAG, "TX !cc1101_g0 timeout");
            break;
        }
    }
    while(furi_hal_gpio_read(&gpio_cc1101_g0)) { // Wait for GDO0 to be cleared -> end of packet
        furi_delay_tick(1);
        if(!--timeout) {
            FURI_LOG_W(TAG, "TX cc1101_g0 timeout");
            break;
        }
    }
    furi_hal_subghz_idle();
    instance->status = SubGhzTxRxWorkerStatusIDLE;
}
/** Worker thread
 * 
 * @param context 
 * @return exit code 
 */
static int32_t subghz_tx_rx_worker_thread(void* context) {
    SubGhzTxRxWorker* instance = context;
    FURI_LOG_I(TAG, "Worker start");

    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetGFSK9_99KbAsync);
    //furi_hal_subghz_load_preset(FuriHalSubGhzPresetMSK99_97KbAsync);
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    furi_hal_subghz_set_frequency_and_path(instance->frequency);
    furi_hal_subghz_flush_rx();

    uint8_t data[SUBGHZ_TXRX_WORKER_MAX_TXRX_SIZE + 1] = {0};
    size_t size_tx = 0;
    uint8_t size_rx[1] = {0};
    uint8_t timeout_tx = 0;
    bool callback_rx = false;

    while(instance->worker_running) {
        //transmit
        size_tx = xStreamBufferBytesAvailable(instance->stream_tx);
        if(size_tx > 0 && !timeout_tx) {
            timeout_tx = 10; //20ms
            if(size_tx > SUBGHZ_TXRX_WORKER_MAX_TXRX_SIZE) {
                xStreamBufferReceive(
                    instance->stream_tx,
                    &data,
                    SUBGHZ_TXRX_WORKER_MAX_TXRX_SIZE,
                    SUBGHZ_TXRX_WORKER_TIMEOUT_READ_WRITE_BUF);
                subghz_tx_rx_worker_tx(instance, data, SUBGHZ_TXRX_WORKER_MAX_TXRX_SIZE);
            } else {
                //todo checking that he managed to write all the data to the TX buffer
                xStreamBufferReceive(
                    instance->stream_tx, &data, size_tx, SUBGHZ_TXRX_WORKER_TIMEOUT_READ_WRITE_BUF);
                subghz_tx_rx_worker_tx(instance, data, size_tx);
            }
        } else {
            //recive
            if(subghz_tx_rx_worker_rx(instance, data, size_rx)) {
                if(xStreamBufferSpacesAvailable(instance->stream_rx) >= size_rx[0]) {
                    if(instance->callback_have_read &&
                       xStreamBufferBytesAvailable(instance->stream_rx) == 0) {
                        callback_rx = true;
                    }
                    //todo checking that he managed to write all the data to the RX buffer
                    xStreamBufferSend(
                        instance->stream_rx,
                        &data,
                        size_rx[0],
                        SUBGHZ_TXRX_WORKER_TIMEOUT_READ_WRITE_BUF);
                    if(callback_rx) {
                        instance->callback_have_read(instance->context_have_read);
                        callback_rx = false;
                    }
                } else {
                    //todo RX buffer overflow
                }
            }
        }

        if(timeout_tx) timeout_tx--;
        furi_delay_tick(1);
    }

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();

    FURI_LOG_I(TAG, "Worker stop");
    return 0;
}

SubGhzTxRxWorker* subghz_tx_rx_worker_alloc() {
    SubGhzTxRxWorker* instance = malloc(sizeof(SubGhzTxRxWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "SubGhzTxRxWorker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, subghz_tx_rx_worker_thread);
    instance->stream_tx =
        xStreamBufferCreate(sizeof(uint8_t) * SUBGHZ_TXRX_WORKER_BUF_SIZE, sizeof(uint8_t));
    instance->stream_rx =
        xStreamBufferCreate(sizeof(uint8_t) * SUBGHZ_TXRX_WORKER_BUF_SIZE, sizeof(uint8_t));

    instance->status = SubGhzTxRxWorkerStatusIDLE;
    instance->worker_stoping = true;

    return instance;
}

void subghz_tx_rx_worker_free(SubGhzTxRxWorker* instance) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);
    vStreamBufferDelete(instance->stream_tx);
    vStreamBufferDelete(instance->stream_rx);
    furi_thread_free(instance->thread);

    free(instance);
}

bool subghz_tx_rx_worker_start(SubGhzTxRxWorker* instance, uint32_t frequency) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);
    bool res = false;
    xStreamBufferReset(instance->stream_tx);
    xStreamBufferReset(instance->stream_rx);

    instance->worker_running = true;

    if(furi_hal_region_is_frequency_allowed(frequency)) {
        instance->frequency = frequency;
        res = true;
    }

    furi_thread_start(instance->thread);

    return res;
}

void subghz_tx_rx_worker_stop(SubGhzTxRxWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->worker_running);

    instance->worker_running = false;

    furi_thread_join(instance->thread);
}

bool subghz_tx_rx_worker_is_running(SubGhzTxRxWorker* instance) {
    furi_assert(instance);
    return instance->worker_running;
}
