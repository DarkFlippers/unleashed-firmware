/*
 * Copyright (c) 2013-2018 Arm Limited. All rights reserved.
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
 * ----------------------------------------------------------------------
 *
 * $Date:        18. June 2018
 * $Revision:    V2.1.3
 *
 * Project:      CMSIS-RTOS API
 * Title:        cmsis_os.h template header file
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
 *     - const attribute removed from the osXxxxDef_t typedefs
 *     - const attribute added to the osXxxxDef macros
 *    Added: osTimerDelete, osMutexDelete, osSemaphoreDelete
 *    Added: osKernelInitialize
 * Version 1.02
 *    Control functions for short timeouts in microsecond resolution:
 *    Added: osKernelSysTick, osKernelSysTickFrequency, osKernelSysTickMicroSec
 *    Removed: osSignalGet 
 * Version 2.0.0
 *    OS objects creation without macros (dynamic creation and resource allocation):
 *     - added: osXxxxNew functions which replace osXxxxCreate
 *     - added: osXxxxAttr_t structures
 *     - deprecated: osXxxxCreate functions, osXxxxDef_t structures
 *     - deprecated: osXxxxDef and osXxxx macros
 *    osStatus codes simplified and renamed to osStatus_t
 *    osEvent return structure deprecated
 *    Kernel:
 *     - added: osKernelInfo_t and osKernelGetInfo
 *     - added: osKernelState_t and osKernelGetState (replaces osKernelRunning)
 *     - added: osKernelLock, osKernelUnlock
 *     - added: osKernelSuspend, osKernelResume
 *     - added: osKernelGetTickCount, osKernelGetTickFreq
 *     - renamed osKernelSysTick to osKernelGetSysTimerCount
 *     - replaced osKernelSysTickFrequency with osKernelGetSysTimerFreq
 *     - deprecated osKernelSysTickMicroSec
 *    Thread:
 *     - extended number of thread priorities
 *     - renamed osPrioriry to osPrioriry_t
 *     - replaced osThreadCreate with osThreadNew
 *     - added: osThreadGetName
 *     - added: osThreadState_t and osThreadGetState
 *     - added: osThreadGetStackSize, osThreadGetStackSpace
 *     - added: osThreadSuspend, osThreadResume
 *     - added: osThreadJoin, osThreadDetach, osThreadExit
 *     - added: osThreadGetCount, osThreadEnumerate
 *     - added: Thread Flags (moved from Signals) 
 *    Signals:
 *     - renamed osSignals to osThreadFlags (moved to Thread Flags)
 *     - changed return value of Set/Clear/Wait functions
 *     - Clear function limited to current running thread
 *     - extended Wait function (options)
 *     - added: osThreadFlagsGet
 *    Event Flags:
 *     - added new independent object for handling Event Flags
 *    Delay and Wait functions:
 *     - added: osDelayUntil
 *     - deprecated: osWait
 *    Timer:
 *     - replaced osTimerCreate with osTimerNew
 *     - added: osTimerGetName, osTimerIsRunning
 *    Mutex:
 *     - extended: attributes (Recursive, Priority Inherit, Robust)
 *     - replaced osMutexCreate with osMutexNew
 *     - renamed osMutexWait to osMutexAcquire
 *     - added: osMutexGetName, osMutexGetOwner
 *    Semaphore:
 *     - extended: maximum and initial token count
 *     - replaced osSemaphoreCreate with osSemaphoreNew
 *     - renamed osSemaphoreWait to osSemaphoreAcquire (changed return value)
 *     - added: osSemaphoreGetName, osSemaphoreGetCount
 *    Memory Pool:
 *     - using osMemoryPool prefix instead of osPool
 *     - replaced osPoolCreate with osMemoryPoolNew
 *     - extended osMemoryPoolAlloc (timeout)
 *     - added: osMemoryPoolGetName
 *     - added: osMemoryPoolGetCapacity, osMemoryPoolGetBlockSize
 *     - added: osMemoryPoolGetCount, osMemoryPoolGetSpace
 *     - added: osMemoryPoolDelete
 *     - deprecated: osPoolCAlloc
 *    Message Queue:
 *     - extended: fixed size message instead of a single 32-bit value
 *     - using osMessageQueue prefix instead of osMessage
 *     - replaced osMessageCreate with osMessageQueueNew
 *     - updated: osMessageQueuePut, osMessageQueueGet
 *     - added: osMessageQueueGetName
 *     - added: osMessageQueueGetCapacity, osMessageQueueGetMsgSize
 *     - added: osMessageQueueGetCount, osMessageQueueGetSpace
 *     - added: osMessageQueueReset, osMessageQueueDelete
 *    Mail Queue: 
 *     - deprecated (superseded by extended Message Queue functionality)
 * Version 2.1.0
 *    Support for critical and uncritical sections (nesting safe):
 *    - updated: osKernelLock, osKernelUnlock
 *    - added: osKernelRestoreLock
 *    Updated Thread and Event Flags:
 *    - changed flags parameter and return type from int32_t to uint32_t
 * Version 2.1.1
 *    Additional functions allowed to be called from Interrupt Service Routines:
 *    - osKernelGetTickCount, osKernelGetTickFreq
 *    Changed Kernel Tick type to uint32_t:
 *    - updated: osKernelGetTickCount, osDelayUntil
 * Version 2.1.2
 *    Additional functions allowed to be called from Interrupt Service Routines:
 *    - osKernelGetInfo, osKernelGetState
 * Version 2.1.3
 *    Additional functions allowed to be called from Interrupt Service Routines:
 *    - osThreadGetId
 *---------------------------------------------------------------------------*/
 
