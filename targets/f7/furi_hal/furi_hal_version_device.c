#include <furi_hal_version.h>

bool furi_hal_version_do_i_belong_here(void) {
    return (furi_hal_version_get_hw_target() == 7) || (furi_hal_version_get_hw_target() == 0);
}

const char* furi_hal_version_get_model_name(void) {
    return "Flipper Zero";
}

const char* furi_hal_version_get_model_code(void) {
    return "FZ.1";
}

const char* furi_hal_version_get_fcc_id(void) {
    return "2A2V6-FZ";
}

const char* furi_hal_version_get_ic_id(void) {
    return "27624-FZ";
}

const char* furi_hal_version_get_mic_id(void) {
    return "210-175991";
}

const char* furi_hal_version_get_srrc_id(void) {
    return "2023DJ16420";
}

const char* furi_hal_version_get_ncc_id(void) {
    return "CCAJ23LP34D0T3";
}
