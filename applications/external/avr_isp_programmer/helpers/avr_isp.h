#pragma once

#include <furi_hal.h>

typedef struct AvrIsp AvrIsp;
typedef void (*AvrIspCallback)(void* context);

struct AvrIspSignature {
    uint8_t vendor;
    uint8_t part_family;
    uint8_t part_number;
};

typedef struct AvrIspSignature AvrIspSignature;

AvrIsp* avr_isp_alloc(void);

void avr_isp_free(AvrIsp* instance);

void avr_isp_set_tx_callback(AvrIsp* instance, AvrIspCallback callback, void* context);

bool avr_isp_auto_set_spi_speed_start_pmode(AvrIsp* instance);

AvrIspSignature avr_isp_read_signature(AvrIsp* instance);

void avr_isp_end_pmode(AvrIsp* instance);

bool avr_isp_erase_chip(AvrIsp* instance);

uint8_t avr_isp_spi_transaction(
    AvrIsp* instance,
    uint8_t cmd,
    uint8_t addr_hi,
    uint8_t addr_lo,
    uint8_t data);

bool avr_isp_read_page(
    AvrIsp* instance,
    uint32_t memtype,
    uint16_t addr,
    uint16_t page_size,
    uint8_t* data,
    uint32_t data_size);

bool avr_isp_write_page(
    AvrIsp* instance,
    uint32_t mem_type,
    uint32_t mem_size,
    uint16_t addr,
    uint16_t page_size,
    uint8_t* data,
    uint32_t data_size);

uint8_t avr_isp_read_lock_byte(AvrIsp* instance);

bool avr_isp_write_lock_byte(AvrIsp* instance, uint8_t lock);

uint8_t avr_isp_read_fuse_low(AvrIsp* instance);

bool avr_isp_write_fuse_low(AvrIsp* instance, uint8_t lfuse);

uint8_t avr_isp_read_fuse_high(AvrIsp* instance);

bool avr_isp_write_fuse_high(AvrIsp* instance, uint8_t hfuse);

uint8_t avr_isp_read_fuse_extended(AvrIsp* instance);

bool avr_isp_write_fuse_extended(AvrIsp* instance, uint8_t efuse);

void avr_isp_write_extended_addr(AvrIsp* instance, uint8_t extended_addr);