#ifndef CMSIS_OS_H_
#define CMSIS_OS_H_
 
/// \b osCMSIS identifies the CMSIS-RTOS API version.
#define osCMSIS             0x20001U    ///< API version (main[31:16].sub[15:0])
 
/// \note CAN BE CHANGED: \b osCMSIS_KERNEL identifies the underlying RTOS kernel and version number.
#define osCMSIS_KERNEL      0x10000U    ///< RTOS identification and version (main[31:16].sub[15:0])
 
/// \note CAN BE CHANGED: \b osKernelSystemId identifies the underlying RTOS kernel.
#define osKernelSystemId "KERNEL V1.0"  ///< RTOS identification string
 
/// \note CAN BE CHANGED: \b osFeature_xxx identifies RTOS features.
#define osFeature_MainThread  0         ///< main thread      1=main can be thread, 0=not available
#define osFeature_Signals     16U       ///< maximum number of Signal Flags available per thread
#define osFeature_Semaphore   65535U    ///< maximum count for \ref osSemaphoreCreate function
#define osFeature_Wait        0         ///< osWait function: 1=available, 0=not available
#define osFeature_SysTick     1         ///< osKernelSysTick functions: 1=available, 0=not available
#define osFeature_Pool        1         ///< Memory Pools:    1=available, 0=not available
#define osFeature_MessageQ    1         ///< Message Queues:  1=available, 0=not available
#define osFeature_MailQ       1         ///< Mail Queues:     1=available, 0=not available
 
#if (osCMSIS >= 0x20000U)
#include "cmsis_os2.h"
#else
#include <stdint.h>
#include <stddef.h>
#endif
 
