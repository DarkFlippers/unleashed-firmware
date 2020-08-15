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

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#ifndef INC_FREERTOS_H
	#error "include FreeRTOS.h" must appear in source files before "include semphr.h"
#endif

#include "queue.h"

typedef QueueHandle_t SemaphoreHandle_t;

#define semBINARY_SEMAPHORE_QUEUE_LENGTH	( ( uint8_t ) 1U )
#define semSEMAPHORE_QUEUE_ITEM_LENGTH		( ( uint8_t ) 0U )
#define semGIVE_BLOCK_TIME					( ( TickType_t ) 0U )


/**
 * semphr. h
 * <pre>vSemaphoreCreateBinary( SemaphoreHandle_t xSemaphore )</pre>
 *
 * In many usage scenarios it is faster and more memory efficient to use a
 * direct to task notification in place of a binary semaphore!
 * http://www.freertos.org/RTOS-task-notifications.html
 *
 * This old vSemaphoreCreateBinary() macro is now deprecated in favour of the
 * xSemaphoreCreateBinary() function.  Note that binary semaphores created using
 * the vSemaphoreCreateBinary() macro are created in a state such that the
 * first call to 'take' the semaphore would pass, whereas binary semaphores
 * created using xSemaphoreCreateBinary() are created in a state such that the
 * the semaphore must first be 'given' before it can be 'taken'.
 *
 * <i>Macro</i> that implements a semaphore by using the existing queue mechanism.
 * The queue length is 1 as this is a binary semaphore.  The data size is 0
 * as we don't want to actually store any data - we just want to know if the
 * queue is empty or full.
 *
 * This type of semaphore can be used for pure synchronisation between tasks or
 * between an interrupt and a task.  The semaphore need not be given back once
 * obtained, so one task/interrupt can continuously 'give' the semaphore while
 * another continuously 'takes' the semaphore.  For this reason this type of
 * semaphore does not use a priority inheritance mechanism.  For an alternative
 * that does use priority inheritance see xSemaphoreCreateMutex().
 *
 * @param xSemaphore Handle to the created semaphore.  Should be of type SemaphoreHandle_t.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xSemaphore = NULL;

 void vATask( void * pvParameters )
 {
    // Semaphore cannot be used before a call to vSemaphoreCreateBinary ().
    // This is a macro so pass the variable in directly.
    vSemaphoreCreateBinary( xSemaphore );

    if( xSemaphore != NULL )
    {
        // The semaphore was created successfully.
        // The semaphore can now be used.
    }
 }
 </pre>
 * \defgroup vSemaphoreCreateBinary vSemaphoreCreateBinary
 * \ingroup Semaphores
 */
#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
	#define vSemaphoreCreateBinary( xSemaphore )																							\
		{																																	\
			( xSemaphore ) = xQueueGenericCreate( ( UBaseType_t ) 1, semSEMAPHORE_QUEUE_ITEM_LENGTH, queueQUEUE_TYPE_BINARY_SEMAPHORE );	\
			if( ( xSemaphore ) != NULL )																									\
			{																																\
				( void ) xSemaphoreGive( ( xSemaphore ) );																					\
			}																																\
		}
#endif

/**
 * semphr. h
 * <pre>SemaphoreHandle_t xSemaphoreCreateBinary( void )</pre>
 *
 * Creates a new binary semaphore instance, and returns a handle by which the
 * new semaphore can be referenced.
 *
 * In many usage scenarios it is faster and more memory efficient to use a
 * direct to task notification in place of a binary semaphore!
 * http://www.freertos.org/RTOS-task-notifications.html
 *
 * Internally, within the FreeRTOS implementation, binary semaphores use a block
 * of memory, in which the semaphore structure is stored.  If a binary semaphore
 * is created using xSemaphoreCreateBinary() then the required memory is
 * automatically dynamically allocated inside the xSemaphoreCreateBinary()
 * function.  (see http://www.freertos.org/a00111.html).  If a binary semaphore
 * is created using xSemaphoreCreateBinaryStatic() then the application writer
 * must provide the memory.  xSemaphoreCreateBinaryStatic() therefore allows a
 * binary semaphore to be created without using any dynamic memory allocation.
 *
 * The old vSemaphoreCreateBinary() macro is now deprecated in favour of this
 * xSemaphoreCreateBinary() function.  Note that binary semaphores created using
 * the vSemaphoreCreateBinary() macro are created in a state such that the
 * first call to 'take' the semaphore would pass, whereas binary semaphores
 * created using xSemaphoreCreateBinary() are created in a state such that the
 * the semaphore must first be 'given' before it can be 'taken'.
 *
 * This type of semaphore can be used for pure synchronisation between tasks or
 * between an interrupt and a task.  The semaphore need not be given back once
 * obtained, so one task/interrupt can continuously 'give' the semaphore while
 * another continuously 'takes' the semaphore.  For this reason this type of
 * semaphore does not use a priority inheritance mechanism.  For an alternative
 * that does use priority inheritance see xSemaphoreCreateMutex().
 *
 * @return Handle to the created semaphore, or NULL if the memory required to
 * hold the semaphore's data structures could not be allocated.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xSemaphore = NULL;

 void vATask( void * pvParameters )
 {
    // Semaphore cannot be used before a call to xSemaphoreCreateBinary().
    // This is a macro so pass the variable in directly.
    xSemaphore = xSemaphoreCreateBinary();

    if( xSemaphore != NULL )
    {
        // The semaphore was created successfully.
        // The semaphore can now be used.
    }
 }
 </pre>
 * \defgroup xSemaphoreCreateBinary xSemaphoreCreateBinary
 * \ingroup Semaphores
 */
#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
	#define xSemaphoreCreateBinary() xQueueGenericCreate( ( UBaseType_t ) 1, semSEMAPHORE_QUEUE_ITEM_LENGTH, queueQUEUE_TYPE_BINARY_SEMAPHORE )
