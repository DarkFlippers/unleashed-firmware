/* ----------------------------------------------------------------------
 * $Date:        5. February 2013
 * $Revision:    V1.02
 *
 * Project:      CMSIS-RTOS API
 * Title:        cmsis_os.h header file
 *
 * Version 0.02
 *    Initial Proposal Phase
 * Version 0.03
 *    osKernelStart added, optional feature: main started as thread
 *    osSemaphores have standard behavior
 *    osTimerCreate does not start the timer, added osTimerStart
 *    osThreadPass is renamed to osThreadYield
 * Version 1.01
 *    Support for C++ interface
 *     - const attribute removed from the osXxxxDef_t typedef's
 *     - const attribute added to the osXxxxDef macros
 *    Added: osTimerDelete, osMutexDelete, osSemaphoreDelete
 *    Added: osKernelInitialize
 * Version 1.02
 *    Control functions for short timeouts in microsecond resolution:
 *    Added: osKernelSysTick, osKernelSysTickFrequency, osKernelSysTickMicroSec
 *    Removed: osSignalGet 
 *    
 *  
 *----------------------------------------------------------------------------
 *
 * Portions Copyright © 2016 STMicroelectronics International N.V. All rights reserved.
 * Portions Copyright (c) 2013 ARM LIMITED
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - Neither the name of ARM  nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

/**
\page cmsis_os_h Header File Template: cmsis_os.h

The file \b cmsis_os.h is a template header file for a CMSIS-RTOS compliant Real-Time Operating System (RTOS).
Each RTOS that is compliant with CMSIS-RTOS shall provide a specific \b cmsis_os.h header file that represents
its implementation.

The file cmsis_os.h contains:
 - CMSIS-RTOS API function definitions
 - struct definitions for parameters and return types
 - status and priority values used by CMSIS-RTOS API functions
 - macros for defining threads and other kernel objects


<b>Name conventions and header file modifications</b>

All definitions are prefixed with \b os to give an unique name space for CMSIS-RTOS functions.
Definitions that are prefixed \b os_ are not used in the application code but local to this header file.
All definitions and functions that belong to a module are grouped and have a common prefix, i.e. \b osThread.

Definitions that are marked with <b>CAN BE CHANGED</b> can be adapted towards the needs of the actual CMSIS-RTOS implementation.
These definitions can be specific to the underlying RTOS kernel.

Definitions that are marked with <b>MUST REMAIN UNCHANGED</b> cannot be altered. Otherwise the CMSIS-RTOS implementation is no longer
compliant to the standard. Note that some functions are optional and need not to be provided by every CMSIS-RTOS implementation.


<b>Function calls from interrupt service routines</b>

The following CMSIS-RTOS functions can be called from threads and interrupt service routines (ISR):
  - \ref osSignalSet
  - \ref osSemaphoreRelease
  - \ref osPoolAlloc, \ref osPoolCAlloc, \ref osPoolFree
  - \ref osMessagePut, \ref osMessageGet
  - \ref osMailAlloc, \ref osMailCAlloc, \ref osMailGet, \ref osMailPut, \ref osMailFree

Functions that cannot be called from an ISR are verifying the interrupt status and return in case that they are called
from an ISR context the status code \b osErrorISR. In some implementations this condition might be caught using the HARD FAULT vector.

Some CMSIS-RTOS implementations support CMSIS-RTOS function calls from multiple ISR at the same time.
If this is impossible, the CMSIS-RTOS rejects calls by nested ISR functions with the status code \b osErrorISRRecursive.


<b>Define and reference object definitions</b>

With <b>\#define osObjectsExternal</b> objects are defined as external symbols. This allows to create a consistent header file
that is used throughout a project as shown below:

<i>Header File</i>
\code
#include <cmsis_os.h>                                         // CMSIS RTOS header file

// Thread definition
extern void thread_sample (void const *argument);             // function prototype
osThreadDef (thread_sample, osPriorityBelowNormal, 1, 100);

// Pool definition
osPoolDef(MyPool, 10, long);
\endcode


This header file defines all objects when included in a C/C++ source file. When <b>\#define osObjectsExternal</b> is
present before the header file, the objects are defined as external symbols. A single consistent header file can therefore be
used throughout the whole project.

<i>Example</i>
\code
#include "osObjects.h"     // Definition of the CMSIS-RTOS objects
\endcode

\code
#define osObjectExternal   // Objects will be defined as external symbols
#include "osObjects.h"     // Reference to the CMSIS-RTOS objects
\endcode

*/

#ifndef _CMSIS_OS_H
#define _CMSIS_OS_H

/// \note MUST REMAIN UNCHANGED: \b osCMSIS identifies the CMSIS-RTOS API version.
#define osCMSIS           0x10002      ///< API version (main [31:16] .sub [15:0])

/// \note CAN BE CHANGED: \b osCMSIS_KERNEL identifies the underlying RTOS kernel and version number.
#define osCMSIS_KERNEL    0x10000	   ///< RTOS identification and version (main [31:16] .sub [15:0])

