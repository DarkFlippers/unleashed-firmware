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
 *      Name:    cmsis_os2.c
 *      Purpose: CMSIS RTOS2 wrapper for FreeRTOS
 *
 *---------------------------------------------------------------------------*/

#include <string.h>

#include <furi/common_defines.h>

#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "cmsis_compiler.h"             // Compiler agnostic definitions
#include "os_tick.h"                    // OS Tick API

#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "event_groups.h"               // ARM.FreeRTOS::RTOS:Event Groups
#include "semphr.h"                     // ARM.FreeRTOS::RTOS:Core
#include "timers.h"                     // ARM.FreeRTOS::RTOS:Timers

#include "freertos_mpool.h"             // osMemoryPool definitions
#include "freertos_os2.h"               // Configuration check and setup

#include CMSIS_device_header

#ifndef CMSIS_TASK_NOTIFY_INDEX
#define CMSIS_TASK_NOTIFY_INDEX 0
#endif

/*---------------------------------------------------------------------------*/
#ifndef __ARM_ARCH_6M__
  #define __ARM_ARCH_6M__         0
#endif
#ifndef __ARM_ARCH_7M__
  #define __ARM_ARCH_7M__         0
#endif
#ifndef __ARM_ARCH_7EM__
  #define __ARM_ARCH_7EM__        0
#endif
#ifndef __ARM_ARCH_8M_MAIN__
  #define __ARM_ARCH_8M_MAIN__    0
#endif
#ifndef __ARM_ARCH_7A__
  #define __ARM_ARCH_7A__         0
#endif

#if   ((__ARM_ARCH_7M__      == 1U) || \
       (__ARM_ARCH_7EM__     == 1U) || \
       (__ARM_ARCH_8M_MAIN__ == 1U))
#define IS_IRQ_MASKED()           ((__get_PRIMASK() != 0U) || (__get_BASEPRI() != 0U))
#elif  (__ARM_ARCH_6M__      == 1U)
#define IS_IRQ_MASKED()           (__get_PRIMASK() != 0U)
#elif (__ARM_ARCH_7A__       == 1U)
/* CPSR mask bits */
#define CPSR_MASKBIT_I            0x80U

#define IS_IRQ_MASKED()           ((__get_CPSR() & CPSR_MASKBIT_I) != 0U)
#else
#define IS_IRQ_MASKED()           (__get_PRIMASK() != 0U)
#endif

#if    (__ARM_ARCH_7A__      == 1U)
/* CPSR mode bitmasks */
#define CPSR_MODE_USER            0x10U
#define CPSR_MODE_SYSTEM          0x1FU

#define IS_IRQ_MODE()             ((__get_mode() != CPSR_MODE_USER) && (__get_mode() != CPSR_MODE_SYSTEM))
#else
#define IS_IRQ_MODE()             (__get_IPSR() != 0U)
#endif

/* Limits */
#define MAX_BITS_TASK_NOTIFY      31U
#define MAX_BITS_EVENT_GROUPS     24U

#define THREAD_FLAGS_INVALID_BITS (~((1UL << MAX_BITS_TASK_NOTIFY)  - 1U))
#define EVENT_FLAGS_INVALID_BITS  (~((1UL << MAX_BITS_EVENT_GROUPS) - 1U))

/* Kernel version and identification string definition (major.minor.rev: mmnnnrrrr dec) */
#define KERNEL_VERSION            (((uint32_t)tskKERNEL_VERSION_MAJOR * 10000000UL) | \
                                   ((uint32_t)tskKERNEL_VERSION_MINOR *    10000UL) | \
                                   ((uint32_t)tskKERNEL_VERSION_BUILD *        1UL))

#define KERNEL_ID                 ("FreeRTOS " tskKERNEL_VERSION_NUMBER)

/* Timer callback information structure definition */
typedef struct {
  osTimerFunc_t func;
  void         *arg;
} TimerCallback_t;

/* Kernel initialization state */
static osKernelState_t KernelState = osKernelInactive;

/*
  Heap region definition used by heap_5 variant

  Define configAPPLICATION_ALLOCATED_HEAP as nonzero value in FreeRTOSConfig.h if
  heap regions are already defined and vPortDefineHeapRegions is called in application.

  Otherwise vPortDefineHeapRegions will be called by osKernelInitialize using
  definition configHEAP_5_REGIONS as parameter. Overriding configHEAP_5_REGIONS
  is possible by defining it globally or in FreeRTOSConfig.h.
*/
#if defined(USE_FreeRTOS_HEAP_5)
#if (configAPPLICATION_ALLOCATED_HEAP == 0)
  /*
    FreeRTOS heap is not defined by the application.
    Single region of size configTOTAL_HEAP_SIZE (defined in FreeRTOSConfig.h)
    is provided by default. Define configHEAP_5_REGIONS to provide custom
    HeapRegion_t array.
  */
  #define HEAP_5_REGION_SETUP   1
  
  #ifndef configHEAP_5_REGIONS
    #define configHEAP_5_REGIONS xHeapRegions

    static uint8_t ucHeap[configTOTAL_HEAP_SIZE];

    static HeapRegion_t xHeapRegions[] = {
      { ucHeap, configTOTAL_HEAP_SIZE },
      { NULL,   0                     }
    };
  #else
    /* Global definition is provided to override default heap array */
    extern HeapRegion_t configHEAP_5_REGIONS[];
  #endif
#else
  /*
    The application already defined the array used for the FreeRTOS heap and
    called vPortDefineHeapRegions to initialize heap.
  */
  #define HEAP_5_REGION_SETUP   0
#endif /* configAPPLICATION_ALLOCATED_HEAP */
#endif /* USE_FreeRTOS_HEAP_5 */

/*
  Setup SVC to reset value.
*/
__STATIC_INLINE void SVC_Setup (void) {
#if (__ARM_ARCH_7A__ == 0U)
  /* Service Call interrupt might be configured before kernel start     */
  /* and when its priority is lower or equal to BASEPRI, svc intruction */
  /* causes a Hard Fault.                                               */
  NVIC_SetPriority (SVCall_IRQn, 0U);
#endif
}

/*
  Function macro used to retrieve semaphore count from ISR
*/
#ifndef uxSemaphoreGetCountFromISR
#define uxSemaphoreGetCountFromISR( xSemaphore ) uxQueueMessagesWaitingFromISR( ( QueueHandle_t ) ( xSemaphore ) )
#endif

/*
  Determine if CPU executes from interrupt context or if interrupts are masked.
*/
__STATIC_INLINE uint32_t IRQ_Context (void) {
  uint32_t irq;
  BaseType_t state;

  irq = 0U;

  if (IS_IRQ_MODE()) {
    /* Called from interrupt context */
    irq = 1U;
  }
  else {
    /* Get FreeRTOS scheduler state */
    state = xTaskGetSchedulerState();

    if (state != taskSCHEDULER_NOT_STARTED) {
      /* Scheduler was started */
      if (IS_IRQ_MASKED()) {
        /* Interrupts are masked */
        irq = 1U;
      }
    }
  }

  /* Return context, 0: thread context, 1: IRQ context */
  return (irq);
}


/* ==== Kernel Management Functions ==== */

/*
  Initialize the RTOS Kernel.
*/
osStatus_t osKernelInitialize (void) {
  osStatus_t stat;
  BaseType_t state;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else {
    state = xTaskGetSchedulerState();

    /* Initialize if scheduler not started and not initialized before */
    if ((state == taskSCHEDULER_NOT_STARTED) && (KernelState == osKernelInactive)) {
      #if defined(USE_TRACE_EVENT_RECORDER)
        /* Initialize the trace macro debugging output channel */
        EvrFreeRTOSSetup(0U);
      #endif
      #if defined(USE_FreeRTOS_HEAP_5) && (HEAP_5_REGION_SETUP == 1)
        /* Initialize the memory regions when using heap_5 variant */
        vPortDefineHeapRegions (configHEAP_5_REGIONS);
      #endif
      KernelState = osKernelReady;
      stat = osOK;
    } else {
      stat = osError;
    }
  }

  /* Return execution status */
  return (stat);
}

/*
  Get RTOS Kernel Information.
*/
osStatus_t osKernelGetInfo (osVersion_t *version, char *id_buf, uint32_t id_size) {

  if (version != NULL) {
    /* Version encoding is major.minor.rev: mmnnnrrrr dec */
    version->api    = KERNEL_VERSION;
    version->kernel = KERNEL_VERSION;
  }

  if ((id_buf != NULL) && (id_size != 0U)) {
    /* Buffer for retrieving identification string is provided */
    if (id_size > sizeof(KERNEL_ID)) {
      id_size = sizeof(KERNEL_ID);
    }
    /* Copy kernel identification string into provided buffer */
    memcpy(id_buf, KERNEL_ID, id_size);
  }

  /* Return execution status */
  return (osOK);
}

/*
  Get the current RTOS Kernel state.
*/
osKernelState_t osKernelGetState (void) {
  osKernelState_t state;

  switch (xTaskGetSchedulerState()) {
    case taskSCHEDULER_RUNNING:
      state = osKernelRunning;
      break;

    case taskSCHEDULER_SUSPENDED:
      state = osKernelLocked;
      break;

    case taskSCHEDULER_NOT_STARTED:
    default:
      if (KernelState == osKernelReady) {
        /* Ready, osKernelInitialize was already called */
        state = osKernelReady;
      } else {
        /* Not initialized */
        state = osKernelInactive;
      }
      break;
  }

  /* Return current state */
  return (state);
}

