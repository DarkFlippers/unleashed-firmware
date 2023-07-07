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

/* true: modulating releases load, false: modulating adds load resistor to field coil */
#define NFCV_LOAD_MODULATION_POLARITY (false)

#define NFCV_FC (13560000.0f) /* MHz */
#define NFCV_RESP_SUBC1_PULSE_32 (1.0f / (NFCV_FC / 32) / 2.0f) /*  1.1799 µs */
#define NFCV_RESP_SUBC1_UNMOD_256 (256.0f / NFCV_FC) /* 18.8791 µs */
#define NFCV_PULSE_DURATION_NS (128.0f * 1000000000.0f / NFCV_FC)

/* ISO/IEC 15693-3:2019(E) 10.4.12: maximum number of blocks is defined as 256 */
#define NFCV_BLOCKS_MAX 256
/* ISO/IEC 15693-3:2019(E) 10.4.12: maximum size of blocks is defined as 32 */
#define NFCV_BLOCKSIZE_MAX 32
/* the resulting memory size a card can have */
#define NFCV_MEMSIZE_MAX (NFCV_BLOCKS_MAX * NFCV_BLOCKSIZE_MAX)
/* ISO/IEC 15693-3:2019(E) 7.1b: standard allows up to 8192, the maxium frame length that we are expected to receive/send is less */
#define NFCV_FRAMESIZE_MAX (1 + NFCV_MEMSIZE_MAX + NFCV_BLOCKS_MAX)

/* maximum string length for log messages */
#define NFCV_LOG_STR_LEN 128
/* maximum of pulses to be buffered by pulse reader */
#define NFCV_PULSE_BUFFER 512

//#define NFCV_DIAGNOSTIC_DUMPS
//#define NFCV_DIAGNOSTIC_DUMP_SIZE 256
//#define NFCV_VERBOSE

/* helpers to calculate the send time based on DWT->CYCCNT */
#define NFCV_FDT_USEC(usec) ((usec)*64)
#define NFCV_FDT_FC(ticks) ((ticks)*6400 / 1356)

/* state machine when receiving frame bits */
#define NFCV_FRAME_STATE_SOF1 0
#define NFCV_FRAME_STATE_SOF2 1
#define NFCV_FRAME_STATE_CODING_4 2
#define NFCV_FRAME_STATE_CODING_256 3
#define NFCV_FRAME_STATE_EOF 4
#define NFCV_FRAME_STATE_RESET 5

/* sequences for every section of a frame */
#define NFCV_SIG_SOF 0
#define NFCV_SIG_BIT0 1
#define NFCV_SIG_BIT1 2
#define NFCV_SIG_EOF 3
#define NFCV_SIG_LOW_SOF 4
#define NFCV_SIG_LOW_BIT0 5
#define NFCV_SIG_LOW_BIT1 6
#define NFCV_SIG_LOW_EOF 7

/* various constants */
#define NFCV_COMMAND_RETRIES 5
#define NFCV_UID_LENGTH 8

/* ISO15693 protocol flags */
typedef enum {
    /* ISO15693 protocol flags when INVENTORY is NOT set */
    NFCV_REQ_FLAG_SUB_CARRIER = (1 << 0),
    NFCV_REQ_FLAG_DATA_RATE = (1 << 1),
    NFCV_REQ_FLAG_INVENTORY = (1 << 2),
    NFCV_REQ_FLAG_PROTOCOL_EXT = (1 << 3),
    NFCV_REQ_FLAG_SELECT = (1 << 4),
    NFCV_REQ_FLAG_ADDRESS = (1 << 5),
    NFCV_REQ_FLAG_OPTION = (1 << 6),
    /* ISO15693 protocol flags when INVENTORY flag is set */
    NFCV_REQ_FLAG_AFI = (1 << 4),
    NFCV_REQ_FLAG_NB_SLOTS = (1 << 5)
} NfcVRequestFlags;

/* ISO15693 protocol flags */
typedef enum {
    NFCV_RES_FLAG_ERROR = (1 << 0),
    NFCV_RES_FLAG_VALIDITY = (1 << 1),
    NFCV_RES_FLAG_FINAL = (1 << 2),
    NFCV_RES_FLAG_PROTOCOL_EXT = (1 << 3),
    NFCV_RES_FLAG_SEC_LEN1 = (1 << 4),
    NFCV_RES_FLAG_SEC_LEN2 = (1 << 5),
    NFCV_RES_FLAG_WAIT_EXT = (1 << 6),
} NfcVRsponseFlags;

