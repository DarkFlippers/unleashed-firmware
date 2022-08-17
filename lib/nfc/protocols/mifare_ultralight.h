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

#define MF_UL_ACK (0xa)
#define MF_UL_NAK_INVALID_ARGUMENT (0x0)
#define MF_UL_NAK_AUTHLIM_REACHED (0x4)

#define MF_UL_NTAG203_COUNTER_PAGE (41)

typedef enum {
    MfUltralightAuthMethodManual,
    MfUltralightAuthMethodAmeebo,
    MfUltralightAuthMethodXiaomi,
} MfUltralightAuthMethod;

// Important: order matters; some features are based on positioning in this enum
typedef enum {
    MfUltralightTypeUnknown,
    MfUltralightTypeNTAG203,
    // Below have config pages and GET_VERSION support
    MfUltralightTypeUL11,
    MfUltralightTypeUL21,
    MfUltralightTypeNTAG213,
    MfUltralightTypeNTAG215,
    MfUltralightTypeNTAG216,
    // Below also have sector select
    // NTAG I2C's *does not* have regular config pages, so it's a bit of an odd duck
    MfUltralightTypeNTAGI2C1K,
    MfUltralightTypeNTAGI2C2K,
    // NTAG I2C Plus has stucture expected from NTAG21x
    MfUltralightTypeNTAGI2CPlus1K,
    MfUltralightTypeNTAGI2CPlus2K,

    // Keep last for number of types calculation
    MfUltralightTypeNum,
} MfUltralightType;

typedef enum {
    MfUltralightSupportNone = 0,
    MfUltralightSupportFastRead = 1 << 0,
    MfUltralightSupportTearingFlags = 1 << 1,
    MfUltralightSupportReadCounter = 1 << 2,
    MfUltralightSupportIncrCounter = 1 << 3,
    MfUltralightSupportSignature = 1 << 4,
    MfUltralightSupportFastWrite = 1 << 5,
    MfUltralightSupportCompatWrite = 1 << 6,
    MfUltralightSupportAuth = 1 << 7,
    MfUltralightSupportVcsl = 1 << 8,
    MfUltralightSupportSectorSelect = 1 << 9,
    // NTAG21x only has counter 2
    MfUltralightSupportSingleCounter = 1 << 10,
    // ASCII mirror is not a command, but handy to have as a flag
    MfUltralightSupportAsciiMirror = 1 << 11,
    // NTAG203 counter that's in memory rather than through a command
    MfUltralightSupportCounterInMemory = 1 << 12,
} MfUltralightFeatures;

typedef enum {
    MfUltralightMirrorNone,
    MfUltralightMirrorUid,
    MfUltralightMirrorCounter,
    MfUltralightMirrorUidCounter,
} MfUltralightMirrorConf;

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
    bool has_auth;
    MfUltralightAuthMethod auth_method;
    uint8_t auth_key[4];
    bool auth_success;
    uint16_t curr_authlim;
    uint16_t data_size;
    uint8_t data[MF_UL_MAX_DUMP_SIZE];
    uint16_t data_read;
} MfUltralightData;

typedef struct __attribute__((packed)) {
    union {
        uint8_t raw[4];
        uint32_t value;
    } pwd;
    union {
        uint8_t raw[2];
        uint16_t value;
    } pack;
} MfUltralightAuth;

// Common configuration pages for MFUL EV1, NTAG21x, and NTAG I2C Plus
typedef struct __attribute__((packed)) {
    union {
        uint8_t value;
        struct {
            uint8_t rfui1 : 2;
            bool strg_mod_en : 1;
            bool rfui2 : 1;
            uint8_t mirror_byte : 2;
            MfUltralightMirrorConf mirror_conf : 2;
        };
    } mirror;
    uint8_t rfui1;
    uint8_t mirror_page;
    uint8_t auth0;
    union {
        uint8_t value;
        struct {
            uint8_t authlim : 3;
            bool nfc_cnt_pwd_prot : 1;
            bool nfc_cnt_en : 1;
            bool nfc_dis_sec1 : 1; // NTAG I2C Plus only
            bool cfglck : 1;
            bool prot : 1;
        };
    } access;
    uint8_t vctid;
    uint8_t rfui2[2];
    MfUltralightAuth auth_data;
    uint8_t rfui3[2];
} MfUltralightConfigPages;

typedef struct {
    uint16_t pages_to_read;
    int16_t pages_read;
    MfUltralightFeatures supported_features;
} MfUltralightReader;

typedef struct {
    MfUltralightData data;
    MfUltralightConfigPages* config;
    // Most config values don't apply until power cycle, so cache config pages
    // for correct behavior
    MfUltralightConfigPages config_cache;
    MfUltralightFeatures supported_features;
    uint16_t page_num;
    bool data_changed;
    bool comp_write_cmd_started;
    uint8_t comp_write_page_addr;
    bool auth_success;
    uint8_t curr_sector;
    bool sector_select_cmd_started;
    bool ntag_i2c_plus_sector3_lockout;
    bool read_counter_incremented;
} MfUltralightEmulator;

void mf_ul_reset(MfUltralightData* data);

bool mf_ul_check_card_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK);

bool mf_ultralight_read_version(
    FuriHalNfcTxRxContext* tx_rx,
    MfUltralightReader* reader,
    MfUltralightData* data);

bool mf_ultralight_read_pages_direct(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t start_index,
    uint8_t* data);

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

bool mf_ultralight_authenticate(FuriHalNfcTxRxContext* tx_rx, uint32_t key, uint16_t* pack);

MfUltralightConfigPages* mf_ultralight_get_config_pages(MfUltralightData* data);

bool mf_ul_read_card(
    FuriHalNfcTxRxContext* tx_rx,
    MfUltralightReader* reader,
    MfUltralightData* data);

void mf_ul_reset_emulation(MfUltralightEmulator* emulator, bool is_power_cycle);

void mf_ul_prepare_emulation(MfUltralightEmulator* emulator, MfUltralightData* data);

bool mf_ul_prepare_emulation_response(
    uint8_t* buff_rx,
    uint16_t buff_rx_len,
    uint8_t* buff_tx,
    uint16_t* buff_tx_len,
    uint32_t* data_type,
    void* context);

uint32_t mf_ul_pwdgen_amiibo(FuriHalNfcDevData* data);

uint32_t mf_ul_pwdgen_xiaomi(FuriHalNfcDevData* data);