#endif

/**
 * semphr. h
 * <pre>SemaphoreHandle_t xSemaphoreCreateBinaryStatic( StaticSemaphore_t *pxSemaphoreBuffer )</pre>
 *
 * Creates a new binary semaphore instance, and returns a handle by which the
 * new semaphore can be referenced.
 *
 * NOTE: In many usage scenarios it is faster and more memory efficient to use a
 * direct to task notification in place of a binary semaphore!
 * http://www.freertos.org/RTOS-task-notifications.html
 *
 * Internally, within the FreeRTOS implementation, binary semaphores use a block
 * of memory, in which the semaphore structure is stored.  If a binary semaphore
 * is created using xSemaphoreCreateBinary() then the required memory is
 * automatically dynamically allocated inside the xSemaphoreCreateBinary()
 * function.  (see http://www.freertos.org/a00111.html).  If a binary semaphore
 * is created using xSemaphoreCreateBinaryStatic() then the application writer
 * must provide the memory.  xSemaphoreCreateBinaryStatic() therefore allows a
 * binary semaphore to be created without using any dynamic memory allocation.
 *
 * This type of semaphore can be used for pure synchronisation between tasks or
 * between an interrupt and a task.  The semaphore need not be given back once
 * obtained, so one task/interrupt can continuously 'give' the semaphore while
 * another continuously 'takes' the semaphore.  For this reason this type of
 * semaphore does not use a priority inheritance mechanism.  For an alternative
 * that does use priority inheritance see xSemaphoreCreateMutex().
 *
 * @param pxSemaphoreBuffer Must point to a variable of type StaticSemaphore_t,
 * which will then be used to hold the semaphore's data structure, removing the
 * need for the memory to be allocated dynamically.
 *
 * @return If the semaphore is created then a handle to the created semaphore is
 * returned.  If pxSemaphoreBuffer is NULL then NULL is returned.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xSemaphore = NULL;
 StaticSemaphore_t xSemaphoreBuffer;

 void vATask( void * pvParameters )
 {
    // Semaphore cannot be used before a call to xSemaphoreCreateBinary().
    // The semaphore's data structures will be placed in the xSemaphoreBuffer
    // variable, the address of which is passed into the function.  The
    // function's parameter is not NULL, so the function will not attempt any
    // dynamic memory allocation, and therefore the function will not return
    // return NULL.
    xSemaphore = xSemaphoreCreateBinary( &xSemaphoreBuffer );

    // Rest of task code goes here.
 }
 </pre>
 * \defgroup xSemaphoreCreateBinaryStatic xSemaphoreCreateBinaryStatic
 * \ingroup Semaphores
 */
#if( configSUPPORT_STATIC_ALLOCATION == 1 )
	#define xSemaphoreCreateBinaryStatic( pxStaticSemaphore ) xQueueGenericCreateStatic( ( UBaseType_t ) 1, semSEMAPHORE_QUEUE_ITEM_LENGTH, NULL, pxStaticSemaphore, queueQUEUE_TYPE_BINARY_SEMAPHORE )
#endif /* configSUPPORT_STATIC_ALLOCATION */

/**
 * semphr. h
 * <pre>xSemaphoreTake(
 *                   SemaphoreHandle_t xSemaphore,
 *                   TickType_t xBlockTime
 *               )</pre>
 *
 * <i>Macro</i> to obtain a semaphore.  The semaphore must have previously been
 * created with a call to xSemaphoreCreateBinary(), xSemaphoreCreateMutex() or
 * xSemaphoreCreateCounting().
 *
 * @param xSemaphore A handle to the semaphore being taken - obtained when
 * the semaphore was created.
 *
 * @param xBlockTime The time in ticks to wait for the semaphore to become
 * available.  The macro portTICK_PERIOD_MS can be used to convert this to a
 * real time.  A block time of zero can be used to poll the semaphore.  A block
 * time of portMAX_DELAY can be used to block indefinitely (provided
 * INCLUDE_vTaskSuspend is set to 1 in FreeRTOSConfig.h).
 *
 * @return pdTRUE if the semaphore was obtained.  pdFALSE
 * if xBlockTime expired without the semaphore becoming available.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xSemaphore = NULL;

 // A task that creates a semaphore.
 void vATask( void * pvParameters )
 {
    // Create the semaphore to guard a shared resource.
    xSemaphore = xSemaphoreCreateBinary();
 }

 // A task that uses the semaphore.
 void vAnotherTask( void * pvParameters )
 {
    // ... Do other things.

    if( xSemaphore != NULL )
    {
        // See if we can obtain the semaphore.  If the semaphore is not available
        // wait 10 ticks to see if it becomes free.
        if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        {
            // We were able to obtain the semaphore and can now access the
            // shared resource.

            // ...

            // We have finished accessing the shared resource.  Release the
            // semaphore.
            xSemaphoreGive( xSemaphore );
        }
        else
        {
            // We could not obtain the semaphore and can therefore not access
            // the shared resource safely.
        }
    }
 }
 </pre>
 * \defgroup xSemaphoreTake xSemaphoreTake
 * \ingroup Semaphores
 */
#define xSemaphoreTake( xSemaphore, xBlockTime )		xQueueSemaphoreTake( ( xSemaphore ), ( xBlockTime ) )