/* flags for SYSINFO response */
typedef enum {
    NFCV_SYSINFO_FLAG_DSFID = (1 << 0),
    NFCV_SYSINFO_FLAG_AFI = (1 << 1),
    NFCV_SYSINFO_FLAG_MEMSIZE = (1 << 2),
    NFCV_SYSINFO_FLAG_ICREF = (1 << 3)
} NfcVSysinfoFlags;

/* ISO15693 command codes */
typedef enum {
    /* mandatory command codes */
    NFCV_CMD_INVENTORY = 0x01,
    NFCV_CMD_STAY_QUIET = 0x02,
    /* optional command codes */
    NFCV_CMD_READ_BLOCK = 0x20,
    NFCV_CMD_WRITE_BLOCK = 0x21,
    NFCV_CMD_LOCK_BLOCK = 0x22,
    NFCV_CMD_READ_MULTI_BLOCK = 0x23,
    NFCV_CMD_WRITE_MULTI_BLOCK = 0x24,
    NFCV_CMD_SELECT = 0x25,
    NFCV_CMD_RESET_TO_READY = 0x26,
    NFCV_CMD_WRITE_AFI = 0x27,
    NFCV_CMD_LOCK_AFI = 0x28,
    NFCV_CMD_WRITE_DSFID = 0x29,
    NFCV_CMD_LOCK_DSFID = 0x2A,
    NFCV_CMD_GET_SYSTEM_INFO = 0x2B,
    NFCV_CMD_READ_MULTI_SECSTATUS = 0x2C,
    /* advanced command codes */
    NFCV_CMD_ADVANCED = 0xA0,
    /* flipper zero custom command codes */
    NFCV_CMD_CUST_ECHO_MODE = 0xDE,
    NFCV_CMD_CUST_ECHO_DATA = 0xDF
} NfcVCommands;

/* ISO15693 Response error codes */
typedef enum {
    NFCV_NOERROR = 0x00,
    NFCV_ERROR_CMD_NOT_SUP = 0x01, // Command not supported
    NFCV_ERROR_CMD_NOT_REC = 0x02, // Command not recognized (eg. parameter error)
    NFCV_ERROR_CMD_OPTION = 0x03, // Command option not supported
    NFCV_ERROR_GENERIC = 0x0F, // No additional Info about this error
    NFCV_ERROR_BLOCK_UNAVAILABLE = 0x10,
    NFCV_ERROR_BLOCK_LOCKED_ALREADY = 0x11, // cannot lock again
    NFCV_ERROR_BLOCK_LOCKED = 0x12, // cannot be changed
    NFCV_ERROR_BLOCK_WRITE = 0x13, // Writing was unsuccessful
    NFCV_ERROR_BLOCL_WRITELOCK = 0x14 // Locking was unsuccessful
} NfcVErrorcodes;

typedef enum {
    NfcVLockBitDsfid = 1 << 0,
    NfcVLockBitAfi = 1 << 1,
    NfcVLockBitEas = 1 << 2,
    NfcVLockBitPpl = 1 << 3,
} NfcVLockBits;

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
    NfcVTypeSniff = 255,
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

/* SLIX specific config flags */
typedef enum {
    NfcVSlixDataFlagsNone = 0,
    NfcVSlixDataFlagsHasKeyRead = 1 << 0,
    NfcVSlixDataFlagsHasKeyWrite = 1 << 1,
    NfcVSlixDataFlagsHasKeyPrivacy = 1 << 2,
    NfcVSlixDataFlagsHasKeyDestroy = 1 << 3,
    NfcVSlixDataFlagsHasKeyEas = 1 << 4,
    NfcVSlixDataFlagsValidKeyRead = 1 << 8,
    NfcVSlixDataFlagsValidKeyWrite = 1 << 9,
    NfcVSlixDataFlagsValidKeyPrivacy = 1 << 10,
    NfcVSlixDataFlagsValidKeyDestroy = 1 << 11,
    NfcVSlixDataFlagsValidKeyEas = 1 << 12,
    NfcVSlixDataFlagsPrivacy = 1 << 16,
    NfcVSlixDataFlagsDestroyed = 1 << 17
} NfcVSlixDataFlags;

