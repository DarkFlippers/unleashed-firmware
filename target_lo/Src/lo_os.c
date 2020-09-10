#include "cmsis_os.h"
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

void osDelay(uint32_t ms) {
    // printf("[DELAY] %d ms\n", ms);
    usleep(ms * 1000);
}

// temporary struct to pass function ptr and param to wrapper
typedef struct {
    TaskFunction_t func;
    void * param;
} PthreadTask;

void* pthread_wrapper(void* p) {
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0x00);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0x00);

    PthreadTask* task = (PthreadTask*)p;
    
    task->func(task->param);

    return NULL;
}

TaskHandle_t xTaskCreateStatic(
    TaskFunction_t pxTaskCode,
    const char * const pcName,
    const uint32_t ulStackDepth,
    void * const pvParameters,
    UBaseType_t uxPriority,
    StackType_t * const puxStackBuffer,
    StaticTask_t * const pxTaskBuffer
) {
    TaskHandle_t thread = malloc(sizeof(TaskHandle_t));
    PthreadTask* task = malloc(sizeof(PthreadTask));

    task->func = pxTaskCode;
    task->param = pvParameters;

    pthread_create(thread, NULL, pthread_wrapper, (void*)task);

    return thread;
}

void vTaskDelete(TaskHandle_t xTask) {

    if(xTask == NULL) {
        // kill itself
        pthread_exit(NULL);
    }

    // maybe thread already join
    if (pthread_kill(*xTask, 0) == ESRCH) return;

    // send thread_child signal to stop it сигнал, который ее завершает
    pthread_cancel(*xTask);

    // wait for join and close descriptor
    pthread_join(*xTask, 0x00);

    // cleanup thread handler
    *xTask = 0;
}

TaskHandle_t xTaskGetCurrentTaskHandle(void) {
    TaskHandle_t thread = malloc(sizeof(TaskHandle_t));
    *thread = pthread_self();
    return thread;
}

bool task_equal(TaskHandle_t a, TaskHandle_t b) {
    if(a == NULL || b == NULL) return false;
    
    return pthread_equal(*a, *b) != 0;
}

BaseType_t xQueueSend(
    QueueHandle_t xQueue, const void * pvItemToQueue, TickType_t xTicksToWait
) {
    // TODO: add implementation
    return pdTRUE;
}

BaseType_t xQueueReceive(
    QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait
) {
    // TODO: add implementation
    osDelay(100);

    return pdFALSE;
}

static uint32_t queue_global_id = 0;

QueueHandle_t xQueueCreateStatic(
    UBaseType_t uxQueueLength,
    UBaseType_t uxItemSize,
    uint8_t* pucQueueStorageBuffer,
    StaticQueue_t *pxQueueBuffer
) {
    // TODO: check this implementation
    int* msgid = malloc(sizeof(int));

    key_t key = queue_global_id;
    queue_global_id++;

    *msgid = msgget(key, IPC_CREAT);

    return (QueueHandle_t)msgid;
}

SemaphoreHandle_t xSemaphoreCreateCountingStatic(
    UBaseType_t uxMaxCount,
    UBaseType_t uxInitialCount,
    StaticSemaphore_t* pxSemaphoreBuffer
) {
    pxSemaphoreBuffer->type = SemaphoreTypeCounting;
    pxSemaphoreBuffer->take_counter = 0;
    pxSemaphoreBuffer->give_counter = 0;
    return pxSemaphoreBuffer;
}

SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t* pxMutexBuffer) {
    pxMutexBuffer->type = SemaphoreTypeMutex;
    pthread_mutex_init(&pxMutexBuffer->mutex, NULL);
    pxMutexBuffer->take_counter = 0;
    pxMutexBuffer->give_counter = 0;
    return pxMutexBuffer;
}


BaseType_t xSemaphoreTake(volatile SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait) {
    if(xSemaphore == NULL) return pdFALSE;

    if (xSemaphore->type == SemaphoreTypeMutex) {
        if (xTicksToWait == portMAX_DELAY) {
            if (pthread_mutex_lock(&xSemaphore->mutex) == 0) {
                return pdTRUE;
            } else {
                return pdFALSE;
            }
        } else {
            TickType_t ticks = xTicksToWait;
            while (ticks >= 0) {
                if (pthread_mutex_trylock(&xSemaphore->mutex) == 0) {
                    return pdTRUE;
                }
                if (ticks > 0) {
                    osDelay(1);
                }
                ticks--;
            }
            return pdFALSE;
        }
    }

    // TODO: need to add inter-process sync or use POSIX primitives
    xSemaphore->take_counter++;

    TickType_t ticks = xTicksToWait;

    while(
        xSemaphore->take_counter != xSemaphore->give_counter
        && (ticks > 0 || xTicksToWait == portMAX_DELAY)
    ) {
        osDelay(1);
        ticks--;
    }

    if(xTicksToWait != 0 && ticks == 0) return pdFALSE;

    return pdTRUE;
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore) {
    if(xSemaphore == NULL) return pdFALSE;

    if (xSemaphore->type == SemaphoreTypeMutex) {
        if (pthread_mutex_unlock(&xSemaphore->mutex) == 0) {
            return pdTRUE;
        } else {
            return pdFALSE;
        }
    }

    // TODO: need to add inter-process sync or use POSIX primitives
    xSemaphore->give_counter++;

    return pdTRUE;
}

#define TLS_ITEM_COUNT 1
static pthread_key_t tls_keys[TLS_ITEM_COUNT];
static pthread_once_t tls_keys_once = PTHREAD_ONCE_INIT;

static void create_tls_keys() {
    for (size_t i = 0; i < TLS_ITEM_COUNT; i++) {
        pthread_key_create(&tls_keys[i], NULL);
    }
}

void* pvTaskGetThreadLocalStoragePointer(
    TaskHandle_t xTaskToQuery, BaseType_t xIndex
) {
    // Non-current task TLS access is not allowed
    if (xTaskToQuery != NULL) {
        return NULL;
    }

    if (xIndex >= TLS_ITEM_COUNT) {
        return NULL;
    }

    pthread_once(&tls_keys_once, create_tls_keys);

    return pthread_getspecific(tls_keys[xIndex]);
}

void vTaskSetThreadLocalStoragePointer(
    TaskHandle_t xTaskToSet, BaseType_t xIndex, void *pvValue
) {
    // Non-current task TLS access is not allowed
    if (xTaskToSet != NULL) {
        return;
    }

    if (xIndex >= TLS_ITEM_COUNT) {
        return;
    }

    pthread_once(&tls_keys_once, create_tls_keys);

    pthread_setspecific(tls_keys[xIndex], pvValue);
}