/// \note MUST REMAIN UNCHANGED: \b osKernelSystemId shall be consistent in every CMSIS-RTOS.
#define osKernelSystemId "KERNEL V1.00"   ///< RTOS identification string

/// \note MUST REMAIN UNCHANGED: \b osFeature_xxx shall be consistent in every CMSIS-RTOS.
#define osFeature_MainThread   1       ///< main thread      1=main can be thread, 0=not available
#define osFeature_Pool         1       ///< Memory Pools:    1=available, 0=not available
#define osFeature_MailQ        1       ///< Mail Queues:     1=available, 0=not available
#define osFeature_MessageQ     1       ///< Message Queues:  1=available, 0=not available
#define osFeature_Signals      8       ///< maximum number of Signal Flags available per thread
#define osFeature_Semaphore    1      ///< osFeature_Semaphore function: 1=available, 0=not available
#define osFeature_Wait         0       ///< osWait function: 1=available, 0=not available
#define osFeature_SysTick      1       ///< osKernelSysTick functions: 1=available, 0=not available

#ifdef  __cplusplus
extern "C"
{
#endif


// ==== Enumeration, structures, defines ====

/// Priority used for thread control.
/// \note MUST REMAIN UNCHANGED: \b osPriority shall be consistent in every CMSIS-RTOS.
typedef enum  {
  osPriorityIdle          = -3,          ///< priority: idle (lowest)
  osPriorityLow           = -2,          ///< priority: low
  osPriorityBelowNormal   = -1,          ///< priority: below normal
  osPriorityNormal        =  0,          ///< priority: normal (default)
  osPriorityAboveNormal   = +1,          ///< priority: above normal
  osPriorityHigh          = +2,          ///< priority: high
  osPriorityRealtime      = +3,          ///< priority: realtime (highest)
  osPriorityError         =  0x84        ///< system cannot determine priority or thread has illegal priority
} osPriority;

/// Timeout value.
/// \note MUST REMAIN UNCHANGED: \b osWaitForever shall be consistent in every CMSIS-RTOS.
#define osWaitForever     0xFFFFFFFF     ///< wait forever timeout value

/// Status code values returned by CMSIS-RTOS functions.
/// \note MUST REMAIN UNCHANGED: \b osStatus shall be consistent in every CMSIS-RTOS.
typedef enum  {
  osOK                    =     0,       ///< function completed; no error or event occurred.
  osEventSignal           =  0x08,       ///< function completed; signal event occurred.
  osEventMessage          =  0x10,       ///< function completed; message event occurred.
  osEventMail             =  0x20,       ///< function completed; mail event occurred.
  osEventTimeout          =  0x40,       ///< function completed; timeout occurred.
  osErrorParameter        =  0x80,       ///< parameter error: a mandatory parameter was missing or specified an incorrect object.
  osErrorResource         =  0x81,       ///< resource not available: a specified resource was not available.
  osErrorTimeoutResource  =  0xC1,       ///< resource not available within given time: a specified resource was not available within the timeout period.
  osErrorISR              =  0x82,       ///< not allowed in ISR context: the function cannot be called from interrupt service routines.
  osErrorISRRecursive     =  0x83,       ///< function called multiple times from ISR with same object.
  osErrorPriority         =  0x84,       ///< system cannot determine priority or thread has illegal priority.
  osErrorNoMemory         =  0x85,       ///< system is out of memory: it was impossible to allocate or reserve memory for the operation.
  osErrorValue            =  0x86,       ///< value of a parameter is out of range.
  osErrorOS               =  0xFF,       ///< unspecified RTOS error: run-time error but no other error message fits.
  os_status_reserved      =  0x7FFFFFFF  ///< prevent from enum down-size compiler optimization.
} osStatus;

#if ( INCLUDE_eTaskGetState == 1 )
/* Thread state returned by osThreadGetState */
typedef enum {
	osThreadRunning   = 0x0,	      /* A thread is querying the state of itself, so must be running. */
	osThreadReady     = 0x1 ,			        /* The thread being queried is in a read or pending ready list. */
	osThreadBlocked   = 0x2,		        /* The thread being queried is in the Blocked state. */
	osThreadSuspended = 0x3,	      /* The thread being queried is in the Suspended state, or is in the Blocked state with an infinite time out. */
	osThreadDeleted   = 0x4,		          /* The thread being queried has been deleted, but its TCB has not yet been freed. */   
  osThreadError     = 0x7FFFFFFF
} osThreadState;
#endif /* INCLUDE_eTaskGetState */

/// Timer type value for the timer definition.
/// \note MUST REMAIN UNCHANGED: \b os_timer_type shall be consistent in every CMSIS-RTOS.
typedef enum  {
  osTimerOnce             =     0,       ///< one-shot timer
  osTimerPeriodic         =     1        ///< repeating timer
} os_timer_type;

/// Entry point of a thread.
/// \note MUST REMAIN UNCHANGED: \b os_pthread shall be consistent in every CMSIS-RTOS.
typedef void (*os_pthread) (void const *argument);

/// Entry point of a timer call back function.
/// \note MUST REMAIN UNCHANGED: \b os_ptimer shall be consistent in every CMSIS-RTOS.
typedef void (*os_ptimer) (void const *argument);

// >>> the following data type definitions may shall adapted towards a specific RTOS

/// Thread ID identifies the thread (pointer to a thread control block).
/// \note CAN BE CHANGED: \b os_thread_cb is implementation specific in every CMSIS-RTOS.
typedef TaskHandle_t osThreadId;

/// Timer ID identifies the timer (pointer to a timer control block).
/// \note CAN BE CHANGED: \b os_timer_cb is implementation specific in every CMSIS-RTOS.
typedef TimerHandle_t osTimerId;

/// Mutex ID identifies the mutex (pointer to a mutex control block).
/// \note CAN BE CHANGED: \b os_mutex_cb is implementation specific in every CMSIS-RTOS.
typedef SemaphoreHandle_t osMutexId;

/// Semaphore ID identifies the semaphore (pointer to a semaphore control block).
/// \note CAN BE CHANGED: \b os_semaphore_cb is implementation specific in every CMSIS-RTOS.
typedef SemaphoreHandle_t osSemaphoreId;

/// Pool ID identifies the memory pool (pointer to a memory pool control block).
/// \note CAN BE CHANGED: \b os_pool_cb is implementation specific in every CMSIS-RTOS.
typedef struct os_pool_cb *osPoolId;

/// Message ID identifies the message queue (pointer to a message queue control block).
/// \note CAN BE CHANGED: \b os_messageQ_cb is implementation specific in every CMSIS-RTOS.
typedef QueueHandle_t osMessageQId;

/// Mail ID identifies the mail queue (pointer to a mail queue control block).
/// \note CAN BE CHANGED: \b os_mailQ_cb is implementation specific in every CMSIS-RTOS.
typedef struct os_mailQ_cb *osMailQId;


#if( configSUPPORT_STATIC_ALLOCATION == 1 )

typedef StaticTask_t               osStaticThreadDef_t;
typedef StaticTimer_t              osStaticTimerDef_t;
typedef StaticSemaphore_t          osStaticMutexDef_t;         
typedef StaticSemaphore_t          osStaticSemaphoreDef_t;
typedef StaticQueue_t              osStaticMessageQDef_t;

#endif




/// Thread Definition structure contains startup information of a thread.
/// \note CAN BE CHANGED: \b os_thread_def is implementation specific in every CMSIS-RTOS.
typedef struct os_thread_def  {
  char                   *name;        ///< Thread name 
  os_pthread             pthread;      ///< start address of thread function
  osPriority             tpriority;    ///< initial thread priority
  uint32_t               instances;    ///< maximum number of instances of that thread function
  uint32_t               stacksize;    ///< stack size requirements in bytes; 0 is default stack size
#if( configSUPPORT_STATIC_ALLOCATION == 1 )
  uint32_t               *buffer;      ///< stack buffer for static allocation; NULL for dynamic allocation
  osStaticThreadDef_t    *controlblock;     ///< control block to hold thread's data for static allocation; NULL for dynamic allocation
#endif
} osThreadDef_t;

/// Timer Definition structure contains timer parameters.
/// \note CAN BE CHANGED: \b os_timer_def is implementation specific in every CMSIS-RTOS.
typedef struct os_timer_def  {
  os_ptimer                 ptimer;    ///< start address of a timer function
#if( configSUPPORT_STATIC_ALLOCATION == 1 )
  osStaticTimerDef_t        *controlblock;      ///< control block to hold timer's data for static allocation; NULL for dynamic allocation
#endif
} osTimerDef_t;

/// Mutex Definition structure contains setup information for a mutex.
/// \note CAN BE CHANGED: \b os_mutex_def is implementation specific in every CMSIS-RTOS.
typedef struct os_mutex_def  {
  uint32_t                   dummy;    ///< dummy value.
#if( configSUPPORT_STATIC_ALLOCATION == 1 )
  osStaticMutexDef_t         *controlblock;      ///< control block for static allocation; NULL for dynamic allocation
#endif
} osMutexDef_t;

/// Semaphore Definition structure contains setup information for a semaphore.
/// \note CAN BE CHANGED: \b os_semaphore_def is implementation specific in every CMSIS-RTOS.
typedef struct os_semaphore_def  {
  uint32_t                   dummy;    ///< dummy value.
#if( configSUPPORT_STATIC_ALLOCATION == 1 )
  osStaticSemaphoreDef_t     *controlblock;      ///< control block for static allocation; NULL for dynamic allocation
#endif
} osSemaphoreDef_t;

/// Definition structure for memory block allocation.
/// \note CAN BE CHANGED: \b os_pool_def is implementation specific in every CMSIS-RTOS.
typedef struct os_pool_def  {
  uint32_t                 pool_sz;    ///< number of items (elements) in the pool
  uint32_t                 item_sz;    ///< size of an item
  void                       *pool;    ///< pointer to memory for pool
} osPoolDef_t;

/// Definition structure for message queue.
/// \note CAN BE CHANGED: \b os_messageQ_def is implementation specific in every CMSIS-RTOS.
typedef struct os_messageQ_def  {
  uint32_t                queue_sz;    ///< number of elements in the queue
  uint32_t                item_sz;    ///< size of an item
#if( configSUPPORT_STATIC_ALLOCATION == 1 )
  uint8_t                 *buffer;      ///< buffer for static allocation; NULL for dynamic allocation
  osStaticMessageQDef_t   *controlblock;     ///< control block to hold queue's data for static allocation; NULL for dynamic allocation
#endif
  //void                       *pool;    ///< memory array for messages
} osMessageQDef_t;

/// Definition structure for mail queue.
/// \note CAN BE CHANGED: \b os_mailQ_def is implementation specific in every CMSIS-RTOS.
typedef struct os_mailQ_def  {
  uint32_t                queue_sz;    ///< number of elements in the queue
  uint32_t                 item_sz;    ///< size of an item
  struct os_mailQ_cb **cb;
} osMailQDef_t;

/// Event structure contains detailed information about an event.
/// \note MUST REMAIN UNCHANGED: \b os_event shall be consistent in every CMSIS-RTOS.
///       However the struct may be extended at the end.
typedef struct  {
  osStatus                 status;     ///< status code: event or error information
  union  {
    uint32_t                    v;     ///< message as 32-bit value
    void                       *p;     ///< message or mail as void pointer
    int32_t               signals;     ///< signal flags
  } value;                             ///< event value
  union  {
    osMailQId             mail_id;     ///< mail id obtained by \ref osMailCreate
    osMessageQId       message_id;     ///< message id obtained by \ref osMessageCreate
  } def;                               ///< event definition
} osEvent;


//  ==== Kernel Control Functions ====

/// Initialize the RTOS Kernel for creating objects.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osKernelInitialize shall be consistent in every CMSIS-RTOS.
osStatus osKernelInitialize (void);

/// Start the RTOS Kernel.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osKernelStart shall be consistent in every CMSIS-RTOS.
osStatus osKernelStart (void);

/// Check if the RTOS kernel is already started.
/// \note MUST REMAIN UNCHANGED: \b osKernelRunning shall be consistent in every CMSIS-RTOS.
/// \return 0 RTOS is not started, 1 RTOS is started.
int32_t osKernelRunning(void);

#if (defined (osFeature_SysTick)  &&  (osFeature_SysTick != 0))     // System Timer available

/// Get the RTOS kernel system timer counter 
/// \note MUST REMAIN UNCHANGED: \b osKernelSysTick shall be consistent in every CMSIS-RTOS.
/// \return RTOS kernel system timer as 32-bit value 
uint32_t osKernelSysTick (void);

/// The RTOS kernel system timer frequency in Hz
/// \note Reflects the system timer setting and is typically defined in a configuration file.
#define osKernelSysTickFrequency      (configTICK_RATE_HZ)

/// Convert a microseconds value to a RTOS kernel system timer value.
/// \param         microsec     time value in microseconds.
/// \return time value normalized to the \ref osKernelSysTickFrequency
#define osKernelSysTickMicroSec(microsec) (((uint64_t)microsec * (osKernelSysTickFrequency)) / 1000000)

#endif    // System Timer available

//  ==== Thread Management ====

/// Create a Thread Definition with function, priority, and stack requirements.
/// \param         name         name of the thread function.
/// \param         priority     initial priority of the thread function.
/// \param         instances    number of possible thread instances.
/// \param         stacksz      stack size (in bytes) requirements for the thread function.
/// \note CAN BE CHANGED: The parameters to \b osThreadDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osThreadDef(name, thread, priority, instances, stacksz)  \
extern const osThreadDef_t os_thread_def_##name
#else                            // define the object

#if( configSUPPORT_STATIC_ALLOCATION == 1 )
#define osThreadDef(name, thread, priority, instances, stacksz)  \
const osThreadDef_t os_thread_def_##name = \
{ #name, (thread), (priority), (instances), (stacksz), NULL, NULL }

#define osThreadStaticDef(name, thread, priority, instances, stacksz, buffer, control)  \
const osThreadDef_t os_thread_def_##name = \
{ #name, (thread), (priority), (instances), (stacksz), (buffer), (control) }
#else //configSUPPORT_STATIC_ALLOCATION == 0

#define osThreadDef(name, thread, priority, instances, stacksz)  \
const osThreadDef_t os_thread_def_##name = \
{ #name, (thread), (priority), (instances), (stacksz)}
#endif
#endif

/// Access a Thread definition.
/// \param         name          name of the thread definition object.
/// \note CAN BE CHANGED: The parameter to \b osThread shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osThread(name)  \
&os_thread_def_##name

/// Create a thread and add it to Active Threads and set it to state READY.
/// \param[in]     thread_def    thread definition referenced with \ref osThread.
/// \param[in]     argument      pointer that is passed to the thread function as start argument.
/// \return thread ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osThreadCreate shall be consistent in every CMSIS-RTOS.
osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument);

/// Return the thread ID of the current running thread.
/// \return thread ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osThreadGetId shall be consistent in every CMSIS-RTOS.
osThreadId osThreadGetId (void);

/// Terminate execution of a thread and remove it from Active Threads.
/// \param[in]     thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadTerminate shall be consistent in every CMSIS-RTOS.
osStatus osThreadTerminate (osThreadId thread_id);

/// Pass control to next thread that is in state \b READY.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadYield shall be consistent in every CMSIS-RTOS.
osStatus osThreadYield (void);

/// Change priority of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \param[in]     priority      new priority value for the thread function.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadSetPriority shall be consistent in every CMSIS-RTOS.
osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority);

/// Get current priority of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \return current priority value of the thread function.
/// \note MUST REMAIN UNCHANGED: \b osThreadGetPriority shall be consistent in every CMSIS-RTOS.
osPriority osThreadGetPriority (osThreadId thread_id);


//  ==== Generic Wait Functions ====

/// Wait for Timeout (Time Delay).
/// \param[in]     millisec      time delay value
/// \return status code that indicates the execution status of the function.
osStatus osDelay (uint32_t millisec);

#if (defined (osFeature_Wait)  &&  (osFeature_Wait != 0))     // Generic Wait available

/// Wait for Signal, Message, Mail, or Timeout.
/// \param[in] millisec          timeout value or 0 in case of no time-out
/// \return event that contains signal, message, or mail information or error code.
/// \note MUST REMAIN UNCHANGED: \b osWait shall be consistent in every CMSIS-RTOS.
osEvent osWait (uint32_t millisec);

#endif  // Generic Wait available


//  ==== Timer Management Functions ====
/// Define a Timer object.
/// \param         name          name of the timer object.
/// \param         function      name of the timer call back function.
/// \note CAN BE CHANGED: The parameter to \b osTimerDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osTimerDef(name, function)  \
extern const osTimerDef_t os_timer_def_##name
#else                            // define the object

#if( configSUPPORT_STATIC_ALLOCATION == 1 ) 
#define osTimerDef(name, function)  \
const osTimerDef_t os_timer_def_##name = \
{ (function), NULL }

#define osTimerStaticDef(name, function, control)  \
const osTimerDef_t os_timer_def_##name = \
{ (function), (control) }
#else //configSUPPORT_STATIC_ALLOCATION == 0
#define osTimerDef(name, function)  \
const osTimerDef_t os_timer_def_##name = \
{ (function) }
#endif
#endif

/// Access a Timer definition.
/// \param         name          name of the timer object.
/// \note CAN BE CHANGED: The parameter to \b osTimer shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osTimer(name) \
&os_timer_def_##name

/// Create a timer.
/// \param[in]     timer_def     timer object referenced with \ref osTimer.
/// \param[in]     type          osTimerOnce for one-shot or osTimerPeriodic for periodic behavior.
/// \param[in]     argument      argument to the timer call back function.
/// \return timer ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osTimerCreate shall be consistent in every CMSIS-RTOS.
osTimerId osTimerCreate (const osTimerDef_t *timer_def, os_timer_type type, void *argument);

/// Start or restart a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerCreate.
/// \param[in]     millisec      time delay value of the timer.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osTimerStart shall be consistent in every CMSIS-RTOS.
osStatus osTimerStart (osTimerId timer_id, uint32_t millisec);

/// Stop the timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osTimerStop shall be consistent in every CMSIS-RTOS.
osStatus osTimerStop (osTimerId timer_id);

/// Delete a timer that was created by \ref osTimerCreate.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osTimerDelete shall be consistent in every CMSIS-RTOS.
osStatus osTimerDelete (osTimerId timer_id);


//  ==== Signal Management ====

/// Set the specified Signal Flags of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \param[in]     signals       specifies the signal flags of the thread that should be set.
/// \return osOK if successful, osErrorOS if failed.
/// \note MUST REMAIN UNCHANGED: \b osSignalSet shall be consistent in every CMSIS-RTOS.
int32_t osSignalSet (osThreadId thread_id, int32_t signals);

/// Clear the specified Signal Flags of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \param[in]     signals       specifies the signal flags of the thread that shall be cleared.
/// \return previous signal flags of the specified thread or 0x80000000 in case of incorrect parameters.
/// \note MUST REMAIN UNCHANGED: \b osSignalClear shall be consistent in every CMSIS-RTOS.
int32_t osSignalClear (osThreadId thread_id, int32_t signals);

/// Wait for one or more Signal Flags to become signaled for the current \b RUNNING thread.
/// \param[in]     signals       wait until all specified signal flags set or 0 for any single signal flag.
/// \param[in]     millisec      timeout value or 0 in case of no time-out.
/// \return event flag information or error code.
/// \note MUST REMAIN UNCHANGED: \b osSignalWait shall be consistent in every CMSIS-RTOS.
osEvent osSignalWait (int32_t signals, uint32_t millisec);


//  ==== Mutex Management ====

/// Define a Mutex.
/// \param         name          name of the mutex object.
/// \note CAN BE CHANGED: The parameter to \b osMutexDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osMutexDef(name)  \
extern const osMutexDef_t os_mutex_def_##name
#else                            // define the object

#if( configSUPPORT_STATIC_ALLOCATION == 1 )
#define osMutexDef(name)  \
const osMutexDef_t os_mutex_def_##name = { 0, NULL }

#define osMutexStaticDef(name, control)  \
const osMutexDef_t os_mutex_def_##name = { 0, (control) }
#else //configSUPPORT_STATIC_ALLOCATION == 0
#define osMutexDef(name)  \
const osMutexDef_t os_mutex_def_##name = { 0 }

#endif

#endif

/// Access a Mutex definition.
/// \param         name          name of the mutex object.
/// \note CAN BE CHANGED: The parameter to \b osMutex shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osMutex(name)  \
&os_mutex_def_##name

/// Create and Initialize a Mutex object.
/// \param[in]     mutex_def     mutex definition referenced with \ref osMutex.
/// \return mutex ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osMutexCreate shall be consistent in every CMSIS-RTOS.
osMutexId osMutexCreate (const osMutexDef_t *mutex_def);

/// Wait until a Mutex becomes available.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osMutexWait shall be consistent in every CMSIS-RTOS.
osStatus osMutexWait (osMutexId mutex_id, uint32_t millisec);

/// Release a Mutex that was obtained by \ref osMutexWait.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osMutexRelease shall be consistent in every CMSIS-RTOS.
osStatus osMutexRelease (osMutexId mutex_id);

/// Delete a Mutex that was created by \ref osMutexCreate.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osMutexDelete shall be consistent in every CMSIS-RTOS.
osStatus osMutexDelete (osMutexId mutex_id);


//  ==== Semaphore Management Functions ====

#if (defined (osFeature_Semaphore)  &&  (osFeature_Semaphore != 0))     // Semaphore available

/// Define a Semaphore object.
/// \param         name          name of the semaphore object.
/// \note CAN BE CHANGED: The parameter to \b osSemaphoreDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osSemaphoreDef(name)  \
extern const osSemaphoreDef_t os_semaphore_def_##name
#else                            // define the object

#if( configSUPPORT_STATIC_ALLOCATION == 1 )
#define osSemaphoreDef(name)  \
const osSemaphoreDef_t os_semaphore_def_##name = { 0, NULL }

#define osSemaphoreStaticDef(name, control)  \
const osSemaphoreDef_t os_semaphore_def_##name = { 0, (control) }

#else //configSUPPORT_STATIC_ALLOCATION == 0
#define osSemaphoreDef(name)  \
const osSemaphoreDef_t os_semaphore_def_##name = { 0 }
#endif
#endif

/// Access a Semaphore definition.
/// \param         name          name of the semaphore object.
/// \note CAN BE CHANGED: The parameter to \b osSemaphore shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osSemaphore(name)  \
&os_semaphore_def_##name

/// Create and Initialize a Semaphore object used for managing resources.
/// \param[in]     semaphore_def semaphore definition referenced with \ref osSemaphore.
/// \param[in]     count         number of available resources.
/// \return semaphore ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreCreate shall be consistent in every CMSIS-RTOS.
osSemaphoreId osSemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count);

/// Wait until a Semaphore token becomes available.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out.
/// \return number of available tokens, or -1 in case of incorrect parameters.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreWait shall be consistent in every CMSIS-RTOS.
int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec);

