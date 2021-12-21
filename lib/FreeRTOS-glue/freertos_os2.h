/* --------------------------------------------------------------------------
 * Copyright (c) 2013-2021 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *      Name:    freertos_os2.h
 *      Purpose: CMSIS RTOS2 wrapper for FreeRTOS
 *
 *---------------------------------------------------------------------------*/

#ifndef FREERTOS_OS2_H_
#define FREERTOS_OS2_H_

#include <string.h>
#include <stdint.h>

#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core

#if defined(_RTE_)
#include "RTE_Components.h"             // Component selection
#include CMSIS_device_header

/* Configuration and component setup check */
#if defined(RTE_Compiler_EventRecorder)
  #if !defined(EVR_FREERTOS_DISABLE)
    #define USE_TRACE_EVENT_RECORDER
    /*
      FreeRTOS provides functions and hooks to support execution tracing. This
      functionality is only enabled if configUSE_TRACE_FACILITY == 1.
      Set #define configUSE_TRACE_FACILITY 1 in FreeRTOSConfig.h to enable trace events.
    */
    #if (configUSE_TRACE_FACILITY == 0)
      #error "Definition configUSE_TRACE_FACILITY must equal 1 to enable FreeRTOS trace events."
    #endif
  #endif
#endif

#if defined(RTE_RTOS_FreeRTOS_HEAP_1)
  #define USE_FreeRTOS_HEAP_1
#endif

#if defined(RTE_RTOS_FreeRTOS_HEAP_5)
  #define USE_FreeRTOS_HEAP_5
#endif
#endif /* _RTE_ */

/*
  CMSIS-RTOS2 FreeRTOS image size optimization definitions.

  Note: Definitions configUSE_OS2 can be used to optimize FreeRTOS image size when
        certain functionality is not required when using CMSIS-RTOS2 API.
        In general optimization decisions are left to the tool chain but in cases
        when coding style prevents it to optimize the code following optional
        definitions can be used.
*/

/*
  Option to exclude CMSIS-RTOS2 functions osThreadSuspend and osThreadResume from
  the application image.
*/
#ifndef configUSE_OS2_THREAD_SUSPEND_RESUME
#define configUSE_OS2_THREAD_SUSPEND_RESUME   1
#endif

/*
  Option to exclude CMSIS-RTOS2 function osThreadEnumerate from the application image.
*/
#ifndef configUSE_OS2_THREAD_ENUMERATE
#define configUSE_OS2_THREAD_ENUMERATE        1
#endif

/*
  Option to disable CMSIS-RTOS2 function osEventFlagsSet and osEventFlagsClear
  operation from ISR.
*/
#ifndef configUSE_OS2_EVENTFLAGS_FROM_ISR
#define configUSE_OS2_EVENTFLAGS_FROM_ISR     1
#endif

/*
  Option to exclude CMSIS-RTOS2 Thread Flags API functions from the application image.
*/
#ifndef configUSE_OS2_THREAD_FLAGS
#define configUSE_OS2_THREAD_FLAGS            configUSE_TASK_NOTIFICATIONS
#endif

/*
  Option to exclude CMSIS-RTOS2 Timer API functions from the application image.
*/
#ifndef configUSE_OS2_TIMER
#define configUSE_OS2_TIMER                   configUSE_TIMERS
#endif

/*
  Option to exclude CMSIS-RTOS2 Mutex API functions from the application image.
*/
#ifndef configUSE_OS2_MUTEX
#define configUSE_OS2_MUTEX                   configUSE_MUTEXES
#endif


/*
  CMSIS-RTOS2 FreeRTOS configuration check (FreeRTOSConfig.h).

  Note: CMSIS-RTOS API requires functions included by using following definitions.
        In case if certain API function is not used compiler will optimize it away.
*/
#if (INCLUDE_xSemaphoreGetMutexHolder == 0)
  /*
    CMSIS-RTOS2 function osMutexGetOwner uses FreeRTOS function xSemaphoreGetMutexHolder. In case if
    osMutexGetOwner is not used in the application image, compiler will optimize it away.
    Set #define INCLUDE_xSemaphoreGetMutexHolder 1 to fix this error.
  */
  #error "Definition INCLUDE_xSemaphoreGetMutexHolder must equal 1 to implement Mutex Management API."
#endif
#if (INCLUDE_vTaskDelay == 0)
  /*
    CMSIS-RTOS2 function osDelay uses FreeRTOS function vTaskDelay. In case if
    osDelay is not used in the application image, compiler will optimize it away.
    Set #define INCLUDE_vTaskDelay 1 to fix this error.
  */
  #error "Definition INCLUDE_vTaskDelay must equal 1 to implement Generic Wait Functions API."
#endif
#if (INCLUDE_xTaskDelayUntil == 0)
  /*
    CMSIS-RTOS2 function osDelayUntil uses FreeRTOS function xTaskDelayUntil. In case if
    osDelayUntil is not used in the application image, compiler will optimize it away.
    Set #define INCLUDE_xTaskDelayUntil 1 to fix this error.
  */
  #error "Definition INCLUDE_xTaskDelayUntil must equal 1 to implement Generic Wait Functions API."
