#pragma once

#include <gui/gui.h>
#include <furi.h>

typedef struct {
    void (*callback)(void* state); //Callback for when the item is dequeued
    void (*render)(
        const void* state,
        Canvas* const canvas); //Callback for the rendering loop while this item is running
    void (*start)(void* state); //Callback when this item is started running
    void* next; //Pointer to the next item
    uint32_t duration; //duration of the item
} QueueItem;

typedef struct {
    unsigned int start; //current queue item start time
    QueueItem* current; //current queue item
    bool running; //is the queue running
} QueueState;

/**
 * Enqueue a new item.
 *
 * @param queue_state   The queue state pointer
 * @param app_state     Your app state
 * @param done          Callback for dequeue event
 * @param start         Callback for when the item is activated
 * @param render        Callback to render loop if needed
 * @param duration      Length of the item
 */
void enqueue(
    QueueState* queue_state,
    void* app_state,
    void (*done)(void* state),
    void (*start)(void* state),
    void (*render)(const void* state, Canvas* const canvas),
    uint32_t duration);
/**
 * Clears all queue items
 *
 * @param queue_state   The queue state pointer
 */
void queue_clear(QueueState* queue_state);

/**
 * Dequeues the active queue item. Usually you don't need to call it directly.
 *
 * @param queue_state   The queue state pointer
 * @param app_state     Your application state
 */
void dequeue(QueueState* queue_state, void* app_state);

/**
 * Runs the queue logic (place it in your tick function)
 *
 * @param queue_state   The queue state pointer
 * @param app_state     Your application state
 * @return              FALSE when there is nothing to run, TRUE otherwise
 */
bool run_queue(QueueState* queue_state, void* app_state);

/**
 * Calls the currently active queue items render callback (if there is any)
 *
 * @param queue_state   The queue state pointer
 * @param app_state     Your application state
 * @param canvas        Pointer to Flipper's canvas object
 */
void render_queue(const QueueState* queue_state, const void* app_state, Canvas* const canvas);