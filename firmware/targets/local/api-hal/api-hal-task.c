#include "api-hal-task.h"

bool task_equal(TaskHandle_t a, TaskHandle_t b) {
    if(a == NULL || b == NULL) return false;
    return pthread_equal(*a, *b) != 0;
}

bool task_is_isr_context(void) {
    return false;
}

void taskDISABLE_INTERRUPTS(void){
    // we cant disable main os sheduler
};