#ifdef  __cplusplus
extern "C"
{
#endif
 
 
// ==== Enumerations, structures, defines ====
 
/// Priority values.
#if (osCMSIS < 0x20000U)
typedef enum {
  osPriorityIdle          = -3,         ///< Priority: idle (lowest)
  osPriorityLow           = -2,         ///< Priority: low
  osPriorityBelowNormal   = -1,         ///< Priority: below normal
  osPriorityNormal        =  0,         ///< Priority: normal (default)
  osPriorityAboveNormal   = +1,         ///< Priority: above normal
  osPriorityHigh          = +2,         ///< Priority: high
  osPriorityRealtime      = +3,         ///< Priority: realtime (highest)
  osPriorityError         = 0x84,       ///< System cannot determine priority or illegal priority.
  osPriorityReserved      = 0x7FFFFFFF  ///< Prevents enum down-size compiler optimization.
} osPriority;
#else
#define osPriority osPriority_t
#endif

/// Entry point of a thread.
typedef void (*os_pthread) (void const *argument);
 
/// Entry point of a timer call back function.
typedef void (*os_ptimer) (void const *argument);
 
/// Timer type.
#if (osCMSIS < 0x20000U)
typedef enum {
  osTimerOnce             = 0,          ///< One-shot timer.
  osTimerPeriodic         = 1           ///< Repeating timer.
} os_timer_type;
#else
#define os_timer_type osTimerType_t
#endif
 
/// Timeout value.
#define osWaitForever       0xFFFFFFFFU ///< Wait forever timeout value.
 
/// Status code values returned by CMSIS-RTOS functions.
#if (osCMSIS < 0x20000U)
typedef enum {
  osOK                    =    0,       ///< Function completed; no error or event occurred.
  osEventSignal           = 0x08,       ///< Function completed; signal event occurred.
  osEventMessage          = 0x10,       ///< Function completed; message event occurred.
  osEventMail             = 0x20,       ///< Function completed; mail event occurred.
  osEventTimeout          = 0x40,       ///< Function completed; timeout occurred.
  osErrorParameter        = 0x80,       ///< Parameter error: a mandatory parameter was missing or specified an incorrect object.
  osErrorResource         = 0x81,       ///< Resource not available: a specified resource was not available.
  osErrorTimeoutResource  = 0xC1,       ///< Resource not available within given time: a specified resource was not available within the timeout period.
  osErrorISR              = 0x82,       ///< Not allowed in ISR context: the function cannot be called from interrupt service routines.
  osErrorISRRecursive     = 0x83,       ///< Function called multiple times from ISR with same object.
  osErrorPriority         = 0x84,       ///< System cannot determine priority or thread has illegal priority.
  osErrorNoMemory         = 0x85,       ///< System is out of memory: it was impossible to allocate or reserve memory for the operation.
  osErrorValue            = 0x86,       ///< Value of a parameter is out of range.
  osErrorOS               = 0xFF,       ///< Unspecified RTOS error: run-time error but no other error message fits.
  osStatusReserved        = 0x7FFFFFFF  ///< Prevents enum down-size compiler optimization.
} osStatus;
#else
typedef int32_t                  osStatus;
#define osEventSignal           (0x08)
#define osEventMessage          (0x10)
#define osEventMail             (0x20)
#define osEventTimeout          (0x40)
#define osErrorOS               osError
#define osErrorTimeoutResource  osErrorTimeout
#define osErrorISRRecursive     (-126)
#define osErrorValue            (-127)
#define osErrorPriority         (-128)
#endif
 
 
// >>> the following data type definitions may be adapted towards a specific RTOS
 
/// Thread ID identifies the thread.
/// \note CAN BE CHANGED: \b implementation specific in every CMSIS-RTOS.
#if (osCMSIS < 0x20000U)
typedef void *osThreadId;
#else
#define osThreadId osThreadId_t
#endif
 
/// Timer ID identifies the timer.
/// \note CAN BE CHANGED: \b implementation specific in every CMSIS-RTOS.
#if (osCMSIS < 0x20000U)
typedef void *osTimerId;
#else
#define osTimerId osTimerId_t
#endif
 
/// Mutex ID identifies the mutex.
/// \note CAN BE CHANGED: \b implementation specific in every CMSIS-RTOS.
#if (osCMSIS < 0x20000U)
typedef void *osMutexId;
#else
#define osMutexId osMutexId_t
#endif
 
/// Semaphore ID identifies the semaphore.
/// \note CAN BE CHANGED: \b implementation specific in every CMSIS-RTOS.
#if (osCMSIS < 0x20000U)
typedef void *osSemaphoreId;
#else
#define osSemaphoreId osSemaphoreId_t
#endif
 
/// Pool ID identifies the memory pool.
/// \note CAN BE CHANGED: \b implementation specific in every CMSIS-RTOS.
typedef void *osPoolId;
 
/// Message ID identifies the message queue.
/// \note CAN BE CHANGED: \b implementation specific in every CMSIS-RTOS.
typedef void *osMessageQId;
 
/// Mail ID identifies the mail queue.
/// \note CAN BE CHANGED: \b implementation specific in every CMSIS-RTOS.
typedef void *osMailQId;
 
 
/// Thread Definition structure contains startup information of a thread.
/// \note CAN BE CHANGED: \b os_thread_def is implementation specific in every CMSIS-RTOS.
#if (osCMSIS < 0x20000U)
typedef struct os_thread_def {
  os_pthread                 pthread;   ///< start address of thread function
  osPriority               tpriority;   ///< initial thread priority
  uint32_t                 instances;   ///< maximum number of instances of that thread function
  uint32_t                 stacksize;   ///< stack size requirements in bytes; 0 is default stack size
} osThreadDef_t;
#else
typedef struct os_thread_def {
  os_pthread                 pthread;   ///< start address of thread function
  osThreadAttr_t                attr;   ///< thread attributes
} osThreadDef_t;
#endif
 
/// Timer Definition structure contains timer parameters.
/// \note CAN BE CHANGED: \b os_timer_def is implementation specific in every CMSIS-RTOS.
#if (osCMSIS < 0x20000U)
typedef struct os_timer_def {
  os_ptimer                   ptimer;   ///< start address of a timer function
} osTimerDef_t;
#else
typedef struct os_timer_def {
  os_ptimer                   ptimer;   ///< start address of a timer function
  osTimerAttr_t                 attr;   ///< timer attributes
} osTimerDef_t;
#endif
 
/// Mutex Definition structure contains setup information for a mutex.
/// \note CAN BE CHANGED: \b os_mutex_def is implementation specific in every CMSIS-RTOS.
#if (osCMSIS < 0x20000U)
typedef struct os_mutex_def {
  uint32_t                     dummy;   ///< dummy value
} osMutexDef_t;
#else
#define osMutexDef_t osMutexAttr_t
#endif
 
/// Semaphore Definition structure contains setup information for a semaphore.
/// \note CAN BE CHANGED: \b os_semaphore_def is implementation specific in every CMSIS-RTOS.
#if (osCMSIS < 0x20000U)
typedef struct os_semaphore_def {
  uint32_t                     dummy;   ///< dummy value
} osSemaphoreDef_t;
#else
#define osSemaphoreDef_t osSemaphoreAttr_t
#endif
 
/// Definition structure for memory block allocation.
/// \note CAN BE CHANGED: \b os_pool_def is implementation specific in every CMSIS-RTOS.
#if (osCMSIS < 0x20000U)
typedef struct os_pool_def {
  uint32_t                   pool_sz;   ///< number of items (elements) in the pool
  uint32_t                   item_sz;   ///< size of an item
  void                         *pool;   ///< pointer to memory for pool
} osPoolDef_t;
#else
typedef struct os_pool_def {
  uint32_t                   pool_sz;   ///< number of items (elements) in the pool
  uint32_t                   item_sz;   ///< size of an item
  osMemoryPoolAttr_t            attr;   ///< memory pool attributes
} osPoolDef_t;
#endif
 
/// Definition structure for message queue.
/// \note CAN BE CHANGED: \b os_messageQ_def is implementation specific in every CMSIS-RTOS.
#if (osCMSIS < 0x20000U)
typedef struct os_messageQ_def {
  uint32_t                  queue_sz;   ///< number of elements in the queue
  void                         *pool;   ///< memory array for messages
} osMessageQDef_t;
#else
typedef struct os_messageQ_def {
  uint32_t                  queue_sz;   ///< number of elements in the queue
  osMessageQueueAttr_t          attr;   ///< message queue attributes
} osMessageQDef_t;
#endif
 
/// Definition structure for mail queue.
/// \note CAN BE CHANGED: \b os_mailQ_def is implementation specific in every CMSIS-RTOS.
#if (osCMSIS < 0x20000U)
typedef struct os_mailQ_def {
  uint32_t                  queue_sz;   ///< number of elements in the queue
  uint32_t                   item_sz;   ///< size of an item
  void                         *pool;   ///< memory array for mail
} osMailQDef_t;
#else
typedef struct os_mailQ_def {
  uint32_t                  queue_sz;   ///< number of elements in the queue
  uint32_t                   item_sz;   ///< size of an item
  void                         *mail;   ///< pointer to mail
  osMemoryPoolAttr_t         mp_attr;   ///< memory pool attributes
  osMessageQueueAttr_t       mq_attr;   ///< message queue attributes
} osMailQDef_t;
#endif
 
 
/// Event structure contains detailed information about an event.
typedef struct {
  osStatus                    status;   ///< status code: event or error information
  union {
    uint32_t                       v;   ///< message as 32-bit value
    void                          *p;   ///< message or mail as void pointer
    int32_t                  signals;   ///< signal flags
  } value;                              ///< event value
  union {
    osMailQId                mail_id;   ///< mail id obtained by \ref osMailCreate
    osMessageQId          message_id;   ///< message id obtained by \ref osMessageCreate
  } def;                                ///< event definition
} osEvent;
 
 
//  ==== Kernel Management Functions ====
 
/// Initialize the RTOS Kernel for creating objects.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osKernelInitialize (void);
#endif
 
/// Start the RTOS Kernel scheduler.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osKernelStart (void);
#endif
 
/// Check if the RTOS kernel is already started.
/// \return 0 RTOS is not started, 1 RTOS is started.
#if (osCMSIS < 0x20000U)
int32_t osKernelRunning(void);
#endif
 
#if (defined(osFeature_SysTick) && (osFeature_SysTick != 0))  // System Timer available
 
/// Get the RTOS kernel system timer counter.
/// \return RTOS kernel system timer as 32-bit value 
#if (osCMSIS < 0x20000U)
uint32_t osKernelSysTick (void);
#else
#define  osKernelSysTick osKernelGetSysTimerCount
#endif
 
/// The RTOS kernel system timer frequency in Hz.
/// \note Reflects the system timer setting and is typically defined in a configuration file.
#if (osCMSIS < 0x20000U)
#define osKernelSysTickFrequency 100000000
#endif
 
/// Convert a microseconds value to a RTOS kernel system timer value.
/// \param         microsec     time value in microseconds.
/// \return time value normalized to the \ref osKernelSysTickFrequency
#if (osCMSIS < 0x20000U)
#define osKernelSysTickMicroSec(microsec) (((uint64_t)microsec * (osKernelSysTickFrequency)) / 1000000)
#else
#define osKernelSysTickMicroSec(microsec) (((uint64_t)microsec *  osKernelGetSysTimerFreq()) / 1000000)
#endif
 
#endif  // System Timer available
 
 
//  ==== Thread Management Functions ====
 
/// Create a Thread Definition with function, priority, and stack requirements.
/// \param         name          name of the thread function.
/// \param         priority      initial priority of the thread function.
/// \param         instances     number of possible thread instances.
/// \param         stacksz       stack size (in bytes) requirements for the thread function.
/// \note CAN BE CHANGED: The parameters to \b osThreadDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osThreadDef(name, priority, instances, stacksz) \
extern const osThreadDef_t os_thread_def_##name
#else                            // define the object
#if (osCMSIS < 0x20000U)
#define osThreadDef(name, priority, instances, stacksz) \
const osThreadDef_t os_thread_def_##name = \
{ (name), (priority), (instances), (stacksz) }
#else
#define osThreadDef(name, priority, instances, stacksz) \
const osThreadDef_t os_thread_def_##name = \
{ (name), \
  { NULL, osThreadDetached, NULL, 0U, NULL, 8*((stacksz+7)/8), (priority), 0U, 0U } }
#endif
#endif
 
/// Access a Thread definition.
/// \param         name          name of the thread definition object.
/// \note CAN BE CHANGED: The parameter to \b osThread shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osThread(name) \
&os_thread_def_##name
 
/// Create a thread and add it to Active Threads and set it to state READY.
/// \param[in]     thread_def    thread definition referenced with \ref osThread.
/// \param[in]     argument      pointer that is passed to the thread function as start argument.
/// \return thread ID for reference by other functions or NULL in case of error.
osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument);
 
