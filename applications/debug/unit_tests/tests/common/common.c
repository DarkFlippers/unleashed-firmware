#include "../test.h"
#include "../minunit_vars.h"

#include <furi.h>

void minunit_print_progress(void) {
    static const char progress[] = {'\\', '|', '/', '-'};
    static uint8_t progress_counter = 0;
    static uint32_t last_tick = 0;
    uint32_t current_tick = furi_get_tick();
    if(current_tick - last_tick > 20) {
        last_tick = current_tick;
        printf("[%c]\033[3D", progress[++progress_counter % COUNT_OF(progress)]);
        fflush(stdout);
    }
}

void minunit_print_fail(const char* str) {
    printf(_FURI_LOG_CLR_E "%s\r\n" _FURI_LOG_CLR_RESET, str);
}

void minunit_printf_warning(const char* format, ...) {
    FuriString* str = furi_string_alloc();
    va_list args;
    va_start(args, format);
    furi_string_vprintf(str, format, args);
    va_end(args);
    printf(_FURI_LOG_CLR_W "%s\r\n" _FURI_LOG_CLR_RESET, furi_string_get_cstr(str));
    furi_string_free(str);
}

int get_minunit_run(void) {
    return minunit_run;
}

int get_minunit_assert(void) {
    return minunit_assert;
}

int get_minunit_status(void) {
    return minunit_status;
}
