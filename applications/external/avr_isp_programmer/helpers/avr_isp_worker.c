#include "avr_isp_worker.h"
#include <furi_hal_pwm.h>
#include "../lib/driver/avr_isp_prog.h"
#include "../lib/driver/avr_isp_prog_cmd.h"
#include "../lib/driver/avr_isp_chip_arr.h"

#include <furi.h>

#define TAG "AvrIspWorker"

typedef enum {
    AvrIspWorkerEvtStop = (1 << 0),

    AvrIspWorkerEvtRx = (1 << 1),
    AvrIspWorkerEvtTxCoplete = (1 << 2),
    AvrIspWorkerEvtTx = (1 << 3),
    AvrIspWorkerEvtState = (1 << 4),

    //AvrIspWorkerEvtCfg = (1 << 5),

} AvrIspWorkerEvt;

struct AvrIspWorker {
    FuriThread* thread;
    volatile bool worker_running;
    uint8_t connect_usb;
    AvrIspWorkerCallback callback;
    void* context;
};

#define AVR_ISP_WORKER_PROG_ALL_EVENTS (AvrIspWorkerEvtStop)
#define AVR_ISP_WORKER_ALL_EVENTS                                                             \
    (AvrIspWorkerEvtTx | AvrIspWorkerEvtTxCoplete | AvrIspWorkerEvtRx | AvrIspWorkerEvtStop | \
     AvrIspWorkerEvtState)

//########################/* VCP CDC */#############################################
#include "usb_cdc.h"
#include <cli/cli_vcp.h>
#include <cli/cli.h>
#include <furi_hal_usb_cdc.h>

#define AVR_ISP_VCP_CDC_CH 1
#define AVR_ISP_VCP_CDC_PKT_LEN CDC_DATA_SZ
#define AVR_ISP_VCP_UART_RX_BUF_SIZE (AVR_ISP_VCP_CDC_PKT_LEN * 5)

static void vcp_on_cdc_tx_complete(void* context);
static void vcp_on_cdc_rx(void* context);
static void vcp_state_callback(void* context, uint8_t state);
static void vcp_on_cdc_control_line(void* context, uint8_t state);
static void vcp_on_line_config(void* context, struct usb_cdc_line_coding* config);

static const CdcCallbacks cdc_cb = {
    vcp_on_cdc_tx_complete,
    vcp_on_cdc_rx,
    vcp_state_callback,
    vcp_on_cdc_control_line,
    vcp_on_line_config,
};

/* VCP callbacks */

static void vcp_on_cdc_tx_complete(void* context) {
    furi_assert(context);
    AvrIspWorker* instance = context;
    furi_thread_flags_set(furi_thread_get_id(instance->thread), AvrIspWorkerEvtTxCoplete);
}

static void vcp_on_cdc_rx(void* context) {
    furi_assert(context);
    AvrIspWorker* instance = context;
    furi_thread_flags_set(furi_thread_get_id(instance->thread), AvrIspWorkerEvtRx);
}

static void vcp_state_callback(void* context, uint8_t state) {
    UNUSED(context);

    AvrIspWorker* instance = context;
    instance->connect_usb = state;
    furi_thread_flags_set(furi_thread_get_id(instance->thread), AvrIspWorkerEvtState);
}

static void vcp_on_cdc_control_line(void* context, uint8_t state) {
    UNUSED(context);
    UNUSED(state);
}

static void vcp_on_line_config(void* context, struct usb_cdc_line_coding* config) {
    UNUSED(context);
    UNUSED(config);
}

static void avr_isp_worker_vcp_cdc_init(void* context) {
    furi_hal_usb_unlock();
    Cli* cli = furi_record_open(RECORD_CLI);
    //close cli
    cli_session_close(cli);
    //disable callbacks VCP_CDC=0
    furi_hal_cdc_set_callbacks(0, NULL, NULL);
    //set 2 cdc
    furi_check(furi_hal_usb_set_config(&usb_cdc_dual, NULL) == true);
    //open cli VCP_CDC=0
    cli_session_open(cli, &cli_vcp);
    furi_record_close(RECORD_CLI);

    furi_hal_cdc_set_callbacks(AVR_ISP_VCP_CDC_CH, (CdcCallbacks*)&cdc_cb, context);
}

static void avr_isp_worker_vcp_cdc_deinit(void) {
    //disable callbacks AVR_ISP_VCP_CDC_CH
    furi_hal_cdc_set_callbacks(AVR_ISP_VCP_CDC_CH, NULL, NULL);

    Cli* cli = furi_record_open(RECORD_CLI);
    //close cli
    cli_session_close(cli);
    furi_hal_usb_unlock();
    //set 1 cdc
    furi_check(furi_hal_usb_set_config(&usb_cdc_single, NULL) == true);
    //open cli VCP_CDC=0
    cli_session_open(cli, &cli_vcp);
    furi_record_close(RECORD_CLI);
}

//#################################################################################