/// Return the thread ID of the current running thread.
/// \return thread ID for reference by other functions or NULL in case of error.
#if (osCMSIS < 0x20000U)
osThreadId osThreadGetId (void);
#endif
 
/// Change priority of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \param[in]     priority      new priority value for the thread function.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority);
#endif
 
/// Get current priority of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \return current priority value of the specified thread.
#if (osCMSIS < 0x20000U)
osPriority osThreadGetPriority (osThreadId thread_id);
#endif
 
/// Pass control to next thread that is in state \b READY.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osThreadYield (void);
#endif
 
/// Terminate execution of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osThreadTerminate (osThreadId thread_id);
#endif
 
 
//  ==== Signal Management ====
 
/// Set the specified Signal Flags of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \param[in]     signals       specifies the signal flags of the thread that should be set.
/// \return previous signal flags of the specified thread or 0x80000000 in case of incorrect parameters.
int32_t osSignalSet (osThreadId thread_id, int32_t signals);
 
/// Clear the specified Signal Flags of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \param[in]     signals       specifies the signal flags of the thread that shall be cleared.
/// \return previous signal flags of the specified thread or 0x80000000 in case of incorrect parameters or call from ISR.
int32_t osSignalClear (osThreadId thread_id, int32_t signals);
 
/// Wait for one or more Signal Flags to become signaled for the current \b RUNNING thread.
/// \param[in]     signals       wait until all specified signal flags set or 0 for any single signal flag.
/// \param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return event flag information or error code.
osEvent osSignalWait (int32_t signals, uint32_t millisec);
 
 
//  ==== Generic Wait Functions ====
 
