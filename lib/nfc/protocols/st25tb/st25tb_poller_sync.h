#pragma once

#include "st25tb.h"
#include <nfc/nfc.h>

#ifdef __cplusplus
extern "C" {
#endif

St25tbError st25tb_poller_sync_read_block(Nfc* nfc, uint8_t block_num, uint32_t* block);

St25tbError st25tb_poller_sync_write_block(Nfc* nfc, uint8_t block_num, uint32_t block);

St25tbError st25tb_poller_sync_detect_type(Nfc* nfc, St25tbType* type);

St25tbError st25tb_poller_sync_read(Nfc* nfc, St25tbData* data);

#ifdef __cplusplus
}
#endif