/**
 * semphr. h
 * xSemaphoreTakeRecursive(
 *                          SemaphoreHandle_t xMutex,
 *                          TickType_t xBlockTime
 *                        )
 *
 * <i>Macro</i> to recursively obtain, or 'take', a mutex type semaphore.
 * The mutex must have previously been created using a call to
 * xSemaphoreCreateRecursiveMutex();
 *
 * configUSE_RECURSIVE_MUTEXES must be set to 1 in FreeRTOSConfig.h for this
 * macro to be available.
 *
 * This macro must not be used on mutexes created using xSemaphoreCreateMutex().
 *
 * A mutex used recursively can be 'taken' repeatedly by the owner. The mutex
 * doesn't become available again until the owner has called
 * xSemaphoreGiveRecursive() for each successful 'take' request.  For example,
 * if a task successfully 'takes' the same mutex 5 times then the mutex will
 * not be available to any other task until it has also  'given' the mutex back
 * exactly five times.
 *
 * @param xMutex A handle to the mutex being obtained.  This is the
 * handle returned by xSemaphoreCreateRecursiveMutex();
 *
 * @param xBlockTime The time in ticks to wait for the semaphore to become
 * available.  The macro portTICK_PERIOD_MS can be used to convert this to a
 * real time.  A block time of zero can be used to poll the semaphore.  If
 * the task already owns the semaphore then xSemaphoreTakeRecursive() will
 * return immediately no matter what the value of xBlockTime.
 *
 * @return pdTRUE if the semaphore was obtained.  pdFALSE if xBlockTime
 * expired without the semaphore becoming available.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xMutex = NULL;

 // A task that creates a mutex.
 void vATask( void * pvParameters )
 {
    // Create the mutex to guard a shared resource.
    xMutex = xSemaphoreCreateRecursiveMutex();
 }

 // A task that uses the mutex.
 void vAnotherTask( void * pvParameters )
 {
    // ... Do other things.

    if( xMutex != NULL )
    {
        // See if we can obtain the mutex.  If the mutex is not available
        // wait 10 ticks to see if it becomes free.
        if( xSemaphoreTakeRecursive( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        {
            // We were able to obtain the mutex and can now access the
            // shared resource.

            // ...
            // For some reason due to the nature of the code further calls to
            // xSemaphoreTakeRecursive() are made on the same mutex.  In real
            // code these would not be just sequential calls as this would make
            // no sense.  Instead the calls are likely to be buried inside
            // a more complex call structure.
            xSemaphoreTakeRecursive( xMutex, ( TickType_t ) 10 );
            xSemaphoreTakeRecursive( xMutex, ( TickType_t ) 10 );

            // The mutex has now been 'taken' three times, so will not be
            // available to another task until it has also been given back
            // three times.  Again it is unlikely that real code would have
            // these calls sequentially, but instead buried in a more complex
            // call structure.  This is just for illustrative purposes.
            xSemaphoreGiveRecursive( xMutex );
            xSemaphoreGiveRecursive( xMutex );
            xSemaphoreGiveRecursive( xMutex );

            // Now the mutex can be taken by other tasks.
        }
        else
        {
            // We could not obtain the mutex and can therefore not access
            // the shared resource safely.
        }
    }
 }
 </pre>
 * \defgroup xSemaphoreTakeRecursive xSemaphoreTakeRecursive
 * \ingroup Semaphores
 */
#if( configUSE_RECURSIVE_MUTEXES == 1 )
	#define xSemaphoreTakeRecursive( xMutex, xBlockTime )	xQueueTakeMutexRecursive( ( xMutex ), ( xBlockTime ) )
#endif

/**
 * semphr. h
 * <pre>xSemaphoreGive( SemaphoreHandle_t xSemaphore )</pre>
 *
 * <i>Macro</i> to release a semaphore.  The semaphore must have previously been
 * created with a call to xSemaphoreCreateBinary(), xSemaphoreCreateMutex() or
 * xSemaphoreCreateCounting(). and obtained using sSemaphoreTake().
 *
 * This macro must not be used from an ISR.  See xSemaphoreGiveFromISR () for
 * an alternative which can be used from an ISR.
 *
 * This macro must also not be used on semaphores created using
 * xSemaphoreCreateRecursiveMutex().
 *
 * @param xSemaphore A handle to the semaphore being released.  This is the
 * handle returned when the semaphore was created.
 *
 * @return pdTRUE if the semaphore was released.  pdFALSE if an error occurred.
 * Semaphores are implemented using queues.  An error can occur if there is
 * no space on the queue to post a message - indicating that the
 * semaphore was not first obtained correctly.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xSemaphore = NULL;

 void vATask( void * pvParameters )
 {
    // Create the semaphore to guard a shared resource.
    xSemaphore = vSemaphoreCreateBinary();

    if( xSemaphore != NULL )
    {
        if( xSemaphoreGive( xSemaphore ) != pdTRUE )
        {
            // We would expect this call to fail because we cannot give
            // a semaphore without first "taking" it!
        }

        // Obtain the semaphore - don't block if the semaphore is not
        // immediately available.
        if( xSemaphoreTake( xSemaphore, ( TickType_t ) 0 ) )
        {
            // We now have the semaphore and can access the shared resource.

            // ...

            // We have finished accessing the shared resource so can free the
            // semaphore.
            if( xSemaphoreGive( xSemaphore ) != pdTRUE )
            {
                // We would not expect this call to fail because we must have
                // obtained the semaphore to get here.
            }
        }
    }
 }
 </pre>
 * \defgroup xSemaphoreGive xSemaphoreGive
 * \ingroup Semaphores
 */
#define xSemaphoreGive( xSemaphore )		xQueueGenericSend( ( QueueHandle_t ) ( xSemaphore ), NULL, semGIVE_BLOCK_TIME, queueSEND_TO_BACK )

