#pragma once

#include <furi.h>

#include "loader.h"

#define LOADER_QUEUE_MAX_SIZE 4

typedef struct {
    char* name_or_path;
    char* args;
    LoaderDeferredLaunchFlag flags;
} LoaderDeferredLaunchRecord;

typedef struct {
    LoaderDeferredLaunchRecord items[LOADER_QUEUE_MAX_SIZE];
    size_t item_cnt;
} LoaderLaunchQueue;

/**
 * @brief Frees internal data in a `DeferredLaunchRecord`
 * 
 * @param[out] item Record to clear
 */
void loader_queue_item_clear(LoaderDeferredLaunchRecord* item);

/**
 * @brief Fetches the next item from the launch queue
 * 
 * @param[inout] queue Queue instance
 * @param[out]   item  Item output
 * 
 * @return `true` if `item` was populated, `false` if queue is empty
 */
bool loader_queue_pop(LoaderLaunchQueue* queue, LoaderDeferredLaunchRecord* item);

/**
 * @brief Puts an item into the launch queue
 * 
 * @param[inout] queue Queue instance
 * @param[in]    item  Item to put in the queue
 * 
 * @return `true` if the item was put into the queue, `false` if there's no more
 * space left
 */
bool loader_queue_push(LoaderLaunchQueue* queue, LoaderDeferredLaunchRecord* item);

/**
 * @brief Clears the launch queue
 * 
 * @param[inout] queue Queue instance
 */
void loader_queue_clear(LoaderLaunchQueue* queue);
