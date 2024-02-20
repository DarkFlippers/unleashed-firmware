#include "mass_storage_scsi.h"

#include <core/log.h>

#define TAG "MassStorageSCSI"

#define SCSI_TEST_UNIT_READY (0x00)
#define SCSI_REQUEST_SENSE (0x03)
#define SCSI_INQUIRY (0x12)
#define SCSI_READ_FORMAT_CAPACITIES (0x23)
#define SCSI_READ_CAPACITY_10 (0x25)
#define SCSI_MODE_SENSE_6 (0x1A)
#define SCSI_READ_10 (0x28)
#define SCSI_PREVENT_MEDIUM_REMOVAL (0x1E)
#define SCSI_START_STOP_UNIT (0x1B)
#define SCSI_WRITE_10 (0x2A)

bool scsi_cmd_start(SCSISession* scsi, uint8_t* cmd, uint8_t len) {
    if(!len) {
        scsi->sk = SCSI_SK_ILLEGAL_REQUEST;
        scsi->asc = SCSI_ASC_INVALID_COMMAND_OPERATION_CODE;
        return false;
    }
    FURI_LOG_T(TAG, "START %02X", cmd[0]);
    scsi->cmd = cmd;
    scsi->cmd_len = len;
    scsi->rx_done = false;
    scsi->tx_done = false;
    switch(cmd[0]) {
    case SCSI_WRITE_10: {
        if(len < 10) return false;
        scsi->write_10.lba = cmd[2] << 24 | cmd[3] << 16 | cmd[4] << 8 | cmd[5];
        scsi->write_10.count = cmd[7] << 8 | cmd[8];
        FURI_LOG_D(TAG, "SCSI_WRITE_10 %08lX %04X", scsi->write_10.lba, scsi->write_10.count);
        return true;
    }; break;
    case SCSI_READ_10: {
        if(len < 10) return false;
        scsi->read_10.lba = cmd[2] << 24 | cmd[3] << 16 | cmd[4] << 8 | cmd[5];
        scsi->read_10.count = cmd[7] << 8 | cmd[8];
        FURI_LOG_D(TAG, "SCSI_READ_10 %08lX %04X", scsi->read_10.lba, scsi->read_10.count);
        return true;
    }; break;
    }
    return true;
}

bool scsi_cmd_rx_data(SCSISession* scsi, uint8_t* data, uint32_t len) {
    FURI_LOG_T(TAG, "RX %02X len %lu", scsi->cmd[0], len);
    if(scsi->rx_done) return false;
    switch(scsi->cmd[0]) {
    case SCSI_WRITE_10: {
        uint32_t block_size = SCSI_BLOCK_SIZE;
        uint16_t blocks = len / block_size;
        bool result =
            scsi->fn.write(scsi->fn.ctx, scsi->write_10.lba, blocks, data, blocks * block_size);
        scsi->write_10.lba += blocks;
        scsi->write_10.count -= blocks;
        if(!scsi->write_10.count) {
            scsi->rx_done = true;
        }
        return result;
    }; break;
    default: {
        FURI_LOG_W(TAG, "unexpected scsi rx data cmd=%02X", scsi->cmd[0]);
        scsi->sk = SCSI_SK_ILLEGAL_REQUEST;
        scsi->asc = SCSI_ASC_INVALID_COMMAND_OPERATION_CODE;
        return false;
    }; break;
    }
}