/**
 * semphr. h
 * <pre>xSemaphoreGiveRecursive( SemaphoreHandle_t xMutex )</pre>
 *
 * <i>Macro</i> to recursively release, or 'give', a mutex type semaphore.
 * The mutex must have previously been created using a call to
 * xSemaphoreCreateRecursiveMutex();
 *
 * configUSE_RECURSIVE_MUTEXES must be set to 1 in FreeRTOSConfig.h for this
 * macro to be available.
 *
 * This macro must not be used on mutexes created using xSemaphoreCreateMutex().
 *
 * A mutex used recursively can be 'taken' repeatedly by the owner. The mutex
 * doesn't become available again until the owner has called
 * xSemaphoreGiveRecursive() for each successful 'take' request.  For example,
 * if a task successfully 'takes' the same mutex 5 times then the mutex will
 * not be available to any other task until it has also  'given' the mutex back
 * exactly five times.
 *
 * @param xMutex A handle to the mutex being released, or 'given'.  This is the
 * handle returned by xSemaphoreCreateMutex();
 *
 * @return pdTRUE if the semaphore was given.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xMutex = NULL;

 // A task that creates a mutex.
 void vATask( void * pvParameters )
 {
    // Create the mutex to guard a shared resource.
    xMutex = xSemaphoreCreateRecursiveMutex();
 }

 // A task that uses the mutex.
 void vAnotherTask( void * pvParameters )
 {
    // ... Do other things.

    if( xMutex != NULL )
    {
        // See if we can obtain the mutex.  If the mutex is not available
        // wait 10 ticks to see if it becomes free.
        if( xSemaphoreTakeRecursive( xMutex, ( TickType_t ) 10 ) == pdTRUE )
        {
            // We were able to obtain the mutex and can now access the
            // shared resource.

            // ...
            // For some reason due to the nature of the code further calls to
			// xSemaphoreTakeRecursive() are made on the same mutex.  In real
			// code these would not be just sequential calls as this would make
			// no sense.  Instead the calls are likely to be buried inside
			// a more complex call structure.
            xSemaphoreTakeRecursive( xMutex, ( TickType_t ) 10 );
            xSemaphoreTakeRecursive( xMutex, ( TickType_t ) 10 );

            // The mutex has now been 'taken' three times, so will not be
			// available to another task until it has also been given back
			// three times.  Again it is unlikely that real code would have
			// these calls sequentially, it would be more likely that the calls
			// to xSemaphoreGiveRecursive() would be called as a call stack
			// unwound.  This is just for demonstrative purposes.
            xSemaphoreGiveRecursive( xMutex );
			xSemaphoreGiveRecursive( xMutex );
			xSemaphoreGiveRecursive( xMutex );

			// Now the mutex can be taken by other tasks.
        }
        else
        {
            // We could not obtain the mutex and can therefore not access
            // the shared resource safely.
        }
    }
 }
 </pre>
 * \defgroup xSemaphoreGiveRecursive xSemaphoreGiveRecursive
 * \ingroup Semaphores
 */
#if( configUSE_RECURSIVE_MUTEXES == 1 )
	#define xSemaphoreGiveRecursive( xMutex )	xQueueGiveMutexRecursive( ( xMutex ) )
#endif

/**
 * semphr. h
 * <pre>
 xSemaphoreGiveFromISR(
                          SemaphoreHandle_t xSemaphore,
                          BaseType_t *pxHigherPriorityTaskWoken
                      )</pre>
 *
 * <i>Macro</i> to  release a semaphore.  The semaphore must have previously been
 * created with a call to xSemaphoreCreateBinary() or xSemaphoreCreateCounting().
 *
 * Mutex type semaphores (those created using a call to xSemaphoreCreateMutex())
 * must not be used with this macro.
 *
 * This macro can be used from an ISR.
 *
 * @param xSemaphore A handle to the semaphore being released.  This is the
 * handle returned when the semaphore was created.
 *
 * @param pxHigherPriorityTaskWoken xSemaphoreGiveFromISR() will set
 * *pxHigherPriorityTaskWoken to pdTRUE if giving the semaphore caused a task
 * to unblock, and the unblocked task has a priority higher than the currently
 * running task.  If xSemaphoreGiveFromISR() sets this value to pdTRUE then
 * a context switch should be requested before the interrupt is exited.
 *
 * @return pdTRUE if the semaphore was successfully given, otherwise errQUEUE_FULL.
 *
 * Example usage:
 <pre>
 \#define LONG_TIME 0xffff
 \#define TICKS_TO_WAIT	10
 SemaphoreHandle_t xSemaphore = NULL;

 // Repetitive task.
 void vATask( void * pvParameters )
 {
    for( ;; )
    {
        // We want this task to run every 10 ticks of a timer.  The semaphore
        // was created before this task was started.

        // Block waiting for the semaphore to become available.
        if( xSemaphoreTake( xSemaphore, LONG_TIME ) == pdTRUE )
        {
            // It is time to execute.

            // ...

            // We have finished our task.  Return to the top of the loop where
            // we will block on the semaphore until it is time to execute
            // again.  Note when using the semaphore for synchronisation with an
			// ISR in this manner there is no need to 'give' the semaphore back.
        }
    }
 }

 // Timer ISR
 void vTimerISR( void * pvParameters )
 {
 static uint8_t ucLocalTickCount = 0;
 static BaseType_t xHigherPriorityTaskWoken;

    // A timer tick has occurred.

    // ... Do other time functions.

    // Is it time for vATask () to run?
	xHigherPriorityTaskWoken = pdFALSE;
    ucLocalTickCount++;
    if( ucLocalTickCount >= TICKS_TO_WAIT )
    {
        // Unblock the task by releasing the semaphore.
        xSemaphoreGiveFromISR( xSemaphore, &xHigherPriorityTaskWoken );

        // Reset the count so we release the semaphore again in 10 ticks time.
        ucLocalTickCount = 0;
    }

    if( xHigherPriorityTaskWoken != pdFALSE )
    {
        // We can force a context switch here.  Context switching from an
        // ISR uses port specific syntax.  Check the demo task for your port
        // to find the syntax required.
    }
 }
 </pre>
 * \defgroup xSemaphoreGiveFromISR xSemaphoreGiveFromISR
 * \ingroup Semaphores
 */
#define xSemaphoreGiveFromISR( xSemaphore, pxHigherPriorityTaskWoken )	xQueueGiveFromISR( ( QueueHandle_t ) ( xSemaphore ), ( pxHigherPriorityTaskWoken ) )

