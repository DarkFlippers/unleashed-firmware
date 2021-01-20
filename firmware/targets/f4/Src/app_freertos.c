#include <cmsis_os2.h>
#include <FreeRTOS.h>
#include <task.h>
#include <main.h>

osThreadId_t systemdHandle;
const osThreadAttr_t systemd_attributes = {
    .name = "systemd",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 1024 * 4
};

void systemd(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName);

__weak void configureTimerForRunTimeStats(void) {
}

__weak unsigned long getRunTimeCounterValue(void) {
    return 0;
}

__weak void vApplicationIdleHook( void ) {
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
    task. It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()). If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
}

__weak void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName) {
    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
    called if a stack overflow is detected. */
}


void MX_FREERTOS_Init(void) {
    systemdHandle = osThreadNew(systemd, NULL, &systemd_attributes);
}
