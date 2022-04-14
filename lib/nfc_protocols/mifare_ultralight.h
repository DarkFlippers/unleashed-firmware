#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MF_UL_MAX_DUMP_SIZE 1024

#define MF_UL_TEARING_FLAG_DEFAULT (0xBD)

#define MF_UL_HALT_START (0x50)
#define MF_UL_GET_VERSION_CMD (0x60)
#define MF_UL_READ_CMD (0x30)
#define MF_UL_FAST_READ_CMD (0x3A)
#define MF_UL_WRITE (0xA2)
#define MF_UL_COMP_WRITE (0xA0)
#define MF_UL_READ_CNT (0x39)
#define MF_UL_INC_CNT (0xA5)
#define MF_UL_AUTH (0x1B)
#define MF_UL_READ_SIG (0x3C)
#define MF_UL_CHECK_TEARING (0x3E)
#define MF_UL_READ_VCSL (0x4B)

typedef enum {
    MfUltralightTypeUnknown,
    MfUltralightTypeUL11,
    MfUltralightTypeUL21,
    MfUltralightTypeNTAG213,
    MfUltralightTypeNTAG215,
    MfUltralightTypeNTAG216,

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
} MifareUlData;

typedef struct {
    uint8_t pwd[4];
    union {
        uint8_t raw[2];
        uint16_t value;
    } pack;
} MifareUlAuthData;

typedef struct {
    uint8_t pages_to_read;
    uint8_t pages_read;
    bool support_fast_read;
    bool data_changed;
    MifareUlData data;
    MifareUlAuthData* auth_data;
    bool comp_write_cmd_started;
    uint8_t comp_write_page_addr;
} MifareUlDevice;

bool mf_ul_check_card_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK);

uint16_t mf_ul_prepare_get_version(uint8_t* dest);
void mf_ul_parse_get_version_response(uint8_t* buff, MifareUlDevice* mf_ul_read);
void mf_ul_set_default_version(MifareUlDevice* mf_ul_read);

uint16_t mf_ul_prepare_read_signature(uint8_t* dest);
void mf_ul_parse_read_signature_response(uint8_t* buff, MifareUlDevice* mf_ul_read);

uint16_t mf_ul_prepare_read_cnt(uint8_t* dest, uint8_t cnt_index);
void mf_ul_parse_read_cnt_response(uint8_t* buff, uint8_t cnt_index, MifareUlDevice* mf_ul_read);

uint16_t mf_ul_prepare_inc_cnt(uint8_t* dest, uint8_t cnt_index, uint32_t value);

uint16_t mf_ul_prepare_check_tearing(uint8_t* dest, uint8_t cnt_index);
void mf_ul_parse_check_tearing_response(
    uint8_t* buff,
    uint8_t cnt_index,
    MifareUlDevice* mf_ul_read);

uint16_t mf_ul_prepare_read(uint8_t* dest, uint8_t start_page);
void mf_ul_parse_read_response(uint8_t* buff, uint16_t page_addr, MifareUlDevice* mf_ul_read);

uint16_t mf_ul_prepare_fast_read(uint8_t* dest, uint8_t start_page, uint8_t end_page);
void mf_ul_parse_fast_read_response(
    uint8_t* buff,
    uint8_t start_page,
    uint8_t end_page,
    MifareUlDevice* mf_ul_read);

uint16_t mf_ul_prepare_write(uint8_t* dest, uint16_t page_addr, uint32_t data);

void mf_ul_prepare_emulation(MifareUlDevice* mf_ul_emulate, MifareUlData* data);
bool mf_ul_prepare_emulation_response(
    uint8_t* buff_rx,
    uint16_t buff_rx_len,
    uint8_t* buff_tx,
    uint16_t* buff_tx_len,
    uint32_t* data_type,
    void* context);