/**
 * semphr. h
 * <pre>
 xSemaphoreTakeFromISR(
                          SemaphoreHandle_t xSemaphore,
                          BaseType_t *pxHigherPriorityTaskWoken
                      )</pre>
 *
 * <i>Macro</i> to  take a semaphore from an ISR.  The semaphore must have
 * previously been created with a call to xSemaphoreCreateBinary() or
 * xSemaphoreCreateCounting().
 *
 * Mutex type semaphores (those created using a call to xSemaphoreCreateMutex())
 * must not be used with this macro.
 *
 * This macro can be used from an ISR, however taking a semaphore from an ISR
 * is not a common operation.  It is likely to only be useful when taking a
 * counting semaphore when an interrupt is obtaining an object from a resource
 * pool (when the semaphore count indicates the number of resources available).
 *
 * @param xSemaphore A handle to the semaphore being taken.  This is the
 * handle returned when the semaphore was created.
 *
 * @param pxHigherPriorityTaskWoken xSemaphoreTakeFromISR() will set
 * *pxHigherPriorityTaskWoken to pdTRUE if taking the semaphore caused a task
 * to unblock, and the unblocked task has a priority higher than the currently
 * running task.  If xSemaphoreTakeFromISR() sets this value to pdTRUE then
 * a context switch should be requested before the interrupt is exited.
 *
 * @return pdTRUE if the semaphore was successfully taken, otherwise
 * pdFALSE
 */
#define xSemaphoreTakeFromISR( xSemaphore, pxHigherPriorityTaskWoken )	xQueueReceiveFromISR( ( QueueHandle_t ) ( xSemaphore ), NULL, ( pxHigherPriorityTaskWoken ) )

/**
 * semphr. h
 * <pre>SemaphoreHandle_t xSemaphoreCreateMutex( void )</pre>
 *
 * Creates a new mutex type semaphore instance, and returns a handle by which
 * the new mutex can be referenced.
 *
 * Internally, within the FreeRTOS implementation, mutex semaphores use a block
 * of memory, in which the mutex structure is stored.  If a mutex is created
 * using xSemaphoreCreateMutex() then the required memory is automatically
 * dynamically allocated inside the xSemaphoreCreateMutex() function.  (see
 * http://www.freertos.org/a00111.html).  If a mutex is created using
 * xSemaphoreCreateMutexStatic() then the application writer must provided the
 * memory.  xSemaphoreCreateMutexStatic() therefore allows a mutex to be created
 * without using any dynamic memory allocation.
 *
 * Mutexes created using this function can be accessed using the xSemaphoreTake()
 * and xSemaphoreGive() macros.  The xSemaphoreTakeRecursive() and
 * xSemaphoreGiveRecursive() macros must not be used.
 *
 * This type of semaphore uses a priority inheritance mechanism so a task
 * 'taking' a semaphore MUST ALWAYS 'give' the semaphore back once the
 * semaphore it is no longer required.
 *
 * Mutex type semaphores cannot be used from within interrupt service routines.
 *
 * See xSemaphoreCreateBinary() for an alternative implementation that can be
 * used for pure synchronisation (where one task or interrupt always 'gives' the
 * semaphore and another always 'takes' the semaphore) and from within interrupt
 * service routines.
 *
 * @return If the mutex was successfully created then a handle to the created
 * semaphore is returned.  If there was not enough heap to allocate the mutex
 * data structures then NULL is returned.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xSemaphore;

 void vATask( void * pvParameters )
 {
    // Semaphore cannot be used before a call to xSemaphoreCreateMutex().
    // This is a macro so pass the variable in directly.
    xSemaphore = xSemaphoreCreateMutex();

    if( xSemaphore != NULL )
    {
        // The semaphore was created successfully.
        // The semaphore can now be used.
    }
 }
 </pre>
 * \defgroup xSemaphoreCreateMutex xSemaphoreCreateMutex
 * \ingroup Semaphores
 */
#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
	#define xSemaphoreCreateMutex() xQueueCreateMutex( queueQUEUE_TYPE_MUTEX )
#endif

/**
 * semphr. h
 * <pre>SemaphoreHandle_t xSemaphoreCreateMutexStatic( StaticSemaphore_t *pxMutexBuffer )</pre>
 *
 * Creates a new mutex type semaphore instance, and returns a handle by which
 * the new mutex can be referenced.
 *
 * Internally, within the FreeRTOS implementation, mutex semaphores use a block
 * of memory, in which the mutex structure is stored.  If a mutex is created
 * using xSemaphoreCreateMutex() then the required memory is automatically
 * dynamically allocated inside the xSemaphoreCreateMutex() function.  (see
 * http://www.freertos.org/a00111.html).  If a mutex is created using
 * xSemaphoreCreateMutexStatic() then the application writer must provided the
 * memory.  xSemaphoreCreateMutexStatic() therefore allows a mutex to be created
 * without using any dynamic memory allocation.
 *
 * Mutexes created using this function can be accessed using the xSemaphoreTake()
 * and xSemaphoreGive() macros.  The xSemaphoreTakeRecursive() and
 * xSemaphoreGiveRecursive() macros must not be used.
 *
 * This type of semaphore uses a priority inheritance mechanism so a task
 * 'taking' a semaphore MUST ALWAYS 'give' the semaphore back once the
 * semaphore it is no longer required.
 *
 * Mutex type semaphores cannot be used from within interrupt service routines.
 *
 * See xSemaphoreCreateBinary() for an alternative implementation that can be
 * used for pure synchronisation (where one task or interrupt always 'gives' the
 * semaphore and another always 'takes' the semaphore) and from within interrupt
 * service routines.
 *
 * @param pxMutexBuffer Must point to a variable of type StaticSemaphore_t,
 * which will be used to hold the mutex's data structure, removing the need for
 * the memory to be allocated dynamically.
 *
 * @return If the mutex was successfully created then a handle to the created
 * mutex is returned.  If pxMutexBuffer was NULL then NULL is returned.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xSemaphore;
 StaticSemaphore_t xMutexBuffer;

 void vATask( void * pvParameters )
 {
    // A mutex cannot be used before it has been created.  xMutexBuffer is
    // into xSemaphoreCreateMutexStatic() so no dynamic memory allocation is
    // attempted.
    xSemaphore = xSemaphoreCreateMutexStatic( &xMutexBuffer );

    // As no dynamic memory allocation was performed, xSemaphore cannot be NULL,
    // so there is no need to check it.
 }
 </pre>
 * \defgroup xSemaphoreCreateMutexStatic xSemaphoreCreateMutexStatic
 * \ingroup Semaphores
 */
 #if( configSUPPORT_STATIC_ALLOCATION == 1 )
	#define xSemaphoreCreateMutexStatic( pxMutexBuffer ) xQueueCreateMutexStatic( queueQUEUE_TYPE_MUTEX, ( pxMutexBuffer ) )
