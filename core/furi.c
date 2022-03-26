#include "furi.h"

void furi_init() {
    furi_log_init();
    furi_record_init();
    furi_stdglue_init();
}
