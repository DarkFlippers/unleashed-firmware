#pragma once

#include <furi.h>

#define SCSI_BLOCK_SIZE (0x200UL)

#define SCSI_SK_ILLEGAL_REQUEST (5)

#define SCSI_ASC_INVALID_COMMAND_OPERATION_CODE (0x20)
#define SCSI_ASC_LBA_OOB (0x21)
#define SCSI_ASC_INVALID_FIELD_IN_CDB (0x24)

typedef struct {
    void* ctx;
    bool (*read)(
        void* ctx,
        uint32_t lba,
        uint16_t count,
        uint8_t* out,
        uint32_t* out_len,
        uint32_t out_cap);
    bool (*write)(void* ctx, uint32_t lba, uint16_t count, uint8_t* buf, uint32_t len);
    uint32_t (*num_blocks)(void* ctx);
    void (*eject)(void* ctx);
} SCSIDeviceFunc;

typedef struct {
    SCSIDeviceFunc fn;

    uint8_t* cmd;
    uint8_t cmd_len;
    bool rx_done;
    bool tx_done;

    uint8_t sk; // sense key
    uint8_t asc; // additional sense code

    // command-specific data
    // valid from cmd_start to cmd_end
    union {
        struct {
            uint16_t count;
            uint32_t lba;
        } read_10; // SCSI_READ_10

        struct {
            uint16_t count;
            uint32_t lba;
        } write_10; // SCSI_WRITE_10
    };
} SCSISession;

bool scsi_cmd_start(SCSISession* scsi, uint8_t* cmd, uint8_t len);
bool scsi_cmd_rx_data(SCSISession* scsi, uint8_t* data, uint32_t len);
bool scsi_cmd_tx_data(SCSISession* scsi, uint8_t* data, uint32_t* len, uint32_t cap);
bool scsi_cmd_end(SCSISession* scsi);