static int32_t avr_isp_worker_prog_thread(void* context) {
    AvrIspProg* prog = context;
    FURI_LOG_D(TAG, "AvrIspProgWorker Start");
    while(1) {
        if(furi_thread_flags_get() & AvrIspWorkerEvtStop) break;
        avr_isp_prog_avrisp(prog);
    }
    FURI_LOG_D(TAG, "AvrIspProgWorker Stop");
    return 0;
}

static void avr_isp_worker_prog_tx_data(void* context) {
    furi_assert(context);
    AvrIspWorker* instance = context;
    furi_thread_flags_set(furi_thread_get_id(instance->thread), AvrIspWorkerEvtTx);
}

/** Worker thread
 * 
 * @param context 
 * @return exit code 
 */
static int32_t avr_isp_worker_thread(void* context) {
    AvrIspWorker* instance = context;
    avr_isp_worker_vcp_cdc_init(instance);

    /* start PWM on &gpio_ext_pa4 */
    furi_hal_pwm_start(FuriHalPwmOutputIdLptim2PA4, 4000000, 50);

    AvrIspProg* prog = avr_isp_prog_init();
    avr_isp_prog_set_tx_callback(prog, avr_isp_worker_prog_tx_data, instance);

    uint8_t buf[AVR_ISP_VCP_UART_RX_BUF_SIZE];
    size_t len = 0;

    FuriThread* prog_thread =
        furi_thread_alloc_ex("AvrIspProgWorker", 1024, avr_isp_worker_prog_thread, prog);
    furi_thread_start(prog_thread);

    FURI_LOG_D(TAG, "Start");

    while(instance->worker_running) {
        uint32_t events =
            furi_thread_flags_wait(AVR_ISP_WORKER_ALL_EVENTS, FuriFlagWaitAny, FuriWaitForever);

        if(events & AvrIspWorkerEvtRx) {
            if(avr_isp_prog_spaces_rx(prog) >= AVR_ISP_VCP_CDC_PKT_LEN) {
                len = furi_hal_cdc_receive(AVR_ISP_VCP_CDC_CH, buf, AVR_ISP_VCP_CDC_PKT_LEN);
                // for(uint8_t i = 0; i < len; i++) {
                //     FURI_LOG_I(TAG, "--> %X", buf[i]);
                // }
                avr_isp_prog_rx(prog, buf, len);
            } else {
                furi_thread_flags_set(furi_thread_get_id(instance->thread), AvrIspWorkerEvtRx);
            }
        }

        if((events & AvrIspWorkerEvtTxCoplete) || (events & AvrIspWorkerEvtTx)) {
            len = avr_isp_prog_tx(prog, buf, AVR_ISP_VCP_CDC_PKT_LEN);

            // for(uint8_t i = 0; i < len; i++) {
            //     FURI_LOG_I(TAG, "<-- %X", buf[i]);
            // }

            if(len > 0) furi_hal_cdc_send(AVR_ISP_VCP_CDC_CH, buf, len);
        }

        if(events & AvrIspWorkerEvtStop) {
            break;
        }

        if(events & AvrIspWorkerEvtState) {
            if(instance->callback)
                instance->callback(instance->context, (bool)instance->connect_usb);
        }
    }

    FURI_LOG_D(TAG, "Stop");

    furi_thread_flags_set(furi_thread_get_id(prog_thread), AvrIspWorkerEvtStop);
    avr_isp_prog_exit(prog);
    furi_delay_ms(10);
    furi_thread_join(prog_thread);
    furi_thread_free(prog_thread);

    avr_isp_prog_free(prog);
    furi_hal_pwm_stop(FuriHalPwmOutputIdLptim2PA4);
    avr_isp_worker_vcp_cdc_deinit();
    return 0;
}

AvrIspWorker* avr_isp_worker_alloc(void* context) {
    furi_assert(context);
    UNUSED(context);
    AvrIspWorker* instance = malloc(sizeof(AvrIspWorker));

    instance->thread = furi_thread_alloc_ex("AvrIspWorker", 2048, avr_isp_worker_thread, instance);
    return instance;
}

void avr_isp_worker_free(AvrIspWorker* instance) {
    furi_assert(instance);

    furi_check(!instance->worker_running);
    furi_thread_free(instance->thread);
    free(instance);
}

void avr_isp_worker_set_callback(
    AvrIspWorker* instance,
    AvrIspWorkerCallback callback,
    void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

void avr_isp_worker_start(AvrIspWorker* instance) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);

    instance->worker_running = true;

    furi_thread_start(instance->thread);
}

void avr_isp_worker_stop(AvrIspWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->worker_running);

    instance->worker_running = false;
    furi_thread_flags_set(furi_thread_get_id(instance->thread), AvrIspWorkerEvtStop);

    furi_thread_join(instance->thread);
}

bool avr_isp_worker_is_running(AvrIspWorker* instance) {
    furi_assert(instance);

    return instance->worker_running;
}
