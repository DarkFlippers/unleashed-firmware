#include "flipper.h"

/*
render_loader

handle_input

handle_menu
*/

void app_loader(void* p) {
    osThreadId_t self_id = osThreadGetId();
    assert(self_id);

    printf("[app loader] start\n");

    osThreadSuspend(self_id);
}