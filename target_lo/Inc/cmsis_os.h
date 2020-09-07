#pragma once

#include "main.h"
#include <stdbool.h>

void osDelay(uint32_t ms);

// some FreeRTOS types
typedef void(*TaskFunction_t)(void*);
typedef size_t UBaseType_t;
typedef uint32_t StackType_t;
typedef uint32_t StaticTask_t;
typedef pthread_t* TaskHandle_t;


typedef enum {
    SemaphoreTypeMutex,
    SemaphoreTypeCounting,
} SemaphoreType;
typedef struct {
    SemaphoreType type;
    pthread_mutex_t mutex;
    uint8_t take_counter;
    uint8_t give_counter;
} StaticSemaphore_t;
typedef StaticSemaphore_t* SemaphoreHandle_t;

typedef uint32_t StaticQueue_t;
typedef StaticQueue_t* QueueHandle_t;

#define portMAX_DELAY -1

typedef enum {
    pdTRUE = 1,
    pdFALSE = 0
} BaseType_t;

typedef int32_t TickType_t;

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

QueueHandle_t xQueueCreateStatic(
    UBaseType_t uxQueueLength,
    UBaseType_t uxItemSize,
    uint8_t* pucQueueStorageBuffer,
    StaticQueue_t* pxQueueBuffer
);

SemaphoreHandle_t xSemaphoreCreateCountingStatic(
    UBaseType_t uxMaxCount,
    UBaseType_t uxInitialCount,
    StaticSemaphore_t *pxSemaphoreBuffer
);
BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait);
BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore);

BaseType_t xQueueSend(
    QueueHandle_t xQueue, const void * pvItemToQueue, TickType_t xTicksToWait
);

BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait);
