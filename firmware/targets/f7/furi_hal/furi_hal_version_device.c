#include <furi_hal_version.h>

bool furi_hal_version_do_i_belong_here() {
    return (furi_hal_version_get_hw_target() == 7) || (furi_hal_version_get_hw_target() == 0);
}

const char* furi_hal_version_get_model_name() {
    return "Flipper Zero";
}

const char* furi_hal_version_get_model_code() {
    return "FZ.1";
}

const char* furi_hal_version_get_fcc_id() {
    return "2A2V6-FZ";
}

const char* furi_hal_version_get_ic_id() {
    return "27624-FZ";
}
