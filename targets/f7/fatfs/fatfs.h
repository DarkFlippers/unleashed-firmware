#pragma once

#include "fatfs/ff.h"
#include "fatfs/ff_gen_drv.h"
#include "user_diskio.h"

#ifdef __cplusplus
extern "C" {
#endif

/** File system object */
extern FATFS fatfs_object;

/** Init file system driver */
void fatfs_init(void);

#ifdef __cplusplus
}
#endif
