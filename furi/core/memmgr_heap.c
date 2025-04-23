/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
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
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*
 * A sample implementation of pvPortMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of https://www.FreeRTOS.org for more information.
 */

#include "memmgr_heap.h"
#include "check.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stm32wbxx.h>
#include <stm32wb55_linker.h>
#include <core/log.h>
#include <core/common_defines.h>

// -V::562
// -V::650

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
 * all the API functions to use the MPU wrappers.  That should only be done when
 * task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include <FreeRTOS.h>
#include <task.h>

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#if(configSUPPORT_DYNAMIC_ALLOCATION == 0)
#error This file must not be used if configSUPPORT_DYNAMIC_ALLOCATION is 0
#endif

#ifndef configHEAP_CLEAR_MEMORY_ON_FREE
#define configHEAP_CLEAR_MEMORY_ON_FREE 0
#endif

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE ((size_t)(xHeapStructSize << 1))

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE ((size_t)8)

/* Max value that fits in a size_t type. */
#define heapSIZE_MAX (~((size_t)0))

/* Check if multiplying a and b will result in overflow. */
#define heapMULTIPLY_WILL_OVERFLOW(a, b) (((a) > 0) && ((b) > (heapSIZE_MAX / (a))))

/* Check if adding a and b will result in overflow. */
#define heapADD_WILL_OVERFLOW(a, b) ((a) > (heapSIZE_MAX - (b)))

/* Check if the subtraction operation ( a - b ) will result in underflow. */
#define heapSUBTRACT_WILL_UNDERFLOW(a, b) ((a) < (b))

/* MSB of the xBlockSize member of an BlockLink_t structure is used to track
 * the allocation status of a block.  When MSB of the xBlockSize member of
 * an BlockLink_t structure is set then the block belongs to the application.
 * When the bit is free the block is still part of the free heap space. */
#define heapBLOCK_ALLOCATED_BITMASK         (((size_t)1) << ((sizeof(size_t) * heapBITS_PER_BYTE) - 1))
#define heapBLOCK_SIZE_IS_VALID(xBlockSize) (((xBlockSize) & heapBLOCK_ALLOCATED_BITMASK) == 0)
#define heapBLOCK_IS_ALLOCATED(pxBlock) \
    (((pxBlock->xBlockSize) & heapBLOCK_ALLOCATED_BITMASK) != 0)
#define heapALLOCATE_BLOCK(pxBlock) ((pxBlock->xBlockSize) |= heapBLOCK_ALLOCATED_BITMASK)
#define heapFREE_BLOCK(pxBlock)     ((pxBlock->xBlockSize) &= ~heapBLOCK_ALLOCATED_BITMASK)

/*-----------------------------------------------------------*/

/* Heap start end symbols provided by linker */
uint8_t* ucHeap = (uint8_t*)&__heap_start__;

/* Define the linked list structure.  This is used to link free blocks in order
 * of their memory address. */
typedef struct A_BLOCK_LINK {
    struct A_BLOCK_LINK* pxNextFreeBlock; /**< The next free block in the list. */
    size_t xBlockSize; /**< The size of the free block. */
} BlockLink_t;

/* Setting configENABLE_HEAP_PROTECTOR to 1 enables heap block pointers
 * protection using an application supplied canary value to catch heap
 * corruption should a heap buffer overflow occur.
 */
#if(configENABLE_HEAP_PROTECTOR == 1)

/**
 * @brief Application provided function to get a random value to be used as canary.
 *
 * @param pxHeapCanary [out] Output parameter to return the canary value.
 */
extern void vApplicationGetRandomHeapCanary(portPOINTER_SIZE_TYPE* pxHeapCanary);

/* Canary value for protecting internal heap pointers. */
PRIVILEGED_DATA static portPOINTER_SIZE_TYPE xHeapCanary;

/* Macro to load/store BlockLink_t pointers to memory. By XORing the
 * pointers with a random canary value, heap overflows will result
 * in randomly unpredictable pointer values which will be caught by
 * heapVALIDATE_BLOCK_POINTER assert. */
