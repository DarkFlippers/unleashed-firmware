/*
 * Copyright (c) 2013-2017 ARM Limited. All rights reserved.
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
 * $Date:        10. January 2017
 * $Revision:    V1.2
 *
 * Project:      CMSIS-RTOS API V1
 * Title:        cmsis_os_v1.c V1 module file
 *---------------------------------------------------------------------------*/

#include <string.h>
#include "cmsis_os.h"

#if (osCMSIS >= 0x20000U)


// Thread
osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument) {

  if (thread_def == NULL) {
    return (osThreadId)NULL;
  }
  return osThreadNew((osThreadFunc_t)thread_def->pthread, argument, &thread_def->attr);
}


// Signals

#define SignalMask ((1U<<osFeature_Signals)-1U)

int32_t osSignalSet (osThreadId thread_id, int32_t signals) {
  uint32_t flags;

  flags = osThreadFlagsSet(thread_id, (uint32_t)signals);
  if ((flags & 0x80000000U) != 0U) {
    return ((int32_t)0x80000000U);
  }
  return ((int32_t)(flags & ~((uint32_t)signals)));
}

int32_t osSignalClear (osThreadId thread_id, int32_t signals) {
  uint32_t flags;

  if (thread_id != osThreadGetId()) {
    return ((int32_t)0x80000000U);
  }
  flags = osThreadFlagsClear((uint32_t)signals);
  if ((flags & 0x80000000U) != 0U) {
    return ((int32_t)0x80000000U);
  }
  return ((int32_t)flags);
}

osEvent osSignalWait (int32_t signals, uint32_t millisec) {
  osEvent  event;
  uint32_t flags;

  if (signals != 0) {
    flags = osThreadFlagsWait((uint32_t)signals, osFlagsWaitAll, millisec);
  } else {
    flags = osThreadFlagsWait(SignalMask,        osFlagsWaitAny, millisec);
  }
  if ((flags > 0U) && (flags < 0x80000000U)) {
    event.status = osEventSignal;
    event.value.signals = (int32_t)flags;
  } else {
    switch ((int32_t)flags) {
      case osErrorResource:
        event.status = osOK;
        break;
      case osErrorTimeout:
        event.status = osEventTimeout;
        break;
      case osErrorParameter:
        event.status = osErrorValue;
        break;
      default:
        event.status = (osStatus)flags;
        break;
    }
  }
  return event;
}


// Timer
osTimerId osTimerCreate (const osTimerDef_t *timer_def, os_timer_type type, void *argument) {

  if (timer_def == NULL) {
    return (osTimerId)NULL;
  }
  return osTimerNew((osTimerFunc_t)timer_def->ptimer, type, argument, &timer_def->attr);
}


// Mutex
osMutexId osMutexCreate (const osMutexDef_t *mutex_def) {

  if (mutex_def == NULL) {
    return (osMutexId)NULL;
  }
  return osMutexNew(mutex_def);
}


// Semaphore

#if (defined (osFeature_Semaphore) && (osFeature_Semaphore != 0U))

osSemaphoreId osSemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count) {

  if (semaphore_def == NULL) {
    return (osSemaphoreId)NULL;
  }
  return osSemaphoreNew((uint32_t)count, (uint32_t)count, semaphore_def);
}

int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec) {
  osStatus_t status;
  uint32_t   count;

  status = osSemaphoreAcquire(semaphore_id, millisec);
  switch (status) {
    case osOK:
      count = osSemaphoreGetCount(semaphore_id);
      return ((int32_t)count + 1);
    case osErrorResource:
    case osErrorTimeout:
      return 0;
    default:
      break;
  }
  return -1;
}

#endif  // Semaphore


// Memory Pool

#if (defined(osFeature_Pool) && (osFeature_Pool != 0))

osPoolId osPoolCreate (const osPoolDef_t *pool_def) {

  if (pool_def == NULL) {
    return (osPoolId)NULL;
  }
  return ((osPoolId)(osMemoryPoolNew(pool_def->pool_sz, pool_def->item_sz, &pool_def->attr)));
}

void *osPoolAlloc (osPoolId pool_id) {
  return osMemoryPoolAlloc((osMemoryPoolId_t)pool_id, 0U);
}

void *osPoolCAlloc (osPoolId pool_id) {
  void    *block;
  uint32_t block_size;

  block_size = osMemoryPoolGetBlockSize((osMemoryPoolId_t)pool_id);
  if (block_size == 0U) {
    return NULL;
  }
  block = osMemoryPoolAlloc((osMemoryPoolId_t)pool_id, 0U);
  if (block != NULL) {
    memset(block, 0, block_size);
  }
  return block;
}

osStatus osPoolFree (osPoolId pool_id, void *block) {
  return osMemoryPoolFree((osMemoryPoolId_t)pool_id, block);
}

#endif  // Memory Pool


// Message Queue

