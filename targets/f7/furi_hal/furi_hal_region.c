#include <furi_hal_region.h>
#include <furi.h>

const FuriHalRegion furi_hal_region_zero = {
    .country_code = "00",
    .bands_count = 1,
    .bands = {
        {
            .start = 0,
            .end = 1000000000,
            .power_limit = 12,
            .duty_cycle = 50,
        },
    }};

static const FuriHalRegion* furi_hal_region = NULL;

const FuriHalRegion* furi_hal_region_get() {
    return &furi_hal_region_zero;
}

void furi_hal_region_set(FuriHalRegion* region) {
    UNUSED(region);
}

const FuriHalRegionBand* furi_hal_region_get_band(uint32_t frequency) {
    furi_hal_region = &furi_hal_region_zero;
    if(!furi_hal_region) {
        return NULL;
    }

    for(size_t i = 0; i < furi_hal_region->bands_count; i++) {
        if(furi_hal_region->bands[i].start <= frequency &&
           furi_hal_region->bands[i].end >= frequency) {
            return &furi_hal_region->bands[i];
        }
    }

    return NULL;
}

bool furi_hal_region_is_frequency_allowed(uint32_t frequency) {
    UNUSED(frequency);
    return true;
}

bool furi_hal_region_is_provisioned() {
    return true;
}

const char* furi_hal_region_get_name() {
    return "00";
}