/*
  Start the RTOS Kernel scheduler.
*/
osStatus_t osKernelStart (void) {
  osStatus_t stat;
  BaseType_t state;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else {
    state = xTaskGetSchedulerState();

    /* Start scheduler if initialized and not started before */
    if ((state == taskSCHEDULER_NOT_STARTED) && (KernelState == osKernelReady)) {
      /* Ensure SVC priority is at the reset value */
      SVC_Setup();
      /* Change state to ensure correct API flow */
      KernelState = osKernelRunning;
      /* Start the kernel scheduler */
      vTaskStartScheduler();
      stat = osOK;
    } else {
      stat = osError;
    }
  }

  /* Return execution status */
  return (stat);
}

/*
  Lock the RTOS Kernel scheduler.
*/
int32_t osKernelLock (void) {
  int32_t lock;

  if (IRQ_Context() != 0U) {
    lock = (int32_t)osErrorISR;
  }
  else {
    switch (xTaskGetSchedulerState()) {
      case taskSCHEDULER_SUSPENDED:
        lock = 1;
        break;

      case taskSCHEDULER_RUNNING:
        vTaskSuspendAll();
        lock = 0;
        break;

      case taskSCHEDULER_NOT_STARTED:
      default:
        lock = (int32_t)osError;
        break;
    }
  }

  /* Return previous lock state */
  return (lock);
}

/*
  Unlock the RTOS Kernel scheduler.
*/
int32_t osKernelUnlock (void) {
  int32_t lock;

  if (IRQ_Context() != 0U) {
    lock = (int32_t)osErrorISR;
  }
  else {
    switch (xTaskGetSchedulerState()) {
      case taskSCHEDULER_SUSPENDED:
        lock = 1;

        if (xTaskResumeAll() != pdTRUE) {
          if (xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED) {
            lock = (int32_t)osError;
          }
        }
        break;

      case taskSCHEDULER_RUNNING:
        lock = 0;
        break;

      case taskSCHEDULER_NOT_STARTED:
      default:
        lock = (int32_t)osError;
        break;
    }
  }

  /* Return previous lock state */
  return (lock);
}

/*
  Restore the RTOS Kernel scheduler lock state.
*/
int32_t osKernelRestoreLock (int32_t lock) {

  if (IRQ_Context() != 0U) {
    lock = (int32_t)osErrorISR;
  }
  else {
    switch (xTaskGetSchedulerState()) {
      case taskSCHEDULER_SUSPENDED:
      case taskSCHEDULER_RUNNING:
        if (lock == 1) {
          vTaskSuspendAll();
        }
        else {
          if (lock != 0) {
            lock = (int32_t)osError;
          }
          else {
            if (xTaskResumeAll() != pdTRUE) {
              if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {
                lock = (int32_t)osError;
              }
            }
          }
        }
        break;

      case taskSCHEDULER_NOT_STARTED:
      default:
        lock = (int32_t)osError;
        break;
    }
  }

  /* Return new lock state */
  return (lock);
}

/*
  Get the RTOS kernel tick count.
*/
uint32_t osKernelGetTickCount (void) {
  TickType_t ticks;

  if (IRQ_Context() != 0U) {
    ticks = xTaskGetTickCountFromISR();
  } else {
    ticks = xTaskGetTickCount();
  }

  /* Return kernel tick count */
  return (ticks);
}

/*
  Get the RTOS kernel tick frequency.
*/
uint32_t osKernelGetTickFreq (void) {
  /* Return frequency in hertz */
  return (configTICK_RATE_HZ);
}

/*
  Get the RTOS kernel system timer count.
*/
uint32_t osKernelGetSysTimerCount (void) {
  TickType_t ticks;
  uint32_t val;

  FURI_CRITICAL_ENTER();

  ticks = xTaskGetTickCount();
  val   = OS_Tick_GetCount();

  /* Update tick count and timer value when timer overflows */
  if (OS_Tick_GetOverflow() != 0U) {
    val = OS_Tick_GetCount();
    ticks++;
  }
  val += ticks * OS_Tick_GetInterval();

  FURI_CRITICAL_EXIT();

  /* Return system timer count */
  return (val);
}

/*
  Get the RTOS kernel system timer frequency.
*/
uint32_t osKernelGetSysTimerFreq (void) {
  /* Return frequency in hertz */
  return (configCPU_CLOCK_HZ);
}


/* ==== Thread Management Functions ==== */

/*
  Create a thread and add it to Active Threads.

  Limitations:
  - The memory for control block and stack must be provided in the osThreadAttr_t
    structure in order to allocate object statically.
  - Attribute osThreadJoinable is not supported, NULL is returned if used.
*/
osThreadId_t osThreadNew (osThreadFunc_t func, void *argument, const osThreadAttr_t *attr) {
  const char *name;
  uint32_t stack;
  TaskHandle_t hTask;
  UBaseType_t prio;
  int32_t mem;

  hTask = NULL;

  if ((IRQ_Context() == 0U) && (func != NULL)) {
    stack = configMINIMAL_STACK_SIZE;
    prio  = (UBaseType_t)osPriorityNormal;

    name = NULL;
    mem  = -1;

    if (attr != NULL) {
      if (attr->name != NULL) {
        name = attr->name;
      }
      if (attr->priority != osPriorityNone) {
        prio = (UBaseType_t)attr->priority;
      }

      if ((prio < osPriorityIdle) || (prio > osPriorityISR) || ((attr->attr_bits & osThreadJoinable) == osThreadJoinable)) {
        /* Invalid priority or unsupported osThreadJoinable attribute used */
        return (NULL);
      }

      if (attr->stack_size > 0U) {
        /* In FreeRTOS stack is not in bytes, but in sizeof(StackType_t) which is 4 on ARM ports.       */
        /* Stack size should be therefore 4 byte aligned in order to avoid division caused side effects */
        stack = attr->stack_size / sizeof(StackType_t);
      }

      if ((attr->cb_mem    != NULL) && (attr->cb_size    >= sizeof(StaticTask_t)) &&
          (attr->stack_mem != NULL) && (attr->stack_size >  0U)) {
        /* The memory for control block and stack is provided, use static object */
        mem = 1;
      }
      else {
        if ((attr->cb_mem == NULL) && (attr->cb_size == 0U) && (attr->stack_mem == NULL)) {
          /* Control block and stack memory will be allocated from the dynamic pool */
          mem = 0;
        }
      }
    }
    else {
      mem = 0;
    }

    if (mem == 1) {
      #if (configSUPPORT_STATIC_ALLOCATION == 1)
        hTask = xTaskCreateStatic ((TaskFunction_t)func, name, stack, argument, prio, (StackType_t  *)attr->stack_mem,
                                                                                      (StaticTask_t *)attr->cb_mem);
      #endif
    }
    else {
      if (mem == 0) {
        #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
          if (xTaskCreate ((TaskFunction_t)func, name, (configSTACK_DEPTH_TYPE)stack, argument, prio, &hTask) != pdPASS) {
            hTask = NULL;
          }
        #endif
      }
    }
  }

  /* Return thread ID */
  return ((osThreadId_t)hTask);
}

/*
  Get name of a thread.
*/
const char *osThreadGetName (osThreadId_t thread_id) {
  TaskHandle_t hTask = (TaskHandle_t)thread_id;
  const char *name;

  if ((IRQ_Context() != 0U) || (hTask == NULL)) {
    name = NULL;
  } else if(osKernelGetState() == osKernelRunning) {
    name = pcTaskGetName (hTask);
  } else {
    name = NULL;
  }

  /* Return name as null-terminated string */
  return (name);
}

/*
  Return the thread ID of the current running thread.
*/
osThreadId_t osThreadGetId (void) {
  osThreadId_t id;

  id = (osThreadId_t)xTaskGetCurrentTaskHandle();

  /* Return thread ID */
  return (id);
}

/*
  Get current thread state of a thread.
*/
osThreadState_t osThreadGetState (osThreadId_t thread_id) {
  TaskHandle_t hTask = (TaskHandle_t)thread_id;
  osThreadState_t state;

  if ((IRQ_Context() != 0U) || (hTask == NULL)) {
    state = osThreadError;
  }
  else {
    switch (eTaskGetState (hTask)) {
      case eRunning:   state = osThreadRunning;    break;
      case eReady:     state = osThreadReady;      break;
      case eBlocked:
      case eSuspended: state = osThreadBlocked;    break;
      case eDeleted:   state = osThreadTerminated; break;
      case eInvalid:
      default:         state = osThreadError;      break;
    }
  }

  /* Return current thread state */
  return (state);
}

/*
  Get available stack space of a thread based on stack watermark recording during execution.
*/
uint32_t osThreadGetStackSpace (osThreadId_t thread_id) {
  TaskHandle_t hTask = (TaskHandle_t)thread_id;
  uint32_t sz;

  if ((IRQ_Context() != 0U) || (hTask == NULL)) {
    sz = 0U;
  } else {
    sz = (uint32_t)(uxTaskGetStackHighWaterMark(hTask) * sizeof(StackType_t));
  }

  /* Return remaining stack space in bytes */
  return (sz);
}

/*
  Change priority of a thread.
*/
osStatus_t osThreadSetPriority (osThreadId_t thread_id, osPriority_t priority) {
  TaskHandle_t hTask = (TaskHandle_t)thread_id;
  osStatus_t stat;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if ((hTask == NULL) || (priority < osPriorityIdle) || (priority > osPriorityISR)) {
    stat = osErrorParameter;
  }
  else {
    stat = osOK;
    vTaskPrioritySet (hTask, (UBaseType_t)priority);
  }

  /* Return execution status */
  return (stat);
}

/*
  Get current priority of a thread.
*/
osPriority_t osThreadGetPriority (osThreadId_t thread_id) {
  TaskHandle_t hTask = (TaskHandle_t)thread_id;
  osPriority_t prio;

  if ((IRQ_Context() != 0U) || (hTask == NULL)) {
    prio = osPriorityError;
  } else {
    prio = (osPriority_t)((int32_t)uxTaskPriorityGet (hTask));
  }

  /* Return current thread priority */
  return (prio);
}