/// Release a Semaphore token.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreRelease shall be consistent in every CMSIS-RTOS.
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id);

/// Delete a Semaphore that was created by \ref osSemaphoreCreate.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreDelete shall be consistent in every CMSIS-RTOS.
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id);

#endif     // Semaphore available


//  ==== Memory Pool Management Functions ====

#if (defined (osFeature_Pool)  &&  (osFeature_Pool != 0))  // Memory Pool Management available

/// \brief Define a Memory Pool.
/// \param         name          name of the memory pool.
/// \param         no            maximum number of blocks (objects) in the memory pool.
/// \param         type          data type of a single block (object).
/// \note CAN BE CHANGED: The parameter to \b osPoolDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osPoolDef(name, no, type)   \
extern const osPoolDef_t os_pool_def_##name
#else                            // define the object
#define osPoolDef(name, no, type)   \
const osPoolDef_t os_pool_def_##name = \
{ (no), sizeof(type), NULL }
#endif

/// \brief Access a Memory Pool definition.
/// \param         name          name of the memory pool
/// \note CAN BE CHANGED: The parameter to \b osPool shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osPool(name) \
&os_pool_def_##name

/// Create and Initialize a memory pool.
/// \param[in]     pool_def      memory pool definition referenced with \ref osPool.
/// \return memory pool ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osPoolCreate shall be consistent in every CMSIS-RTOS.
osPoolId osPoolCreate (const osPoolDef_t *pool_def);

