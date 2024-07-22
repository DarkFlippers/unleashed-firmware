/*
 * FreeRTOS Kernel V10.2.1
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

/*
 * A sample implementation of pvPortMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of http://www.FreeRTOS.org for more information.
 */

#include "memmgr_heap.h"
#include "check.h"
#include <stdlib.h>
#include <stdio.h>
#include <stm32wbxx.h>
#include <stm32wb55_linker.h>
#include <core/log.h>
#include <core/common_defines.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include <FreeRTOS.h>
#include <task.h>

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#ifdef HEAP_PRINT_DEBUG
#error This feature is broken, logging transport must be replaced with RTT
#endif

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE ((size_t)(xHeapStructSize << 1))

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE ((size_t)8)

/* Heap start end symbols provided by linker */
uint8_t* ucHeap = (uint8_t*)&__heap_start__;

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct A_BLOCK_LINK {
    struct A_BLOCK_LINK* pxNextFreeBlock; /*<< The next free block in the list. */
    size_t xBlockSize; /*<< The size of the free block. */
} BlockLink_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList(BlockLink_t* pxBlockToInsert);

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit(void);

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const size_t xHeapStructSize = (sizeof(BlockLink_t) + ((size_t)(portBYTE_ALIGNMENT - 1))) &
                                      ~((size_t)portBYTE_ALIGNMENT_MASK);

/* Create a couple of list links to mark the start and end of the list. */
static BlockLink_t xStart, *pxEnd = NULL;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xFreeBytesRemaining = 0U;
static size_t xMinimumEverFreeBytesRemaining = 0U;

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
member of an BlockLink_t structure is set then the block belongs to the
application.  When the bit is free the block is still part of the free heap
space. */
static size_t xBlockAllocatedBit = 0;

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

                    if((pxLink->xBlockSize & xBlockAllocatedBit) != 0 &&
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
    size_t max_free_size = 0;
    BlockLink_t* pxBlock;
    vTaskSuspendAll();

    pxBlock = xStart.pxNextFreeBlock;
    while(pxBlock->pxNextFreeBlock != NULL) {
        if(pxBlock->xBlockSize > max_free_size) {
            max_free_size = pxBlock->xBlockSize;
        }
        pxBlock = pxBlock->pxNextFreeBlock;
    }

    xTaskResumeAll();
    return max_free_size;
}

void memmgr_heap_printf_free_blocks(void) {
    BlockLink_t* pxBlock;
    //can be enabled once we can do printf with a locked scheduler
    //vTaskSuspendAll();

    pxBlock = xStart.pxNextFreeBlock;
    while(pxBlock->pxNextFreeBlock != NULL) {
        printf("A %p S %lu\r\n", (void*)pxBlock, (uint32_t)pxBlock->xBlockSize);
        pxBlock = pxBlock->pxNextFreeBlock;
    }

    //xTaskResumeAll();
}

#ifdef HEAP_PRINT_DEBUG
char* ultoa(unsigned long num, char* str, int radix) {
    char temp[33]; // at radix 2 the string is at most 32 + 1 null long.
    int temp_loc = 0;
    int digit;
    int str_loc = 0;

    //construct a backward string of the number.
    do {
        digit = (unsigned long)num % ((unsigned long)radix);
        if(digit < 10)
            temp[temp_loc++] = digit + '0';
        else
            temp[temp_loc++] = digit - 10 + 'A';
        num = ((unsigned long)num) / ((unsigned long)radix);
    } while((unsigned long)num > 0);

    temp_loc--;

    //now reverse the string.
    while(temp_loc >= 0) { // while there are still chars
        str[str_loc++] = temp[temp_loc--];
    }
    str[str_loc] = 0; // add null termination.

    return str;
}