/*
  Pass control to next thread that is in state READY.
*/
osStatus_t osThreadYield (void) {
  osStatus_t stat;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  } else {
    stat = osOK;
    taskYIELD();
  }

  /* Return execution status */
  return (stat);
}

#if (configUSE_OS2_THREAD_SUSPEND_RESUME == 1)
/*
  Suspend execution of a thread.
*/
osStatus_t osThreadSuspend (osThreadId_t thread_id) {
  TaskHandle_t hTask = (TaskHandle_t)thread_id;
  osStatus_t stat;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hTask == NULL) {
    stat = osErrorParameter;
  }
  else {
    stat = osOK;
    vTaskSuspend (hTask);
  }

  /* Return execution status */
  return (stat);
}

/*
  Resume execution of a thread.
*/
osStatus_t osThreadResume (osThreadId_t thread_id) {
  TaskHandle_t hTask = (TaskHandle_t)thread_id;
  osStatus_t stat;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hTask == NULL) {
    stat = osErrorParameter;
  }
  else {
    stat = osOK;
    vTaskResume (hTask);
  }

  /* Return execution status */
  return (stat);
}
#endif /* (configUSE_OS2_THREAD_SUSPEND_RESUME == 1) */

/*
  Terminate execution of current running thread.
*/
__NO_RETURN void osThreadExit (void) {
#ifndef USE_FreeRTOS_HEAP_1
  vTaskDelete (NULL);
#endif
  for (;;);
}

/*
  Terminate execution of a thread.
*/
osStatus_t osThreadTerminate (osThreadId_t thread_id) {
  TaskHandle_t hTask = (TaskHandle_t)thread_id;
  osStatus_t stat;
#ifndef USE_FreeRTOS_HEAP_1
  eTaskState tstate;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hTask == NULL) {
    stat = osErrorParameter;
  }
  else {
    tstate = eTaskGetState (hTask);

    if (tstate != eDeleted) {
      stat = osOK;
      vTaskDelete (hTask);
    } else {
      stat = osErrorResource;
    }
  }
#else
  stat = osError;
#endif

  /* Return execution status */
  return (stat);
}

/*
  Get number of active threads.
*/
uint32_t osThreadGetCount (void) {
  uint32_t count;

  if (IRQ_Context() != 0U) {
    count = 0U;
  } else {
    count = uxTaskGetNumberOfTasks();
  }

  /* Return number of active threads */
  return (count);
}

#if (configUSE_OS2_THREAD_ENUMERATE == 1)
/*
  Enumerate active threads.
*/
uint32_t osThreadEnumerate (osThreadId_t *thread_array, uint32_t array_items) {
  uint32_t i, count;
  TaskStatus_t *task;

  if ((IRQ_Context() != 0U) || (thread_array == NULL) || (array_items == 0U)) {
    count = 0U;
  } else {
    vTaskSuspendAll();

    /* Allocate memory on heap to temporarily store TaskStatus_t information */
    count = uxTaskGetNumberOfTasks();
    task  = pvPortMalloc (count * sizeof(TaskStatus_t));

    if (task != NULL) {
      /* Retrieve task status information */
      count = uxTaskGetSystemState (task, count, NULL);

      /* Copy handles from task status array into provided thread array */
      for (i = 0U; (i < count) && (i < array_items); i++) {
        thread_array[i] = (osThreadId_t)task[i].xHandle;
      }
      count = i;
    }
    (void)xTaskResumeAll();

    vPortFree (task);
  }

  /* Return number of enumerated threads */
  return (count);
}
#endif /* (configUSE_OS2_THREAD_ENUMERATE == 1) */


/* ==== Thread Flags Functions ==== */

#if (configUSE_OS2_THREAD_FLAGS == 1)
/*
  Set the specified Thread Flags of a thread.
*/
uint32_t osThreadFlagsSet (osThreadId_t thread_id, uint32_t flags) {
  TaskHandle_t hTask = (TaskHandle_t)thread_id;
  uint32_t rflags;
  BaseType_t yield;

  if ((hTask == NULL) || ((flags & THREAD_FLAGS_INVALID_BITS) != 0U)) {
    rflags = (uint32_t)osErrorParameter;
  }
  else {
    rflags = (uint32_t)osError;

    if (IRQ_Context() != 0U) {
      yield = pdFALSE;

      (void)xTaskNotifyIndexedFromISR (hTask, CMSIS_TASK_NOTIFY_INDEX, flags, eSetBits, &yield);
      (void)xTaskNotifyAndQueryIndexedFromISR (hTask, CMSIS_TASK_NOTIFY_INDEX, 0, eNoAction, &rflags, NULL);

      portYIELD_FROM_ISR (yield);
    }
    else {
      (void)xTaskNotifyIndexed (hTask, CMSIS_TASK_NOTIFY_INDEX, flags, eSetBits);
      (void)xTaskNotifyAndQueryIndexed (hTask, CMSIS_TASK_NOTIFY_INDEX, 0, eNoAction, &rflags);
    }
  }
  /* Return flags after setting */
  return (rflags);
}

/*
  Clear the specified Thread Flags of current running thread.
*/
uint32_t osThreadFlagsClear (uint32_t flags) {
  TaskHandle_t hTask;
  uint32_t rflags, cflags;

  if (IRQ_Context() != 0U) {
    rflags = (uint32_t)osErrorISR;
  }
  else if ((flags & THREAD_FLAGS_INVALID_BITS) != 0U) {
    rflags = (uint32_t)osErrorParameter;
  }
  else {
    hTask = xTaskGetCurrentTaskHandle();

    if (xTaskNotifyAndQueryIndexed (hTask, CMSIS_TASK_NOTIFY_INDEX, 0, eNoAction, &cflags) == pdPASS) {
      rflags = cflags;
      cflags &= ~flags;

      if (xTaskNotifyIndexed (hTask, CMSIS_TASK_NOTIFY_INDEX, cflags, eSetValueWithOverwrite) != pdPASS) {
        rflags = (uint32_t)osError;
      }
    }
    else {
      rflags = (uint32_t)osError;
    }
  }

  /* Return flags before clearing */
  return (rflags);
}

/*
  Get the current Thread Flags of current running thread.
*/
uint32_t osThreadFlagsGet (void) {
  TaskHandle_t hTask;
  uint32_t rflags;

  if (IRQ_Context() != 0U) {
    rflags = (uint32_t)osErrorISR;
  }
  else {
    hTask = xTaskGetCurrentTaskHandle();

    if (xTaskNotifyAndQueryIndexed (hTask, CMSIS_TASK_NOTIFY_INDEX, 0, eNoAction, &rflags) != pdPASS) {
      rflags = (uint32_t)osError;
    }
  }

  /* Return current flags */
  return (rflags);
}

/*
  Wait for one or more Thread Flags of the current running thread to become signaled.
*/
uint32_t osThreadFlagsWait (uint32_t flags, uint32_t options, uint32_t timeout) {
  uint32_t rflags, nval;
  uint32_t clear;
  TickType_t t0, td, tout;
  BaseType_t rval;

  if (IRQ_Context() != 0U) {
    rflags = (uint32_t)osErrorISR;
  }
  else if ((flags & THREAD_FLAGS_INVALID_BITS) != 0U) {
    rflags = (uint32_t)osErrorParameter;
  }
  else {
    if ((options & osFlagsNoClear) == osFlagsNoClear) {
      clear = 0U;
    } else {
      clear = flags;
    }

    rflags = 0U;
    tout   = timeout;

    t0 = xTaskGetTickCount();
    do {
      rval = xTaskNotifyWaitIndexed (CMSIS_TASK_NOTIFY_INDEX, 0, clear, &nval, tout);

      if (rval == pdPASS) {
        rflags &= flags;
        rflags |= nval;

        if ((options & osFlagsWaitAll) == osFlagsWaitAll) {
          if ((flags & rflags) == flags) {
            break;
          } else {
            if (timeout == 0U) {
              rflags = (uint32_t)osErrorResource;
              break;
            }
          }
        }
        else {
          if ((flags & rflags) != 0) {
            break;
          } else {
            if (timeout == 0U) {
              rflags = (uint32_t)osErrorResource;
              break;
            }
          }
        }

        /* Update timeout */
        td = xTaskGetTickCount() - t0;

        if (td > timeout) {
          tout  = 0;
        } else {
          tout = timeout - td;
        }
      }
      else {
        if (timeout == 0) {
          rflags = (uint32_t)osErrorResource;
        } else {
          rflags = (uint32_t)osErrorTimeout;
        }
      }
    }
    while (rval != pdFAIL);
  }

  /* Return flags before clearing */
  return (rflags);
}
#endif /* (configUSE_OS2_THREAD_FLAGS == 1) */


/* ==== Generic Wait Functions ==== */

/*
  Wait for Timeout (Time Delay).
*/
osStatus_t osDelay (uint32_t ticks) {
  osStatus_t stat;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else {
    stat = osOK;

    if (ticks != 0U) {
      vTaskDelay(ticks);
    }
  }

  /* Return execution status */
  return (stat);
}

/*
  Wait until specified time.
*/
osStatus_t osDelayUntil (uint32_t ticks) {
  TickType_t tcnt, delay;
  osStatus_t stat;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else {
    stat = osOK;
    tcnt = xTaskGetTickCount();

    /* Determine remaining number of ticks to delay */
    delay = (TickType_t)ticks - tcnt;

    /* Check if target tick has not expired */
    if((delay != 0U) && (0 == (delay >> (8 * sizeof(TickType_t) - 1)))) {
      if (xTaskDelayUntil (&tcnt, delay) == pdFALSE) {
        /* Did not delay */
        stat = osError;
      }
    }
    else
    {
      /* No delay or already expired */
      stat = osErrorParameter;
    }
  }

  /* Return execution status */
  return (stat);
}