#endif /* configSUPPORT_STATIC_ALLOCATION */


/**
 * semphr. h
 * <pre>SemaphoreHandle_t xSemaphoreCreateRecursiveMutex( void )</pre>
 *
 * Creates a new recursive mutex type semaphore instance, and returns a handle
 * by which the new recursive mutex can be referenced.
 *
 * Internally, within the FreeRTOS implementation, recursive mutexs use a block
 * of memory, in which the mutex structure is stored.  If a recursive mutex is
 * created using xSemaphoreCreateRecursiveMutex() then the required memory is
 * automatically dynamically allocated inside the
 * xSemaphoreCreateRecursiveMutex() function.  (see
 * http://www.freertos.org/a00111.html).  If a recursive mutex is created using
 * xSemaphoreCreateRecursiveMutexStatic() then the application writer must
 * provide the memory that will get used by the mutex.
 * xSemaphoreCreateRecursiveMutexStatic() therefore allows a recursive mutex to
 * be created without using any dynamic memory allocation.
 *
 * Mutexes created using this macro can be accessed using the
 * xSemaphoreTakeRecursive() and xSemaphoreGiveRecursive() macros.  The
 * xSemaphoreTake() and xSemaphoreGive() macros must not be used.
 *
 * A mutex used recursively can be 'taken' repeatedly by the owner. The mutex
 * doesn't become available again until the owner has called
 * xSemaphoreGiveRecursive() for each successful 'take' request.  For example,
 * if a task successfully 'takes' the same mutex 5 times then the mutex will
 * not be available to any other task until it has also  'given' the mutex back
 * exactly five times.
 *
 * This type of semaphore uses a priority inheritance mechanism so a task
 * 'taking' a semaphore MUST ALWAYS 'give' the semaphore back once the
 * semaphore it is no longer required.
 *
 * Mutex type semaphores cannot be used from within interrupt service routines.
 *
 * See xSemaphoreCreateBinary() for an alternative implementation that can be
 * used for pure synchronisation (where one task or interrupt always 'gives' the
 * semaphore and another always 'takes' the semaphore) and from within interrupt
 * service routines.
 *
 * @return xSemaphore Handle to the created mutex semaphore.  Should be of type
 * SemaphoreHandle_t.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xSemaphore;

 void vATask( void * pvParameters )
 {
    // Semaphore cannot be used before a call to xSemaphoreCreateMutex().
    // This is a macro so pass the variable in directly.
    xSemaphore = xSemaphoreCreateRecursiveMutex();

    if( xSemaphore != NULL )
    {
        // The semaphore was created successfully.
        // The semaphore can now be used.
    }
 }
 </pre>
 * \defgroup xSemaphoreCreateRecursiveMutex xSemaphoreCreateRecursiveMutex
 * \ingroup Semaphores
 */
#if( ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configUSE_RECURSIVE_MUTEXES == 1 ) )
	#define xSemaphoreCreateRecursiveMutex() xQueueCreateMutex( queueQUEUE_TYPE_RECURSIVE_MUTEX )
#endif

/**
 * semphr. h
 * <pre>SemaphoreHandle_t xSemaphoreCreateRecursiveMutexStatic( StaticSemaphore_t *pxMutexBuffer )</pre>
 *
 * Creates a new recursive mutex type semaphore instance, and returns a handle
 * by which the new recursive mutex can be referenced.
 *
 * Internally, within the FreeRTOS implementation, recursive mutexs use a block
 * of memory, in which the mutex structure is stored.  If a recursive mutex is
 * created using xSemaphoreCreateRecursiveMutex() then the required memory is
 * automatically dynamically allocated inside the
 * xSemaphoreCreateRecursiveMutex() function.  (see
 * http://www.freertos.org/a00111.html).  If a recursive mutex is created using
 * xSemaphoreCreateRecursiveMutexStatic() then the application writer must
 * provide the memory that will get used by the mutex.
 * xSemaphoreCreateRecursiveMutexStatic() therefore allows a recursive mutex to
 * be created without using any dynamic memory allocation.
 *
 * Mutexes created using this macro can be accessed using the
 * xSemaphoreTakeRecursive() and xSemaphoreGiveRecursive() macros.  The
 * xSemaphoreTake() and xSemaphoreGive() macros must not be used.
 *
 * A mutex used recursively can be 'taken' repeatedly by the owner. The mutex
 * doesn't become available again until the owner has called
 * xSemaphoreGiveRecursive() for each successful 'take' request.  For example,
 * if a task successfully 'takes' the same mutex 5 times then the mutex will
 * not be available to any other task until it has also  'given' the mutex back
 * exactly five times.
 *
 * This type of semaphore uses a priority inheritance mechanism so a task
 * 'taking' a semaphore MUST ALWAYS 'give' the semaphore back once the
 * semaphore it is no longer required.
 *
 * Mutex type semaphores cannot be used from within interrupt service routines.
 *
 * See xSemaphoreCreateBinary() for an alternative implementation that can be
 * used for pure synchronisation (where one task or interrupt always 'gives' the
 * semaphore and another always 'takes' the semaphore) and from within interrupt
 * service routines.
 *
 * @param pxMutexBuffer Must point to a variable of type StaticSemaphore_t,
 * which will then be used to hold the recursive mutex's data structure,
 * removing the need for the memory to be allocated dynamically.
 *
 * @return If the recursive mutex was successfully created then a handle to the
 * created recursive mutex is returned.  If pxMutexBuffer was NULL then NULL is
 * returned.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xSemaphore;
 StaticSemaphore_t xMutexBuffer;

 void vATask( void * pvParameters )
 {
    // A recursive semaphore cannot be used before it is created.  Here a
    // recursive mutex is created using xSemaphoreCreateRecursiveMutexStatic().
    // The address of xMutexBuffer is passed into the function, and will hold
    // the mutexes data structures - so no dynamic memory allocation will be
    // attempted.
    xSemaphore = xSemaphoreCreateRecursiveMutexStatic( &xMutexBuffer );

    // As no dynamic memory allocation was performed, xSemaphore cannot be NULL,
    // so there is no need to check it.
 }
 </pre>
 * \defgroup xSemaphoreCreateRecursiveMutexStatic xSemaphoreCreateRecursiveMutexStatic
 * \ingroup Semaphores
 */