#define heapPROTECT_BLOCK_POINTER(pxBlock) \
    ((BlockLink_t*)(((portPOINTER_SIZE_TYPE)(pxBlock)) ^ xHeapCanary))
#else

#define heapPROTECT_BLOCK_POINTER(pxBlock) (pxBlock)

#endif /* configENABLE_HEAP_PROTECTOR */

/* Assert that a heap block pointer is within the heap bounds. */
#define heapVALIDATE_BLOCK_POINTER(pxBlock)      \
    configASSERT(                                \
        ((uint8_t*)(pxBlock) >= &(ucHeap[0])) && \
        ((uint8_t*)(pxBlock) <= &(ucHeap[configTOTAL_HEAP_SIZE - 1])))

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList(BlockLink_t* pxBlockToInsert) PRIVILEGED_FUNCTION;

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit(void) PRIVILEGED_FUNCTION;

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
 * block must by correctly byte aligned. */
static const size_t xHeapStructSize = (sizeof(BlockLink_t) + ((size_t)(portBYTE_ALIGNMENT - 1))) &
                                      ~((size_t)portBYTE_ALIGNMENT_MASK);

/* Create a couple of list links to mark the start and end of the list. */
PRIVILEGED_DATA static BlockLink_t xStart;
PRIVILEGED_DATA static BlockLink_t* pxEnd = NULL;

/* Keeps track of the number of calls to allocate and free memory as well as the
 * number of free bytes remaining, but says nothing about fragmentation. */
PRIVILEGED_DATA static size_t xFreeBytesRemaining = (size_t)0U;
PRIVILEGED_DATA static size_t xMinimumEverFreeBytesRemaining = (size_t)0U;
PRIVILEGED_DATA static size_t xNumberOfSuccessfulAllocations = (size_t)0U;
PRIVILEGED_DATA static size_t xNumberOfSuccessfulFrees = (size_t)0U;

/* Furi heap extension */
#include <m-dict.h>

/* Allocation tracking types */
DICT_DEF2(MemmgrHeapAllocDict, uint32_t, uint32_t) //-V1048

DICT_DEF2( //-V1048
    MemmgrHeapThreadDict,
    uint32_t,
    M_DEFAULT_OPLIST,
    MemmgrHeapAllocDict_t,
    DICT_OPLIST(MemmgrHeapAllocDict))

/* Thread allocation tracing storage */
static MemmgrHeapThreadDict_t memmgr_heap_thread_dict = {0};
static volatile uint32_t memmgr_heap_thread_trace_depth = 0;

/* Initialize tracing storage on start */
void memmgr_heap_init(void) {
    MemmgrHeapThreadDict_init(memmgr_heap_thread_dict);
}

void memmgr_heap_enable_thread_trace(FuriThreadId thread_id) {
    vTaskSuspendAll();
    {
        memmgr_heap_thread_trace_depth++;
        furi_check(MemmgrHeapThreadDict_get(memmgr_heap_thread_dict, (uint32_t)thread_id) == NULL);
        MemmgrHeapAllocDict_t alloc_dict;
        MemmgrHeapAllocDict_init(alloc_dict);
        MemmgrHeapThreadDict_set_at(memmgr_heap_thread_dict, (uint32_t)thread_id, alloc_dict);
        MemmgrHeapAllocDict_clear(alloc_dict);
        memmgr_heap_thread_trace_depth--;
    }
    (void)xTaskResumeAll();
}

void memmgr_heap_disable_thread_trace(FuriThreadId thread_id) {
    vTaskSuspendAll();
    {
        memmgr_heap_thread_trace_depth++;
        furi_check(MemmgrHeapThreadDict_erase(memmgr_heap_thread_dict, (uint32_t)thread_id));
        memmgr_heap_thread_trace_depth--;
    }
    (void)xTaskResumeAll();
}