/* ==== Timer Management Functions ==== */

#if (configUSE_OS2_TIMER == 1)

static void TimerCallback (TimerHandle_t hTimer) {
  TimerCallback_t *callb;

  /* Retrieve pointer to callback function and argument */
  callb = (TimerCallback_t *)pvTimerGetTimerID (hTimer);

  /* Remove dynamic allocation flag */
  callb = (TimerCallback_t *)((uint32_t)callb & ~1U);

  if (callb != NULL) {
    callb->func (callb->arg);
  }
}

/*
  Create and Initialize a timer.
*/
osTimerId_t osTimerNew (osTimerFunc_t func, osTimerType_t type, void *argument, const osTimerAttr_t *attr) {
  const char *name;
  TimerHandle_t hTimer;
  TimerCallback_t *callb;
  UBaseType_t reload;
  int32_t mem;
  uint32_t callb_dyn;

  hTimer = NULL;

  if ((IRQ_Context() == 0U) && (func != NULL)) {
    callb     = NULL;
    callb_dyn = 0U;

    #if (configSUPPORT_STATIC_ALLOCATION == 1)
      /* Static memory allocation is available: check if memory for control block */
      /* is provided and if it also contains space for callback and its argument  */
      if ((attr != NULL) && (attr->cb_mem != NULL)) {
        if (attr->cb_size >= (sizeof(StaticTimer_t) + sizeof(TimerCallback_t))) {
          callb = (TimerCallback_t *)((uint32_t)attr->cb_mem + sizeof(StaticTimer_t));
        }
      }
    #endif

    #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
      /* Dynamic memory allocation is available: if memory for callback and */
      /* its argument is not provided, allocate it from dynamic memory pool */
      if (callb == NULL) {
        callb = (TimerCallback_t *)pvPortMalloc (sizeof(TimerCallback_t));

        if (callb != NULL) {
          /* Callback memory was allocated from dynamic pool, set flag */
          callb_dyn = 1U;
        }
      }
    #endif

    if (callb != NULL) {
      callb->func = func;
      callb->arg  = argument;

      if (type == osTimerOnce) {
        reload = pdFALSE;
      } else {
        reload = pdTRUE;
      }

      mem  = -1;
      name = NULL;

      if (attr != NULL) {
        if (attr->name != NULL) {
          name = attr->name;
        }

        if ((attr->cb_mem != NULL) && (attr->cb_size >= sizeof(StaticTimer_t))) {
          /* The memory for control block is provided, use static object */
          mem = 1;
        }
        else {
          if ((attr->cb_mem == NULL) && (attr->cb_size == 0U)) {
            /* Control block will be allocated from the dynamic pool */
            mem = 0;
          }
        }
      }
      else {
        mem = 0;
      }
      /* Store callback memory dynamic allocation flag */
      callb = (TimerCallback_t *)((uint32_t)callb | callb_dyn);
      /*
        TimerCallback function is always provided as a callback and is used to call application
        specified function with its argument both stored in structure callb.
      */
      if (mem == 1) {
        #if (configSUPPORT_STATIC_ALLOCATION == 1)
          hTimer = xTimerCreateStatic (name, 1, reload, callb, TimerCallback, (StaticTimer_t *)attr->cb_mem);
        #endif
      }
      else {
        if (mem == 0) {
          #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
            hTimer = xTimerCreate (name, 1, reload, callb, TimerCallback);
          #endif
        }
      }

      #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
      if ((hTimer == NULL) && (callb != NULL) && (callb_dyn == 1U)) {
        /* Failed to create a timer, release allocated resources */
        callb = (TimerCallback_t *)((uint32_t)callb & ~1U);

        vPortFree (callb);
      }
      #endif
    }
  }

  /* Return timer ID */
  return ((osTimerId_t)hTimer);
}

/*
  Get name of a timer.
*/
const char *osTimerGetName (osTimerId_t timer_id) {
  TimerHandle_t hTimer = (TimerHandle_t)timer_id;
  const char *p;

  if ((IRQ_Context() != 0U) || (hTimer == NULL)) {
    p = NULL;
  } else {
    p = pcTimerGetName (hTimer);
  }

  /* Return name as null-terminated string */
  return (p);
}

/*
  Start or restart a timer.
*/
osStatus_t osTimerStart (osTimerId_t timer_id, uint32_t ticks) {
  TimerHandle_t hTimer = (TimerHandle_t)timer_id;
  osStatus_t stat;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hTimer == NULL) {
    stat = osErrorParameter;
  }
  else {
    if (xTimerChangePeriod (hTimer, ticks, portMAX_DELAY) == pdPASS) {
      stat = osOK;
    } else {
      stat = osErrorResource;
    }
  }

  /* Return execution status */
  return (stat);
}

/*
  Stop a timer.
*/
osStatus_t osTimerStop (osTimerId_t timer_id) {
  TimerHandle_t hTimer = (TimerHandle_t)timer_id;
  osStatus_t stat;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hTimer == NULL) {
    stat = osErrorParameter;
  }
  else {
    if (xTimerIsTimerActive (hTimer) == pdFALSE) {
      stat = osErrorResource;
    }
    else {
      if (xTimerStop (hTimer, portMAX_DELAY) == pdPASS) {
        stat = osOK;
      } else {
        stat = osError;
      }
    }
  }

  /* Return execution status */
  return (stat);
}

/*
  Check if a timer is running.
*/
uint32_t osTimerIsRunning (osTimerId_t timer_id) {
  TimerHandle_t hTimer = (TimerHandle_t)timer_id;
  uint32_t running;

  if ((IRQ_Context() != 0U) || (hTimer == NULL)) {
    running = 0U;
  } else {
    running = (uint32_t)xTimerIsTimerActive (hTimer);
  }

  /* Return 0: not running, 1: running */
  return (running);
}

/*
  Delete a timer.
*/
osStatus_t osTimerDelete (osTimerId_t timer_id) {
  TimerHandle_t hTimer = (TimerHandle_t)timer_id;
  osStatus_t stat;
#ifndef USE_FreeRTOS_HEAP_1
#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
  TimerCallback_t *callb;
#endif

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hTimer == NULL) {
    stat = osErrorParameter;
  }
  else {
    #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
    callb = (TimerCallback_t *)pvTimerGetTimerID (hTimer);
    #endif

    if (xTimerDelete (hTimer, portMAX_DELAY) == pdPASS) {
      #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
        if ((uint32_t)callb & 1U) {
          /* Callback memory was allocated from dynamic pool, clear flag */
          callb = (TimerCallback_t *)((uint32_t)callb & ~1U);

          /* Return allocated memory to dynamic pool */
          vPortFree (callb);
        }
      #endif
      stat = osOK;
    } else {
      stat = osErrorResource;
    }
  }
#else
  stat = osError;
#endif

  /* Return execution status */
  return (stat);
}
#endif /* (configUSE_OS2_TIMER == 1) */


/* ==== Event Flags Management Functions ==== */

/*
  Create and Initialize an Event Flags object.

  Limitations:
  - Event flags are limited to 24 bits.
*/
osEventFlagsId_t osEventFlagsNew (const osEventFlagsAttr_t *attr) {
  EventGroupHandle_t hEventGroup;
  int32_t mem;

  hEventGroup = NULL;

  if (IRQ_Context() == 0U) {
    mem = -1;

    if (attr != NULL) {
      if ((attr->cb_mem != NULL) && (attr->cb_size >= sizeof(StaticEventGroup_t))) {
        /* The memory for control block is provided, use static object */
        mem = 1;
      }
      else {
        if ((attr->cb_mem == NULL) && (attr->cb_size == 0U)) {
          /* Control block will be allocated from the dynamic pool */
          mem = 0;
        }
      }
    }
    else {
      mem = 0;
    }

    if (mem == 1) {
      #if (configSUPPORT_STATIC_ALLOCATION == 1)
      hEventGroup = xEventGroupCreateStatic (attr->cb_mem);
      #endif
    }
    else {
      if (mem == 0) {
        #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
          hEventGroup = xEventGroupCreate();
        #endif
      }
    }
  }

  /* Return event flags ID */
  return ((osEventFlagsId_t)hEventGroup);
}

/*
  Set the specified Event Flags.

  Limitations:
  - Event flags are limited to 24 bits.
*/
uint32_t osEventFlagsSet (osEventFlagsId_t ef_id, uint32_t flags) {
  EventGroupHandle_t hEventGroup = (EventGroupHandle_t)ef_id;
  uint32_t rflags;
  BaseType_t yield;

  if ((hEventGroup == NULL) || ((flags & EVENT_FLAGS_INVALID_BITS) != 0U)) {
    rflags = (uint32_t)osErrorParameter;
  }
  else if (IRQ_Context() != 0U) {
  #if (configUSE_OS2_EVENTFLAGS_FROM_ISR == 0)
    (void)yield;
    /* Enable timers and xTimerPendFunctionCall function to support osEventFlagsSet from ISR */
    rflags = (uint32_t)osErrorResource;
  #else
    yield = pdFALSE;

    if (xEventGroupSetBitsFromISR (hEventGroup, (EventBits_t)flags, &yield) == pdFAIL) {
      rflags = (uint32_t)osErrorResource;
    } else {
      rflags = flags;
      portYIELD_FROM_ISR (yield);
    }
  #endif
  }
  else {
    rflags = xEventGroupSetBits (hEventGroup, (EventBits_t)flags);
  }

  /* Return event flags after setting */
  return (rflags);
}