#if (defined(osFeature_MessageQ) && (osFeature_MessageQ != 0))

osMessageQId osMessageCreate (const osMessageQDef_t *queue_def, osThreadId thread_id) {
  (void)thread_id;

  if (queue_def == NULL) {
    return (osMessageQId)NULL;
  }
  return ((osMessageQId)(osMessageQueueNew(queue_def->queue_sz, sizeof(uint32_t), &queue_def->attr)));
}

osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec) {
  return osMessageQueuePut((osMessageQueueId_t)queue_id, &info, 0U, millisec);
}

osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec) {
  osStatus_t status;
  osEvent    event;
  uint32_t   message;

  status = osMessageQueueGet((osMessageQueueId_t)queue_id, &message, NULL, millisec);
  switch (status) {
    case osOK:
      event.status = osEventMessage;
      event.value.v = message;
      break;
    case osErrorResource:
      event.status = osOK;
      break;
    case osErrorTimeout:
      event.status = osEventTimeout;
      break;
    default:
      event.status = status;
      break;
  }
  return event;
}

#endif  // Message Queue


// Mail Queue

#if (defined(osFeature_MailQ) && (osFeature_MailQ != 0))

typedef struct os_mail_queue_s {
  osMemoryPoolId_t   mp_id;
  osMessageQueueId_t mq_id;
} os_mail_queue_t;

osMailQId osMailCreate (const osMailQDef_t *queue_def, osThreadId thread_id) {
  os_mail_queue_t *ptr;
  (void)thread_id;

  if (queue_def == NULL) {
    return (osMailQId)NULL;
  }

  ptr = queue_def->mail;
  if (ptr == NULL) {
    return (osMailQId)NULL;
  }

  ptr->mp_id = osMemoryPoolNew  (queue_def->queue_sz, queue_def->item_sz, &queue_def->mp_attr);
  ptr->mq_id = osMessageQueueNew(queue_def->queue_sz, sizeof(void *), &queue_def->mq_attr);
  if ((ptr->mp_id == (osMemoryPoolId_t)NULL) || (ptr->mq_id == (osMessageQueueId_t)NULL)) {
    if (ptr->mp_id != (osMemoryPoolId_t)NULL) {
      osMemoryPoolDelete(ptr->mp_id);
    }
    if (ptr->mq_id != (osMessageQueueId_t)NULL) {
      osMessageQueueDelete(ptr->mq_id);
    }
    return (osMailQId)NULL;
  }

  return (osMailQId)ptr;
}

void *osMailAlloc (osMailQId queue_id, uint32_t millisec) {
  os_mail_queue_t *ptr = (os_mail_queue_t *)queue_id;

  if (ptr == NULL) {
    return NULL;
  }
  return osMemoryPoolAlloc(ptr->mp_id, millisec);
}

void *osMailCAlloc (osMailQId queue_id, uint32_t millisec) {
  os_mail_queue_t *ptr = (os_mail_queue_t *)queue_id;
  void            *block;
  uint32_t         block_size;

  if (ptr == NULL) {
    return NULL;
  }
  block_size = osMemoryPoolGetBlockSize(ptr->mp_id);
  if (block_size == 0U) {
    return NULL;
  }
  block = osMemoryPoolAlloc(ptr->mp_id, millisec);
  if (block != NULL) {
    memset(block, 0, block_size);
  }

  return block;

}

osStatus osMailPut (osMailQId queue_id, const void *mail) {
  os_mail_queue_t *ptr = (os_mail_queue_t *)queue_id;

  if (ptr == NULL) {
    return osErrorParameter;
  }
  if (mail == NULL) {
    return osErrorValue;
  }
  return osMessageQueuePut(ptr->mq_id, &mail, 0U, 0U);
}

osEvent osMailGet (osMailQId queue_id, uint32_t millisec) {
  os_mail_queue_t *ptr = (os_mail_queue_t *)queue_id;
  osStatus_t       status;
  osEvent          event;
  void            *mail;

  if (ptr == NULL) {
    event.status = osErrorParameter;
    return event;
  }

  status = osMessageQueueGet(ptr->mq_id, &mail, NULL, millisec);
  switch (status) {
    case osOK:
      event.status = osEventMail;
      event.value.p = mail;
      break;
    case osErrorResource:
      event.status = osOK;
      break;
    case osErrorTimeout:
      event.status = osEventTimeout;
      break;
    default:
      event.status = status;
      break;
  }
  return event;
}

osStatus osMailFree (osMailQId queue_id, void *mail) {
  os_mail_queue_t *ptr = (os_mail_queue_t *)queue_id;

  if (ptr == NULL) {
    return osErrorParameter;
  }
  if (mail == NULL) {
    return osErrorValue;
  }
  return osMemoryPoolFree(ptr->mp_id, mail);
}

#endif  // Mail Queue


#endif  // osCMSIS
