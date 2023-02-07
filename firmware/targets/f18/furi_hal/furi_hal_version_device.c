#include <furi_hal_version.h>

bool furi_hal_version_do_i_belong_here() {
    return (furi_hal_version_get_hw_target() == 18) || (furi_hal_version_get_hw_target() == 0);
}

const char* furi_hal_version_get_model_name() {
    return "Komi";
}

const char* furi_hal_version_get_model_code() {
    return "N/A";
}

const char* furi_hal_version_get_fcc_id() {
    return "N/A";
}

const char* furi_hal_version_get_ic_id() {
    return "N/A";
}
