#include "main.h"
#include <stdbool.h>

void osDelay(uint32_t ms);

// some FreeRTOS types
typedef void(*TaskFunction_t)(void*);
typedef uint32_t UBaseType_t;
typedef uint32_t StackType_t;
typedef uint32_t StaticTask_t;
typedef pthread_t* TaskHandle_t;
typedef uint32_t StaticSemaphore_t;
typedef void* SemaphoreHandle_t;

#define tskIDLE_PRIORITY 0

TaskHandle_t xTaskCreateStatic(
    TaskFunction_t pxTaskCode,
    const char * const pcName,
    const uint32_t ulStackDepth,
    void * const pvParameters,
    UBaseType_t uxPriority,
    StackType_t * const puxStackBuffer,
    StaticTask_t * const pxTaskBuffer
);

void vTaskDelete(TaskHandle_t xTask);
TaskHandle_t xTaskGetCurrentTaskHandle(void);
SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t* pxMutexBuffer);
bool task_equal(TaskHandle_t a, TaskHandle_t b);
