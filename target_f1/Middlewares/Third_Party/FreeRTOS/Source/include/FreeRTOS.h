/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#ifndef INC_FREERTOS_H
#define INC_FREERTOS_H

/*
 * Include the generic headers required for the FreeRTOS port being used.
 */
#include <stddef.h>

/*
 * If stdint.h cannot be located then:
 *   + If using GCC ensure the -nostdint options is *not* being used.
 *   + Ensure the project's include path includes the directory in which your
 *     compiler stores stdint.h.
 *   + Set any compiler options necessary for it to support C99, as technically
 *     stdint.h is only mandatory with C99 (FreeRTOS does not require C99 in any
 *     other way).
 *   + The FreeRTOS download includes a simple stdint.h definition that can be
 *     used in cases where none is provided by the compiler.  The files only
 *     contains the typedefs required to build FreeRTOS.  Read the instructions
 *     in FreeRTOS/source/stdint.readme for more information.
 */
#include <stdint.h> /* READ COMMENT ABOVE. */

#ifdef __cplusplus
extern "C" {
#endif

/* Application specific configuration options. */
#include "FreeRTOSConfig.h"

/* Basic FreeRTOS definitions. */
#include "projdefs.h"

/* Definitions specific to the port being used. */
#include "portable.h"

/* Must be defaulted before configUSE_NEWLIB_REENTRANT is used below. */
#ifndef configUSE_NEWLIB_REENTRANT
	#define configUSE_NEWLIB_REENTRANT 0
#endif

/* Required if struct _reent is used. */
#if ( configUSE_NEWLIB_REENTRANT == 1 )
	#include <reent.h>
#endif
/*
 * Check all the required application specific macros have been defined.
 * These macros are application specific and (as downloaded) are defined
 * within FreeRTOSConfig.h.
 */

#ifndef configMINIMAL_STACK_SIZE
	#error Missing definition:  configMINIMAL_STACK_SIZE must be defined in FreeRTOSConfig.h.  configMINIMAL_STACK_SIZE defines the size (in words) of the stack allocated to the idle task.  Refer to the demo project provided for your port for a suitable value.
#endif

#ifndef configMAX_PRIORITIES
	#error Missing definition:  configMAX_PRIORITIES must be defined in FreeRTOSConfig.h.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#if configMAX_PRIORITIES < 1
	#error configMAX_PRIORITIES must be defined to be greater than or equal to 1.
#endif

#ifndef configUSE_PREEMPTION
	#error Missing definition:  configUSE_PREEMPTION must be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef configUSE_IDLE_HOOK
	#error Missing definition:  configUSE_IDLE_HOOK must be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef configUSE_TICK_HOOK
	#error Missing definition:  configUSE_TICK_HOOK must be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef configUSE_16_BIT_TICKS
	#error Missing definition:  configUSE_16_BIT_TICKS must be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef configUSE_CO_ROUTINES
	#define configUSE_CO_ROUTINES 0
#endif

#ifndef INCLUDE_vTaskPrioritySet
	#define INCLUDE_vTaskPrioritySet 0
#endif

#ifndef INCLUDE_uxTaskPriorityGet
	#define INCLUDE_uxTaskPriorityGet 0
#endif

#ifndef INCLUDE_vTaskDelete
	#define INCLUDE_vTaskDelete 0
#endif

#ifndef INCLUDE_vTaskSuspend
	#define INCLUDE_vTaskSuspend 0
#endif

#ifndef INCLUDE_vTaskDelayUntil
	#define INCLUDE_vTaskDelayUntil 0
#endif

#ifndef INCLUDE_vTaskDelay
	#define INCLUDE_vTaskDelay 0
#endif

#ifndef INCLUDE_xTaskGetIdleTaskHandle
	#define INCLUDE_xTaskGetIdleTaskHandle 0
#endif

#ifndef INCLUDE_xTaskAbortDelay
	#define INCLUDE_xTaskAbortDelay 0
#endif

#ifndef INCLUDE_xQueueGetMutexHolder
	#define INCLUDE_xQueueGetMutexHolder 0
#endif

#ifndef INCLUDE_xSemaphoreGetMutexHolder
	#define INCLUDE_xSemaphoreGetMutexHolder INCLUDE_xQueueGetMutexHolder
#endif

#ifndef INCLUDE_xTaskGetHandle
	#define INCLUDE_xTaskGetHandle 0
#endif

#ifndef INCLUDE_uxTaskGetStackHighWaterMark
	#define INCLUDE_uxTaskGetStackHighWaterMark 0
#endif

#ifndef INCLUDE_eTaskGetState
	#define INCLUDE_eTaskGetState 0
#endif

#ifndef INCLUDE_xTaskResumeFromISR
	#define INCLUDE_xTaskResumeFromISR 1
#endif

#ifndef INCLUDE_xTimerPendFunctionCall
	#define INCLUDE_xTimerPendFunctionCall 0
#endif

#ifndef INCLUDE_xTaskGetSchedulerState
	#define INCLUDE_xTaskGetSchedulerState 0
#endif

#ifndef INCLUDE_xTaskGetCurrentTaskHandle
	#define INCLUDE_xTaskGetCurrentTaskHandle 0
#endif

#if configUSE_CO_ROUTINES != 0
	#ifndef configMAX_CO_ROUTINE_PRIORITIES
		#error configMAX_CO_ROUTINE_PRIORITIES must be greater than or equal to 1.
	#endif
#endif

#ifndef configUSE_DAEMON_TASK_STARTUP_HOOK
	#define configUSE_DAEMON_TASK_STARTUP_HOOK 0
#endif

#ifndef configUSE_APPLICATION_TASK_TAG
	#define configUSE_APPLICATION_TASK_TAG 0
#endif

#ifndef configNUM_THREAD_LOCAL_STORAGE_POINTERS
	#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0
#endif

#ifndef configUSE_RECURSIVE_MUTEXES
	#define configUSE_RECURSIVE_MUTEXES 0
#endif

#ifndef configUSE_MUTEXES
	#define configUSE_MUTEXES 0
#endif

#ifndef configUSE_TIMERS
	#define configUSE_TIMERS 0
#endif

#ifndef configUSE_COUNTING_SEMAPHORES
	#define configUSE_COUNTING_SEMAPHORES 0
#endif

#ifndef configUSE_ALTERNATIVE_API
	#define configUSE_ALTERNATIVE_API 0
#endif

#ifndef portCRITICAL_NESTING_IN_TCB
	#define portCRITICAL_NESTING_IN_TCB 0
#endif

#ifndef configMAX_TASK_NAME_LEN
	#define configMAX_TASK_NAME_LEN 16
#endif

#ifndef configIDLE_SHOULD_YIELD
	#define configIDLE_SHOULD_YIELD		1
#endif

#if configMAX_TASK_NAME_LEN < 1
	#error configMAX_TASK_NAME_LEN must be set to a minimum of 1 in FreeRTOSConfig.h
#endif

#ifndef configASSERT
	#define configASSERT( x )
	#define configASSERT_DEFINED 0
#else
	#define configASSERT_DEFINED 1
#endif

/* The timers module relies on xTaskGetSchedulerState(). */
#if configUSE_TIMERS == 1

	#ifndef configTIMER_TASK_PRIORITY
		#error If configUSE_TIMERS is set to 1 then configTIMER_TASK_PRIORITY must also be defined.
	#endif /* configTIMER_TASK_PRIORITY */

	#ifndef configTIMER_QUEUE_LENGTH
		#error If configUSE_TIMERS is set to 1 then configTIMER_QUEUE_LENGTH must also be defined.
	#endif /* configTIMER_QUEUE_LENGTH */

	#ifndef configTIMER_TASK_STACK_DEPTH
		#error If configUSE_TIMERS is set to 1 then configTIMER_TASK_STACK_DEPTH must also be defined.
	#endif /* configTIMER_TASK_STACK_DEPTH */

#endif /* configUSE_TIMERS */

#ifndef portSET_INTERRUPT_MASK_FROM_ISR
	#define portSET_INTERRUPT_MASK_FROM_ISR() 0
#endif

#ifndef portCLEAR_INTERRUPT_MASK_FROM_ISR
	#define portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedStatusValue ) ( void ) uxSavedStatusValue
#endif

#ifndef portCLEAN_UP_TCB
	#define portCLEAN_UP_TCB( pxTCB ) ( void ) pxTCB
#endif

#ifndef portPRE_TASK_DELETE_HOOK
	#define portPRE_TASK_DELETE_HOOK( pvTaskToDelete, pxYieldPending )
#endif

#ifndef portSETUP_TCB
	#define portSETUP_TCB( pxTCB ) ( void ) pxTCB
#endif

#ifndef configQUEUE_REGISTRY_SIZE
	#define configQUEUE_REGISTRY_SIZE 0U
#endif

#if ( configQUEUE_REGISTRY_SIZE < 1 )
	#define vQueueAddToRegistry( xQueue, pcName )
	#define vQueueUnregisterQueue( xQueue )
	#define pcQueueGetName( xQueue )
#endif

#ifndef portPOINTER_SIZE_TYPE
	#define portPOINTER_SIZE_TYPE uint32_t
#endif

/* Remove any unused trace macros. */
#ifndef traceSTART
	/* Used to perform any necessary initialisation - for example, open a file
	into which trace is to be written. */
	#define traceSTART()
#endif

#ifndef traceEND
	/* Use to close a trace, for example close a file into which trace has been
	written. */
	#define traceEND()
#endif

#ifndef traceTASK_SWITCHED_IN
	/* Called after a task has been selected to run.  pxCurrentTCB holds a pointer
	to the task control block of the selected task. */
	#define traceTASK_SWITCHED_IN()
#endif

#ifndef traceINCREASE_TICK_COUNT
	/* Called before stepping the tick count after waking from tickless idle
	sleep. */
	#define traceINCREASE_TICK_COUNT( x )
#endif

#ifndef traceLOW_POWER_IDLE_BEGIN
	/* Called immediately before entering tickless idle. */
	#define traceLOW_POWER_IDLE_BEGIN()
#endif

#ifndef	traceLOW_POWER_IDLE_END
	/* Called when returning to the Idle task after a tickless idle. */
	#define traceLOW_POWER_IDLE_END()
#endif

#ifndef traceTASK_SWITCHED_OUT
	/* Called before a task has been selected to run.  pxCurrentTCB holds a pointer
	to the task control block of the task being switched out. */
	#define traceTASK_SWITCHED_OUT()
#endif

#ifndef traceTASK_PRIORITY_INHERIT
	/* Called when a task attempts to take a mutex that is already held by a
	lower priority task.  pxTCBOfMutexHolder is a pointer to the TCB of the task
	that holds the mutex.  uxInheritedPriority is the priority the mutex holder
	will inherit (the priority of the task that is attempting to obtain the
	muted. */
	#define traceTASK_PRIORITY_INHERIT( pxTCBOfMutexHolder, uxInheritedPriority )
#endif

#ifndef traceTASK_PRIORITY_DISINHERIT
	/* Called when a task releases a mutex, the holding of which had resulted in
	the task inheriting the priority of a higher priority task.
	pxTCBOfMutexHolder is a pointer to the TCB of the task that is releasing the
	mutex.  uxOriginalPriority is the task's configured (base) priority. */
	#define traceTASK_PRIORITY_DISINHERIT( pxTCBOfMutexHolder, uxOriginalPriority )
#endif

#ifndef traceBLOCKING_ON_QUEUE_RECEIVE
	/* Task is about to block because it cannot read from a
	queue/mutex/semaphore.  pxQueue is a pointer to the queue/mutex/semaphore
	upon which the read was attempted.  pxCurrentTCB points to the TCB of the
	task that attempted the read. */
	#define traceBLOCKING_ON_QUEUE_RECEIVE( pxQueue )
#endif

#ifndef traceBLOCKING_ON_QUEUE_PEEK
	/* Task is about to block because it cannot read from a
	queue/mutex/semaphore.  pxQueue is a pointer to the queue/mutex/semaphore
	upon which the read was attempted.  pxCurrentTCB points to the TCB of the
	task that attempted the read. */
	#define traceBLOCKING_ON_QUEUE_PEEK( pxQueue )
#endif

#ifndef traceBLOCKING_ON_QUEUE_SEND
	/* Task is about to block because it cannot write to a
	queue/mutex/semaphore.  pxQueue is a pointer to the queue/mutex/semaphore
	upon which the write was attempted.  pxCurrentTCB points to the TCB of the
	task that attempted the write. */
	#define traceBLOCKING_ON_QUEUE_SEND( pxQueue )
#endif

#ifndef configCHECK_FOR_STACK_OVERFLOW
	#define configCHECK_FOR_STACK_OVERFLOW 0
#endif

#ifndef configRECORD_STACK_HIGH_ADDRESS
	#define configRECORD_STACK_HIGH_ADDRESS 0
#endif

#ifndef configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H
	#define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H 0
#endif

/* The following event macros are embedded in the kernel API calls. */

#ifndef traceMOVED_TASK_TO_READY_STATE
	#define traceMOVED_TASK_TO_READY_STATE( pxTCB )
#endif

#ifndef tracePOST_MOVED_TASK_TO_READY_STATE
	#define tracePOST_MOVED_TASK_TO_READY_STATE( pxTCB )
#endif

#ifndef traceQUEUE_CREATE
	#define traceQUEUE_CREATE( pxNewQueue )
#endif

#ifndef traceQUEUE_CREATE_FAILED
	#define traceQUEUE_CREATE_FAILED( ucQueueType )
#endif

#ifndef traceCREATE_MUTEX
	#define traceCREATE_MUTEX( pxNewQueue )
#endif

#ifndef traceCREATE_MUTEX_FAILED
	#define traceCREATE_MUTEX_FAILED()
#endif

#ifndef traceGIVE_MUTEX_RECURSIVE
	#define traceGIVE_MUTEX_RECURSIVE( pxMutex )
#endif

#ifndef traceGIVE_MUTEX_RECURSIVE_FAILED
	#define traceGIVE_MUTEX_RECURSIVE_FAILED( pxMutex )
#endif

#ifndef traceTAKE_MUTEX_RECURSIVE
	#define traceTAKE_MUTEX_RECURSIVE( pxMutex )
#endif

#ifndef traceTAKE_MUTEX_RECURSIVE_FAILED
	#define traceTAKE_MUTEX_RECURSIVE_FAILED( pxMutex )
#endif

#ifndef traceCREATE_COUNTING_SEMAPHORE
	#define traceCREATE_COUNTING_SEMAPHORE()
#endif

#ifndef traceCREATE_COUNTING_SEMAPHORE_FAILED
	#define traceCREATE_COUNTING_SEMAPHORE_FAILED()
#endif

#ifndef traceQUEUE_SEND
	#define traceQUEUE_SEND( pxQueue )
#endif

#ifndef traceQUEUE_SEND_FAILED
	#define traceQUEUE_SEND_FAILED( pxQueue )
#endif

#ifndef traceQUEUE_RECEIVE
	#define traceQUEUE_RECEIVE( pxQueue )
#endif

#ifndef traceQUEUE_PEEK
	#define traceQUEUE_PEEK( pxQueue )
#endif

#ifndef traceQUEUE_PEEK_FAILED
	#define traceQUEUE_PEEK_FAILED( pxQueue )
#endif

#ifndef traceQUEUE_PEEK_FROM_ISR
	#define traceQUEUE_PEEK_FROM_ISR( pxQueue )
#endif

#ifndef traceQUEUE_RECEIVE_FAILED
	#define traceQUEUE_RECEIVE_FAILED( pxQueue )
#endif

#ifndef traceQUEUE_SEND_FROM_ISR
	#define traceQUEUE_SEND_FROM_ISR( pxQueue )
#endif

#ifndef traceQUEUE_SEND_FROM_ISR_FAILED
	#define traceQUEUE_SEND_FROM_ISR_FAILED( pxQueue )
#endif

#ifndef traceQUEUE_RECEIVE_FROM_ISR
	#define traceQUEUE_RECEIVE_FROM_ISR( pxQueue )
#endif

#ifndef traceQUEUE_RECEIVE_FROM_ISR_FAILED
	#define traceQUEUE_RECEIVE_FROM_ISR_FAILED( pxQueue )
#endif

#ifndef traceQUEUE_PEEK_FROM_ISR_FAILED
	#define traceQUEUE_PEEK_FROM_ISR_FAILED( pxQueue )
#endif

#ifndef traceQUEUE_DELETE
	#define traceQUEUE_DELETE( pxQueue )
#endif

#ifndef traceTASK_CREATE
	#define traceTASK_CREATE( pxNewTCB )
#endif

#ifndef traceTASK_CREATE_FAILED
	#define traceTASK_CREATE_FAILED()
#endif

#ifndef traceTASK_DELETE
	#define traceTASK_DELETE( pxTaskToDelete )
#endif

#ifndef traceTASK_DELAY_UNTIL
	#define traceTASK_DELAY_UNTIL( x )
#endif

#ifndef traceTASK_DELAY
	#define traceTASK_DELAY()
#endif

#ifndef traceTASK_PRIORITY_SET
	#define traceTASK_PRIORITY_SET( pxTask, uxNewPriority )
#endif

#ifndef traceTASK_SUSPEND
	#define traceTASK_SUSPEND( pxTaskToSuspend )
#endif

#ifndef traceTASK_RESUME
	#define traceTASK_RESUME( pxTaskToResume )
#endif

#ifndef traceTASK_RESUME_FROM_ISR
	#define traceTASK_RESUME_FROM_ISR( pxTaskToResume )
#endif

#ifndef traceTASK_INCREMENT_TICK
	#define traceTASK_INCREMENT_TICK( xTickCount )
#endif

#ifndef traceTIMER_CREATE
	#define traceTIMER_CREATE( pxNewTimer )
#endif

#ifndef traceTIMER_CREATE_FAILED
	#define traceTIMER_CREATE_FAILED()
#endif

#ifndef traceTIMER_COMMAND_SEND
	#define traceTIMER_COMMAND_SEND( xTimer, xMessageID, xMessageValueValue, xReturn )
#endif

#ifndef traceTIMER_EXPIRED
	#define traceTIMER_EXPIRED( pxTimer )
#endif

#ifndef traceTIMER_COMMAND_RECEIVED
	#define traceTIMER_COMMAND_RECEIVED( pxTimer, xMessageID, xMessageValue )
#endif

#ifndef traceMALLOC
    #define traceMALLOC( pvAddress, uiSize )
#endif

#ifndef traceFREE
    #define traceFREE( pvAddress, uiSize )
#endif

#ifndef traceEVENT_GROUP_CREATE
	#define traceEVENT_GROUP_CREATE( xEventGroup )
#endif

#ifndef traceEVENT_GROUP_CREATE_FAILED
	#define traceEVENT_GROUP_CREATE_FAILED()
#endif

#ifndef traceEVENT_GROUP_SYNC_BLOCK
	#define traceEVENT_GROUP_SYNC_BLOCK( xEventGroup, uxBitsToSet, uxBitsToWaitFor )
#endif

#ifndef traceEVENT_GROUP_SYNC_END
	#define traceEVENT_GROUP_SYNC_END( xEventGroup, uxBitsToSet, uxBitsToWaitFor, xTimeoutOccurred ) ( void ) xTimeoutOccurred
#endif

#ifndef traceEVENT_GROUP_WAIT_BITS_BLOCK
	#define traceEVENT_GROUP_WAIT_BITS_BLOCK( xEventGroup, uxBitsToWaitFor )
#endif

#ifndef traceEVENT_GROUP_WAIT_BITS_END
	#define traceEVENT_GROUP_WAIT_BITS_END( xEventGroup, uxBitsToWaitFor, xTimeoutOccurred ) ( void ) xTimeoutOccurred
#endif

#ifndef traceEVENT_GROUP_CLEAR_BITS
	#define traceEVENT_GROUP_CLEAR_BITS( xEventGroup, uxBitsToClear )
#endif

#ifndef traceEVENT_GROUP_CLEAR_BITS_FROM_ISR
	#define traceEVENT_GROUP_CLEAR_BITS_FROM_ISR( xEventGroup, uxBitsToClear )
#endif

#ifndef traceEVENT_GROUP_SET_BITS
	#define traceEVENT_GROUP_SET_BITS( xEventGroup, uxBitsToSet )
#endif

#ifndef traceEVENT_GROUP_SET_BITS_FROM_ISR
	#define traceEVENT_GROUP_SET_BITS_FROM_ISR( xEventGroup, uxBitsToSet )
#endif

#ifndef traceEVENT_GROUP_DELETE
	#define traceEVENT_GROUP_DELETE( xEventGroup )
#endif

#ifndef tracePEND_FUNC_CALL
	#define tracePEND_FUNC_CALL(xFunctionToPend, pvParameter1, ulParameter2, ret)
#endif

#ifndef tracePEND_FUNC_CALL_FROM_ISR
	#define tracePEND_FUNC_CALL_FROM_ISR(xFunctionToPend, pvParameter1, ulParameter2, ret)
#endif

#ifndef traceQUEUE_REGISTRY_ADD
	#define traceQUEUE_REGISTRY_ADD(xQueue, pcQueueName)
#endif

#ifndef traceTASK_NOTIFY_TAKE_BLOCK
	#define traceTASK_NOTIFY_TAKE_BLOCK()
#endif

#ifndef traceTASK_NOTIFY_TAKE
	#define traceTASK_NOTIFY_TAKE()
#endif

#ifndef traceTASK_NOTIFY_WAIT_BLOCK
	#define traceTASK_NOTIFY_WAIT_BLOCK()
#endif

#ifndef traceTASK_NOTIFY_WAIT
	#define traceTASK_NOTIFY_WAIT()
#endif

#ifndef traceTASK_NOTIFY
	#define traceTASK_NOTIFY()
#endif

#ifndef traceTASK_NOTIFY_FROM_ISR
	#define traceTASK_NOTIFY_FROM_ISR()
#endif

#ifndef traceTASK_NOTIFY_GIVE_FROM_ISR
	#define traceTASK_NOTIFY_GIVE_FROM_ISR()
#endif

#ifndef traceSTREAM_BUFFER_CREATE_FAILED
	#define traceSTREAM_BUFFER_CREATE_FAILED( xIsMessageBuffer )
#endif

#ifndef traceSTREAM_BUFFER_CREATE_STATIC_FAILED
	#define traceSTREAM_BUFFER_CREATE_STATIC_FAILED( xReturn, xIsMessageBuffer )
#endif

#ifndef traceSTREAM_BUFFER_CREATE
	#define traceSTREAM_BUFFER_CREATE( pxStreamBuffer, xIsMessageBuffer )
#endif

#ifndef traceSTREAM_BUFFER_DELETE
	#define traceSTREAM_BUFFER_DELETE( xStreamBuffer )
#endif

#ifndef traceSTREAM_BUFFER_RESET
	#define traceSTREAM_BUFFER_RESET( xStreamBuffer )
#endif

#ifndef traceBLOCKING_ON_STREAM_BUFFER_SEND
	#define traceBLOCKING_ON_STREAM_BUFFER_SEND( xStreamBuffer )
#endif

#ifndef traceSTREAM_BUFFER_SEND
	#define traceSTREAM_BUFFER_SEND( xStreamBuffer, xBytesSent )
#endif

#ifndef traceSTREAM_BUFFER_SEND_FAILED
	#define traceSTREAM_BUFFER_SEND_FAILED( xStreamBuffer )
#endif

#ifndef traceSTREAM_BUFFER_SEND_FROM_ISR
	#define traceSTREAM_BUFFER_SEND_FROM_ISR( xStreamBuffer, xBytesSent )
#endif

#ifndef traceBLOCKING_ON_STREAM_BUFFER_RECEIVE
	#define traceBLOCKING_ON_STREAM_BUFFER_RECEIVE( xStreamBuffer )
#endif

#ifndef traceSTREAM_BUFFER_RECEIVE
	#define traceSTREAM_BUFFER_RECEIVE( xStreamBuffer, xReceivedLength )
#endif

#ifndef traceSTREAM_BUFFER_RECEIVE_FAILED
	#define traceSTREAM_BUFFER_RECEIVE_FAILED( xStreamBuffer )
#endif

#ifndef traceSTREAM_BUFFER_RECEIVE_FROM_ISR
	#define traceSTREAM_BUFFER_RECEIVE_FROM_ISR( xStreamBuffer, xReceivedLength )
#endif

#ifndef configGENERATE_RUN_TIME_STATS
	#define configGENERATE_RUN_TIME_STATS 0
#endif

#if ( configGENERATE_RUN_TIME_STATS == 1 )

	#ifndef portCONFIGURE_TIMER_FOR_RUN_TIME_STATS
		#error If configGENERATE_RUN_TIME_STATS is defined then portCONFIGURE_TIMER_FOR_RUN_TIME_STATS must also be defined.  portCONFIGURE_TIMER_FOR_RUN_TIME_STATS should call a port layer function to setup a peripheral timer/counter that can then be used as the run time counter time base.
	#endif /* portCONFIGURE_TIMER_FOR_RUN_TIME_STATS */

	#ifndef portGET_RUN_TIME_COUNTER_VALUE
		#ifndef portALT_GET_RUN_TIME_COUNTER_VALUE
			#error If configGENERATE_RUN_TIME_STATS is defined then either portGET_RUN_TIME_COUNTER_VALUE or portALT_GET_RUN_TIME_COUNTER_VALUE must also be defined.  See the examples provided and the FreeRTOS web site for more information.
		#endif /* portALT_GET_RUN_TIME_COUNTER_VALUE */
	#endif /* portGET_RUN_TIME_COUNTER_VALUE */

#endif /* configGENERATE_RUN_TIME_STATS */

#ifndef portCONFIGURE_TIMER_FOR_RUN_TIME_STATS
	#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#endif

#ifndef configUSE_MALLOC_FAILED_HOOK
	#define configUSE_MALLOC_FAILED_HOOK 0
#endif

#ifndef portPRIVILEGE_BIT
	#define portPRIVILEGE_BIT ( ( UBaseType_t ) 0x00 )
#endif

#ifndef portYIELD_WITHIN_API
	#define portYIELD_WITHIN_API portYIELD
#endif

#ifndef portSUPPRESS_TICKS_AND_SLEEP
	#define portSUPPRESS_TICKS_AND_SLEEP( xExpectedIdleTime )
#endif

#ifndef configEXPECTED_IDLE_TIME_BEFORE_SLEEP
	#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP 2
#endif

#if configEXPECTED_IDLE_TIME_BEFORE_SLEEP < 2
	#error configEXPECTED_IDLE_TIME_BEFORE_SLEEP must not be less than 2
#endif

#ifndef configUSE_TICKLESS_IDLE
	#define configUSE_TICKLESS_IDLE 0
#endif

#ifndef configPRE_SUPPRESS_TICKS_AND_SLEEP_PROCESSING
	#define configPRE_SUPPRESS_TICKS_AND_SLEEP_PROCESSING( x )
#endif

#ifndef configPRE_SLEEP_PROCESSING
	#define configPRE_SLEEP_PROCESSING( x )
#endif

#ifndef configPOST_SLEEP_PROCESSING
	#define configPOST_SLEEP_PROCESSING( x )
#endif

#ifndef configUSE_QUEUE_SETS
	#define configUSE_QUEUE_SETS 0
#endif

#ifndef portTASK_USES_FLOATING_POINT
	#define portTASK_USES_FLOATING_POINT()
#endif

#ifndef portTASK_CALLS_SECURE_FUNCTIONS
	#define portTASK_CALLS_SECURE_FUNCTIONS()
#endif

#ifndef configUSE_TIME_SLICING
	#define configUSE_TIME_SLICING 1
#endif

#ifndef configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS
	#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0
#endif

#ifndef configUSE_STATS_FORMATTING_FUNCTIONS
	#define configUSE_STATS_FORMATTING_FUNCTIONS 0
#endif

#ifndef portASSERT_IF_INTERRUPT_PRIORITY_INVALID
	#define portASSERT_IF_INTERRUPT_PRIORITY_INVALID()
#endif

#ifndef configUSE_TRACE_FACILITY
	#define configUSE_TRACE_FACILITY 0
#endif

#ifndef mtCOVERAGE_TEST_MARKER
	#define mtCOVERAGE_TEST_MARKER()
#endif

#ifndef mtCOVERAGE_TEST_DELAY
	#define mtCOVERAGE_TEST_DELAY()
#endif

#ifndef portASSERT_IF_IN_ISR
	#define portASSERT_IF_IN_ISR()
#endif

#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
	#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#endif

#ifndef configAPPLICATION_ALLOCATED_HEAP
	#define configAPPLICATION_ALLOCATED_HEAP 0
#endif

#ifndef configUSE_TASK_NOTIFICATIONS
	#define configUSE_TASK_NOTIFICATIONS 1
#endif

#ifndef portTICK_TYPE_IS_ATOMIC
	#define portTICK_TYPE_IS_ATOMIC 0
#endif

#ifndef configSUPPORT_STATIC_ALLOCATION
	/* Defaults to 0 for backward compatibility. */
	#define configSUPPORT_STATIC_ALLOCATION 0
#endif

#ifndef configSUPPORT_DYNAMIC_ALLOCATION
	/* Defaults to 1 for backward compatibility. */
	#define configSUPPORT_DYNAMIC_ALLOCATION 1
#endif

#ifndef configSTACK_DEPTH_TYPE
	/* Defaults to uint16_t for backward compatibility, but can be overridden
	in FreeRTOSConfig.h if uint16_t is too restrictive. */
	#define configSTACK_DEPTH_TYPE uint16_t
#endif

/* Sanity check the configuration. */
#if( configUSE_TICKLESS_IDLE != 0 )
	#if( INCLUDE_vTaskSuspend != 1 )
		#error INCLUDE_vTaskSuspend must be set to 1 if configUSE_TICKLESS_IDLE is not set to 0
	#endif /* INCLUDE_vTaskSuspend */
#endif /* configUSE_TICKLESS_IDLE */

#if( ( configSUPPORT_STATIC_ALLOCATION == 0 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 0 ) )
	#error configSUPPORT_STATIC_ALLOCATION and configSUPPORT_DYNAMIC_ALLOCATION cannot both be 0, but can both be 1.
#endif

#if( ( configUSE_RECURSIVE_MUTEXES == 1 ) && ( configUSE_MUTEXES != 1 ) )
	#error configUSE_MUTEXES must be set to 1 to use recursive mutexes
#endif

#ifndef configINITIAL_TICK_COUNT
	#define configINITIAL_TICK_COUNT 0
#endif

#if( portTICK_TYPE_IS_ATOMIC == 0 )
	/* Either variables of tick type cannot be read atomically, or
	portTICK_TYPE_IS_ATOMIC was not set - map the critical sections used when
	the tick count is returned to the standard critical section macros. */
	#define portTICK_TYPE_ENTER_CRITICAL() portENTER_CRITICAL()
	#define portTICK_TYPE_EXIT_CRITICAL() portEXIT_CRITICAL()
	#define portTICK_TYPE_SET_INTERRUPT_MASK_FROM_ISR() portSET_INTERRUPT_MASK_FROM_ISR()
	#define portTICK_TYPE_CLEAR_INTERRUPT_MASK_FROM_ISR( x ) portCLEAR_INTERRUPT_MASK_FROM_ISR( ( x ) )
#else
	/* The tick type can be read atomically, so critical sections used when the
	tick count is returned can be defined away. */
	#define portTICK_TYPE_ENTER_CRITICAL()
	#define portTICK_TYPE_EXIT_CRITICAL()
	#define portTICK_TYPE_SET_INTERRUPT_MASK_FROM_ISR() 0
	#define portTICK_TYPE_CLEAR_INTERRUPT_MASK_FROM_ISR( x ) ( void ) x
#endif

/* Definitions to allow backward compatibility with FreeRTOS versions prior to
V8 if desired. */
#ifndef configENABLE_BACKWARD_COMPATIBILITY
	#define configENABLE_BACKWARD_COMPATIBILITY 1
#endif

#ifndef configPRINTF
	/* configPRINTF() was not defined, so define it away to nothing.  To use
	configPRINTF() then define it as follows (where MyPrintFunction() is
	provided by the application writer):

	void MyPrintFunction(const char *pcFormat, ... );
	#define configPRINTF( X )   MyPrintFunction X

	Then call like a standard printf() function, but placing brackets around
	all parameters so they are passed as a single parameter.  For example:
	configPRINTF( ("Value = %d", MyVariable) ); */
	#define configPRINTF( X )
#endif

#ifndef configMAX
	/* The application writer has not provided their own MAX macro, so define
	the following generic implementation. */
	#define configMAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif

#ifndef configMIN
	/* The application writer has not provided their own MAX macro, so define
	the following generic implementation. */
	#define configMIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#endif

#if configENABLE_BACKWARD_COMPATIBILITY == 1
	#define eTaskStateGet eTaskGetState
	#define portTickType TickType_t
	#define xTaskHandle TaskHandle_t
	#define xQueueHandle QueueHandle_t
	#define xSemaphoreHandle SemaphoreHandle_t
	#define xQueueSetHandle QueueSetHandle_t
	#define xQueueSetMemberHandle QueueSetMemberHandle_t
	#define xTimeOutType TimeOut_t
	#define xMemoryRegion MemoryRegion_t
	#define xTaskParameters TaskParameters_t
	#define xTaskStatusType	TaskStatus_t
	#define xTimerHandle TimerHandle_t
	#define xCoRoutineHandle CoRoutineHandle_t
	#define pdTASK_HOOK_CODE TaskHookFunction_t
	#define portTICK_RATE_MS portTICK_PERIOD_MS
	#define pcTaskGetTaskName pcTaskGetName
	#define pcTimerGetTimerName pcTimerGetName
	#define pcQueueGetQueueName pcQueueGetName
	#define vTaskGetTaskInfo vTaskGetInfo

	/* Backward compatibility within the scheduler code only - these definitions
	are not really required but are included for completeness. */
	#define tmrTIMER_CALLBACK TimerCallbackFunction_t
	#define pdTASK_CODE TaskFunction_t
	#define xListItem ListItem_t
	#define xList List_t
#endif /* configENABLE_BACKWARD_COMPATIBILITY */

#if( configUSE_ALTERNATIVE_API != 0 )
	#error The alternative API was deprecated some time ago, and was removed in FreeRTOS V9.0 0
#endif

/* Set configUSE_TASK_FPU_SUPPORT to 0 to omit floating point support even
if floating point hardware is otherwise supported by the FreeRTOS port in use.
This constant is not supported by all FreeRTOS ports that include floating
point support. */
#ifndef configUSE_TASK_FPU_SUPPORT
	#define configUSE_TASK_FPU_SUPPORT 1
#endif

/*
 * In line with software engineering best practice, FreeRTOS implements a strict
 * data hiding policy, so the real structures used by FreeRTOS to maintain the
 * state of tasks, queues, semaphores, etc. are not accessible to the application
 * code.  However, if the application writer wants to statically allocate such
 * an object then the size of the object needs to be know.  Dummy structures
 * that are guaranteed to have the same size and alignment requirements of the
 * real objects are used for this purpose.  The dummy list and list item
 * structures below are used for inclusion in such a dummy structure.
 */
struct xSTATIC_LIST_ITEM
{
	TickType_t xDummy1;
	void *pvDummy2[ 4 ];
};
typedef struct xSTATIC_LIST_ITEM StaticListItem_t;

/* See the comments above the struct xSTATIC_LIST_ITEM definition. */
struct xSTATIC_MINI_LIST_ITEM
{
	TickType_t xDummy1;
	void *pvDummy2[ 2 ];
};
typedef struct xSTATIC_MINI_LIST_ITEM StaticMiniListItem_t;

/* See the comments above the struct xSTATIC_LIST_ITEM definition. */
typedef struct xSTATIC_LIST
{
	UBaseType_t uxDummy1;
	void *pvDummy2;
	StaticMiniListItem_t xDummy3;
} StaticList_t;

/*
 * In line with software engineering best practice, especially when supplying a
 * library that is likely to change in future versions, FreeRTOS implements a
 * strict data hiding policy.  This means the Task structure used internally by
 * FreeRTOS is not accessible to application code.  However, if the application
 * writer wants to statically allocate the memory required to create a task then
 * the size of the task object needs to be know.  The StaticTask_t structure
 * below is provided for this purpose.  Its sizes and alignment requirements are
 * guaranteed to match those of the genuine structure, no matter which
 * architecture is being used, and no matter how the values in FreeRTOSConfig.h
 * are set.  Its contents are somewhat obfuscated in the hope users will
 * recognise that it would be unwise to make direct use of the structure members.
 */
typedef struct xSTATIC_TCB
{
	void				*pxDummy1;
	#if ( portUSING_MPU_WRAPPERS == 1 )
		xMPU_SETTINGS	xDummy2;
	#endif
	StaticListItem_t	xDummy3[ 2 ];
	UBaseType_t			uxDummy5;
	void				*pxDummy6;
	uint8_t				ucDummy7[ configMAX_TASK_NAME_LEN ];
	#if ( ( portSTACK_GROWTH > 0 ) || ( configRECORD_STACK_HIGH_ADDRESS == 1 ) )
		void			*pxDummy8;
	#endif
	#if ( portCRITICAL_NESTING_IN_TCB == 1 )
		UBaseType_t		uxDummy9;
	#endif
	#if ( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t		uxDummy10[ 2 ];
	#endif
	#if ( configUSE_MUTEXES == 1 )
		UBaseType_t		uxDummy12[ 2 ];
	#endif
	#if ( configUSE_APPLICATION_TASK_TAG == 1 )
		void			*pxDummy14;
	#endif
	#if( configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0 )
		void			*pvDummy15[ configNUM_THREAD_LOCAL_STORAGE_POINTERS ];
	#endif
	#if ( configGENERATE_RUN_TIME_STATS == 1 )
		uint32_t		ulDummy16;
	#endif
	#if ( configUSE_NEWLIB_REENTRANT == 1 )
		struct	_reent	xDummy17;
	#endif
	#if ( configUSE_TASK_NOTIFICATIONS == 1 )
		uint32_t 		ulDummy18;
		uint8_t 		ucDummy19;
	#endif
	#if( ( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) ) || ( portUSING_MPU_WRAPPERS == 1 ) )
		uint8_t			uxDummy20;
	#endif

	#if( INCLUDE_xTaskAbortDelay == 1 )
		uint8_t ucDummy21;
	#endif

} StaticTask_t;

/*
 * In line with software engineering best practice, especially when supplying a
 * library that is likely to change in future versions, FreeRTOS implements a
 * strict data hiding policy.  This means the Queue structure used internally by
 * FreeRTOS is not accessible to application code.  However, if the application
 * writer wants to statically allocate the memory required to create a queue
 * then the size of the queue object needs to be know.  The StaticQueue_t
 * structure below is provided for this purpose.  Its sizes and alignment
 * requirements are guaranteed to match those of the genuine structure, no
 * matter which architecture is being used, and no matter how the values in
 * FreeRTOSConfig.h are set.  Its contents are somewhat obfuscated in the hope
 * users will recognise that it would be unwise to make direct use of the
 * structure members.
 */
typedef struct xSTATIC_QUEUE
{
	void *pvDummy1[ 3 ];

	union
	{
		void *pvDummy2;
		UBaseType_t uxDummy2;
	} u;

	StaticList_t xDummy3[ 2 ];
	UBaseType_t uxDummy4[ 3 ];
	uint8_t ucDummy5[ 2 ];

	#if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
		uint8_t ucDummy6;
	#endif

	#if ( configUSE_QUEUE_SETS == 1 )
		void *pvDummy7;
	#endif

	#if ( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t uxDummy8;
		uint8_t ucDummy9;
	#endif

} StaticQueue_t;
typedef StaticQueue_t StaticSemaphore_t;

/*
 * In line with software engineering best practice, especially when supplying a
 * library that is likely to change in future versions, FreeRTOS implements a
 * strict data hiding policy.  This means the event group structure used
 * internally by FreeRTOS is not accessible to application code.  However, if
 * the application writer wants to statically allocate the memory required to
 * create an event group then the size of the event group object needs to be
 * know.  The StaticEventGroup_t structure below is provided for this purpose.
 * Its sizes and alignment requirements are guaranteed to match those of the
 * genuine structure, no matter which architecture is being used, and no matter
 * how the values in FreeRTOSConfig.h are set.  Its contents are somewhat
 * obfuscated in the hope users will recognise that it would be unwise to make
 * direct use of the structure members.
 */
typedef struct xSTATIC_EVENT_GROUP
{
	TickType_t xDummy1;
	StaticList_t xDummy2;

	#if( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t uxDummy3;
	#endif

	#if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
			uint8_t ucDummy4;
	#endif

} StaticEventGroup_t;

/*
 * In line with software engineering best practice, especially when supplying a
 * library that is likely to change in future versions, FreeRTOS implements a
 * strict data hiding policy.  This means the software timer structure used
 * internally by FreeRTOS is not accessible to application code.  However, if
 * the application writer wants to statically allocate the memory required to
 * create a software timer then the size of the queue object needs to be know.
 * The StaticTimer_t structure below is provided for this purpose.  Its sizes
 * and alignment requirements are guaranteed to match those of the genuine
 * structure, no matter which architecture is being used, and no matter how the
 * values in FreeRTOSConfig.h are set.  Its contents are somewhat obfuscated in
 * the hope users will recognise that it would be unwise to make direct use of
 * the structure members.
 */
typedef struct xSTATIC_TIMER
{
	void				*pvDummy1;
	StaticListItem_t	xDummy2;
	TickType_t			xDummy3;
	UBaseType_t			uxDummy4;
	void 				*pvDummy5[ 2 ];
	#if( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t		uxDummy6;
	#endif

	#if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
		uint8_t 		ucDummy7;
	#endif

} StaticTimer_t;

/*
* In line with software engineering best practice, especially when supplying a
* library that is likely to change in future versions, FreeRTOS implements a
* strict data hiding policy.  This means the stream buffer structure used
* internally by FreeRTOS is not accessible to application code.  However, if
* the application writer wants to statically allocate the memory required to
* create a stream buffer then the size of the stream buffer object needs to be
* know.  The StaticStreamBuffer_t structure below is provided for this purpose.
* Its size and alignment requirements are guaranteed to match those of the
* genuine structure, no matter which architecture is being used, and no matter
* how the values in FreeRTOSConfig.h are set.  Its contents are somewhat
* obfuscated in the hope users will recognise that it would be unwise to make
* direct use of the structure members.
*/
typedef struct xSTATIC_STREAM_BUFFER
{
	size_t uxDummy1[ 4 ];
	void * pvDummy2[ 3 ];
	uint8_t ucDummy3;
	#if ( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t uxDummy4;
	#endif
} StaticStreamBuffer_t;

/* Message buffers are built on stream buffers. */
typedef StaticStreamBuffer_t StaticMessageBuffer_t;

#ifdef __cplusplus
}
#endif

#endif /* INC_FREERTOS_H */

