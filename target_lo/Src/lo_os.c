#include "cmsis_os.h"
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

void osDelay(uint32_t ms) {
    printf("[DELAY] %d ms\n", ms);
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

SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t* pxMutexBuffer) {
    // TODO add posix mutex init
    return NULL;
}