#if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configUSE_RECURSIVE_MUTEXES == 1 ) )
	#define xSemaphoreCreateRecursiveMutexStatic( pxStaticSemaphore ) xQueueCreateMutexStatic( queueQUEUE_TYPE_RECURSIVE_MUTEX, pxStaticSemaphore )
#endif /* configSUPPORT_STATIC_ALLOCATION */

/**
 * semphr. h
 * <pre>SemaphoreHandle_t xSemaphoreCreateCounting( UBaseType_t uxMaxCount, UBaseType_t uxInitialCount )</pre>
 *
 * Creates a new counting semaphore instance, and returns a handle by which the
 * new counting semaphore can be referenced.
 *
 * In many usage scenarios it is faster and more memory efficient to use a
 * direct to task notification in place of a counting semaphore!
 * http://www.freertos.org/RTOS-task-notifications.html
 *
 * Internally, within the FreeRTOS implementation, counting semaphores use a
 * block of memory, in which the counting semaphore structure is stored.  If a
 * counting semaphore is created using xSemaphoreCreateCounting() then the
 * required memory is automatically dynamically allocated inside the
 * xSemaphoreCreateCounting() function.  (see
 * http://www.freertos.org/a00111.html).  If a counting semaphore is created
 * using xSemaphoreCreateCountingStatic() then the application writer can
 * instead optionally provide the memory that will get used by the counting
 * semaphore.  xSemaphoreCreateCountingStatic() therefore allows a counting
 * semaphore to be created without using any dynamic memory allocation.
 *
 * Counting semaphores are typically used for two things:
 *
 * 1) Counting events.
 *
 *    In this usage scenario an event handler will 'give' a semaphore each time
 *    an event occurs (incrementing the semaphore count value), and a handler
 *    task will 'take' a semaphore each time it processes an event
 *    (decrementing the semaphore count value).  The count value is therefore
 *    the difference between the number of events that have occurred and the
 *    number that have been processed.  In this case it is desirable for the
 *    initial count value to be zero.
 *
 * 2) Resource management.
 *
 *    In this usage scenario the count value indicates the number of resources
 *    available.  To obtain control of a resource a task must first obtain a
 *    semaphore - decrementing the semaphore count value.  When the count value
 *    reaches zero there are no free resources.  When a task finishes with the
 *    resource it 'gives' the semaphore back - incrementing the semaphore count
 *    value.  In this case it is desirable for the initial count value to be
 *    equal to the maximum count value, indicating that all resources are free.
 *
 * @param uxMaxCount The maximum count value that can be reached.  When the
 *        semaphore reaches this value it can no longer be 'given'.
 *
 * @param uxInitialCount The count value assigned to the semaphore when it is
 *        created.
 *
 * @return Handle to the created semaphore.  Null if the semaphore could not be
 *         created.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xSemaphore;

 void vATask( void * pvParameters )
 {
 SemaphoreHandle_t xSemaphore = NULL;

    // Semaphore cannot be used before a call to xSemaphoreCreateCounting().
    // The max value to which the semaphore can count should be 10, and the
    // initial value assigned to the count should be 0.
    xSemaphore = xSemaphoreCreateCounting( 10, 0 );

    if( xSemaphore != NULL )
    {
        // The semaphore was created successfully.
        // The semaphore can now be used.
    }
 }
 </pre>
 * \defgroup xSemaphoreCreateCounting xSemaphoreCreateCounting
 * \ingroup Semaphores
 */
#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
	#define xSemaphoreCreateCounting( uxMaxCount, uxInitialCount ) xQueueCreateCountingSemaphore( ( uxMaxCount ), ( uxInitialCount ) )
#endif