size_t memmgr_heap_get_thread_memory(FuriThreadId thread_id) {
    size_t leftovers = MEMMGR_HEAP_UNKNOWN;
    vTaskSuspendAll();
    {
        memmgr_heap_thread_trace_depth++;
        MemmgrHeapAllocDict_t* alloc_dict =
            MemmgrHeapThreadDict_get(memmgr_heap_thread_dict, (uint32_t)thread_id);
        if(alloc_dict) {
            leftovers = 0;
            MemmgrHeapAllocDict_it_t alloc_dict_it;
            for(MemmgrHeapAllocDict_it(alloc_dict_it, *alloc_dict);
                !MemmgrHeapAllocDict_end_p(alloc_dict_it);
                MemmgrHeapAllocDict_next(alloc_dict_it)) {
                MemmgrHeapAllocDict_itref_t* data = MemmgrHeapAllocDict_ref(alloc_dict_it);
                if(data->key != 0) {
                    uint8_t* puc = (uint8_t*)data->key;
                    puc -= xHeapStructSize;
                    BlockLink_t* pxLink = (void*)puc;

                    if((pxLink->xBlockSize & heapBLOCK_ALLOCATED_BITMASK) &&
                       pxLink->pxNextFreeBlock == NULL) {
                        leftovers += data->value;
                    }
                }
            }
        }
        memmgr_heap_thread_trace_depth--;
    }
    (void)xTaskResumeAll();
    return leftovers;
}

#undef traceMALLOC
static inline void traceMALLOC(void* pointer, size_t size) {
    FuriThreadId thread_id = furi_thread_get_current_id();
    if(thread_id && memmgr_heap_thread_trace_depth == 0) {
        memmgr_heap_thread_trace_depth++;
        MemmgrHeapAllocDict_t* alloc_dict =
            MemmgrHeapThreadDict_get(memmgr_heap_thread_dict, (uint32_t)thread_id);
        if(alloc_dict) {
            MemmgrHeapAllocDict_set_at(*alloc_dict, (uint32_t)pointer, (uint32_t)size);
        }
        memmgr_heap_thread_trace_depth--;
    }
}

#undef traceFREE
static inline void traceFREE(void* pointer, size_t size) {
    UNUSED(size);
    FuriThreadId thread_id = furi_thread_get_current_id();
    if(thread_id && memmgr_heap_thread_trace_depth == 0) {
        memmgr_heap_thread_trace_depth++;
        MemmgrHeapAllocDict_t* alloc_dict =
            MemmgrHeapThreadDict_get(memmgr_heap_thread_dict, (uint32_t)thread_id);
        if(alloc_dict) {
            // In some cases thread may want to release memory that was not allocated by it
            const bool res = MemmgrHeapAllocDict_erase(*alloc_dict, (uint32_t)pointer);
            UNUSED(res);
        }
        memmgr_heap_thread_trace_depth--;
    }
}

size_t memmgr_heap_get_max_free_block(void) {
    HeapStats_t heap_stats;
    vPortGetHeapStats(&heap_stats);
    return heap_stats.xSizeOfLargestFreeBlockInBytes;
}

void memmgr_heap_printf_free_blocks(void) {
    BlockLink_t* pxBlock;
    //can be enabled once we can do printf with a locked scheduler
    //vTaskSuspendAll();

    pxBlock = heapPROTECT_BLOCK_POINTER(xStart.pxNextFreeBlock);
    heapVALIDATE_BLOCK_POINTER(pxBlock);
    while(pxBlock->pxNextFreeBlock != heapPROTECT_BLOCK_POINTER(NULL)) {
        printf("A %p S %lu\r\n", (void*)pxBlock, (uint32_t)pxBlock->xBlockSize);
        pxBlock = heapPROTECT_BLOCK_POINTER(pxBlock->pxNextFreeBlock);
        heapVALIDATE_BLOCK_POINTER(pxBlock);
    }

    //xTaskResumeAll();
}

/*-----------------------------------------------------------*/

