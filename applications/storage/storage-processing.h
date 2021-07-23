#pragma once
#include <furi.h>
#include "storage.h"
#include "storage-i.h"
#include "storage-message.h"
#include "storage-glue.h"

#ifdef __cplusplus
extern "C" {
#endif

void storage_process_message(Storage* app, StorageMessage* message);

#ifdef __cplusplus
}
#endif