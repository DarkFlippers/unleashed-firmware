#include <cmsis_os2.h>
#include <FreeRTOS.h>
#include <task.h>
#include <main.h>

void systemd(void *argument);

osThreadId_t systemdHandle;
const osThreadAttr_t systemd_attributes = {
    .name = "systemd",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 1024
};

void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName) {
    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
    called if a stack overflow is detected. */
    asm("bkpt 1");
    while(1);
}

void MX_FREERTOS_Init(void) {
    systemdHandle = osThreadNew(systemd, NULL, &systemd_attributes);
}
