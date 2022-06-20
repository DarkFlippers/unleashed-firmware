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

#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "timers.h"                     // ARM.FreeRTOS::RTOS:Timers
#include "queue.h"

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

#define THREAD_FLAGS_INVALID_BITS (~((1UL << MAX_BITS_TASK_NOTIFY)  - 1U))

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
  Get the RTOS kernel system timer frequency.
*/
uint32_t osKernelGetSysTimerFreq (void) {
  /* Return frequency in hertz */
  return (configCPU_CLOCK_HZ);
}

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