/*
  Clear the specified Event Flags.

  Limitations:
  - Event flags are limited to 24 bits.
*/
uint32_t osEventFlagsClear (osEventFlagsId_t ef_id, uint32_t flags) {
  EventGroupHandle_t hEventGroup = (EventGroupHandle_t)ef_id;
  uint32_t rflags;

  if ((hEventGroup == NULL) || ((flags & EVENT_FLAGS_INVALID_BITS) != 0U)) {
    rflags = (uint32_t)osErrorParameter;
  }
  else if (IRQ_Context() != 0U) {
  #if (configUSE_OS2_EVENTFLAGS_FROM_ISR == 0)
    /* Enable timers and xTimerPendFunctionCall function to support osEventFlagsSet from ISR */
    rflags = (uint32_t)osErrorResource;
  #else
    rflags = xEventGroupGetBitsFromISR (hEventGroup);

    if (xEventGroupClearBitsFromISR (hEventGroup, (EventBits_t)flags) == pdFAIL) {
      rflags = (uint32_t)osErrorResource;
    }
    else {
      /* xEventGroupClearBitsFromISR only registers clear operation in the timer command queue. */
      /* Yield is required here otherwise clear operation might not execute in the right order. */
      /* See https://github.com/FreeRTOS/FreeRTOS-Kernel/issues/93 for more info.               */
      portYIELD_FROM_ISR (pdTRUE);
    }
  #endif
  }
  else {
    rflags = xEventGroupClearBits (hEventGroup, (EventBits_t)flags);
  }

  /* Return event flags before clearing */
  return (rflags);
}

/*
  Get the current Event Flags.

  Limitations:
  - Event flags are limited to 24 bits.
*/
uint32_t osEventFlagsGet (osEventFlagsId_t ef_id) {
  EventGroupHandle_t hEventGroup = (EventGroupHandle_t)ef_id;
  uint32_t rflags;

  if (ef_id == NULL) {
    rflags = 0U;
  }
  else if (IRQ_Context() != 0U) {
    rflags = xEventGroupGetBitsFromISR (hEventGroup);
  }
  else {
    rflags = xEventGroupGetBits (hEventGroup);
  }

  /* Return current event flags */
  return (rflags);
}

/*
  Wait for one or more Event Flags to become signaled.

  Limitations:
  - Event flags are limited to 24 bits.
  - osEventFlagsWait cannot be called from an ISR.
*/
uint32_t osEventFlagsWait (osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout) {
  EventGroupHandle_t hEventGroup = (EventGroupHandle_t)ef_id;
  BaseType_t wait_all;
  BaseType_t exit_clr;
  uint32_t rflags;

  if ((hEventGroup == NULL) || ((flags & EVENT_FLAGS_INVALID_BITS) != 0U)) {
    rflags = (uint32_t)osErrorParameter;
  }
  else if (IRQ_Context() != 0U) {
    rflags = (uint32_t)osErrorISR;
  }
  else {
    if (options & osFlagsWaitAll) {
      wait_all = pdTRUE;
    } else {
      wait_all = pdFAIL;
    }

    if (options & osFlagsNoClear) {
      exit_clr = pdFAIL;
    } else {
      exit_clr = pdTRUE;
    }

    rflags = xEventGroupWaitBits (hEventGroup, (EventBits_t)flags, exit_clr, wait_all, (TickType_t)timeout);

    if (options & osFlagsWaitAll) {
      if ((flags & rflags) != flags) {
        if (timeout > 0U) {
          rflags = (uint32_t)osErrorTimeout;
        } else {
          rflags = (uint32_t)osErrorResource;
        }
      }
    }
    else {
      if ((flags & rflags) == 0U) {
        if (timeout > 0U) {
          rflags = (uint32_t)osErrorTimeout;
        } else {
          rflags = (uint32_t)osErrorResource;
        }
      }
    }
  }

  /* Return event flags before clearing */
  return (rflags);
}

/*
  Delete an Event Flags object.
*/
osStatus_t osEventFlagsDelete (osEventFlagsId_t ef_id) {
  EventGroupHandle_t hEventGroup = (EventGroupHandle_t)ef_id;
  osStatus_t stat;

#ifndef USE_FreeRTOS_HEAP_1
  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hEventGroup == NULL) {
    stat = osErrorParameter;
  }
  else {
    stat = osOK;
    vEventGroupDelete (hEventGroup);
  }
#else
  stat = osError;
#endif

  /* Return execution status */
  return (stat);
}


/* ==== Mutex Management Functions ==== */

#if (configUSE_OS2_MUTEX == 1)
/*
  Create and Initialize a Mutex object.

  Limitations:
  - Priority inherit protocol is used by default, osMutexPrioInherit attribute is ignored.
  - Robust mutex is not supported, NULL is returned if used.
*/
osMutexId_t osMutexNew (const osMutexAttr_t *attr) {
  SemaphoreHandle_t hMutex;
  uint32_t type;
  uint32_t rmtx;
  int32_t  mem;

  hMutex = NULL;

  if (IRQ_Context() == 0U) {
    if (attr != NULL) {
      type = attr->attr_bits;
    } else {
      type = 0U;
    }

    if ((type & osMutexRecursive) == osMutexRecursive) {
      rmtx = 1U;
    } else {
      rmtx = 0U;
    }

    if ((type & osMutexRobust) != osMutexRobust) {
      mem = -1;

      if (attr != NULL) {
        if ((attr->cb_mem != NULL) && (attr->cb_size >= sizeof(StaticSemaphore_t))) {
          /* The memory for control block is provided, use static object */
          mem = 1;
        }
        else {
          if ((attr->cb_mem == NULL) && (attr->cb_size == 0U)) {
            /* Control block will be allocated from the dynamic pool */
            mem = 0;
          }
        }
      }
      else {
        mem = 0;
      }

      if (mem == 1) {
        #if (configSUPPORT_STATIC_ALLOCATION == 1)
          if (rmtx != 0U) {
            #if (configUSE_RECURSIVE_MUTEXES == 1)
            hMutex = xSemaphoreCreateRecursiveMutexStatic (attr->cb_mem);
            #endif
          }
          else {
            hMutex = xSemaphoreCreateMutexStatic (attr->cb_mem);
          }
        #endif
      }
      else {
        if (mem == 0) {
          #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
            if (rmtx != 0U) {
              #if (configUSE_RECURSIVE_MUTEXES == 1)
              hMutex = xSemaphoreCreateRecursiveMutex ();
              #endif
            } else {
              hMutex = xSemaphoreCreateMutex ();
            }
          #endif
        }
      }

      #if (configQUEUE_REGISTRY_SIZE > 0)
      if (hMutex != NULL) {
        if ((attr != NULL) && (attr->name != NULL)) {
          /* Only non-NULL name objects are added to the Queue Registry */
          vQueueAddToRegistry (hMutex, attr->name);
        }
      }
      #endif

      if ((hMutex != NULL) && (rmtx != 0U)) {
        /* Set LSB as 'recursive mutex flag' */
        hMutex = (SemaphoreHandle_t)((uint32_t)hMutex | 1U);
      }
    }
  }

  /* Return mutex ID */
  return ((osMutexId_t)hMutex);
}

/*
  Acquire a Mutex or timeout if it is locked.
*/
osStatus_t osMutexAcquire (osMutexId_t mutex_id, uint32_t timeout) {
  SemaphoreHandle_t hMutex;
  osStatus_t stat;
  uint32_t rmtx;

  hMutex = (SemaphoreHandle_t)((uint32_t)mutex_id & ~1U);

  /* Extract recursive mutex flag */
  rmtx = (uint32_t)mutex_id & 1U;

  stat = osOK;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hMutex == NULL) {
    stat = osErrorParameter;
  }
  else {
    if (rmtx != 0U) {
      #if (configUSE_RECURSIVE_MUTEXES == 1)
      if (xSemaphoreTakeRecursive (hMutex, timeout) != pdPASS) {
        if (timeout != 0U) {
          stat = osErrorTimeout;
        } else {
          stat = osErrorResource;
        }
      }
      #endif
    }
    else {
      if (xSemaphoreTake (hMutex, timeout) != pdPASS) {
        if (timeout != 0U) {
          stat = osErrorTimeout;
        } else {
          stat = osErrorResource;
        }
      }
    }
  }

  /* Return execution status */
  return (stat);
}

/*
  Release a Mutex that was acquired by osMutexAcquire.
*/
osStatus_t osMutexRelease (osMutexId_t mutex_id) {
  SemaphoreHandle_t hMutex;
  osStatus_t stat;
  uint32_t rmtx;

  hMutex = (SemaphoreHandle_t)((uint32_t)mutex_id & ~1U);

  /* Extract recursive mutex flag */
  rmtx = (uint32_t)mutex_id & 1U;

  stat = osOK;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hMutex == NULL) {
    stat = osErrorParameter;
  }
  else {
    if (rmtx != 0U) {
      #if (configUSE_RECURSIVE_MUTEXES == 1)
      if (xSemaphoreGiveRecursive (hMutex) != pdPASS) {
        stat = osErrorResource;
      }
      #endif
    }
    else {
      if (xSemaphoreGive (hMutex) != pdPASS) {
        stat = osErrorResource;
      }
    }
  }

  /* Return execution status */
  return (stat);
}

/*
  Get Thread which owns a Mutex object.
*/
osThreadId_t osMutexGetOwner (osMutexId_t mutex_id) {
  SemaphoreHandle_t hMutex;
  osThreadId_t owner;

  hMutex = (SemaphoreHandle_t)((uint32_t)mutex_id & ~1U);

  if ((IRQ_Context() != 0U) || (hMutex == NULL)) {
    owner = NULL;
  } else {
    owner = (osThreadId_t)xSemaphoreGetMutexHolder (hMutex);
  }

  /* Return owner thread ID */
  return (owner);
}

