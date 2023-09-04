#include "ibutton_worker_i.h"

#include <core/check.h>

#include <furi_hal_rfid.h>
#include <furi_hal_power.h>

#include "ibutton_protocols.h"

static void ibutton_worker_mode_idle_start(iButtonWorker* worker);
static void ibutton_worker_mode_idle_tick(iButtonWorker* worker);
static void ibutton_worker_mode_idle_stop(iButtonWorker* worker);

static void ibutton_worker_mode_emulate_start(iButtonWorker* worker);
static void ibutton_worker_mode_emulate_tick(iButtonWorker* worker);
static void ibutton_worker_mode_emulate_stop(iButtonWorker* worker);

static void ibutton_worker_mode_read_start(iButtonWorker* worker);
static void ibutton_worker_mode_read_tick(iButtonWorker* worker);
static void ibutton_worker_mode_read_stop(iButtonWorker* worker);

static void ibutton_worker_mode_write_common_start(iButtonWorker* worker);
static void ibutton_worker_mode_write_blank_tick(iButtonWorker* worker);
static void ibutton_worker_mode_write_copy_tick(iButtonWorker* worker);
static void ibutton_worker_mode_write_common_stop(iButtonWorker* worker);

const iButtonWorkerModeType ibutton_worker_modes[] = {
    {
        .quant = FuriWaitForever,
        .start = ibutton_worker_mode_idle_start,
        .tick = ibutton_worker_mode_idle_tick,
        .stop = ibutton_worker_mode_idle_stop,
    },
    {
        .quant = 100,
        .start = ibutton_worker_mode_read_start,
        .tick = ibutton_worker_mode_read_tick,
        .stop = ibutton_worker_mode_read_stop,
    },
    {
        .quant = 1000,
        .start = ibutton_worker_mode_write_common_start,
        .tick = ibutton_worker_mode_write_blank_tick,
        .stop = ibutton_worker_mode_write_common_stop,
    },
    {
        .quant = 1000,
        .start = ibutton_worker_mode_write_common_start,
        .tick = ibutton_worker_mode_write_copy_tick,
        .stop = ibutton_worker_mode_write_common_stop,
    },
    {
        .quant = 1000,
        .start = ibutton_worker_mode_emulate_start,
        .tick = ibutton_worker_mode_emulate_tick,
        .stop = ibutton_worker_mode_emulate_stop,
    },
};

/*********************** IDLE ***********************/

void ibutton_worker_mode_idle_start(iButtonWorker* worker) {
    UNUSED(worker);
}

void ibutton_worker_mode_idle_tick(iButtonWorker* worker) {
    UNUSED(worker);
}

void ibutton_worker_mode_idle_stop(iButtonWorker* worker) {
    UNUSED(worker);
}

/*********************** READ ***********************/

void ibutton_worker_mode_read_start(iButtonWorker* worker) {
    UNUSED(worker);
    furi_hal_power_enable_otg();
}

void ibutton_worker_mode_read_tick(iButtonWorker* worker) {
    if(ibutton_protocols_read(worker->protocols, worker->key)) {
        if(worker->read_cb != NULL) {
            worker->read_cb(worker->cb_ctx);
        }

        ibutton_worker_switch_mode(worker, iButtonWorkerModeIdle);
    }
}

void ibutton_worker_mode_read_stop(iButtonWorker* worker) {
    UNUSED(worker);
    furi_hal_power_disable_otg();
}

/*********************** EMULATE ***********************/

void ibutton_worker_mode_emulate_start(iButtonWorker* worker) {
    furi_assert(worker->key);

    furi_hal_rfid_pins_reset();
    furi_hal_rfid_pin_pull_pulldown();

    ibutton_protocols_emulate_start(worker->protocols, worker->key);
}

void ibutton_worker_mode_emulate_tick(iButtonWorker* worker) {
    UNUSED(worker);
}

void ibutton_worker_mode_emulate_stop(iButtonWorker* worker) {
    furi_assert(worker->key);

    ibutton_protocols_emulate_stop(worker->protocols, worker->key);

    furi_hal_rfid_pins_reset();
}

/*********************** WRITE ***********************/

void ibutton_worker_mode_write_common_start(iButtonWorker* worker) { //-V524
    UNUSED(worker);
    furi_hal_power_enable_otg();
}

void ibutton_worker_mode_write_blank_tick(iButtonWorker* worker) {
    furi_assert(worker->key);

    const bool success = ibutton_protocols_write_blank(worker->protocols, worker->key);
    // TODO FL-3527: pass a proper result to the callback
    const iButtonWorkerWriteResult result = success ? iButtonWorkerWriteOK :
                                                      iButtonWorkerWriteNoDetect;
    if(worker->write_cb != NULL) {
        worker->write_cb(worker->cb_ctx, result);
    }
}

void ibutton_worker_mode_write_copy_tick(iButtonWorker* worker) {
    furi_assert(worker->key);

    const bool success = ibutton_protocols_write_copy(worker->protocols, worker->key);
    // TODO FL-3527: pass a proper result to the callback
    const iButtonWorkerWriteResult result = success ? iButtonWorkerWriteOK :
                                                      iButtonWorkerWriteNoDetect;
    if(worker->write_cb != NULL) {
        worker->write_cb(worker->cb_ctx, result);
    }
}

void ibutton_worker_mode_write_common_stop(iButtonWorker* worker) { //-V524
    UNUSED(worker);
    furi_hal_power_disable_otg();
}
