#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sd_spi_io.h"
#include "fatfs/ff_gen_drv.h"

extern Diskio_drvTypeDef sd_fatfs_driver;

#ifdef __cplusplus
}
#endif