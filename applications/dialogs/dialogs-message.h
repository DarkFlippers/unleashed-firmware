#pragma once
#include <furi.h>
#include "dialogs-i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* path;
    const char* extension;
    char* result;
    uint8_t result_size;
    const char* preselected_filename;
} DialogsAppMessageDataFileSelect;

typedef struct {
    const DialogMessage* message;
} DialogsAppMessageDataDialog;

typedef union {
    DialogsAppMessageDataFileSelect file_select;
    DialogsAppMessageDataDialog dialog;
} DialogsAppData;

typedef union {
    bool bool_value;
    DialogMessageButton dialog_value;
} DialogsAppReturn;

typedef enum {
    DialogsAppCommandFileOpen,
    DialogsAppCommandDialog,
} DialogsAppCommand;

typedef struct {
    osSemaphoreId_t semaphore;
    DialogsAppCommand command;
    DialogsAppData* data;
    DialogsAppReturn* return_data;
} DialogsAppMessage;

#ifdef __cplusplus
}
#endif