/*
  Delete a Mutex object.
*/
osStatus_t osMutexDelete (osMutexId_t mutex_id) {
  osStatus_t stat;
#ifndef USE_FreeRTOS_HEAP_1
  SemaphoreHandle_t hMutex;

  hMutex = (SemaphoreHandle_t)((uint32_t)mutex_id & ~1U);

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hMutex == NULL) {
    stat = osErrorParameter;
  }
  else {
    #if (configQUEUE_REGISTRY_SIZE > 0)
    vQueueUnregisterQueue (hMutex);
    #endif
    stat = osOK;
    vSemaphoreDelete (hMutex);
  }
#else
  stat = osError;
#endif

  /* Return execution status */
  return (stat);
}
#endif /* (configUSE_OS2_MUTEX == 1) */


/* ==== Semaphore Management Functions ==== */

/*
  Create and Initialize a Semaphore object.
*/
osSemaphoreId_t osSemaphoreNew (uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr) {
  SemaphoreHandle_t hSemaphore;
  int32_t mem;

  hSemaphore = NULL;

  if ((IRQ_Context() == 0U) && (max_count > 0U) && (initial_count <= max_count)) {
    mem = -1;

    if (attr != NULL) {
      if ((attr->cb_mem != NULL) && (attr->cb_size >= sizeof(StaticSemaphore_t))) {
        /* The memory for control block is provided, use static object */
        mem = 1;
      }
      else {
        if ((attr->cb_mem == NULL) && (attr->cb_size == 0U)) {
          /* Control block will be allocated from the dynamic pool */
          mem = 0;
        }
      }
    }
    else {
      mem = 0;
    }

    if (mem != -1) {
      if (max_count == 1U) {
        if (mem == 1) {
          #if (configSUPPORT_STATIC_ALLOCATION == 1)
            hSemaphore = xSemaphoreCreateBinaryStatic ((StaticSemaphore_t *)attr->cb_mem);
          #endif
        }
        else {
          #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
            hSemaphore = xSemaphoreCreateBinary();
          #endif
        }

        if ((hSemaphore != NULL) && (initial_count != 0U)) {
          if (xSemaphoreGive (hSemaphore) != pdPASS) {
            vSemaphoreDelete (hSemaphore);
            hSemaphore = NULL;
          }
        }
      }
      else {
        if (mem == 1) {
          #if (configSUPPORT_STATIC_ALLOCATION == 1)
            hSemaphore = xSemaphoreCreateCountingStatic (max_count, initial_count, (StaticSemaphore_t *)attr->cb_mem);
          #endif
        }
        else {
          #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
            hSemaphore = xSemaphoreCreateCounting (max_count, initial_count);
          #endif
        }
      }
      
      #if (configQUEUE_REGISTRY_SIZE > 0)
      if (hSemaphore != NULL) {
        if ((attr != NULL) && (attr->name != NULL)) {
          /* Only non-NULL name objects are added to the Queue Registry */
          vQueueAddToRegistry (hSemaphore, attr->name);
        }
      }
      #endif
    }
  }

  /* Return semaphore ID */
  return ((osSemaphoreId_t)hSemaphore);
}

/*
  Acquire a Semaphore token or timeout if no tokens are available.
*/
osStatus_t osSemaphoreAcquire (osSemaphoreId_t semaphore_id, uint32_t timeout) {
  SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)semaphore_id;
  osStatus_t stat;
  BaseType_t yield;

  stat = osOK;

  if (hSemaphore == NULL) {
    stat = osErrorParameter;
  }
  else if (IRQ_Context() != 0U) {
    if (timeout != 0U) {
      stat = osErrorParameter;
    }
    else {
      yield = pdFALSE;

      if (xSemaphoreTakeFromISR (hSemaphore, &yield) != pdPASS) {
        stat = osErrorResource;
      } else {
        portYIELD_FROM_ISR (yield);
      }
    }
  }
  else {
    if (xSemaphoreTake (hSemaphore, (TickType_t)timeout) != pdPASS) {
      if (timeout != 0U) {
        stat = osErrorTimeout;
      } else {
        stat = osErrorResource;
      }
    }
  }

  /* Return execution status */
  return (stat);
}

/*
  Release a Semaphore token up to the initial maximum count.
*/
osStatus_t osSemaphoreRelease (osSemaphoreId_t semaphore_id) {
  SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)semaphore_id;
  osStatus_t stat;
  BaseType_t yield;

  stat = osOK;

  if (hSemaphore == NULL) {
    stat = osErrorParameter;
  }
  else if (IRQ_Context() != 0U) {
    yield = pdFALSE;

    if (xSemaphoreGiveFromISR (hSemaphore, &yield) != pdTRUE) {
      stat = osErrorResource;
    } else {
      portYIELD_FROM_ISR (yield);
    }
  }
  else {
    if (xSemaphoreGive (hSemaphore) != pdPASS) {
      stat = osErrorResource;
    }
  }

  /* Return execution status */
  return (stat);
}

/*
  Get current Semaphore token count.
*/
uint32_t osSemaphoreGetCount (osSemaphoreId_t semaphore_id) {
  SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)semaphore_id;
  uint32_t count;

  if (hSemaphore == NULL) {
    count = 0U;
  }
  else if (IRQ_Context() != 0U) {
    count = (uint32_t)uxSemaphoreGetCountFromISR (hSemaphore);
  } else {
    count = (uint32_t)uxSemaphoreGetCount (hSemaphore);
  }

  /* Return number of tokens */
  return (count);
}

/*
  Delete a Semaphore object.
*/
osStatus_t osSemaphoreDelete (osSemaphoreId_t semaphore_id) {
  SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)semaphore_id;
  osStatus_t stat;

#ifndef USE_FreeRTOS_HEAP_1
  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hSemaphore == NULL) {
    stat = osErrorParameter;
  }
  else {
    #if (configQUEUE_REGISTRY_SIZE > 0)
    vQueueUnregisterQueue (hSemaphore);
    #endif

    stat = osOK;
    vSemaphoreDelete (hSemaphore);
  }
#else
  stat = osError;
#endif

  /* Return execution status */
  return (stat);
}


/* ==== Message Queue Management Functions ==== */

/*
  Create and Initialize a Message Queue object.

  Limitations:
  - The memory for control block and and message data must be provided in the
    osThreadAttr_t structure in order to allocate object statically.
*/
osMessageQueueId_t osMessageQueueNew (uint32_t msg_count, uint32_t msg_size, const osMessageQueueAttr_t *attr) {
  QueueHandle_t hQueue;
  int32_t mem;

  hQueue = NULL;

  if ((IRQ_Context() == 0U) && (msg_count > 0U) && (msg_size > 0U)) {
    mem = -1;

    if (attr != NULL) {
      if ((attr->cb_mem != NULL) && (attr->cb_size >= sizeof(StaticQueue_t)) &&
          (attr->mq_mem != NULL) && (attr->mq_size >= (msg_count * msg_size))) {
        /* The memory for control block and message data is provided, use static object */
        mem = 1;
      }
      else {
        if ((attr->cb_mem == NULL) && (attr->cb_size == 0U) &&
            (attr->mq_mem == NULL) && (attr->mq_size == 0U)) {
          /* Control block will be allocated from the dynamic pool */
          mem = 0;
        }
      }
    }
    else {
      mem = 0;
    }

    if (mem == 1) {
      #if (configSUPPORT_STATIC_ALLOCATION == 1)
        hQueue = xQueueCreateStatic (msg_count, msg_size, attr->mq_mem, attr->cb_mem);
      #endif
    }
    else {
      if (mem == 0) {
        #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
          hQueue = xQueueCreate (msg_count, msg_size);
        #endif
      }
    }

    #if (configQUEUE_REGISTRY_SIZE > 0)
    if (hQueue != NULL) {
      if ((attr != NULL) && (attr->name != NULL)) {
        /* Only non-NULL name objects are added to the Queue Registry */
        vQueueAddToRegistry (hQueue, attr->name);
      }
    }
    #endif

  }

  /* Return message queue ID */
  return ((osMessageQueueId_t)hQueue);
}

/*
  Put a Message into a Queue or timeout if Queue is full.

  Limitations:
  - Message priority is ignored
*/
osStatus_t osMessageQueuePut (osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout) {
  QueueHandle_t hQueue = (QueueHandle_t)mq_id;
  osStatus_t stat;
  BaseType_t yield;

  (void)msg_prio; /* Message priority is ignored */

  stat = osOK;

  if (IRQ_Context() != 0U) {
    if ((hQueue == NULL) || (msg_ptr == NULL) || (timeout != 0U)) {
      stat = osErrorParameter;
    }
    else {
      yield = pdFALSE;

      if (xQueueSendToBackFromISR (hQueue, msg_ptr, &yield) != pdTRUE) {
        stat = osErrorResource;
      } else {
        portYIELD_FROM_ISR (yield);
      }
    }
  }
  else {
    if ((hQueue == NULL) || (msg_ptr == NULL)) {
      stat = osErrorParameter;
    }
    else {
      if (xQueueSendToBack (hQueue, msg_ptr, (TickType_t)timeout) != pdPASS) {
        if (timeout != 0U) {
          stat = osErrorTimeout;
        } else {
          stat = osErrorResource;
        }
      }
    }
  }

  /* Return execution status */
  return (stat);
}

/*
  Get a Message from a Queue or timeout if Queue is empty.

  Limitations:
  - Message priority is ignored
*/
osStatus_t osMessageQueueGet (osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout) {
  QueueHandle_t hQueue = (QueueHandle_t)mq_id;
  osStatus_t stat;
  BaseType_t yield;

  (void)msg_prio; /* Message priority is ignored */

  stat = osOK;

  if (IRQ_Context() != 0U) {
    if ((hQueue == NULL) || (msg_ptr == NULL) || (timeout != 0U)) {
      stat = osErrorParameter;
    }
    else {
      yield = pdFALSE;

      if (xQueueReceiveFromISR (hQueue, msg_ptr, &yield) != pdPASS) {
        stat = osErrorResource;
      } else {
        portYIELD_FROM_ISR (yield);
      }
    }
  }
  else {
    if ((hQueue == NULL) || (msg_ptr == NULL)) {
      stat = osErrorParameter;
    }
    else {
      if (xQueueReceive (hQueue, msg_ptr, (TickType_t)timeout) != pdPASS) {
        if (timeout != 0U) {
          stat = osErrorTimeout;
        } else {
          stat = osErrorResource;
        }
      }
    }
  }

  /* Return execution status */
  return (stat);
}

