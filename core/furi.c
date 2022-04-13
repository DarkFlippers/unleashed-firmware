#include "furi.h"

void furi_init() {
    osKernelInitialize();

    furi_log_init();
    furi_record_init();
    furi_stdglue_init();
}

void furi_run() {
    osKernelStart();
}