/// Wait for Timeout (Time Delay).
/// \param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue "time delay" value
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osDelay (uint32_t millisec);
#endif
 
#if (defined (osFeature_Wait) && (osFeature_Wait != 0))  // Generic Wait available
 
/// Wait for Signal, Message, Mail, or Timeout.
/// \param[in] millisec          \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out
/// \return event that contains signal, message, or mail information or error code.
osEvent osWait (uint32_t millisec);
 
#endif  // Generic Wait available
 
 
//  ==== Timer Management Functions ====
 
/// Define a Timer object.
/// \param         name          name of the timer object.
/// \param         function      name of the timer call back function.
/// \note CAN BE CHANGED: The parameter to \b osTimerDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osTimerDef(name, function) \
extern const osTimerDef_t os_timer_def_##name
#else                            // define the object
#if (osCMSIS < 0x20000U)
#define osTimerDef(name, function) \
const osTimerDef_t os_timer_def_##name = { (function) }
#else
#define osTimerDef(name, function) \
const osTimerDef_t os_timer_def_##name = \
{ (function), { NULL, 0U, NULL, 0U } }
#endif
#endif
 
/// Access a Timer definition.
/// \param         name          name of the timer object.
/// \note CAN BE CHANGED: The parameter to \b osTimer shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osTimer(name) \
&os_timer_def_##name
 