#endif
#if (INCLUDE_vTaskDelete == 0)
  /*
    CMSIS-RTOS2 function osThreadTerminate and osThreadExit uses FreeRTOS function
    vTaskDelete. In case if they are not used in the application image, compiler
    will optimize them away.
    Set #define INCLUDE_vTaskDelete 1 to fix this error.
  */
  #error "Definition INCLUDE_vTaskDelete must equal 1 to implement Thread Management API."
#endif
#if (INCLUDE_xTaskGetCurrentTaskHandle == 0)
  /*
    CMSIS-RTOS2 API uses FreeRTOS function xTaskGetCurrentTaskHandle to implement
    functions osThreadGetId, osThreadFlagsClear and osThreadFlagsGet. In case if these
    functions are not used in the application image, compiler will optimize them away.
    Set #define INCLUDE_xTaskGetCurrentTaskHandle 1 to fix this error.
  */
  #error "Definition INCLUDE_xTaskGetCurrentTaskHandle must equal 1 to implement Thread Management API."
#endif
#if (INCLUDE_xTaskGetSchedulerState == 0)
  /*
    CMSIS-RTOS2 API uses FreeRTOS function xTaskGetSchedulerState to implement Kernel
    tick handling and therefore it is vital that xTaskGetSchedulerState is included into
    the application image.
    Set #define INCLUDE_xTaskGetSchedulerState 1 to fix this error.
  */
  #error "Definition INCLUDE_xTaskGetSchedulerState must equal 1 to implement Kernel Information and Control API."
#endif
#if (INCLUDE_uxTaskGetStackHighWaterMark == 0)
  /*
    CMSIS-RTOS2 function osThreadGetStackSpace uses FreeRTOS function uxTaskGetStackHighWaterMark.
    In case if osThreadGetStackSpace is not used in the application image, compiler will
    optimize it away.
    Set #define INCLUDE_uxTaskGetStackHighWaterMark 1 to fix this error.
  */
  #error "Definition INCLUDE_uxTaskGetStackHighWaterMark must equal 1 to implement Thread Management API."
#endif
#if (INCLUDE_uxTaskPriorityGet == 0)
  /*
    CMSIS-RTOS2 function osThreadGetPriority uses FreeRTOS function uxTaskPriorityGet. In case if
    osThreadGetPriority is not used in the application image, compiler will optimize it away.
    Set #define INCLUDE_uxTaskPriorityGet 1 to fix this error.
  */
  #error "Definition INCLUDE_uxTaskPriorityGet must equal 1 to implement Thread Management API."
#endif
#if (INCLUDE_vTaskPrioritySet == 0)
  /*
    CMSIS-RTOS2 function osThreadSetPriority uses FreeRTOS function vTaskPrioritySet. In case if
    osThreadSetPriority is not used in the application image, compiler will optimize it away.
    Set #define INCLUDE_vTaskPrioritySet 1 to fix this error.
  */
  #error "Definition INCLUDE_vTaskPrioritySet must equal 1 to implement Thread Management API."
#endif
#if (INCLUDE_eTaskGetState == 0)
  /*
    CMSIS-RTOS2 API uses FreeRTOS function vTaskDelayUntil to implement functions osThreadGetState
    and osThreadTerminate. In case if these functions are not used in the application image,
    compiler will optimize them away.
    Set #define INCLUDE_eTaskGetState 1 to fix this error.
  */
  #error "Definition INCLUDE_eTaskGetState must equal 1 to implement Thread Management API."
#endif
#if (INCLUDE_vTaskSuspend == 0)
  /*
    CMSIS-RTOS2 API uses FreeRTOS functions vTaskSuspend and vTaskResume to implement
    functions osThreadSuspend and osThreadResume. In case if these functions are not
    used in the application image, compiler will optimize them away.
    Set #define INCLUDE_vTaskSuspend 1 to fix this error.

    Alternatively, if the application does not use osThreadSuspend and
    osThreadResume they can be excluded from the image code by setting:
    #define configUSE_OS2_THREAD_SUSPEND_RESUME 0 (in FreeRTOSConfig.h)
  */
  #if (configUSE_OS2_THREAD_SUSPEND_RESUME == 1)
    #error "Definition INCLUDE_vTaskSuspend must equal 1 to implement Kernel Information and Control API."
  #endif
#endif
#if (INCLUDE_xTimerPendFunctionCall == 0)
  /*
    CMSIS-RTOS2 function osEventFlagsSet and osEventFlagsClear, when called from
    the ISR, call FreeRTOS functions xEventGroupSetBitsFromISR and
    xEventGroupClearBitsFromISR which are only enabled if timers are operational and
    xTimerPendFunctionCall in enabled.
    Set #define INCLUDE_xTimerPendFunctionCall 1 and #define configUSE_TIMERS 1
    to fix this error.

    Alternatively, if the application does not use osEventFlagsSet and osEventFlagsClear
    from the ISR their operation from ISR can be restricted by setting:
    #define configUSE_OS2_EVENTFLAGS_FROM_ISR 0 (in FreeRTOSConfig.h)
  */
  #if (configUSE_OS2_EVENTFLAGS_FROM_ISR == 1)
    #error "Definition INCLUDE_xTimerPendFunctionCall must equal 1 to implement Event Flags API."
  #endif
