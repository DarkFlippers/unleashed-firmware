#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <lib/digital_signal/digital_signal.h>
#include <lib/pulse_reader/pulse_reader.h>
#include "nfc_util.h"
#include <furi_hal_nfc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NFCV_FC (13560000.0f) /* MHz */
#define NFCV_RESP_SUBC1_PULSE_32 (1.0f / (NFCV_FC / 32) / 2.0f) /*  1.1799 µs */
#define NFCV_RESP_SUBC1_UNMOD_256 (256.0f / NFCV_FC) /* 18.8791 µs */

#define PULSE_DURATION_NS (128.0f * 1000000000.0f / NFCV_FC) /* ns */

#define DIGITAL_SIGNAL_UNIT_S (100000000000.0f)
#define DIGITAL_SIGNAL_UNIT_US (100000.0f)

#define NFCV_TOTAL_BLOCKS_MAX 256
#define NFCV_BLOCK_SIZE 4
#define NFCV_MAX_DUMP_SIZE (NFCV_BLOCK_SIZE * NFCV_TOTAL_BLOCKS_MAX)

/* helpers to calculate the send time based on DWT->CYCCNT */
#define NFCV_FDT_USEC(usec) (usec * 64)
#define NFCV_FDT_FC(ticks) (ticks * 6400 / 1356)

#define NFCV_FRAME_STATE_SOF1 0
#define NFCV_FRAME_STATE_SOF2 1
#define NFCV_FRAME_STATE_CODING_4 2
#define NFCV_FRAME_STATE_CODING_256 3
#define NFCV_FRAME_STATE_EOF 4
#define NFCV_FRAME_STATE_RESET 5

#define NFCV_SIG_SOF 0
#define NFCV_SIG_BIT0 1
#define NFCV_SIG_BIT1 2
#define NFCV_SIG_EOF 3

/* ISO15693 command codes */
#define ISO15693_INVENTORY 0x01
#define ISO15693_STAYQUIET 0x02
#define ISO15693_READBLOCK 0x20
#define ISO15693_WRITEBLOCK 0x21
#define ISO15693_LOCKBLOCK 0x22
#define ISO15693_READ_MULTI_BLOCK 0x23
#define ISO15693_WRITE_MULTI_BLOCK 0x24
#define ISO15693_SELECT 0x25
#define ISO15693_RESET_TO_READY 0x26
#define ISO15693_WRITE_AFI 0x27
#define ISO15693_LOCK_AFI 0x28
#define ISO15693_WRITE_DSFID 0x29
#define ISO15693_LOCK_DSFID 0x2A
#define ISO15693_GET_SYSTEM_INFO 0x2B
#define ISO15693_READ_MULTI_SECSTATUS 0x2C

/* ISO15693 RESPONSE ERROR CODES */
#define ISO15693_NOERROR 0x00
#define ISO15693_ERROR_CMD_NOT_SUP 0x01 // Command not supported
#define ISO15693_ERROR_CMD_NOT_REC 0x02 // Command not recognized (eg. parameter error)
#define ISO15693_ERROR_CMD_OPTION 0x03 // Command option not supported
#define ISO15693_ERROR_GENERIC 0x0F // No additional Info about this error
#define ISO15693_ERROR_BLOCK_UNAVAILABLE 0x10
#define ISO15693_ERROR_BLOCK_LOCKED_ALREADY 0x11 // cannot lock again
#define ISO15693_ERROR_BLOCK_LOCKED 0x12 // cannot be changed
#define ISO15693_ERROR_BLOCK_WRITE 0x13 // Writing was unsuccessful
#define ISO15693_ERROR_BLOCL_WRITELOCK 0x14 // Locking was unsuccessful

typedef enum {
    NfcVAuthMethodManual,
    NfcVAuthMethodTonieBox,
} NfcVAuthMethod;

typedef enum {
    NfcVTypePlain = 0,
    NfcVTypeSlix = 1,
    NfcVTypeSlixS = 2,
    NfcVTypeSlixL = 3,
    NfcVTypeSlix2 = 4,
} NfcVSubtype;

typedef enum {
    NfcVSendFlagsNormal = 0,
    NfcVSendFlagsSof = 1 << 0,
    NfcVSendFlagsCrc = 1 << 1,
    NfcVSendFlagsEof = 1 << 2,
    NfcVSendFlagsOneSubcarrier = 0,
    NfcVSendFlagsTwoSubcarrier = 1 << 3,
    NfcVSendFlagsLowRate = 0,
    NfcVSendFlagsHighRate = 1 << 4
} NfcVSendFlags;

