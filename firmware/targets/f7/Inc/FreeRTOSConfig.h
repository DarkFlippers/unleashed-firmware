#pragma once

#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
#include <stdint.h>
#pragma GCC diagnostic ignored "-Wredundant-decls"
extern uint32_t SystemCoreClock;
#endif

#ifndef CMSIS_device_header
#define CMSIS_device_header "stm32wbxx.h"
#endif /* CMSIS_device_header */

#define configENABLE_FPU 1
#define configENABLE_MPU 0

#define configUSE_PREEMPTION 1
#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configCPU_CLOCK_HZ (SystemCoreClock)
#define configTICK_RATE_HZ_RAW 1000
#define configTICK_RATE_HZ ((TickType_t)configTICK_RATE_HZ_RAW)
#define configMAX_PRIORITIES (32)
#define configMINIMAL_STACK_SIZE ((uint16_t)128)

/* Heap size determined automatically by linker */
// #define configTOTAL_HEAP_SIZE                    ((size_t)0)
#define configMAX_TASK_NAME_LEN (16)
#define configGENERATE_RUN_TIME_STATS 0
#define configUSE_TRACE_FACILITY 1
#define configUSE_16_BIT_TICKS 0
#define configUSE_MUTEXES 1
#define configQUEUE_REGISTRY_SIZE 0
#define configCHECK_FOR_STACK_OVERFLOW 0
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1
#define configENABLE_BACKWARD_COMPATIBILITY 0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_TICKLESS_IDLE 2
#define configRECORD_STACK_HIGH_ADDRESS 1
#define configUSE_NEWLIB_REENTRANT 0

/* Defaults to size_t for backward compatibility, but can be changed
   if lengths will always be less than the number of bytes in a size_t. */
#define configMESSAGE_BUFFER_LENGTH_TYPE size_t
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 1
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP 4

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 0

/* Software timer definitions. */
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY (2)
#define configTIMER_QUEUE_LENGTH 32
#define configTIMER_TASK_STACK_DEPTH 256
#define configTIMER_SERVICE_TASK_NAME "TimersSrv"

#define configIDLE_TASK_NAME "(-_-)"

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_xTaskGetHandle 1
#define INCLUDE_eTaskGetState 1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskCleanUpResources 0
#define INCLUDE_vTaskDelay 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_xQueueGetMutexHolder 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xTimerPendFunctionCall 1

/* Furi-specific */
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 2

extern __attribute__((__noreturn__)) void furi_thread_catch();
#define configTASK_RETURN_ADDRESS (furi_thread_catch + 2)

/*
 * The CMSIS-RTOS V2 FreeRTOS wrapper is dependent on the heap implementation used
 * by the application thus the correct define need to be enabled below
 */
#define USE_FreeRTOS_HEAP_4

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 4
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
#ifdef DEBUG
#include <core/check.h>
#define configASSERT(x)                \
    if((x) == 0) {                     \
        furi_crash("FreeRTOS Assert"); \
    }
#endif

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler

#define USE_CUSTOM_SYSTICK_HANDLER_IMPLEMENTATION 1
#define configOVERRIDE_DEFAULT_TICK_CONFIGURATION \
    1 /* required only for Keil but does not hurt otherwise */

#define traceTASK_SWITCHED_IN()                                     \
    extern void furi_hal_mpu_set_stack_protection(uint32_t* stack); \
    furi_hal_mpu_set_stack_protection((uint32_t*)pxCurrentTCB->pxStack)
