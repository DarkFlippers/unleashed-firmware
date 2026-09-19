#include "ibutton_worker_i.h"

#include <core/check.h>
#include <core/kernel.h>
#include <core/record.h>

#include <furi_hal_rfid.h>
#include <power/power_service/power.h>

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
static void ibutton_worker_mode_write_id_tick(iButtonWorker* worker);
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
        .tick = ibutton_worker_mode_write_id_tick,
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
    Power* power = furi_record_open(RECORD_POWER);
    power_enable_otg(power, true);
    furi_record_close(RECORD_POWER);
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
    Power* power = furi_record_open(RECORD_POWER);
    power_enable_otg(power, false);
    furi_record_close(RECORD_POWER);
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

// One spelling for the four places a write reports, and the only place that reads the
// callback pointer.
static void ibutton_worker_write_report(iButtonWorker* worker, iButtonWorkerWriteResult result) {
    if(worker->write_cb) worker->write_cb(worker->cb_ctx, result);
}

void ibutton_worker_mode_write_common_start(iButtonWorker* worker) {
    // Not carried over from the previous write, which was a different blank.
    worker->write_target = iButtonWriteTargetMax;

    Power* power = furi_record_open(RECORD_POWER);
    power_enable_otg(power, true);
    furi_record_close(RECORD_POWER);
}

// Record the blank type now being attempted and notify the UI. Runs on the worker thread,
// outside the critical section the write itself uses.
static void ibutton_worker_write_set_target(iButtonWriteTarget target, void* context) {
    iButtonWorker* worker = context;

    worker->write_target = target;
    if(!worker->write_cb) return;

    ibutton_worker_write_report(worker, iButtonWorkerWriteStartTarget);
    // The next attempt masks the scheduler for most of a second, so the app and GUI threads
    // have to be scheduled and the frame drawn before it starts, or the screen stays on the
    // previous blank type for the whole attempt. Empirical.
    furi_delay_ms(50);
}

void ibutton_worker_mode_write_id_tick(iButtonWorker* worker) {
    furi_assert(worker->key);

    // Nothing enabled that can carry this key: the loop below would simply do nothing,
    // which on screen is indistinguishable from no blank on the reader.
    const iButtonWriteTargetMask supported =
        ibutton_protocols_get_write_targets(worker->protocols, worker->key);
    if((supported & worker->write_target_mask) == 0) {
        // Terminal, unlike every other result here: nothing can change while this screen is
        // up, so reporting it once per tick would rebuild the screen once a second forever.
        ibutton_worker_write_report(worker, iButtonWorkerWriteNoEnabledTarget);
        ibutton_worker_switch_mode(worker, iButtonWorkerModeIdle);
        return;
    }

    const iButtonWriteTargetContext write_ctx = {
        .mask = worker->write_target_mask,
        .target_cb = ibutton_worker_write_set_target,
        .context = worker,
    };

    const bool success =
        ibutton_protocols_write_id_targets(worker->protocols, worker->key, &write_ctx);
    // TODO FL-3527: pass a proper result to the callback
    ibutton_worker_write_report(
        worker, success ? iButtonWorkerWriteOK : iButtonWorkerWriteNoDetect);
}

void ibutton_worker_mode_write_copy_tick(iButtonWorker* worker) {
    furi_assert(worker->key);

    const bool success = ibutton_protocols_write_copy(worker->protocols, worker->key);
    // TODO FL-3527: pass a proper result to the callback
    ibutton_worker_write_report(
        worker, success ? iButtonWorkerWriteOK : iButtonWorkerWriteNoDetect);
}

void ibutton_worker_mode_write_common_stop(iButtonWorker* worker) { //-V524
    UNUSED(worker);
    Power* power = furi_record_open(RECORD_POWER);
    power_enable_otg(power, false);
    furi_record_close(RECORD_POWER);
}
