#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RfBandIsolation = 0,
    RfBand1 = 1,
    RfBand2 = 2,
    RfBand3 = 3
} RfBand;

void api_hal_rf_band_set(RfBand band);

#ifdef __cplusplus
}
#endif