static void print_heap_init(void) {
    char tmp_str[33];
    size_t heap_start = (size_t)&__heap_start__;
    size_t heap_end = (size_t)&__heap_end__;

    // {PHStart|heap_start|heap_end}
    FURI_CRITICAL_ENTER();
    furi_log_puts("{PHStart|");
    ultoa(heap_start, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts("|");
    ultoa(heap_end, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts("}\r\n");
    FURI_CRITICAL_EXIT();
}

static void print_heap_malloc(void* ptr, size_t size) {
    char tmp_str[33];
    const char* name = furi_thread_get_name(furi_thread_get_current_id());
    if(!name) {
        name = "";
    }

    // {thread name|m|address|size}
    FURI_CRITICAL_ENTER();
    furi_log_puts("{");
    furi_log_puts(name);
    furi_log_puts("|m|0x");
    ultoa((unsigned long)ptr, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts("|");
    utoa(size, tmp_str, 10);
    furi_log_puts(tmp_str);
    furi_log_puts("}\r\n");
    FURI_CRITICAL_EXIT();
}

static void print_heap_free(void* ptr) {
    char tmp_str[33];
    const char* name = furi_thread_get_name(furi_thread_get_current_id());
    if(!name) {
        name = "";
    }

    // {thread name|f|address}
    FURI_CRITICAL_ENTER();
    furi_log_puts("{");
    furi_log_puts(name);
    furi_log_puts("|f|0x");
    ultoa((unsigned long)ptr, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts("}\r\n");
    FURI_CRITICAL_EXIT();
}
#endif
/*-----------------------------------------------------------*/

void* pvPortMalloc(size_t xWantedSize) {
    BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
    void* pvReturn = NULL;
    size_t to_wipe = xWantedSize;

    if(FURI_IS_IRQ_MODE()) {
        furi_crash("memmgt in ISR");
    }

#ifdef HEAP_PRINT_DEBUG
    BlockLink_t* print_heap_block = NULL;
#endif

    /* If this is the first call to malloc then the heap will require
        initialisation to setup the list of free blocks. */
    if(pxEnd == NULL) {
#ifdef HEAP_PRINT_DEBUG
        print_heap_init();
#endif

        vTaskSuspendAll();
        {
            prvHeapInit();
            memmgr_heap_init();
        }
        (void)xTaskResumeAll();
    } else {
        mtCOVERAGE_TEST_MARKER();
    }

    vTaskSuspendAll();
    {
        /* Check the requested block size is not so large that the top bit is
        set.  The top bit of the block size member of the BlockLink_t structure
        is used to determine who owns the block - the application or the
        kernel, so it must be free. */
        if((xWantedSize & xBlockAllocatedBit) == 0) {
            /* The wanted size is increased so it can contain a BlockLink_t
            structure in addition to the requested amount of bytes. */
            if(xWantedSize > 0) {
                xWantedSize += xHeapStructSize;

                /* Ensure that blocks are always aligned to the required number
                of bytes. */
                if((xWantedSize & portBYTE_ALIGNMENT_MASK) != 0x00) {
                    /* Byte alignment required. */
                    xWantedSize += (portBYTE_ALIGNMENT - (xWantedSize & portBYTE_ALIGNMENT_MASK));
                    configASSERT((xWantedSize & portBYTE_ALIGNMENT_MASK) == 0);
                } else {
                    mtCOVERAGE_TEST_MARKER();
                }
            } else {
                mtCOVERAGE_TEST_MARKER();
            }

            if((xWantedSize > 0) && (xWantedSize <= xFreeBytesRemaining)) {
                /* Traverse the list from the start (lowest address) block until
                one of adequate size is found. */
                pxPreviousBlock = &xStart;
                pxBlock = xStart.pxNextFreeBlock;
                while((pxBlock->xBlockSize < xWantedSize) && (pxBlock->pxNextFreeBlock != NULL)) {
                    pxPreviousBlock = pxBlock;
                    pxBlock = pxBlock->pxNextFreeBlock;
                }

                /* If the end marker was reached then a block of adequate size
                was not found. */
                if(pxBlock != pxEnd) {
                    /* Return the memory space pointed to - jumping over the
                    BlockLink_t structure at its start. */
                    pvReturn =
                        (void*)(((uint8_t*)pxPreviousBlock->pxNextFreeBlock) + xHeapStructSize);

                    /* This block is being returned for use so must be taken out
                    of the list of free blocks. */
                    pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                    /* If the block is larger than required it can be split into
                    two. */
                    if((pxBlock->xBlockSize - xWantedSize) > heapMINIMUM_BLOCK_SIZE) {
                        /* This block is to be split into two.  Create a new
                        block following the number of bytes requested. The void
                        cast is used to prevent byte alignment warnings from the
                        compiler. */
                        pxNewBlockLink = (void*)(((uint8_t*)pxBlock) + xWantedSize);
                        configASSERT((((size_t)pxNewBlockLink) & portBYTE_ALIGNMENT_MASK) == 0);

                        /* Calculate the sizes of two blocks split from the
                        single block. */
                        pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                        pxBlock->xBlockSize = xWantedSize;

                        /* Insert the new block into the list of free blocks. */
                        prvInsertBlockIntoFreeList(pxNewBlockLink);
                    } else {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    xFreeBytesRemaining -= pxBlock->xBlockSize;

                    if(xFreeBytesRemaining < xMinimumEverFreeBytesRemaining) {
                        xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
                    } else {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    /* The block is being returned - it is allocated and owned
                    by the application and has no "next" block. */
                    pxBlock->xBlockSize |= xBlockAllocatedBit;
                    pxBlock->pxNextFreeBlock = NULL;

#ifdef HEAP_PRINT_DEBUG
                    print_heap_block = pxBlock;
#endif
                } else {
                    mtCOVERAGE_TEST_MARKER();
                }
            } else {
                mtCOVERAGE_TEST_MARKER();
            }
        } else {
            mtCOVERAGE_TEST_MARKER();
        }

        traceMALLOC(pvReturn, xWantedSize);
    }
    (void)xTaskResumeAll();

#ifdef HEAP_PRINT_DEBUG
    print_heap_malloc(print_heap_block, print_heap_block->xBlockSize & ~xBlockAllocatedBit);
#endif

#if(configUSE_MALLOC_FAILED_HOOK == 1)
    {
        if(pvReturn == NULL) {
            extern void vApplicationMallocFailedHook(void);
            vApplicationMallocFailedHook();
        } else {
            mtCOVERAGE_TEST_MARKER();
        }
    }
#endif

    configASSERT((((size_t)pvReturn) & (size_t)portBYTE_ALIGNMENT_MASK) == 0);

    furi_check(pvReturn, xWantedSize ? "out of memory" : "malloc(0)");
    pvReturn = memset(pvReturn, 0, to_wipe);
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
        before it. */
        puc -= xHeapStructSize;

        /* This casting is to keep the compiler from issuing warnings. */
        pxLink = (void*)puc;

        /* Check the block is actually allocated. */
        configASSERT((pxLink->xBlockSize & xBlockAllocatedBit) != 0);
        configASSERT(pxLink->pxNextFreeBlock == NULL);

        if((pxLink->xBlockSize & xBlockAllocatedBit) != 0) {
            if(pxLink->pxNextFreeBlock == NULL) {
                /* The block is being returned to the heap - it is no longer
                allocated. */
                pxLink->xBlockSize &= ~xBlockAllocatedBit;

#ifdef HEAP_PRINT_DEBUG
                print_heap_free(pxLink);
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
                    memset(pv, 0, pxLink->xBlockSize - xHeapStructSize);
                    prvInsertBlockIntoFreeList((BlockLink_t*)pxLink);
                }
                (void)xTaskResumeAll();
            } else {
                mtCOVERAGE_TEST_MARKER();
            }
        } else {
            mtCOVERAGE_TEST_MARKER();
        }
    } else {
#ifdef HEAP_PRINT_DEBUG
        print_heap_free(pv);
#endif
    }
}
/*-----------------------------------------------------------*/

size_t xPortGetTotalHeapSize(void) {
    return (size_t)&__heap_end__ - (size_t)&__heap_start__;
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

void vPortInitialiseBlocks(void) {
    /* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/

static void prvHeapInit(void) {
    BlockLink_t* pxFirstFreeBlock;
    uint8_t* pucAlignedHeap;
    size_t uxAddress;
    size_t xTotalHeapSize = (size_t)&__heap_end__ - (size_t)&__heap_start__;

    /* Ensure the heap starts on a correctly aligned boundary. */
    uxAddress = (size_t)ucHeap;

    if((uxAddress & portBYTE_ALIGNMENT_MASK) != 0) {
        uxAddress += (portBYTE_ALIGNMENT - 1);
        uxAddress &= ~((size_t)portBYTE_ALIGNMENT_MASK);
        xTotalHeapSize -= uxAddress - (size_t)ucHeap;
    }

    pucAlignedHeap = (uint8_t*)uxAddress;

    /* xStart is used to hold a pointer to the first item in the list of free
    blocks.  The void cast is used to prevent compiler warnings. */
    xStart.pxNextFreeBlock = (void*)pucAlignedHeap;
    xStart.xBlockSize = (size_t)0;

    /* pxEnd is used to mark the end of the list of free blocks and is inserted
    at the end of the heap space. */
    uxAddress = ((size_t)pucAlignedHeap) + xTotalHeapSize;
    uxAddress -= xHeapStructSize;
    uxAddress &= ~((size_t)portBYTE_ALIGNMENT_MASK);
    pxEnd = (void*)uxAddress;
    pxEnd->xBlockSize = 0;
    pxEnd->pxNextFreeBlock = NULL;

    /* To start with there is a single free block that is sized to take up the
    entire heap space, minus the space taken by pxEnd. */
    pxFirstFreeBlock = (void*)pucAlignedHeap;
    pxFirstFreeBlock->xBlockSize = uxAddress - (size_t)pxFirstFreeBlock;
    pxFirstFreeBlock->pxNextFreeBlock = pxEnd;

    /* Only one block exists - and it covers the entire usable heap space. */
    xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;

    /* Work out the position of the top bit in a size_t variable. */
    xBlockAllocatedBit = ((size_t)1) << ((sizeof(size_t) * heapBITS_PER_BYTE) - 1);
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList(BlockLink_t* pxBlockToInsert) {
    BlockLink_t* pxIterator;
    uint8_t* puc;

    /* Iterate through the list until a block is found that has a higher address
    than the block being inserted. */
    for(pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert;
        pxIterator = pxIterator->pxNextFreeBlock) {
        /* Nothing to do here, just iterate to the right position. */
    }

    /* Do the block being inserted, and the block it is being inserted after
    make a contiguous block of memory? */
    puc = (uint8_t*)pxIterator;
    if((puc + pxIterator->xBlockSize) == (uint8_t*)pxBlockToInsert) {
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    } else {
        mtCOVERAGE_TEST_MARKER();
    }

    /* Do the block being inserted, and the block it is being inserted before
    make a contiguous block of memory? */
    puc = (uint8_t*)pxBlockToInsert;
    if((puc + pxBlockToInsert->xBlockSize) == (uint8_t*)pxIterator->pxNextFreeBlock) {
        if(pxIterator->pxNextFreeBlock != pxEnd) {
            /* Form one big block from the two blocks. */
            pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
        } else {
            pxBlockToInsert->pxNextFreeBlock = pxEnd;
        }
    } else {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
    before and the block after, then it's pxNextFreeBlock pointer will have
    already been set, and should not be set here as that would make it point
    to itself. */
    if(pxIterator != pxBlockToInsert) {
        pxIterator->pxNextFreeBlock = pxBlockToInsert;
    } else {
        mtCOVERAGE_TEST_MARKER();
    }
}
