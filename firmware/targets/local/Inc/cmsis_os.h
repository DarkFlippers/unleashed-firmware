#pragma once

#include "main.h"
#include <stdbool.h>
#include <pthread.h>

void osDelay(uint32_t ms);

// some FreeRTOS types
typedef void (*TaskFunction_t)(void*);
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

typedef enum { pdTRUE = 1, pdFALSE = 0 } BaseType_t;

typedef int32_t TickType_t;

#define tskIDLE_PRIORITY 0

TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                               const char* const pcName,
                               const uint32_t ulStackDepth,
                               void* const pvParameters,
                               UBaseType_t uxPriority,
                               StackType_t* const puxStackBuffer,
                               StaticTask_t* const pxTaskBuffer);

void vTaskDelete(TaskHandle_t xTask);
TaskHandle_t xTaskGetCurrentTaskHandle(void);
SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t* pxMutexBuffer);

QueueHandle_t xQueueCreateStatic(UBaseType_t uxQueueLength,
                                 UBaseType_t uxItemSize,
                                 uint8_t* pucQueueStorageBuffer,
                                 StaticQueue_t* pxQueueBuffer);

SemaphoreHandle_t xSemaphoreCreateCountingStatic(UBaseType_t uxMaxCount,
                                                 UBaseType_t uxInitialCount,
                                                 StaticSemaphore_t* pxSemaphoreBuffer);
BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait);
BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore);

BaseType_t xQueueSend(QueueHandle_t xQueue, const void* pvItemToQueue, TickType_t xTicksToWait);

BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait);

void* pvTaskGetThreadLocalStoragePointer(TaskHandle_t xTaskToQuery, BaseType_t xIndex);
void vTaskSetThreadLocalStoragePointer(TaskHandle_t xTaskToSet, BaseType_t xIndex, void *pvValue);

QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize);

typedef struct {
  const char                   *name;   ///< name of the mutex
  uint32_t                 attr_bits;   ///< attribute bits
  void                      *cb_mem;    ///< memory for control block
  uint32_t                   cb_size;   ///< size of provided memory for control block
} osMutexAttr_t;

typedef SemaphoreHandle_t osMutexId_t;

osMutexId_t osMutexNew(const osMutexAttr_t *attr);

/// Status code values returned by CMSIS-RTOS functions.
typedef enum {
  osOK                      =  0,         ///< Operation completed successfully.
  osError                   = -1,         ///< Unspecified RTOS error: run-time error but no other error message fits.
  osErrorTimeout            = -2,         ///< Operation not completed within the timeout period.
  osErrorResource           = -3,         ///< Resource not available.
  osErrorParameter          = -4,         ///< Parameter error.
  osErrorNoMemory           = -5,         ///< System is out of memory: it was impossible to allocate or reserve memory for the operation.
  osErrorISR                = -6,         ///< Not allowed in ISR context: the function cannot be called from interrupt service routines.
  osStatusReserved          = 0x7FFFFFFF  ///< Prevents enum down-size compiler optimization.
} osStatus_t;

osStatus_t osMutexAcquire (osMutexId_t mutex_id, uint32_t timeout);
osStatus_t osMutexRelease (osMutexId_t mutex_id);
osStatus_t osMutexDelete (osMutexId_t mutex_id);

#define osWaitForever portMAX_DELAY

typedef StaticSemaphore_t osSemaphoreDef_t;
typedef SemaphoreHandle_t osSemaphoreId_t;
typedef struct {} osSemaphoreAttr_t;

osSemaphoreId_t osSemaphoreNew(uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr);
osStatus_t osSemaphoreAcquire(osSemaphoreId_t semaphore_id, uint32_t timeout);
osStatus_t osSemaphoreRelease(osSemaphoreId_t semaphore_id);
osStatus_t osSemaphoreDelete(osSemaphoreId_t semaphore_id);
