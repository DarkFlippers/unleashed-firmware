#include "queue.h"

void render_queue(const QueueState* queue_state, const void* app_state, Canvas* const canvas) {
    if(queue_state->current != NULL && queue_state->current->render != NULL)
        ((QueueItem*)queue_state->current)->render(app_state, canvas);
}

bool run_queue(QueueState* queue_state, void* app_state) {
    if(queue_state->current != NULL) {
        queue_state->running = true;
        if((furi_get_tick() - queue_state->start) >= queue_state->current->duration)
            dequeue(queue_state, app_state);

        return true;
    }
    return false;
}

void dequeue(QueueState* queue_state, void* app_state) {
    ((QueueItem*)queue_state->current)->callback(app_state);
    QueueItem* f = queue_state->current;
    queue_state->current = f->next;
    free(f);
    if(queue_state->current != NULL) {
        if(queue_state->current->start != NULL) queue_state->current->start(app_state);
        queue_state->start = furi_get_tick();
    } else {
        queue_state->running = false;
    }
}

void queue_clear(QueueState* queue_state) {
    queue_state->running = false;
    QueueItem* curr = queue_state->current;
    while(curr != NULL) {
        QueueItem* f = curr;
        curr = curr->next;
        free(f);
    }
}

void enqueue(
    QueueState* queue_state,
    void* app_state,
    void (*done)(void* state),
    void (*start)(void* state),
    void (*render)(const void* state, Canvas* const canvas),
    uint32_t duration) {
    QueueItem* next;
    if(queue_state->current == NULL) {
        queue_state->start = furi_get_tick();
        queue_state->current = malloc(sizeof(QueueItem));
        next = queue_state->current;
        if(next->start != NULL) next->start(app_state);

    } else {
        next = queue_state->current;
        while(next->next != NULL) {
            next = (QueueItem*)(next->next);
        }
        next->next = malloc(sizeof(QueueItem));
        next = next->next;
    }
    next->callback = done;
    next->render = render;
    next->start = start;
    next->duration = duration;
    next->next = NULL;
}