/*
  Get maximum number of messages in a Message Queue.
*/
uint32_t osMessageQueueGetCapacity (osMessageQueueId_t mq_id) {
  StaticQueue_t *mq = (StaticQueue_t *)mq_id;
  uint32_t capacity;

  if (mq == NULL) {
    capacity = 0U;
  } else {
    /* capacity = pxQueue->uxLength */
    capacity = mq->uxDummy4[1];
  }

  /* Return maximum number of messages */
  return (capacity);
}

/*
  Get maximum message size in a Message Queue.
*/
uint32_t osMessageQueueGetMsgSize (osMessageQueueId_t mq_id) {
  StaticQueue_t *mq = (StaticQueue_t *)mq_id;
  uint32_t size;

  if (mq == NULL) {
    size = 0U;
  } else {
    /* size = pxQueue->uxItemSize */
    size = mq->uxDummy4[2];
  }

  /* Return maximum message size */
  return (size);
}

/*
  Get number of queued messages in a Message Queue.
*/
uint32_t osMessageQueueGetCount (osMessageQueueId_t mq_id) {
  QueueHandle_t hQueue = (QueueHandle_t)mq_id;
  UBaseType_t count;

  if (hQueue == NULL) {
    count = 0U;
  }
  else if (IRQ_Context() != 0U) {
    count = uxQueueMessagesWaitingFromISR (hQueue);
  }
  else {
    count = uxQueueMessagesWaiting (hQueue);
  }

  /* Return number of queued messages */
  return ((uint32_t)count);
}

/*
  Get number of available slots for messages in a Message Queue.
*/
uint32_t osMessageQueueGetSpace (osMessageQueueId_t mq_id) {
  StaticQueue_t *mq = (StaticQueue_t *)mq_id;
  uint32_t space;
  uint32_t isrm;

  if (mq == NULL) {
    space = 0U;
  }
  else if (IRQ_Context() != 0U) {
    isrm = taskENTER_CRITICAL_FROM_ISR();

    /* space = pxQueue->uxLength - pxQueue->uxMessagesWaiting; */
    space = mq->uxDummy4[1] - mq->uxDummy4[0];

    taskEXIT_CRITICAL_FROM_ISR(isrm);
  }
  else {
    space = (uint32_t)uxQueueSpacesAvailable ((QueueHandle_t)mq);
  }

  /* Return number of available slots */
  return (space);
}

/*
  Reset a Message Queue to initial empty state.
*/
osStatus_t osMessageQueueReset (osMessageQueueId_t mq_id) {
  QueueHandle_t hQueue = (QueueHandle_t)mq_id;
  osStatus_t stat;

  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hQueue == NULL) {
    stat = osErrorParameter;
  }
  else {
    stat = osOK;
    (void)xQueueReset (hQueue);
  }

  /* Return execution status */
  return (stat);
}

/*
  Delete a Message Queue object.
*/
osStatus_t osMessageQueueDelete (osMessageQueueId_t mq_id) {
  QueueHandle_t hQueue = (QueueHandle_t)mq_id;
  osStatus_t stat;

#ifndef USE_FreeRTOS_HEAP_1
  if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else if (hQueue == NULL) {
    stat = osErrorParameter;
  }
  else {
    #if (configQUEUE_REGISTRY_SIZE > 0)
    vQueueUnregisterQueue (hQueue);
    #endif

    stat = osOK;
    vQueueDelete (hQueue);
  }
#else
  stat = osError;
#endif

  /* Return execution status */
  return (stat);
}


/* ==== Memory Pool Management Functions ==== */

#ifdef FREERTOS_MPOOL_H_
/* Static memory pool functions */
static void  FreeBlock   (MemPool_t *mp, void *block);
static void *AllocBlock  (MemPool_t *mp);
static void *CreateBlock (MemPool_t *mp);

/*
  Create and Initialize a Memory Pool object.
*/
osMemoryPoolId_t osMemoryPoolNew (uint32_t block_count, uint32_t block_size, const osMemoryPoolAttr_t *attr) {
  MemPool_t *mp;
  const char *name;
  int32_t mem_cb, mem_mp;
  uint32_t sz;

  if (IRQ_Context() != 0U) {
    mp = NULL;
  }
  else if ((block_count == 0U) || (block_size == 0U)) {
    mp = NULL;
  }
  else {
    mp = NULL;
    sz = MEMPOOL_ARR_SIZE (block_count, block_size);

    name = NULL;
    mem_cb = -1;
    mem_mp = -1;

    if (attr != NULL) {
      if (attr->name != NULL) {
        name = attr->name;
      }

      if ((attr->cb_mem != NULL) && (attr->cb_size >= sizeof(MemPool_t))) {
        /* Static control block is provided */
        mem_cb = 1;
      }
      else if ((attr->cb_mem == NULL) && (attr->cb_size == 0U)) {
        /* Allocate control block memory on heap */
        mem_cb = 0;
      }

      if ((attr->mp_mem == NULL) && (attr->mp_size == 0U)) {
        /* Allocate memory array on heap */
          mem_mp = 0;
      }
      else {
        if (attr->mp_mem != NULL) {
          /* Check if array is 4-byte aligned */
          if (((uint32_t)attr->mp_mem & 3U) == 0U) {
            /* Check if array big enough */
            if (attr->mp_size >= sz) {
              /* Static memory pool array is provided */
              mem_mp = 1;
            }
          }
        }
      }
    }
    else {
      /* Attributes not provided, allocate memory on heap */
      mem_cb = 0;
      mem_mp = 0;
    }

    if (mem_cb == 0) {
      mp = pvPortMalloc (sizeof(MemPool_t));
    } else {
      mp = attr->cb_mem;
    }

    if (mp != NULL) {
      /* Create a semaphore (max count == initial count == block_count) */
      #if (configSUPPORT_STATIC_ALLOCATION == 1)
        mp->sem = xSemaphoreCreateCountingStatic (block_count, block_count, &mp->mem_sem);
      #elif (configSUPPORT_DYNAMIC_ALLOCATION == 1)
        mp->sem = xSemaphoreCreateCounting (block_count, block_count);
      #else
        mp->sem = NULL;
      #endif

      if (mp->sem != NULL) {
        /* Setup memory array */
        if (mem_mp == 0) {
          mp->mem_arr = pvPortMalloc (sz);
        } else {
          mp->mem_arr = attr->mp_mem;
        }
      }
    }

    if ((mp != NULL) && (mp->mem_arr != NULL)) {
      /* Memory pool can be created */
      mp->head    = NULL;
      mp->mem_sz  = sz;
      mp->name    = name;
      mp->bl_sz   = block_size;
      mp->bl_cnt  = block_count;
      mp->n       = 0U;

      /* Set heap allocated memory flags */
      mp->status = MPOOL_STATUS;

      if (mem_cb == 0) {
        /* Control block on heap */
        mp->status |= 1U;
      }
      if (mem_mp == 0) {
        /* Memory array on heap */
        mp->status |= 2U;
      }
    }
    else {
      /* Memory pool cannot be created, release allocated resources */
      if ((mem_cb == 0) && (mp != NULL)) {
        /* Free control block memory */
        vPortFree (mp);
      }
      mp = NULL;
    }
  }

  /* Return memory pool ID */
  return (mp);
}

/*
  Get name of a Memory Pool object.
*/
const char *osMemoryPoolGetName (osMemoryPoolId_t mp_id) {
  MemPool_t *mp = (osMemoryPoolId_t)mp_id;
  const char *p;

  if (IRQ_Context() != 0U) {
    p = NULL;
  }
  else if (mp_id == NULL) {
    p = NULL;
  }
  else {
    p = mp->name;
  }

  /* Return name as null-terminated string */
  return (p);
}

/*
  Allocate a memory block from a Memory Pool.
*/
void *osMemoryPoolAlloc (osMemoryPoolId_t mp_id, uint32_t timeout) {
  MemPool_t *mp;
  void *block;
  uint32_t isrm;

  if (mp_id == NULL) {
    /* Invalid input parameters */
    block = NULL;
  }
  else {
    block = NULL;

    mp = (MemPool_t *)mp_id;

    if ((mp->status & MPOOL_STATUS) == MPOOL_STATUS) {
      if (IRQ_Context() != 0U) {
        if (timeout == 0U) {
          if (xSemaphoreTakeFromISR (mp->sem, NULL) == pdTRUE) {
            if ((mp->status & MPOOL_STATUS) == MPOOL_STATUS) {
              isrm  = taskENTER_CRITICAL_FROM_ISR();

              /* Get a block from the free-list */
              block = AllocBlock(mp);

              if (block == NULL) {
                /* List of free blocks is empty, 'create' new block */
                block = CreateBlock(mp);
              }

              taskEXIT_CRITICAL_FROM_ISR(isrm);
            }
          }
        }
      }
      else {
        if (xSemaphoreTake (mp->sem, (TickType_t)timeout) == pdTRUE) {
          if ((mp->status & MPOOL_STATUS) == MPOOL_STATUS) {
            taskENTER_CRITICAL();

            /* Get a block from the free-list */
            block = AllocBlock(mp);

            if (block == NULL) {
              /* List of free blocks is empty, 'create' new block */
              block = CreateBlock(mp);
            }

            taskEXIT_CRITICAL();
          }
        }
      }
    }
  }

  /* Return memory block address */
  return (block);
}