typedef struct {
    uint8_t key_read[4];
    uint8_t key_write[4];
    uint8_t key_privacy[4];
    uint8_t key_destroy[4];
    uint8_t key_eas[4];
    uint8_t rand[2];
    bool privacy;
} NfcVSlixData;

typedef union {
    NfcVSlixData slix;
} NfcVSubtypeData;

typedef struct {
    PulseReader* reader_signal;
    DigitalSignal* nfcv_resp_pulse_32;
    DigitalSignal* nfcv_resp_unmod;
    DigitalSignal* nfcv_resp_one;
    DigitalSignal* nfcv_resp_zero;
    DigitalSignal* nfcv_resp_sof;
    DigitalSignal* nfcv_resp_eof;
    DigitalSignal* nfcv_resp_unmod_256;
    DigitalSignal* nfcv_resp_unmod_768;
    DigitalSequence* nfcv_signal;
} NfcVEmuAir;

typedef void (*NfcVEmuProtocolHandler)(
    FuriHalNfcTxRxContext* tx_rx,
    FuriHalNfcDevData* nfc_data,
    void* nfcv_data);
typedef bool (*NfcVEmuProtocolFilter)(
    FuriHalNfcTxRxContext* tx_rx,
    FuriHalNfcDevData* nfc_data,
    void* nfcv_data);

typedef struct {
    uint8_t flags; /* ISO15693-3 flags of the header as specified */
    uint8_t command; /* ISO15693-3 command at offset 1 as specified */
    bool addressed; /* ISO15693-3 flags: addressed frame */
    bool advanced; /* ISO15693-3 command: advanced command */
    uint8_t address_offset; /* ISO15693-3 offset of the address in frame, if addressed is set */
    uint8_t payload_offset; /* ISO15693-3 offset of the payload in frame */

    uint8_t response_buffer[128]; /* pre-allocated response buffer */
    NfcVSendFlags response_flags; /* flags to use when sending response */
    uint32_t send_time; /* timestamp when to send the response */

    NfcVEmuProtocolFilter emu_protocol_filter;
} NfcVEmuProtocolCtx;

typedef struct {
    /* common ISO15693 fields, being specified in ISO15693-3 */
    uint8_t dsfid;
    uint8_t afi;
    uint8_t ic_ref;
    uint16_t block_num;
    uint8_t block_size;
    uint8_t data[NFCV_MAX_DUMP_SIZE];

    /* specfic variant infos */
    NfcVSubtype sub_type;
    NfcVSubtypeData sub_data;
    NfcVAuthMethod auth_method;

    /* precalced air level data */
    NfcVEmuAir emu_air;

    uint8_t* frame; /* ISO15693-2 incoming raw data from air layer */
    uint8_t frame_length; /* ISO15693-2 length of incoming data */
    uint32_t eof_timestamp; /* ISO15693-2 EOF timestamp, read from DWT->CYCCNT */

    /* handler for the protocol layer as specified in ISO15693-3 */
    NfcVEmuProtocolHandler emu_protocol_handler;
    void* emu_protocol_ctx;

    /* runtime data */
    char last_command[128];
    char error[32];
} NfcVData;

typedef struct {
    uint16_t blocks_to_read;
    int16_t blocks_read;
} NfcVReader;

ReturnCode nfcv_read_blocks(NfcVReader* reader, NfcVData* data);
ReturnCode nfcv_read_sysinfo(FuriHalNfcDevData* nfc_data, NfcVData* data);
ReturnCode nfcv_inventory(uint8_t* uid);
bool nfcv_read_card(NfcVReader* reader, FuriHalNfcDevData* nfc_data, NfcVData* data);

void nfcv_emu_init(FuriHalNfcDevData* nfc_data, NfcVData* nfcv_data);
void nfcv_emu_deinit(NfcVData* nfcv_data);
bool nfcv_emu_loop(
    FuriHalNfcTxRxContext* tx_rx,
    FuriHalNfcDevData* nfc_data,
    NfcVData* nfcv_data,
    uint32_t timeout_ms);
void nfcv_emu_send(
    FuriHalNfcTxRxContext* tx_rx,
    NfcVData* nfcv,
    uint8_t* data,
    uint8_t length,
    NfcVSendFlags flags,
    uint32_t send_time);

#ifdef __cplusplus
}
#endif
