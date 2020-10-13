#pragma once

#include "cmsis_os.h"
#ifdef HAVE_FREERTOS
#include <semphr.h>
#endif
#include <stdbool.h>
#include <stdint.h>

#define MAX_TASK_RECORDS 8
#define MAX_RECORD_SUBSCRIBERS 8

/// application is just a function
typedef void (*FlipperApplication)(void*);

/// pointer to value callback function
typedef void (*FlipperRecordCallback)(const void*, size_t, void*);

typedef enum {
    FlipperRecordStateMute, ///< record open and mute this handler
    FlipperRecordStateUnmute, ///< record unmuted
    FlipperRecordStateDeleted ///< record owner halt
} FlipperRecordState;

/// pointer to state callback function
typedef void (*FlipperRecordStateCallback)(FlipperRecordState, void*);

struct _FuriRecord;

typedef struct {
    bool allocated;
    FlipperRecordCallback cb; ///< value cb
    FlipperRecordStateCallback state_cb; ///< state cb
    uint8_t mute_counter; ///< see "wiki/FURI#mute-algorithm"
    bool no_mute;
    struct _FuriRecord* record; ///< parent record
    void* ctx;
} FuriRecordSubscriber;

/// FURI record handler
struct _FuriRecord {
    const char* name;
    void* value;
    size_t size;
    StaticSemaphore_t mutex_buffer;
    SemaphoreHandle_t mutex;
    uint8_t mute_counter;
    FuriRecordSubscriber subscribers[MAX_RECORD_SUBSCRIBERS];
};

typedef struct _FuriRecord FuriRecord;

/// store info about active task
typedef struct {
    const char* name;
    FlipperApplication application;
    const char* prev_name;
    FlipperApplication prev;
    TaskHandle_t handler;
    uint8_t records_count; ///< count of records which task open
    FuriRecord* records[MAX_TASK_RECORDS]; ///< list of records which task open

    bool ready;
} FuriApp;

// application dependency info
typedef struct {
    uint8_t count;
    const char** name;
} FlipperAppLibrary;

// application startup info
typedef struct {
    FlipperApplication app;
    const char* name;
    FlipperAppLibrary libs;
} FlipperStartupApp;

/*!
Simply starts application.
It call app entrypoint with param passed as argument.
Useful for daemon applications and pop-up.
*/
FuriApp* furiac_start(FlipperApplication app, const char* name, void* param);

/*!
Swtich to other application.
FURI stop current app, call app entrypoint with param passed as
argument and save current application entrypoint to prev field
in current application registry.
Useful for UI or "active" application.
*/
void furiac_switch(FlipperApplication app, char* name, void* param);

/*!
Stop current application
(stop thread and clear application's stack), start application
from prev entry in current application registry, cleanup current
application registry.
*/
void furiac_exit(void* param);

/*!
Mark application as prepared and ready to perform actions
*/
void furiac_ready();

/* 
Wait for the libraries we depend on
*/
void furiac_wait_libs(const FlipperAppLibrary* libs);

/*!
Stop specified app without returning to prev application.
*/
bool furiac_kill(FuriApp* app);

// find task pointer by handle
FuriApp* find_task(TaskHandle_t handler);

/*!
Creates named FURI record.
\param[in] name you can open this record anywhere
\param[in] value pointer to data.
\param[in] size size of data.
If NULL, create FURI Pipe (only callbacks management, no data/mutex)

Returns false if registry have not enough memory for creating.
*/
bool furi_create_deprecated(const char* name, void* value, size_t size);

/*!
Opens existing FURI record by name.
Returns NULL if record does not exist.
\param[in] solo if true another applications handlers set into "muted" state.
When appication has exited or record has closed, all handlers is unmuted.
It may be useful for concurrently acces to resources like framebuffer or beeper.
\param[in] no_mute if true, another applications cannot mute this handler.
*/
FuriRecordSubscriber* furi_open_deprecated(
    const char* name,
    bool solo,
    bool no_mute,
    FlipperRecordCallback value_callback,
    FlipperRecordStateCallback state_callback,
    void* ctx);

/*!

*/
void furi_close(FuriRecordSubscriber* handler);

/*!
read message from record.
Returns true if success, false otherwise (closed/non-existent record)
Also return false if you try to read from FURI pipe

TODO: enum return value with execution status
*/
bool furi_read(FuriRecordSubscriber* record, void* data, size_t size);

/*!
write message to record.
Returns true if success, false otherwise (closed/non-existent record or muted).

TODO: enum return value with execution status
*/
bool furi_write(FuriRecordSubscriber* record, const void* data, size_t size);

/*!
lock value mutex.
It can be useful if records contain pointer to buffer which you want to change.
You must call furi_give after operation on data and
you shouldn't block executing between take and give calls

Returns pointer to data, NULL if closed/non-existent record or muted

TODO: enum return value with execution status
*/
void* furi_take(FuriRecordSubscriber* record);

/*!
unlock value mutex.
*/
void furi_give(FuriRecordSubscriber* record);

/*!
unlock value mutex and notify subscribers that data is chaned.
*/
void furi_commit(FuriRecordSubscriber* handler);