/*
  Return an allocated memory block back to a Memory Pool.
*/
osStatus_t osMemoryPoolFree (osMemoryPoolId_t mp_id, void *block) {
  MemPool_t *mp;
  osStatus_t stat;
  uint32_t isrm;
  BaseType_t yield;

  if ((mp_id == NULL) || (block == NULL)) {
    /* Invalid input parameters */
    stat = osErrorParameter;
  }
  else {
    mp = (MemPool_t *)mp_id;

    if ((mp->status & MPOOL_STATUS) != MPOOL_STATUS) {
      /* Invalid object status */
      stat = osErrorResource;
    }
    else if ((block < (void *)&mp->mem_arr[0]) || (block > (void*)&mp->mem_arr[mp->mem_sz-1])) {
      /* Block pointer outside of memory array area */
      stat = osErrorParameter;
    }
    else {
      stat = osOK;

      if (IRQ_Context() != 0U) {
        if (uxSemaphoreGetCountFromISR (mp->sem) == mp->bl_cnt) {
          stat = osErrorResource;
        }
        else {
          isrm = taskENTER_CRITICAL_FROM_ISR();

          /* Add block to the list of free blocks */
          FreeBlock(mp, block);

          taskEXIT_CRITICAL_FROM_ISR(isrm);

          yield = pdFALSE;
          xSemaphoreGiveFromISR (mp->sem, &yield);
          portYIELD_FROM_ISR (yield);
        }
      }
      else {
        if (uxSemaphoreGetCount (mp->sem) == mp->bl_cnt) {
          stat = osErrorResource;
        }
        else {
          taskENTER_CRITICAL();

          /* Add block to the list of free blocks */
          FreeBlock(mp, block);

          taskEXIT_CRITICAL();

          xSemaphoreGive (mp->sem);
        }
      }
    }
  }

  /* Return execution status */
  return (stat);
}

/*
  Get maximum number of memory blocks in a Memory Pool.
*/
uint32_t osMemoryPoolGetCapacity (osMemoryPoolId_t mp_id) {
  MemPool_t *mp;
  uint32_t  n;

  if (mp_id == NULL) {
    /* Invalid input parameters */
    n = 0U;
  }
  else {
    mp = (MemPool_t *)mp_id;

    if ((mp->status & MPOOL_STATUS) != MPOOL_STATUS) {
      /* Invalid object status */
      n = 0U;
    }
    else {
      n = mp->bl_cnt;
    }
  }

  /* Return maximum number of memory blocks */
  return (n);
}

/*
  Get memory block size in a Memory Pool.
*/
uint32_t osMemoryPoolGetBlockSize (osMemoryPoolId_t mp_id) {
  MemPool_t *mp;
  uint32_t  sz;

  if (mp_id == NULL) {
    /* Invalid input parameters */
    sz = 0U;
  }
  else {
    mp = (MemPool_t *)mp_id;

    if ((mp->status & MPOOL_STATUS) != MPOOL_STATUS) {
      /* Invalid object status */
      sz = 0U;
    }
    else {
      sz = mp->bl_sz;
    }
  }

  /* Return memory block size in bytes */
  return (sz);
}

/*
  Get number of memory blocks used in a Memory Pool.
*/
uint32_t osMemoryPoolGetCount (osMemoryPoolId_t mp_id) {
  MemPool_t *mp;
  uint32_t  n;

  if (mp_id == NULL) {
    /* Invalid input parameters */
    n = 0U;
  }
  else {
    mp = (MemPool_t *)mp_id;

    if ((mp->status & MPOOL_STATUS) != MPOOL_STATUS) {
      /* Invalid object status */
      n = 0U;
    }
    else {
      if (IRQ_Context() != 0U) {
        n = uxSemaphoreGetCountFromISR (mp->sem);
      } else {
        n = uxSemaphoreGetCount        (mp->sem);
      }

      n = mp->bl_cnt - n;
    }
  }

  /* Return number of memory blocks used */
  return (n);
}

/*
  Get number of memory blocks available in a Memory Pool.
*/
uint32_t osMemoryPoolGetSpace (osMemoryPoolId_t mp_id) {
  MemPool_t *mp;
  uint32_t  n;

  if (mp_id == NULL) {
    /* Invalid input parameters */
    n = 0U;
  }
  else {
    mp = (MemPool_t *)mp_id;

    if ((mp->status & MPOOL_STATUS) != MPOOL_STATUS) {
      /* Invalid object status */
      n = 0U;
    }
    else {
      if (IRQ_Context() != 0U) {
        n = uxSemaphoreGetCountFromISR (mp->sem);
      } else {
        n = uxSemaphoreGetCount        (mp->sem);
      }
    }
  }

  /* Return number of memory blocks available */
  return (n);
}

/*
  Delete a Memory Pool object.
*/
osStatus_t osMemoryPoolDelete (osMemoryPoolId_t mp_id) {
  MemPool_t *mp;
  osStatus_t stat;

  if (mp_id == NULL) {
    /* Invalid input parameters */
    stat = osErrorParameter;
  }
  else if (IRQ_Context() != 0U) {
    stat = osErrorISR;
  }
  else {
    mp = (MemPool_t *)mp_id;

    taskENTER_CRITICAL();

    /* Invalidate control block status */
    mp->status  = mp->status & 3U;

    /* Wake-up tasks waiting for pool semaphore */
    while (xSemaphoreGive (mp->sem) == pdTRUE);

    mp->head    = NULL;
    mp->bl_sz   = 0U;
    mp->bl_cnt  = 0U;

    if ((mp->status & 2U) != 0U) {
      /* Memory pool array allocated on heap */
      vPortFree (mp->mem_arr);
    }
    if ((mp->status & 1U) != 0U) {
      /* Memory pool control block allocated on heap */
      vPortFree (mp);
    }

    taskEXIT_CRITICAL();

    stat = osOK;
  }

  /* Return execution status */
  return (stat);
}

/*
  Create new block given according to the current block index.
*/
static void *CreateBlock (MemPool_t *mp) {
  MemPoolBlock_t *p = NULL;

  if (mp->n < mp->bl_cnt) {
    /* Unallocated blocks exist, set pointer to new block */
    p = (void *)(mp->mem_arr + (mp->bl_sz * mp->n));

    /* Increment block index */
    mp->n += 1U;
  }

  return (p);
}

/*
  Allocate a block by reading the list of free blocks.
*/
static void *AllocBlock (MemPool_t *mp) {
  MemPoolBlock_t *p = NULL;

  if (mp->head != NULL) {
    /* List of free block exists, get head block */
    p = mp->head;

    /* Head block is now next on the list */
    mp->head = p->next;
  }

  return (p);
}

/*
  Free block by putting it to the list of free blocks.
*/
static void FreeBlock (MemPool_t *mp, void *block) {
  MemPoolBlock_t *p = block;

  /* Store current head into block memory space */
  p->next = mp->head;

  /* Store current block as new head */
  mp->head = p;
}
#endif /* FREERTOS_MPOOL_H_ */
/*---------------------------------------------------------------------------*/

/* Callback function prototypes */
extern void vApplicationIdleHook (void);
extern void vApplicationMallocFailedHook (void);
extern void vApplicationDaemonTaskStartupHook (void);

/**
  Dummy implementation of the callback function vApplicationIdleHook().
*/
#if (configUSE_IDLE_HOOK == 1)
__WEAK void vApplicationIdleHook (void){}
#endif

/**
  Dummy implementation of the callback function vApplicationTickHook().
*/
#if (configUSE_TICK_HOOK == 1)
 __WEAK void vApplicationTickHook (void){}
#endif

/**
  Dummy implementation of the callback function vApplicationMallocFailedHook().
*/
#if (configUSE_MALLOC_FAILED_HOOK == 1)
__WEAK void vApplicationMallocFailedHook (void) {
  /* Assert when malloc failed hook is enabled but no application defined function exists */
  configASSERT(0);
}
#endif

/**
  Dummy implementation of the callback function vApplicationDaemonTaskStartupHook().
*/
#if (configUSE_DAEMON_TASK_STARTUP_HOOK == 1)
__WEAK void vApplicationDaemonTaskStartupHook (void){}
#endif

/**
  Dummy implementation of the callback function vApplicationStackOverflowHook().
*/
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
__WEAK void vApplicationStackOverflowHook (TaskHandle_t xTask, char *pcTaskName) {
  (void)xTask;
  (void)pcTaskName;

  /* Assert when stack overflow is enabled but no application defined function exists */
  configASSERT(0);
}
#endif

/*---------------------------------------------------------------------------*/
#if (configSUPPORT_STATIC_ALLOCATION == 1)
/*
  vApplicationGetIdleTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
__WEAK void vApplicationGetIdleTaskMemory (StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  /* Idle task control block and stack */
  static StaticTask_t Idle_TCB;
  static StackType_t  Idle_Stack[configMINIMAL_STACK_SIZE];

  *ppxIdleTaskTCBBuffer   = &Idle_TCB;
  *ppxIdleTaskStackBuffer = &Idle_Stack[0];
  *pulIdleTaskStackSize   = (uint32_t)configMINIMAL_STACK_SIZE;
}

/*
  vApplicationGetTimerTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
__WEAK void vApplicationGetTimerTaskMemory (StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  /* Timer task control block and stack */
  static StaticTask_t Timer_TCB;
  static StackType_t  Timer_Stack[configTIMER_TASK_STACK_DEPTH];

  *ppxTimerTaskTCBBuffer   = &Timer_TCB;
  *ppxTimerTaskStackBuffer = &Timer_Stack[0];
  *pulTimerTaskStackSize   = (uint32_t)configTIMER_TASK_STACK_DEPTH;
}
#endif