/// Create and Initialize a timer.
/// \param[in]     timer_def     timer object referenced with \ref osTimer.
/// \param[in]     type          osTimerOnce for one-shot or osTimerPeriodic for periodic behavior.
/// \param[in]     argument      argument to the timer call back function.
/// \return timer ID for reference by other functions or NULL in case of error.
osTimerId osTimerCreate (const osTimerDef_t *timer_def, os_timer_type type, void *argument);
 
/// Start or restart a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerCreate.
/// \param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue "time delay" value of the timer.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osTimerStart (osTimerId timer_id, uint32_t millisec);
#endif
 
/// Stop a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerCreate.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osTimerStop (osTimerId timer_id);
#endif
 
/// Delete a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerCreate.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osTimerDelete (osTimerId timer_id);
#endif
 
 
//  ==== Mutex Management Functions ====
 
/// Define a Mutex.
/// \param         name          name of the mutex object.
/// \note CAN BE CHANGED: The parameter to \b osMutexDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osMutexDef(name) \
extern const osMutexDef_t os_mutex_def_##name
#else                            // define the object
#if (osCMSIS < 0x20000U)
#define osMutexDef(name) \
const osMutexDef_t os_mutex_def_##name = { 0 }
#else
#define osMutexDef(name) \
const osMutexDef_t os_mutex_def_##name = \
{ NULL, osMutexRecursive | osMutexPrioInherit | osMutexRobust, NULL, 0U }
#endif
#endif
 
/// Access a Mutex definition.
/// \param         name          name of the mutex object.
/// \note CAN BE CHANGED: The parameter to \b osMutex shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osMutex(name) \
&os_mutex_def_##name
 
/// Create and Initialize a Mutex object.
/// \param[in]     mutex_def     mutex definition referenced with \ref osMutex.
/// \return mutex ID for reference by other functions or NULL in case of error.
osMutexId osMutexCreate (const osMutexDef_t *mutex_def);
 
/// Wait until a Mutex becomes available.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexCreate.
/// \param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osMutexWait (osMutexId mutex_id, uint32_t millisec);
#else
#define  osMutexWait osMutexAcquire
#endif
 
/// Release a Mutex that was obtained by \ref osMutexWait.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexCreate.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osMutexRelease (osMutexId mutex_id);
#endif
 
/// Delete a Mutex object.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexCreate.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osMutexDelete (osMutexId mutex_id);
#endif
 
 
//  ==== Semaphore Management Functions ====
 
