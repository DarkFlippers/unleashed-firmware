#include "flipper.h"
#include "api-hal-task.h"

// TODO: this file contains printf, that not implemented on uC target

#ifdef FURI_DEBUG
#include <stdio.h>
#endif

#include <string.h>

#define INVALID_TASK_ID UINT16_MAX

static StaticTask_t task_info_buffer[MAX_TASK_COUNT];
static StackType_t stack_buffer[MAX_TASK_COUNT][DEFAULT_STACK_SIZE / 4];
static FuriApp task_buffer[MAX_TASK_COUNT];

static size_t current_buffer_idx = 0;

uint16_t furiac_get_task_id_by_name(const char* app_name) {
    for(size_t i = 0; i < MAX_TASK_RECORDS; i++) {
        if(strcmp(task_buffer[i].name, app_name) == 0) return i;
    }

    return INVALID_TASK_ID;
}

void furiac_wait_libs(const FlipperAppLibrary* libs) {
    for(uint8_t i = 0; i < libs->count; i++) {
        uint16_t app_id = furiac_get_task_id_by_name(libs->name[i]);

        if(app_id == INVALID_TASK_ID) {
#ifdef FURI_DEBUG
            printf("[FURIAC] Invalid library name %s\n", libs->name[i]);
#endif
        } else {
            while(!task_buffer[app_id].ready) {
#ifdef FURI_DEBUG
                printf("[FURIAC] waiting for library \"%s\"\n", libs->name[i]);
#endif
                osDelay(50);
            }
        }
    }
}

// find task pointer by handle
FuriApp* find_task(TaskHandle_t handler) {
    FuriApp* res = NULL;
    for(size_t i = 0; i < MAX_TASK_COUNT; i++) {
        if(task_equal(task_buffer[i].handler, handler)) {
            res = &task_buffer[i];
        }
    }

    return res;
}

FuriApp* furiac_start(FlipperApplication app, const char* name, void* param) {
#ifdef FURI_DEBUG
    printf("[FURIAC] start %s\n", name);
#endif

    // TODO check first free item (.handler == NULL) and use it

    if(current_buffer_idx >= MAX_TASK_COUNT) {
// max task count exceed
#ifdef FURI_DEBUG
        printf("[FURIAC] max task count exceed\n");
#endif
        return NULL;
    }

    // application ready
    task_buffer[current_buffer_idx].ready = false;

    // create task on static stack memory
    task_buffer[current_buffer_idx].handler = xTaskCreateStatic(
        (TaskFunction_t)app,
        (const char* const)name,
        DEFAULT_STACK_SIZE / 4, // freertos specify stack size in words
        (void* const)param,
        tskIDLE_PRIORITY + 3, // normal priority
        stack_buffer[current_buffer_idx],
        &task_info_buffer[current_buffer_idx]);

    // save task
    task_buffer[current_buffer_idx].application = app;
    task_buffer[current_buffer_idx].prev_name = NULL;
    task_buffer[current_buffer_idx].prev = NULL;
    task_buffer[current_buffer_idx].records_count = 0;
    task_buffer[current_buffer_idx].name = name;

    current_buffer_idx++;

    return &task_buffer[current_buffer_idx - 1];
}

bool furiac_kill(FuriApp* app) {
#ifdef FURI_DEBUG
    printf("[FURIAC] kill %s\n", app->name);
#endif

    // check handler
    if(app == NULL || app->handler == NULL) return false;

    // kill task
    vTaskDelete(app->handler);

    // cleanup its registry
    // TODO realy free memory
    app->handler = NULL;

    return true;
}

void furiac_exit(void* param) {
    // get current task handler
    FuriApp* current_task = find_task(xTaskGetCurrentTaskHandle());

    // run prev
    if(current_task != NULL) {
#ifdef FURI_DEBUG
        printf("[FURIAC] exit %s\n", current_task->name);
#endif

        if(current_task->prev != NULL) {
            furiac_start(current_task->prev, current_task->prev_name, param);
        } else {
#ifdef FURI_DEBUG
            printf("[FURIAC] no prev\n");
#endif
        }

        // cleanup registry
        // TODO realy free memory
        current_task->handler = NULL;
    }

    // kill itself
    vTaskDelete(NULL);
}

void furiac_switch(FlipperApplication app, char* name, void* param) {
    // get current task handler
    FuriApp* current_task = find_task(xTaskGetCurrentTaskHandle());

    if(current_task == NULL) {
#ifdef FURI_DEBUG
        printf("[FURIAC] no current task found\n");
#endif
    }

#ifdef FURI_DEBUG
    printf("[FURIAC] switch %s to %s\n", current_task->name, name);
#endif

    // run next
    FuriApp* next = furiac_start(app, name, param);

    if(next != NULL) {
        // save current application pointer as prev
        next->prev = current_task->application;
        next->prev_name = current_task->name;

        // kill itself
        vTaskDelete(NULL);
    }
}

// set task to ready state
void furiac_ready() {
    /* 
        TODO:
        Currently i think that better way is to use application name
        and restrict applications to "one task per application"
    */
    FuriApp* app = find_task(xTaskGetCurrentTaskHandle());

    if(app == NULL) {
#ifdef FURI_DEBUG
        printf("[FURIAC] cannot find task to set ready state\n");
#endif
    } else {
#ifdef FURI_DEBUG
        printf("[FURIAC] task is ready\n");
#endif
        app->ready = true;
    }
}