void* pvPortMalloc(size_t xWantedSize) {
    BlockLink_t* pxBlock;
    BlockLink_t* pxPreviousBlock;
    BlockLink_t* pxNewBlockLink;
    void* pvReturn = NULL;
    size_t xToWipe = xWantedSize;
    size_t xAdditionalRequiredSize;
    size_t xAllocatedBlockSize = 0;

    if(FURI_IS_IRQ_MODE()) {
        furi_crash("memmgt in ISR");
    }

    if(xWantedSize > 0) {
        /* The wanted size must be increased so it can contain a BlockLink_t
         * structure in addition to the requested amount of bytes. */
        if(heapADD_WILL_OVERFLOW(xWantedSize, xHeapStructSize) == 0) {
            xWantedSize += xHeapStructSize;

            /* Ensure that blocks are always aligned to the required number
             * of bytes. */
            if((xWantedSize & portBYTE_ALIGNMENT_MASK) != 0x00) {
                /* Byte alignment required. */
                xAdditionalRequiredSize =
                    portBYTE_ALIGNMENT - (xWantedSize & portBYTE_ALIGNMENT_MASK);

                if(heapADD_WILL_OVERFLOW(xWantedSize, xAdditionalRequiredSize) == 0) {
                    xWantedSize += xAdditionalRequiredSize;
                } else {
                    xWantedSize = 0;
                }
            } else {
                mtCOVERAGE_TEST_MARKER();
            }
        } else {
            xWantedSize = 0;
        }
    } else {
        mtCOVERAGE_TEST_MARKER();
    }

    vTaskSuspendAll();
    {
        /* If this is the first call to malloc then the heap will require
         * initialisation to setup the list of free blocks. */
        if(pxEnd == NULL) {
            prvHeapInit();
            memmgr_heap_init();
        } else {
            mtCOVERAGE_TEST_MARKER();
        }

        /* Check the block size we are trying to allocate is not so large that the
         * top bit is set.  The top bit of the block size member of the BlockLink_t
         * structure is used to determine who owns the block - the application or
         * the kernel, so it must be free. */
        if(heapBLOCK_SIZE_IS_VALID(xWantedSize) != 0) {
            if((xWantedSize > 0) && (xWantedSize <= xFreeBytesRemaining)) {
                /* Traverse the list from the start (lowest address) block until
                 * one of adequate size is found. */
                pxPreviousBlock = &xStart;
                pxBlock = heapPROTECT_BLOCK_POINTER(xStart.pxNextFreeBlock);
                heapVALIDATE_BLOCK_POINTER(pxBlock);

                while((pxBlock->xBlockSize < xWantedSize) &&
                      (pxBlock->pxNextFreeBlock != heapPROTECT_BLOCK_POINTER(NULL))) {
                    pxPreviousBlock = pxBlock;
                    pxBlock = heapPROTECT_BLOCK_POINTER(pxBlock->pxNextFreeBlock);
                    heapVALIDATE_BLOCK_POINTER(pxBlock);
                }

                /* If the end marker was reached then a block of adequate size
                 * was not found. */
                if(pxBlock != pxEnd) {
                    /* Return the memory space pointed to - jumping over the
                     * BlockLink_t structure at its start. */
                    pvReturn = (void*)(((uint8_t*)heapPROTECT_BLOCK_POINTER(
                                           pxPreviousBlock->pxNextFreeBlock)) +
                                       xHeapStructSize);
                    heapVALIDATE_BLOCK_POINTER(pvReturn);

                    /* This block is being returned for use so must be taken out
                     * of the list of free blocks. */
                    pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                    /* If the block is larger than required it can be split into
                     * two. */
                    configASSERT(
                        heapSUBTRACT_WILL_UNDERFLOW(pxBlock->xBlockSize, xWantedSize) == 0);

                    if((pxBlock->xBlockSize - xWantedSize) > heapMINIMUM_BLOCK_SIZE) {
                        /* This block is to be split into two.  Create a new
                         * block following the number of bytes requested. The void
                         * cast is used to prevent byte alignment warnings from the
                         * compiler. */
                        pxNewBlockLink = (void*)(((uint8_t*)pxBlock) + xWantedSize);
                        configASSERT((((size_t)pxNewBlockLink) & portBYTE_ALIGNMENT_MASK) == 0);

                        /* Calculate the sizes of two blocks split from the
                         * single block. */
                        pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                        pxBlock->xBlockSize = xWantedSize;

                        /* Insert the new block into the list of free blocks. */
                        pxNewBlockLink->pxNextFreeBlock = pxPreviousBlock->pxNextFreeBlock;
                        pxPreviousBlock->pxNextFreeBlock =
                            heapPROTECT_BLOCK_POINTER(pxNewBlockLink);
                    } else {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    xFreeBytesRemaining -= pxBlock->xBlockSize;

                    if(xFreeBytesRemaining < xMinimumEverFreeBytesRemaining) {
                        xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
                    } else {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    xAllocatedBlockSize = pxBlock->xBlockSize;

                    /* The block is being returned - it is allocated and owned
                     * by the application and has no "next" block. */
                    heapALLOCATE_BLOCK(pxBlock);
                    pxBlock->pxNextFreeBlock = heapPROTECT_BLOCK_POINTER(NULL);
                    xNumberOfSuccessfulAllocations++;
                } else {
                    mtCOVERAGE_TEST_MARKER();
                }
            } else {
                mtCOVERAGE_TEST_MARKER();
            }
        } else {
            mtCOVERAGE_TEST_MARKER();
        }

        traceMALLOC(pvReturn, xAllocatedBlockSize);

        /* Prevent compiler warnings when trace macros are not used. */
        (void)xAllocatedBlockSize;
    }
    (void)xTaskResumeAll();

#if(configUSE_MALLOC_FAILED_HOOK == 1)
    {
        if(pvReturn == NULL) {
            vApplicationMallocFailedHook();
        } else {
            mtCOVERAGE_TEST_MARKER();
        }
    }
#endif /* if ( configUSE_MALLOC_FAILED_HOOK == 1 ) */

    configASSERT((((size_t)pvReturn) & (size_t)portBYTE_ALIGNMENT_MASK) == 0);

    furi_check(pvReturn, xWantedSize ? "out of memory" : "malloc(0)");
    pvReturn = memset(pvReturn, 0, xToWipe);
    return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree(void* pv) {
    uint8_t* puc = (uint8_t*)pv;
    BlockLink_t* pxLink;

    if(FURI_IS_IRQ_MODE()) {
        furi_crash("memmgt in ISR");
    }

    if(pv != NULL) {
        /* The memory being freed will have an BlockLink_t structure immediately
         * before it. */
        puc -= xHeapStructSize;

        /* This casting is to keep the compiler from issuing warnings. */
        pxLink = (void*)puc;

        heapVALIDATE_BLOCK_POINTER(pxLink);
        configASSERT(heapBLOCK_IS_ALLOCATED(pxLink) != 0);
        configASSERT(pxLink->pxNextFreeBlock == heapPROTECT_BLOCK_POINTER(NULL));

        if(heapBLOCK_IS_ALLOCATED(pxLink) != 0) {
            if(pxLink->pxNextFreeBlock == heapPROTECT_BLOCK_POINTER(NULL)) {
                /* The block is being returned to the heap - it is no longer
                 * allocated. */
                heapFREE_BLOCK(pxLink);
#if(configHEAP_CLEAR_MEMORY_ON_FREE == 1)
                {
                    /* Check for underflow as this can occur if xBlockSize is
                     * overwritten in a heap block. */
                    if(heapSUBTRACT_WILL_UNDERFLOW(pxLink->xBlockSize, xHeapStructSize) == 0) {
                        (void)memset(
                            puc + xHeapStructSize, 0, pxLink->xBlockSize - xHeapStructSize);
                    }
                }
#endif

                vTaskSuspendAll();
                {
                    furi_assert((size_t)pv >= SRAM_BASE);
                    furi_assert((size_t)pv < SRAM_BASE + 1024 * 256);
                    furi_assert(pxLink->xBlockSize >= xHeapStructSize);
                    furi_assert((pxLink->xBlockSize - xHeapStructSize) < 1024 * 256);

                    /* Add this block to the list of free blocks. */
                    xFreeBytesRemaining += pxLink->xBlockSize;
                    traceFREE(pv, pxLink->xBlockSize);
                    prvInsertBlockIntoFreeList(((BlockLink_t*)pxLink));
                    xNumberOfSuccessfulFrees++;
                }
                (void)xTaskResumeAll();
            } else {
                mtCOVERAGE_TEST_MARKER();
            }
        } else {
            mtCOVERAGE_TEST_MARKER();
        }
    }
}
/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize(void) {
    return xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

size_t xPortGetMinimumEverFreeHeapSize(void) {
    return xMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void xPortResetHeapMinimumEverFreeHeapSize(void) {
    xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks(void) {
    /* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/

void* pvPortCalloc(size_t xNum, size_t xSize) {
    void* pv = NULL;

    if(heapMULTIPLY_WILL_OVERFLOW(xNum, xSize) == 0) {
        pv = pvPortMalloc(xNum * xSize);

        if(pv != NULL) {
            (void)memset(pv, 0, xNum * xSize);
        }
    }

    return pv;
}
/*-----------------------------------------------------------*/

static void prvHeapInit(void) /* PRIVILEGED_FUNCTION */
{
    BlockLink_t* pxFirstFreeBlock;
    portPOINTER_SIZE_TYPE uxStartAddress, uxEndAddress;
    size_t xTotalHeapSize = configTOTAL_HEAP_SIZE;

    /* Ensure the heap starts on a correctly aligned boundary. */
    uxStartAddress = (portPOINTER_SIZE_TYPE)ucHeap;

    if((uxStartAddress & portBYTE_ALIGNMENT_MASK) != 0) {
        uxStartAddress += (portBYTE_ALIGNMENT - 1);
        uxStartAddress &= ~((portPOINTER_SIZE_TYPE)portBYTE_ALIGNMENT_MASK);
        xTotalHeapSize -= (size_t)(uxStartAddress - (portPOINTER_SIZE_TYPE)ucHeap);
    }

#if(configENABLE_HEAP_PROTECTOR == 1)
    { vApplicationGetRandomHeapCanary(&(xHeapCanary)); }
#endif

    /* xStart is used to hold a pointer to the first item in the list of free
     * blocks.  The void cast is used to prevent compiler warnings. */
    xStart.pxNextFreeBlock = (void*)heapPROTECT_BLOCK_POINTER(uxStartAddress);
    xStart.xBlockSize = (size_t)0;

    /* pxEnd is used to mark the end of the list of free blocks and is inserted
     * at the end of the heap space. */
    uxEndAddress = uxStartAddress + (portPOINTER_SIZE_TYPE)xTotalHeapSize;
    uxEndAddress -= (portPOINTER_SIZE_TYPE)xHeapStructSize;
    uxEndAddress &= ~((portPOINTER_SIZE_TYPE)portBYTE_ALIGNMENT_MASK);
    pxEnd = (BlockLink_t*)uxEndAddress;
    pxEnd->xBlockSize = 0;
    pxEnd->pxNextFreeBlock = heapPROTECT_BLOCK_POINTER(NULL);

    /* To start with there is a single free block that is sized to take up the
     * entire heap space, minus the space taken by pxEnd. */
    pxFirstFreeBlock = (BlockLink_t*)uxStartAddress;
    pxFirstFreeBlock->xBlockSize =
        (size_t)(uxEndAddress - (portPOINTER_SIZE_TYPE)pxFirstFreeBlock);
    pxFirstFreeBlock->pxNextFreeBlock = heapPROTECT_BLOCK_POINTER(pxEnd);

    /* Only one block exists - and it covers the entire usable heap space. */
    xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList(BlockLink_t* pxBlockToInsert) /* PRIVILEGED_FUNCTION */
{
    BlockLink_t* pxIterator;
    uint8_t* puc;

    /* Iterate through the list until a block is found that has a higher address
     * than the block being inserted. */
    for(pxIterator = &xStart;
        heapPROTECT_BLOCK_POINTER(pxIterator->pxNextFreeBlock) < pxBlockToInsert;
        pxIterator = heapPROTECT_BLOCK_POINTER(pxIterator->pxNextFreeBlock)) {
        /* Nothing to do here, just iterate to the right position. */
    }

    if(pxIterator != &xStart) {
        heapVALIDATE_BLOCK_POINTER(pxIterator);
    }

    /* Do the block being inserted, and the block it is being inserted after
     * make a contiguous block of memory? */
    puc = (uint8_t*)pxIterator;

    if((puc + pxIterator->xBlockSize) == (uint8_t*)pxBlockToInsert) {
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    } else {
        mtCOVERAGE_TEST_MARKER();
    }

    /* Do the block being inserted, and the block it is being inserted before
     * make a contiguous block of memory? */
    puc = (uint8_t*)pxBlockToInsert;

    if((puc + pxBlockToInsert->xBlockSize) ==
       (uint8_t*)heapPROTECT_BLOCK_POINTER(pxIterator->pxNextFreeBlock)) {
        if(heapPROTECT_BLOCK_POINTER(pxIterator->pxNextFreeBlock) != pxEnd) {
            /* Form one big block from the two blocks. */
            pxBlockToInsert->xBlockSize +=
                heapPROTECT_BLOCK_POINTER(pxIterator->pxNextFreeBlock)->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock =
                heapPROTECT_BLOCK_POINTER(pxIterator->pxNextFreeBlock)->pxNextFreeBlock;
        } else {
            pxBlockToInsert->pxNextFreeBlock = heapPROTECT_BLOCK_POINTER(pxEnd);
        }
    } else {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
    }

    /* If the block being inserted plugged a gap, so was merged with the block
     * before and the block after, then it's pxNextFreeBlock pointer will have
     * already been set, and should not be set here as that would make it point
     * to itself. */
    if(pxIterator != pxBlockToInsert) {
        pxIterator->pxNextFreeBlock = heapPROTECT_BLOCK_POINTER(pxBlockToInsert);
    } else {
        mtCOVERAGE_TEST_MARKER();
    }
}
/*-----------------------------------------------------------*/

void vPortGetHeapStats(HeapStats_t* pxHeapStats) {
    BlockLink_t* pxBlock;
    size_t
        xBlocks = 0,
        xMaxSize = 0,
        xMinSize =
            portMAX_DELAY; /* portMAX_DELAY used as a portable way of getting the maximum value. */

    vTaskSuspendAll();
    {
        pxBlock = heapPROTECT_BLOCK_POINTER(xStart.pxNextFreeBlock);

        /* pxBlock will be NULL if the heap has not been initialised.  The heap
         * is initialised automatically when the first allocation is made. */
        if(pxBlock != NULL) {
            while(pxBlock != pxEnd) {
                /* Increment the number of blocks and record the largest block seen
                 * so far. */
                xBlocks++;

                if(pxBlock->xBlockSize > xMaxSize) {
                    xMaxSize = pxBlock->xBlockSize;
                }

                if(pxBlock->xBlockSize < xMinSize) {
                    xMinSize = pxBlock->xBlockSize;
                }

                /* Move to the next block in the chain until the last block is
                 * reached. */
                pxBlock = heapPROTECT_BLOCK_POINTER(pxBlock->pxNextFreeBlock);
            }
        }
    }
    (void)xTaskResumeAll();

    pxHeapStats->xSizeOfLargestFreeBlockInBytes = xMaxSize;
    pxHeapStats->xSizeOfSmallestFreeBlockInBytes = xMinSize;
    pxHeapStats->xNumberOfFreeBlocks = xBlocks;

    taskENTER_CRITICAL();
    {
        pxHeapStats->xAvailableHeapSpaceInBytes = xFreeBytesRemaining;
        pxHeapStats->xNumberOfSuccessfulAllocations = xNumberOfSuccessfulAllocations;
        pxHeapStats->xNumberOfSuccessfulFrees = xNumberOfSuccessfulFrees;
        pxHeapStats->xMinimumEverFreeBytesRemaining = xMinimumEverFreeBytesRemaining;
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

/*
 * Reset the state in this file. This state is normally initialized at start up.
 * This function must be called by the application before restarting the
 * scheduler.
 */
void vPortHeapResetState(void) {
    pxEnd = NULL;

    xFreeBytesRemaining = (size_t)0U;
    xMinimumEverFreeBytesRemaining = (size_t)0U;
    xNumberOfSuccessfulAllocations = (size_t)0U;
    xNumberOfSuccessfulFrees = (size_t)0U;
}
/*-----------------------------------------------------------*/
