#include "findmy_state.h"
#include <furi_hal.h>
#include <bt/bt_service/bt.h>
#include <storage/storage.h>
#include <toolbox/run_parallel.h>

#define TAG "FindMyStartup"

static int32_t findmy_startup_apply(void* context) {
    UNUSED(context);
    FURI_LOG_D(TAG, "Loading state");

    // Wait for BT init and check core2
    furi_record_open(RECORD_BT);
    furi_record_close(RECORD_BT);
    if(!furi_hal_bt_is_gatt_gap_supported()) return 0;

    FindMyState state;
    if(findmy_state_load(&state)) {
        FURI_LOG_D(TAG, "Activating beacon");
        findmy_state_apply(&state);
    } else {
        FURI_LOG_D(TAG, "Beacon not active, bailing");
    }

    return 0;
}

static void findmy_startup_mount_callback(const void* message, void* context) {
    UNUSED(context);
    const StorageEvent* event = message;

    if(event->type == StorageEventTypeCardMount) {
        run_parallel(findmy_startup_apply, NULL, 2048);
    }
}

void findmy_startup() {
    if(furi_hal_rtc_get_boot_mode() != FuriHalRtcBootModeNormal) return;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    furi_pubsub_subscribe(storage_get_pubsub(storage), findmy_startup_mount_callback, NULL);

    if(storage_sd_status(storage) != FSE_OK) {
        FURI_LOG_D(TAG, "SD Card not ready, skipping startup hook");
    } else {
        findmy_startup_apply(NULL);
    }

    furi_record_close(RECORD_STORAGE);
}
