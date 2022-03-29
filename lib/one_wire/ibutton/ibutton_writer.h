/**
 * @file ibutton_writer.h
 * 
 * iButton blanks writer
 */

#pragma once
#include <furi_hal_gpio.h>
#include "ibutton_key.h"
#include "../one_wire_host.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    iButtonWriterOK,
    iButtonWriterSameKey,
    iButtonWriterNoDetect,
    iButtonWriterCannotWrite,
} iButtonWriterResult;

typedef struct iButtonWriter iButtonWriter;

/**
 * Allocate writer
 * @param host 
 * @return iButtonWriter* 
 */
iButtonWriter* ibutton_writer_alloc(OneWireHost* host);

/**
 * Deallocate writer
 * @param writer 
 */
void ibutton_writer_free(iButtonWriter* writer);

/**
 * Write key to blank
 * @param writer 
 * @param key 
 * @return iButtonWriterResult 
 */
iButtonWriterResult ibutton_writer_write(iButtonWriter* writer, iButtonKey* key);

/**
 * Start writing. Must be called before write attempt
 * @param writer 
 */
void ibutton_writer_start(iButtonWriter* writer);

/**
 * Stop writing
 * @param writer 
 */
void ibutton_writer_stop(iButtonWriter* writer);

#ifdef __cplusplus
}
#endif