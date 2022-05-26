#pragma once

#include <furi_hal_nfc.h>

// Largest tag is NTAG I2C Plus 2K, both data sectors plus SRAM
#define MF_UL_MAX_DUMP_SIZE ((238 + 256 + 16) * 4)

#define MF_UL_TEARING_FLAG_DEFAULT (0xBD)

#define MF_UL_HALT_START (0x50)
#define MF_UL_GET_VERSION_CMD (0x60)
#define MF_UL_READ_CMD (0x30)
#define MF_UL_FAST_READ_CMD (0x3A)
#define MF_UL_WRITE (0xA2)
#define MF_UL_FAST_WRITE (0xA6)
#define MF_UL_COMP_WRITE (0xA0)
#define MF_UL_READ_CNT (0x39)
#define MF_UL_INC_CNT (0xA5)
#define MF_UL_AUTH (0x1B)
#define MF_UL_READ_SIG (0x3C)
#define MF_UL_CHECK_TEARING (0x3E)
#define MF_UL_READ_VCSL (0x4B)
#define MF_UL_SECTOR_SELECT (0xC2)

typedef enum {
    MfUltralightTypeUnknown,
    MfUltralightTypeUL11,
    MfUltralightTypeUL21,
    MfUltralightTypeNTAG213,
    MfUltralightTypeNTAG215,
    MfUltralightTypeNTAG216,
    MfUltralightTypeNTAGI2C1K,
    MfUltralightTypeNTAGI2C2K,
    MfUltralightTypeNTAGI2CPlus1K,
    MfUltralightTypeNTAGI2CPlus2K,

    // Keep last for number of types calculation
    MfUltralightTypeNum,
} MfUltralightType;

typedef struct {
    uint8_t header;
    uint8_t vendor_id;
    uint8_t prod_type;
    uint8_t prod_subtype;
    uint8_t prod_ver_major;
    uint8_t prod_ver_minor;
    uint8_t storage_size;
    uint8_t protocol_type;
} MfUltralightVersion;

typedef struct {
    uint8_t sn0[3];
    uint8_t btBCC0;
    uint8_t sn1[4];
    uint8_t btBCC1;
    uint8_t internal;
    uint8_t lock[2];
    uint8_t otp[4];
} MfUltralightManufacturerBlock;

typedef struct {
    MfUltralightType type;
    MfUltralightVersion version;
    uint8_t signature[32];
    uint32_t counter[3];
    uint8_t tearing[3];
    uint16_t data_size;
    uint8_t data[MF_UL_MAX_DUMP_SIZE];
} MfUltralightData;

typedef struct {
    uint8_t pwd[4];
    union {
        uint8_t raw[2];
        uint16_t value;
    } pack;
} MfUltralightAuth;

typedef struct {
    uint16_t pages_to_read;
    int16_t pages_read;
    bool support_fast_read;
    bool support_tearing_flags;
    bool support_counters;
    bool support_signature;
} MfUltralightReader;

typedef struct {
    MfUltralightData data;
    bool support_fast_read;
    bool data_changed;
    bool comp_write_cmd_started;
    uint8_t comp_write_page_addr;
    MfUltralightAuth* auth_data;
    bool auth_success;
    uint8_t curr_sector;
    bool sector_select_cmd_started;
    bool ntag_i2c_plus_sector3_lockout;
} MfUltralightEmulator;

bool mf_ul_check_card_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK);

bool mf_ultralight_read_version(
    FuriHalNfcTxRxContext* tx_rx,
    MfUltralightReader* reader,
    MfUltralightData* data);

bool mf_ultralight_read_pages(
    FuriHalNfcTxRxContext* tx_rx,
    MfUltralightReader* reader,
    MfUltralightData* data);

bool mf_ultralight_fast_read_pages(
    FuriHalNfcTxRxContext* tx_rx,
    MfUltralightReader* reader,
    MfUltralightData* data);

bool mf_ultralight_read_signature(FuriHalNfcTxRxContext* tx_rx, MfUltralightData* data);

bool mf_ultralight_read_counters(FuriHalNfcTxRxContext* tx_rx, MfUltralightData* data);

bool mf_ultralight_read_tearing_flags(FuriHalNfcTxRxContext* tx_rx, MfUltralightData* data);

bool mf_ul_read_card(
    FuriHalNfcTxRxContext* tx_rx,
    MfUltralightReader* reader,
    MfUltralightData* data);

void mf_ul_prepare_emulation(MfUltralightEmulator* emulator, MfUltralightData* data);

bool mf_ul_prepare_emulation_response(
    uint8_t* buff_rx,
    uint16_t buff_rx_len,
    uint8_t* buff_tx,
    uint16_t* buff_tx_len,
    uint32_t* data_type,
    void* context);