#endif

#if (configUSE_TIMERS == 0)
  /*
    CMSIS-RTOS2 Timer Management API functions use FreeRTOS timer functions to implement
    timer management. In case if these functions are not used in the application image,
    compiler will optimize them away.
    Set #define configUSE_TIMERS 1 to fix this error.

    Alternatively, if the application does not use timer functions they can be
    excluded from the image code by setting:
    #define configUSE_OS2_TIMER 0 (in FreeRTOSConfig.h)
  */
  #if (configUSE_OS2_TIMER == 1)
    #error "Definition configUSE_TIMERS must equal 1 to implement Timer Management API."
  #endif
#endif

#if (configUSE_MUTEXES == 0)
  /*
    CMSIS-RTOS2 Mutex Management API functions use FreeRTOS mutex functions to implement
    mutex management. In case if these functions are not used in the application image,
    compiler will optimize them away.
    Set #define configUSE_MUTEXES 1 to fix this error.

    Alternatively, if the application does not use mutex functions they can be
    excluded from the image code by setting:
    #define configUSE_OS2_MUTEX 0 (in FreeRTOSConfig.h)
  */
  #if (configUSE_OS2_MUTEX == 1)
    #error "Definition configUSE_MUTEXES must equal 1 to implement Mutex Management API."
  #endif
#endif

#if (configUSE_COUNTING_SEMAPHORES == 0)
  /*
    CMSIS-RTOS2 Memory Pool functions use FreeRTOS function xSemaphoreCreateCounting
    to implement memory pools. In case if these functions are not used in the application image,
    compiler will optimize them away.
    Set #define configUSE_COUNTING_SEMAPHORES 1 to fix this error.
  */
  #error "Definition configUSE_COUNTING_SEMAPHORES must equal 1 to implement Memory Pool API."
#endif
#if (configUSE_TASK_NOTIFICATIONS == 0)
  /*
    CMSIS-RTOS2 Thread Flags API functions use FreeRTOS Task Notification functions to implement
    thread flag management. In case if these functions are not used in the application image,
    compiler will optimize them away.
    Set #define configUSE_TASK_NOTIFICATIONS 1 to fix this error.

    Alternatively, if the application does not use thread flags functions they can be
    excluded from the image code by setting:
    #define configUSE_OS2_THREAD_FLAGS 0 (in FreeRTOSConfig.h)
  */
  #if (configUSE_OS2_THREAD_FLAGS == 1)
    #error "Definition configUSE_TASK_NOTIFICATIONS must equal 1 to implement Thread Flags API."
  #endif
#endif

#if (configUSE_TRACE_FACILITY == 0)
  /*
    CMSIS-RTOS2 function osThreadEnumerate requires FreeRTOS function uxTaskGetSystemState
    which is only enabled if configUSE_TRACE_FACILITY == 1.
    Set #define configUSE_TRACE_FACILITY 1 to fix this error.

    Alternatively, if the application does not use osThreadEnumerate it can be
    excluded from the image code by setting:
    #define configUSE_OS2_THREAD_ENUMERATE 0 (in FreeRTOSConfig.h)
  */
  #if (configUSE_OS2_THREAD_ENUMERATE == 1)
    #error "Definition configUSE_TRACE_FACILITY must equal 1 to implement osThreadEnumerate."
  #endif
#endif

#if (configUSE_16_BIT_TICKS == 1)
  /*
    CMSIS-RTOS2 wrapper for FreeRTOS relies on 32-bit tick timer which is also optimal on
    a 32-bit CPU architectures.
    Set #define configUSE_16_BIT_TICKS 0 to fix this error.
  */
  #error "Definition configUSE_16_BIT_TICKS must be zero to implement CMSIS-RTOS2 API."
#endif

#if (configMAX_PRIORITIES != 56)
  /*
    CMSIS-RTOS2 defines 56 different priorities (see osPriority_t) and portable CMSIS-RTOS2
    implementation should implement the same number of priorities.
    Set #define configMAX_PRIORITIES 56 to fix this error.
  */
  #error "Definition configMAX_PRIORITIES must equal 56 to implement Thread Management API."
#endif
#if (configUSE_PORT_OPTIMISED_TASK_SELECTION != 0)
  /*
    CMSIS-RTOS2 requires handling of 56 different priorities (see osPriority_t) while FreeRTOS port
    optimised selection for Cortex core only handles 32 different priorities.
    Set #define configUSE_PORT_OPTIMISED_TASK_SELECTION 0 to fix this error.
  */
  #error "Definition configUSE_PORT_OPTIMISED_TASK_SELECTION must be zero to implement Thread Management API."
#endif

#endif /* FREERTOS_OS2_H_ */