/**
 * semphr. h
 * <pre>SemaphoreHandle_t xSemaphoreCreateCountingStatic( UBaseType_t uxMaxCount, UBaseType_t uxInitialCount, StaticSemaphore_t *pxSemaphoreBuffer )</pre>
 *
 * Creates a new counting semaphore instance, and returns a handle by which the
 * new counting semaphore can be referenced.
 *
 * In many usage scenarios it is faster and more memory efficient to use a
 * direct to task notification in place of a counting semaphore!
 * http://www.freertos.org/RTOS-task-notifications.html
 *
 * Internally, within the FreeRTOS implementation, counting semaphores use a
 * block of memory, in which the counting semaphore structure is stored.  If a
 * counting semaphore is created using xSemaphoreCreateCounting() then the
 * required memory is automatically dynamically allocated inside the
 * xSemaphoreCreateCounting() function.  (see
 * http://www.freertos.org/a00111.html).  If a counting semaphore is created
 * using xSemaphoreCreateCountingStatic() then the application writer must
 * provide the memory.  xSemaphoreCreateCountingStatic() therefore allows a
 * counting semaphore to be created without using any dynamic memory allocation.
 *
 * Counting semaphores are typically used for two things:
 *
 * 1) Counting events.
 *
 *    In this usage scenario an event handler will 'give' a semaphore each time
 *    an event occurs (incrementing the semaphore count value), and a handler
 *    task will 'take' a semaphore each time it processes an event
 *    (decrementing the semaphore count value).  The count value is therefore
 *    the difference between the number of events that have occurred and the
 *    number that have been processed.  In this case it is desirable for the
 *    initial count value to be zero.
 *
 * 2) Resource management.
 *
 *    In this usage scenario the count value indicates the number of resources
 *    available.  To obtain control of a resource a task must first obtain a
 *    semaphore - decrementing the semaphore count value.  When the count value
 *    reaches zero there are no free resources.  When a task finishes with the
 *    resource it 'gives' the semaphore back - incrementing the semaphore count
 *    value.  In this case it is desirable for the initial count value to be
 *    equal to the maximum count value, indicating that all resources are free.
 *
 * @param uxMaxCount The maximum count value that can be reached.  When the
 *        semaphore reaches this value it can no longer be 'given'.
 *
 * @param uxInitialCount The count value assigned to the semaphore when it is
 *        created.
 *
 * @param pxSemaphoreBuffer Must point to a variable of type StaticSemaphore_t,
 * which will then be used to hold the semaphore's data structure, removing the
 * need for the memory to be allocated dynamically.
 *
 * @return If the counting semaphore was successfully created then a handle to
 * the created counting semaphore is returned.  If pxSemaphoreBuffer was NULL
 * then NULL is returned.
 *
 * Example usage:
 <pre>
 SemaphoreHandle_t xSemaphore;
 StaticSemaphore_t xSemaphoreBuffer;

 void vATask( void * pvParameters )
 {
 SemaphoreHandle_t xSemaphore = NULL;

    // Counting semaphore cannot be used before they have been created.  Create
    // a counting semaphore using xSemaphoreCreateCountingStatic().  The max
    // value to which the semaphore can count is 10, and the initial value
    // assigned to the count will be 0.  The address of xSemaphoreBuffer is
    // passed in and will be used to hold the semaphore structure, so no dynamic
    // memory allocation will be used.
    xSemaphore = xSemaphoreCreateCounting( 10, 0, &xSemaphoreBuffer );

    // No memory allocation was attempted so xSemaphore cannot be NULL, so there
    // is no need to check its value.
 }
 </pre>
 * \defgroup xSemaphoreCreateCountingStatic xSemaphoreCreateCountingStatic
 * \ingroup Semaphores
 */
#if( configSUPPORT_STATIC_ALLOCATION == 1 )
	#define xSemaphoreCreateCountingStatic( uxMaxCount, uxInitialCount, pxSemaphoreBuffer ) xQueueCreateCountingSemaphoreStatic( ( uxMaxCount ), ( uxInitialCount ), ( pxSemaphoreBuffer ) )
#endif /* configSUPPORT_STATIC_ALLOCATION */

/**
 * semphr. h
 * <pre>void vSemaphoreDelete( SemaphoreHandle_t xSemaphore );</pre>
 *
 * Delete a semaphore.  This function must be used with care.  For example,
 * do not delete a mutex type semaphore if the mutex is held by a task.
 *
 * @param xSemaphore A handle to the semaphore to be deleted.
 *
 * \defgroup vSemaphoreDelete vSemaphoreDelete
 * \ingroup Semaphores
 */
#define vSemaphoreDelete( xSemaphore ) vQueueDelete( ( QueueHandle_t ) ( xSemaphore ) )

/**
 * semphr.h
 * <pre>TaskHandle_t xSemaphoreGetMutexHolder( SemaphoreHandle_t xMutex );</pre>
 *
 * If xMutex is indeed a mutex type semaphore, return the current mutex holder.
 * If xMutex is not a mutex type semaphore, or the mutex is available (not held
 * by a task), return NULL.
 *
 * Note: This is a good way of determining if the calling task is the mutex
 * holder, but not a good way of determining the identity of the mutex holder as
 * the holder may change between the function exiting and the returned value
 * being tested.
 */
#define xSemaphoreGetMutexHolder( xSemaphore ) xQueueGetMutexHolder( ( xSemaphore ) )

/**
 * semphr.h
 * <pre>TaskHandle_t xSemaphoreGetMutexHolderFromISR( SemaphoreHandle_t xMutex );</pre>
 *
 * If xMutex is indeed a mutex type semaphore, return the current mutex holder.
 * If xMutex is not a mutex type semaphore, or the mutex is available (not held
 * by a task), return NULL.
 *
 */
#define xSemaphoreGetMutexHolderFromISR( xSemaphore ) xQueueGetMutexHolderFromISR( ( xSemaphore ) )

/**
 * semphr.h
 * <pre>UBaseType_t uxSemaphoreGetCount( SemaphoreHandle_t xSemaphore );</pre>
 *
 * If the semaphore is a counting semaphore then uxSemaphoreGetCount() returns
 * its current count value.  If the semaphore is a binary semaphore then
 * uxSemaphoreGetCount() returns 1 if the semaphore is available, and 0 if the
 * semaphore is not available.
 *
 */
#define uxSemaphoreGetCount( xSemaphore ) uxQueueMessagesWaiting( ( QueueHandle_t ) ( xSemaphore ) )

#endif /* SEMAPHORE_H */


