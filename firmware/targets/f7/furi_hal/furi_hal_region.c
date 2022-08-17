#include <furi_hal_region.h>

bool furi_hal_region_is_provisioned() {
    return true;
}

const char* furi_hal_region_get_name() {
    return "00";
}