/// Allocate a memory block from a memory pool.
/// \param[in]     pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
/// \return address of the allocated memory block or NULL in case of no memory available.
/// \note MUST REMAIN UNCHANGED: \b osPoolAlloc shall be consistent in every CMSIS-RTOS.
void *osPoolAlloc (osPoolId pool_id);

/// Allocate a memory block from a memory pool and set memory block to zero.
/// \param[in]     pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
/// \return address of the allocated memory block or NULL in case of no memory available.
/// \note MUST REMAIN UNCHANGED: \b osPoolCAlloc shall be consistent in every CMSIS-RTOS.
void *osPoolCAlloc (osPoolId pool_id);

/// Return an allocated memory block back to a specific memory pool.
/// \param[in]     pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
/// \param[in]     block         address of the allocated memory block that is returned to the memory pool.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osPoolFree shall be consistent in every CMSIS-RTOS.
osStatus osPoolFree (osPoolId pool_id, void *block);

#endif   // Memory Pool Management available


//  ==== Message Queue Management Functions ====

#if (defined (osFeature_MessageQ)  &&  (osFeature_MessageQ != 0))     // Message Queues available

/// \brief Create a Message Queue Definition.
/// \param         name          name of the queue.
/// \param         queue_sz      maximum number of messages in the queue.
/// \param         type          data type of a single message element (for debugger).
/// \note CAN BE CHANGED: The parameter to \b osMessageQDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osMessageQDef(name, queue_sz, type)   \
extern const osMessageQDef_t os_messageQ_def_##name
#else                            // define the object
#if( configSUPPORT_STATIC_ALLOCATION == 1 )
#define osMessageQDef(name, queue_sz, type)   \
const osMessageQDef_t os_messageQ_def_##name = \
{ (queue_sz), sizeof (type), NULL, NULL  }

