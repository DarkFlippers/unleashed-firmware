#include "loader_queue.h"

void loader_queue_item_clear(LoaderDeferredLaunchRecord* item) {
    free(item->args);
    free(item->name_or_path);
}

bool loader_queue_pop(LoaderLaunchQueue* queue, LoaderDeferredLaunchRecord* item) {
    if(!queue->item_cnt) return false;

    *item = queue->items[0];
    queue->item_cnt--;
    memmove(
        &queue->items[0], &queue->items[1], queue->item_cnt * sizeof(LoaderDeferredLaunchRecord));

    return true;
}

bool loader_queue_push(LoaderLaunchQueue* queue, LoaderDeferredLaunchRecord* item) {
    if(queue->item_cnt == LOADER_QUEUE_MAX_SIZE) return false;

    queue->items[queue->item_cnt] = *item;
    queue->item_cnt++;

    return true;
}

void loader_queue_clear(LoaderLaunchQueue* queue) {
    for(size_t i = 0; i < queue->item_cnt; i++)
        loader_queue_item_clear(&queue->items[i]);
    queue->item_cnt = 0;
}