#if (defined (osFeature_Semaphore) && (osFeature_Semaphore != 0U))  // Semaphore available
 
/// Define a Semaphore object.
/// \param         name          name of the semaphore object.
/// \note CAN BE CHANGED: The parameter to \b osSemaphoreDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osSemaphoreDef(name) \
extern const osSemaphoreDef_t os_semaphore_def_##name
#else                            // define the object
#if (osCMSIS < 0x20000U)
#define osSemaphoreDef(name) \
const osSemaphoreDef_t os_semaphore_def_##name = { 0 }
#else
#define osSemaphoreDef(name) \
const osSemaphoreDef_t os_semaphore_def_##name = \
{ NULL, 0U, NULL, 0U }
#endif
#endif
 
/// Access a Semaphore definition.
/// \param         name          name of the semaphore object.
/// \note CAN BE CHANGED: The parameter to \b osSemaphore shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osSemaphore(name) \
&os_semaphore_def_##name
 
/// Create and Initialize a Semaphore object.
/// \param[in]     semaphore_def semaphore definition referenced with \ref osSemaphore.
/// \param[in]     count         maximum and initial number of available tokens.
/// \return semaphore ID for reference by other functions or NULL in case of error.
osSemaphoreId osSemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count);
 
/// Wait until a Semaphore token becomes available.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return number of available tokens, or -1 in case of incorrect parameters.
int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec);
 
/// Release a Semaphore token.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id);
#endif
 
/// Delete a Semaphore object.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
#if (osCMSIS < 0x20000U)
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id);
#endif
 
#endif  // Semaphore available
 
 
//  ==== Memory Pool Management Functions ====
 
#if (defined(osFeature_Pool) && (osFeature_Pool != 0))  // Memory Pool available
 
/// \brief Define a Memory Pool.
/// \param         name          name of the memory pool.
/// \param         no            maximum number of blocks (objects) in the memory pool.
/// \param         type          data type of a single block (object).
/// \note CAN BE CHANGED: The parameter to \b osPoolDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osPoolDef(name, no, type) \
extern const osPoolDef_t os_pool_def_##name
#else                            // define the object
#if (osCMSIS < 0x20000U)
#define osPoolDef(name, no, type) \
const osPoolDef_t os_pool_def_##name = \
{ (no), sizeof(type), NULL }
#else
#define osPoolDef(name, no, type) \
const osPoolDef_t os_pool_def_##name = \
{ (no), sizeof(type), { NULL, 0U, NULL, 0U, NULL, 0U } }
#endif
#endif
 
/// \brief Access a Memory Pool definition.
/// \param         name          name of the memory pool
/// \note CAN BE CHANGED: The parameter to \b osPool shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osPool(name) \
&os_pool_def_##name
 
/// Create and Initialize a Memory Pool object.
/// \param[in]     pool_def      memory pool definition referenced with \ref osPool.
/// \return memory pool ID for reference by other functions or NULL in case of error.
osPoolId osPoolCreate (const osPoolDef_t *pool_def);
 
/// Allocate a memory block from a Memory Pool.
/// \param[in]     pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
/// \return address of the allocated memory block or NULL in case of no memory available.
void *osPoolAlloc (osPoolId pool_id);
 
/// Allocate a memory block from a Memory Pool and set memory block to zero.
/// \param[in]     pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
/// \return address of the allocated memory block or NULL in case of no memory available.
void *osPoolCAlloc (osPoolId pool_id);
 
/// Return an allocated memory block back to a Memory Pool.
/// \param[in]     pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
/// \param[in]     block         address of the allocated memory block to be returned to the memory pool.
/// \return status code that indicates the execution status of the function.
osStatus osPoolFree (osPoolId pool_id, void *block);
 
#endif  // Memory Pool available
 
 
//  ==== Message Queue Management Functions ====
 
#if (defined(osFeature_MessageQ) && (osFeature_MessageQ != 0))  // Message Queue available
  