bool scsi_cmd_tx_data(SCSISession* scsi, uint8_t* data, uint32_t* len, uint32_t cap) {
    FURI_LOG_T(TAG, "TX %02X cap %lu", scsi->cmd[0], cap);
    if(scsi->tx_done) return false;
    switch(scsi->cmd[0]) {
    case SCSI_REQUEST_SENSE: {
        FURI_LOG_D(TAG, "SCSI_REQUEST_SENSE");
        if(cap < 18) return false;
        memset(data, 0, cap);
        data[0] = 0x70; // fixed format sense data
        data[1] = 0; // obsolete
        data[2] = scsi->sk; // sense key
        data[3] = 0; // information
        data[4] = 0; // information
        data[5] = 0; // information
        data[6] = 0; // information
        data[7] = 10; // additional sense length (len-8)
        data[8] = 0; // command specific information
        data[9] = 0; // command specific information
        data[10] = 0; // command specific information
        data[11] = 0; // command specific information
        data[12] = scsi->asc; // additional sense code
        data[13] = 0; // additional sense code qualifier
        data[14] = 0; // field replaceable unit code
        data[15] = 0; // sense key specific information
        data[16] = 0; // sense key specific information
        data[17] = 0; // sense key specific information
        *len = 18;
        scsi->sk = 0;
        scsi->asc = 0;
        scsi->tx_done = true;
        return true;
    }; break;
    case SCSI_INQUIRY: {
        FURI_LOG_D(TAG, "SCSI_INQUIRY");
        if(scsi->cmd_len < 5) return false;

        if(cap < 36) return false;

        bool evpd = scsi->cmd[1] & 1;
        uint8_t page_code = scsi->cmd[2];
        if(evpd == 0) {
            if(page_code != 0) return false;

            data[0] = 0x00; // device type: direct access block device
            data[1] = 0x80; // removable: true
            data[2] = 0x04; // version
            data[3] = 0x02; // response data format
            data[4] = 31; // additional length (len - 5)
            data[5] = 0; // flags
            data[6] = 0; // flags
            data[7] = 0; // flags
            memcpy(data + 8, "Flipper ", 8); // vendor id
            memcpy(data + 16, "Mass Storage    ", 16); // product id
            memcpy(data + 32, "0001", 4); // product revision level
            *len = 36;
            scsi->tx_done = true;
            return true;
        } else {
            if(page_code != 0x80) {
                FURI_LOG_W(TAG, "Unsupported VPD code %02X", page_code);
                return false;
            }
            data[0] = 0x00;
            data[1] = 0x80;
            data[2] = 0x00;
            data[3] = 0x01; // Serial len
            data[4] = '0';
            *len = 5;
            scsi->tx_done = true;
            return true;
        }
    }; break;
    case SCSI_READ_FORMAT_CAPACITIES: {
        FURI_LOG_D(TAG, "SCSI_READ_FORMAT_CAPACITIES");
        if(cap < 12) {
            return false;
        }
        uint32_t n_blocks = scsi->fn.num_blocks(scsi->fn.ctx);
        uint32_t block_size = SCSI_BLOCK_SIZE;
        // Capacity List Header
        data[0] = 0;
        data[1] = 0;
        data[2] = 0;
        data[3] = 8;

        // Capacity Descriptor
        data[4] = (n_blocks - 1) >> 24;
        data[5] = (n_blocks - 1) >> 16;
        data[6] = (n_blocks - 1) >> 8;
        data[7] = (n_blocks - 1) & 0xFF;
        data[8] = 0x02; // Formatted media
        data[9] = block_size >> 16;
        data[10] = block_size >> 8;
        data[11] = block_size & 0xFF;
        *len = 12;
        scsi->tx_done = true;
        return true;
    }; break;
    case SCSI_READ_CAPACITY_10: {
        FURI_LOG_D(TAG, "SCSI_READ_CAPACITY_10");
        if(cap < 8) return false;
        uint32_t n_blocks = scsi->fn.num_blocks(scsi->fn.ctx);
        uint32_t block_size = SCSI_BLOCK_SIZE;
        data[0] = (n_blocks - 1) >> 24;
        data[1] = (n_blocks - 1) >> 16;
        data[2] = (n_blocks - 1) >> 8;
        data[3] = (n_blocks - 1) & 0xFF;
        data[4] = block_size >> 24;
        data[5] = block_size >> 16;
        data[6] = block_size >> 8;
        data[7] = block_size & 0xFF;
        *len = 8;
        scsi->tx_done = true;
        return true;
    }; break;
    case SCSI_MODE_SENSE_6: {
        FURI_LOG_D(TAG, "SCSI_MODE_SENSE_6 %lu", cap);
        if(cap < 4) return false;
        data[0] = 3; // mode data length (len - 1)
        data[1] = 0; // medium type
        data[2] = 0; // device-specific parameter
        data[3] = 0; // block descriptor length
        *len = 4;
        scsi->tx_done = true;
        return true;
    }; break;
    case SCSI_READ_10: {
        uint32_t block_size = SCSI_BLOCK_SIZE;
        bool result =
            scsi->fn.read(scsi->fn.ctx, scsi->read_10.lba, scsi->read_10.count, data, len, cap);
        *len -= *len % block_size;
        uint16_t blocks = *len / block_size;
        scsi->read_10.lba += blocks;
        scsi->read_10.count -= blocks;
        if(!scsi->read_10.count) {
            scsi->tx_done = true;
        }
        return result;
    }; break;
    default: {
        FURI_LOG_W(TAG, "unexpected scsi tx data cmd=%02X", scsi->cmd[0]);
        scsi->sk = SCSI_SK_ILLEGAL_REQUEST;
        scsi->asc = SCSI_ASC_INVALID_COMMAND_OPERATION_CODE;
        return false;
    }; break;
    }
}

bool scsi_cmd_end(SCSISession* scsi) {
    FURI_LOG_T(TAG, "END %02X", scsi->cmd[0]);
    uint8_t* cmd = scsi->cmd;
    uint8_t len = scsi->cmd_len;
    scsi->cmd = NULL;
    scsi->cmd_len = 0;
    switch(cmd[0]) {
    case SCSI_WRITE_10:
        return scsi->rx_done;

    case SCSI_REQUEST_SENSE:
    case SCSI_INQUIRY:
    case SCSI_READ_FORMAT_CAPACITIES:
    case SCSI_READ_CAPACITY_10:
    case SCSI_MODE_SENSE_6:
    case SCSI_READ_10:
        return scsi->tx_done;

    case SCSI_TEST_UNIT_READY: {
        FURI_LOG_D(TAG, "SCSI_TEST_UNIT_READY");
        return true;
    }; break;
    case SCSI_PREVENT_MEDIUM_REMOVAL: {
        if(len < 6) return false;
        bool prevent = cmd[5];
        FURI_LOG_D(TAG, "SCSI_PREVENT_MEDIUM_REMOVAL prevent=%d", prevent);
        return !prevent;
    }; break;
    case SCSI_START_STOP_UNIT: {
        if(len < 6) return false;
        bool eject = (cmd[4] & 2) != 0;
        bool start = (cmd[4] & 1) != 0;
        FURI_LOG_D(TAG, "SCSI_START_STOP_UNIT eject=%d start=%d", eject, start);
        if(eject) {
            scsi->fn.eject(scsi->fn.ctx);
        }
        return true;
    }; break;
    default: {
        FURI_LOG_W(TAG, "unexpected scsi cmd=%02X", cmd[0]);
        scsi->sk = SCSI_SK_ILLEGAL_REQUEST;
        scsi->asc = SCSI_ASC_INVALID_COMMAND_OPERATION_CODE;
        return false;
    }; break;
    }
}
