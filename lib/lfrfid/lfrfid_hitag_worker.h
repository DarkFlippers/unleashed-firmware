#pragma once
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_rfid.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>
#include <flipper_format/flipper_format.h>
//#include <toolbox/protocols/protocol_dict.h>
#include "protocols/lfrfid_protocols.h"

#include <toolbox/stream/file_stream.h>
#include <toolbox/buffer_stream.h>
#include <toolbox/varint.h>
#include <tools/varint_pair.h>

#include <inttypes.h>

#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_comp.h>
#include <stm32wbxx_ll_dma.h>

#include <lfrfid_worker_i.h>

#include <dolphin/dolphin.h>

#define HITAG_BLOCKS 16
#define HITAG_BLOCKPAGES 4
#define HITAG_PAGES 64
#define HITAG_PAGEBYTES 4

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LFRFIDHitagWorkerSettingRead,
    LFRFIDHitagWorkerSettingEmulate,
} LFRFIDHitagWorkerSetting;

typedef enum {
    LFRFIDHitagStatusScanning,
    LFRFIDHitagStatusDetected,
    LFRFIDHitagStatusRead,
} LFRFIDHitagStatus;

typedef struct LFRFIDHitagWorker LFRFIDHitagWorker;

/**
 * @brief Get the tag read status
 *
 * @return tag read status
 */
LFRFIDHitagStatus lfrfid_hitag_worker_get_status(LFRFIDHitagWorker* worker);

/**
 * @brief Allocate a new LFRFIDHitagWorker instance
 * 
 * @return LFRFIDHitagWorker* 
 */
LFRFIDHitagWorker* lfrfid_hitag_worker_alloc(ProtocolDict* dict);

/**
 * @brief Free a LFRFIDHitagWorker instance
 * 
 * @param worker LFRFIDHitagWorker instance
 */
void lfrfid_hitag_worker_free(LFRFIDHitagWorker* worker);

/**
 * @brief Start hitag worker  from own generated field
 * 
 * @param worker LFRFIDHitagWorker instance
 * @param setting read/emulate
 */
void lfrfid_hitag_worker_start(LFRFIDHitagWorker* worker, LFRFIDHitagWorkerSetting setting);

/**
 * @brief Stop worker
 * 
 * @param worker 
 */
void lfrfid_hitag_worker_stop(LFRFIDHitagWorker* worker);

#ifdef __cplusplus
}
#endif