/// \brief Create a Message Queue Definition.
/// \param         name          name of the queue.
/// \param         queue_sz      maximum number of messages in the queue.
/// \param         type          data type of a single message element (for debugger).
/// \note CAN BE CHANGED: The parameter to \b osMessageQDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osMessageQDef(name, queue_sz, type) \
extern const osMessageQDef_t os_messageQ_def_##name
#else                            // define the object
#if (osCMSIS < 0x20000U)
#define osMessageQDef(name, queue_sz, type) \
const osMessageQDef_t os_messageQ_def_##name = \
{ (queue_sz), NULL }
#else
#define osMessageQDef(name, queue_sz, type) \
const osMessageQDef_t os_messageQ_def_##name = \
{ (queue_sz), { NULL, 0U, NULL, 0U, NULL, 0U } }
#endif
#endif
 
/// \brief Access a Message Queue Definition.
/// \param         name          name of the queue
/// \note CAN BE CHANGED: The parameter to \b osMessageQ shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osMessageQ(name) \
&os_messageQ_def_##name
 
/// Create and Initialize a Message Queue object.
/// \param[in]     queue_def     message queue definition referenced with \ref osMessageQ.
/// \param[in]     thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
/// \return message queue ID for reference by other functions or NULL in case of error.
osMessageQId osMessageCreate (const osMessageQDef_t *queue_def, osThreadId thread_id);
 
/// Put a Message to a Queue.
/// \param[in]     queue_id      message queue ID obtained with \ref osMessageCreate.
/// \param[in]     info          message information.
/// \param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec);
 
/// Get a Message from a Queue or timeout if Queue is empty.
/// \param[in]     queue_id      message queue ID obtained with \ref osMessageCreate.
/// \param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return event information that includes status code.
osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec);
 
#endif  // Message Queue available
 
 
//  ==== Mail Queue Management Functions ====
 
#if (defined(osFeature_MailQ) && (osFeature_MailQ != 0))  // Mail Queue available
 
/// \brief Create a Mail Queue Definition.
/// \param         name          name of the queue.
/// \param         queue_sz      maximum number of mails in the queue.
/// \param         type          data type of a single mail element.
/// \note CAN BE CHANGED: The parameter to \b osMailQDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osMailQDef(name, queue_sz, type) \
extern const osMailQDef_t os_mailQ_def_##name
#else                            // define the object
#if (osCMSIS < 0x20000U)
#define osMailQDef(name, queue_sz, type) \
const osMailQDef_t os_mailQ_def_##name = \
{ (queue_sz), sizeof(type), NULL }
#else
#define osMailQDef(name, queue_sz, type) \
static void *os_mail_p_##name[2]; \
const osMailQDef_t os_mailQ_def_##name = \
{ (queue_sz), sizeof(type), (&os_mail_p_##name), \
  { NULL, 0U, NULL, 0U, NULL, 0U }, \
  { NULL, 0U, NULL, 0U, NULL, 0U } }
#endif
#endif
 
/// \brief Access a Mail Queue Definition.
/// \param         name          name of the queue
/// \note CAN BE CHANGED: The parameter to \b osMailQ shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osMailQ(name) \
&os_mailQ_def_##name
 
/// Create and Initialize a Mail Queue object.
/// \param[in]     queue_def     mail queue definition referenced with \ref osMailQ.
/// \param[in]     thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
/// \return mail queue ID for reference by other functions or NULL in case of error.
osMailQId osMailCreate (const osMailQDef_t *queue_def, osThreadId thread_id);
 
/// Allocate a memory block for mail from a mail memory pool.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out
/// \return pointer to memory block that can be filled with mail or NULL in case of error.
void *osMailAlloc (osMailQId queue_id, uint32_t millisec);
 
/// Allocate a memory block for mail from a mail memory pool and set memory block to zero.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out
/// \return pointer to memory block that can be filled with mail or NULL in case of error.
void *osMailCAlloc (osMailQId queue_id, uint32_t millisec);
 
/// Put a Mail into a Queue.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     mail          pointer to memory with mail to put into a queue.
/// \return status code that indicates the execution status of the function.
osStatus osMailPut (osMailQId queue_id, const void *mail);
 
/// Get a Mail from a Queue or timeout if Queue is empty.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return event information that includes status code.
osEvent osMailGet (osMailQId queue_id, uint32_t millisec);
 
/// Free a memory block by returning it to a mail memory pool.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     mail          pointer to memory block that was obtained with \ref osMailGet.
/// \return status code that indicates the execution status of the function.
osStatus osMailFree (osMailQId queue_id, void *mail);
 
#endif  // Mail Queue available
 
 
#ifdef  __cplusplus
}
#endif
 
#endif  // CMSIS_OS_H_
