#pragma once
#include <furi.h>
#include "../storage_glue.h"
#include "../storage_sd_api.h"

#ifdef __cplusplus
extern "C" {
#endif

void storage_ext_init(StorageData* storage);
FS_Error sd_unmount_card(StorageData* storage);
FS_Error sd_format_card(StorageData* storage);
FS_Error sd_card_info(StorageData* storage, SDInfo* sd_info);
#ifdef __cplusplus
}
#endif