#define osMessageQStaticDef(name, queue_sz, type, buffer, control)   \
const osMessageQDef_t os_messageQ_def_##name = \
{ (queue_sz), sizeof (type) , (buffer), (control)}
#else //configSUPPORT_STATIC_ALLOCATION == 1
#define osMessageQDef(name, queue_sz, type)   \
const osMessageQDef_t os_messageQ_def_##name = \
{ (queue_sz), sizeof (type) }

#endif
#endif

/// \brief Access a Message Queue Definition.
/// \param         name          name of the queue
/// \note CAN BE CHANGED: The parameter to \b osMessageQ shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osMessageQ(name) \
&os_messageQ_def_##name

/// Create and Initialize a Message Queue.
/// \param[in]     queue_def     queue definition referenced with \ref osMessageQ.
/// \param[in]     thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
/// \return message queue ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osMessageCreate shall be consistent in every CMSIS-RTOS.
osMessageQId osMessageCreate (const osMessageQDef_t *queue_def, osThreadId thread_id);

/// Put a Message to a Queue.
/// \param[in]     queue_id      message queue ID obtained with \ref osMessageCreate.
/// \param[in]     info          message information.
/// \param[in]     millisec      timeout value or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osMessagePut shall be consistent in every CMSIS-RTOS.
osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec);

