#include <furi.h>
#include <furi_hal_infrared.h>

#define TAG "InfraredTest"

#define CARRIER_FREQ_HZ (38000UL)
#define CARRIER_DUTY    (0.33f)

#define BURST_DURATION_US (600UL)
#define BURST_COUNT       (50UL)

typedef struct {
    bool level;
    uint32_t count;
} InfraredTestApp;

static FuriHalInfraredTxGetDataState
    infrared_test_app_tx_data_callback(void* context, uint32_t* duration, bool* level) {
    furi_assert(context);
    furi_assert(duration);
    furi_assert(level);

    InfraredTestApp* app = context;

    *duration = BURST_DURATION_US;
    *level = app->level;

    app->level = !app->level;
    app->count += 1;

    if(app->count < BURST_COUNT * 2) {
        return FuriHalInfraredTxGetDataStateOk;
    } else {
        return FuriHalInfraredTxGetDataStateLastDone;
    }
}

int32_t infrared_test_app(void* arg) {
    UNUSED(arg);

    InfraredTestApp app = {
        .level = true,
    };

    FURI_LOG_I(TAG, "Starting test signal on PA7");

    furi_hal_infrared_set_tx_output(FuriHalInfraredTxPinExtPA7);
    furi_hal_infrared_async_tx_set_data_isr_callback(infrared_test_app_tx_data_callback, &app);
    furi_hal_infrared_async_tx_start(CARRIER_FREQ_HZ, CARRIER_DUTY);
    furi_hal_infrared_async_tx_wait_termination();
    furi_hal_infrared_set_tx_output(FuriHalInfraredTxPinInternal);

    FURI_LOG_I(TAG, "Test signal end");
    FURI_LOG_I(
        TAG,
        "The measured signal should be %luus +-%.1fus",
        (app.count - 1) * BURST_DURATION_US,
        (double)1000000.0 / CARRIER_FREQ_HZ);

    return 0;
}
