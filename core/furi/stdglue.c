#include "stdglue.h"
#include "check.h"
#include "memmgr.h"

#include <FreeRTOS.h>
#include <task.h>

#include <furi_hal.h>
#include <m-dict.h>

DICT_DEF2(
    FuriStdglueCallbackDict,
    uint32_t,
    M_DEFAULT_OPLIST,
    FuriStdglueWriteCallback,
    M_PTR_OPLIST)

typedef struct {
    osMutexId_t mutex;
    FuriStdglueCallbackDict_t thread_outputs;
} FuriStdglue;

static FuriStdglue* furi_stdglue = NULL;

static ssize_t stdout_write(void* _cookie, const char* data, size_t size) {
    furi_assert(furi_stdglue);
    bool consumed = false;
    osKernelState_t state = osKernelGetState();
    osThreadId_t thread_id = osThreadGetId();
    if(state == osKernelRunning && thread_id &&
       osMutexAcquire(furi_stdglue->mutex, osWaitForever) == osOK) {
        // We are in the thread context
        // Handle thread callbacks
        FuriStdglueWriteCallback* callback_ptr =
            FuriStdglueCallbackDict_get(furi_stdglue->thread_outputs, (uint32_t)thread_id);
        if(callback_ptr) {
            (*callback_ptr)(_cookie, data, size);
            consumed = true;
        }
        furi_check(osMutexRelease(furi_stdglue->mutex) == osOK);
    }
    // Flush
    if(data == 0) {
        /*
         * This means that we should flush internal buffers.  Since we
         * don't we just return.  (Remember, "handle" == -1 means that all
         * handles should be flushed.)
         */
        return 0;
    }
    // Debug uart
    if(!consumed) furi_hal_console_tx((const uint8_t*)data, size);
    // All data consumed
    return size;
}

void furi_stdglue_init() {
    furi_stdglue = malloc(sizeof(FuriStdglue));
    // Init outputs structures
    furi_stdglue->mutex = osMutexNew(NULL);
    furi_check(furi_stdglue->mutex);
    FuriStdglueCallbackDict_init(furi_stdglue->thread_outputs);
    // Prepare and set stdout descriptor
    FILE* fp = fopencookie(
        NULL,
        "w",
        (cookie_io_functions_t){
            .read = NULL,
            .write = stdout_write,
            .seek = NULL,
            .close = NULL,
        });
    setvbuf(fp, NULL, _IOLBF, 0);
    stdout = fp;
}

bool furi_stdglue_set_thread_stdout_callback(FuriStdglueWriteCallback callback) {
    furi_assert(furi_stdglue);
    osThreadId_t thread_id = osThreadGetId();
    if(thread_id) {
        furi_check(osMutexAcquire(furi_stdglue->mutex, osWaitForever) == osOK);
        if(callback) {
            FuriStdglueCallbackDict_set_at(
                furi_stdglue->thread_outputs, (uint32_t)thread_id, callback);
        } else {
            FuriStdglueCallbackDict_erase(furi_stdglue->thread_outputs, (uint32_t)thread_id);
        }
        furi_check(osMutexRelease(furi_stdglue->mutex) == osOK);
        return true;
    } else {
        return false;
    }
}

void __malloc_lock(struct _reent* REENT) {
    vTaskSuspendAll();
}

void __malloc_unlock(struct _reent* REENT) {
    xTaskResumeAll();
}