/* abstract the file read/write operations for all SLIX types to reduce duplicated code */
typedef enum {
    SlixFeatureRead = 1 << 0,
    SlixFeatureWrite = 1 << 1,
    SlixFeaturePrivacy = 1 << 2,
    SlixFeatureDestroy = 1 << 3,
    SlixFeatureEas = 1 << 4,
    SlixFeatureSignature = 1 << 5,
    SlixFeatureProtection = 1 << 6,

    SlixFeatureSlix = SlixFeatureEas,
    SlixFeatureSlixS =
        (SlixFeatureRead | SlixFeatureWrite | SlixFeaturePrivacy | SlixFeatureDestroy |
         SlixFeatureEas),
    SlixFeatureSlixL = (SlixFeaturePrivacy | SlixFeatureDestroy | SlixFeatureEas),
    SlixFeatureSlix2 =
        (SlixFeatureRead | SlixFeatureWrite | SlixFeaturePrivacy | SlixFeatureDestroy |
         SlixFeatureEas | SlixFeatureSignature | SlixFeatureProtection),
} SlixTypeFeatures;

typedef struct {
    uint32_t flags;
    uint8_t key_read[4];
    uint8_t key_write[4];
    uint8_t key_privacy[4];
    uint8_t key_destroy[4];
    uint8_t key_eas[4];
    uint8_t rand[2];
    uint8_t signature[32];
    /* SLIX2 options */
    uint8_t pp_pointer;
    uint8_t pp_condition;
} NfcVSlixData;

typedef union {
    NfcVSlixData slix;
} NfcVSubtypeData;

typedef struct {
    DigitalSignal* nfcv_resp_sof;
    DigitalSignal* nfcv_resp_one;
    DigitalSignal* nfcv_resp_zero;
    DigitalSignal* nfcv_resp_eof;
} NfcVEmuAirSignals;

typedef struct {
    PulseReader* reader_signal;
    DigitalSignal* nfcv_resp_pulse; /* pulse length, fc/32 */
    DigitalSignal* nfcv_resp_half_pulse; /* half pulse length, fc/32 */
    DigitalSignal* nfcv_resp_unmod; /* unmodulated length 256/fc */
    NfcVEmuAirSignals signals_high;
    NfcVEmuAirSignals signals_low;
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

/* the default ISO15693 handler context */
typedef struct {
    uint8_t flags; /* ISO15693-3 flags of the header as specified */
    uint8_t command; /* ISO15693-3 command at offset 1 as specified */
    bool selected; /* ISO15693-3 flags: selected frame */
    bool addressed; /* ISO15693-3 flags: addressed frame */
    bool advanced; /* ISO15693-3 command: advanced command */
    uint8_t address_offset; /* ISO15693-3 offset of the address in frame, if addressed is set */
    uint8_t payload_offset; /* ISO15693-3 offset of the payload in frame */

    uint8_t response_buffer[NFCV_FRAMESIZE_MAX]; /* pre-allocated response buffer */
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
    uint8_t data[NFCV_MEMSIZE_MAX];
    uint8_t security_status[1 + NFCV_BLOCKS_MAX];
    bool selected;
    bool quiet;

    bool modified;
    bool ready;
    bool echo_mode;

    /* specfic variant infos */
    NfcVSubtype sub_type;
    NfcVSubtypeData sub_data;
    NfcVAuthMethod auth_method;

    /* precalced air level data */
    NfcVEmuAir emu_air;

    uint8_t* frame; /* [NFCV_FRAMESIZE_MAX] ISO15693-2 incoming raw data from air layer */
    uint8_t frame_length; /* ISO15693-2 length of incoming data */
    uint32_t eof_timestamp; /* ISO15693-2 EOF timestamp, read from DWT->CYCCNT */

    /* handler for the protocol layer as specified in ISO15693-3 */
    NfcVEmuProtocolHandler emu_protocol_handler;
    void* emu_protocol_ctx;
    /* runtime data */
    char last_command[NFCV_LOG_STR_LEN];
    char error[NFCV_LOG_STR_LEN];
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