/// Get a Message or Wait for a Message from a Queue.
/// \param[in]     queue_id      message queue ID obtained with \ref osMessageCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out.
/// \return event information that includes status code.
/// \note MUST REMAIN UNCHANGED: \b osMessageGet shall be consistent in every CMSIS-RTOS.
osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec);

#endif     // Message Queues available


//  ==== Mail Queue Management Functions ====

#if (defined (osFeature_MailQ)  &&  (osFeature_MailQ != 0))     // Mail Queues available

/// \brief Create a Mail Queue Definition.
/// \param         name          name of the queue
/// \param         queue_sz      maximum number of messages in queue
/// \param         type          data type of a single message element
/// \note CAN BE CHANGED: The parameter to \b osMailQDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osMailQDef(name, queue_sz, type) \
extern struct os_mailQ_cb *os_mailQ_cb_##name \
extern osMailQDef_t os_mailQ_def_##name
#else                            // define the object
#define osMailQDef(name, queue_sz, type) \
struct os_mailQ_cb *os_mailQ_cb_##name; \
const osMailQDef_t os_mailQ_def_##name =  \
{ (queue_sz), sizeof (type), (&os_mailQ_cb_##name) }
#endif

/// \brief Access a Mail Queue Definition.
/// \param         name          name of the queue
/// \note CAN BE CHANGED: The parameter to \b osMailQ shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osMailQ(name)  \
&os_mailQ_def_##name

/// Create and Initialize mail queue.
/// \param[in]     queue_def     reference to the mail queue definition obtain with \ref osMailQ
/// \param[in]     thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
/// \return mail queue ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osMailCreate shall be consistent in every CMSIS-RTOS.
osMailQId osMailCreate (const osMailQDef_t *queue_def, osThreadId thread_id);

/// Allocate a memory block from a mail.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out
/// \return pointer to memory block that can be filled with mail or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osMailAlloc shall be consistent in every CMSIS-RTOS.
void *osMailAlloc (osMailQId queue_id, uint32_t millisec);

/// Allocate a memory block from a mail and set memory block to zero.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out
/// \return pointer to memory block that can be filled with mail or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osMailCAlloc shall be consistent in every CMSIS-RTOS.
void *osMailCAlloc (osMailQId queue_id, uint32_t millisec);

/// Put a mail to a queue.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     mail          memory block previously allocated with \ref osMailAlloc or \ref osMailCAlloc.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osMailPut shall be consistent in every CMSIS-RTOS.
osStatus osMailPut (osMailQId queue_id, void *mail);

/// Get a mail from a queue.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out
/// \return event that contains mail information or error code.
/// \note MUST REMAIN UNCHANGED: \b osMailGet shall be consistent in every CMSIS-RTOS.
osEvent osMailGet (osMailQId queue_id, uint32_t millisec);

/// Free a memory block from a mail.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     mail          pointer to the memory block that was obtained with \ref osMailGet.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osMailFree shall be consistent in every CMSIS-RTOS.
osStatus osMailFree (osMailQId queue_id, void *mail);

#endif  // Mail Queues available

/*************************** Additional specific APIs to Free RTOS ************/
/**
* @brief  Handles the tick increment
* @param  none.
* @retval none.
*/
void osSystickHandler(void);

#if ( INCLUDE_eTaskGetState == 1 )
/**
* @brief  Obtain the state of any thread.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  the stae of the thread, states are encoded by the osThreadState enumerated type.
*/
osThreadState osThreadGetState(osThreadId thread_id);
#endif /* INCLUDE_eTaskGetState */

#if ( INCLUDE_eTaskGetState == 1 )
/**
* @brief Check if a thread is already suspended or not.
* @param thread_id thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval status code that indicates the execution status of the function.
*/

osStatus osThreadIsSuspended(osThreadId thread_id);

#endif /* INCLUDE_eTaskGetState */

/**
* @brief  Suspend execution of a thread.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadSuspend (osThreadId thread_id);

/**
* @brief  Resume execution of a suspended thread.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadResume (osThreadId thread_id);

/**
* @brief  Suspend execution of a all active threads.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadSuspendAll (void);

/**
* @brief  Resume execution of a all suspended threads.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadResumeAll (void);

/**
* @brief  Delay a task until a specified time
* @param   PreviousWakeTime   Pointer to a variable that holds the time at which the 
*          task was last unblocked. PreviousWakeTime must be initialised with the current time
*          prior to its first use (PreviousWakeTime = osKernelSysTick() )
* @param   millisec    time delay value
* @retval  status code that indicates the execution status of the function.
*/
osStatus osDelayUntil (uint32_t *PreviousWakeTime, uint32_t millisec);

/**
* @brief   Abort the delay for a specific thread
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId   
* @retval  status code that indicates the execution status of the function.
*/
osStatus osAbortDelay(osThreadId thread_id);

/**
* @brief   Lists all the current threads, along with their current state 
*          and stack usage high water mark.
* @param   buffer   A buffer into which the above mentioned details
*          will be written
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadList (uint8_t *buffer);

/**
* @brief  Receive an item from a queue without removing the item from the queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @param  millisec  timeout value or 0 in case of no time-out.
* @retval event information that includes status code.
*/
osEvent osMessagePeek (osMessageQId queue_id, uint32_t millisec);

/**
* @brief  Get the number of messaged stored in a queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @retval number of messages stored in a queue.
*/
uint32_t osMessageWaiting(osMessageQId queue_id);

/**
* @brief  Get the available space in a message queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @retval available space in a message queue.
*/
uint32_t osMessageAvailableSpace(osMessageQId queue_id);

/**
* @brief Delete a Message Queue
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osMessageDelete (osMessageQId queue_id);

/**
* @brief  Create and Initialize a Recursive Mutex
* @param  mutex_def     mutex definition referenced with \ref osMutex.
* @retval  mutex ID for reference by other functions or NULL in case of error..
*/
osMutexId osRecursiveMutexCreate (const osMutexDef_t *mutex_def);

/**
* @brief  Release a Recursive Mutex
* @param   mutex_id      mutex ID obtained by \ref osRecursiveMutexCreate.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osRecursiveMutexRelease (osMutexId mutex_id);

/**
* @brief  Release a Recursive Mutex
* @param   mutex_id    mutex ID obtained by \ref osRecursiveMutexCreate.
* @param millisec      timeout value or 0 in case of no time-out.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osRecursiveMutexWait (osMutexId mutex_id, uint32_t millisec);

/**
* @brief  Returns the current count value of a counting semaphore
* @param   semaphore_id  semaphore_id ID obtained by \ref osSemaphoreCreate.
* @retval  count value
*/
uint32_t osSemaphoreGetCount(osSemaphoreId semaphore_id);

#ifdef  __cplusplus
}
#endif

#endif  // _CMSIS_OS_H
