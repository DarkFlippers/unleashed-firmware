#pragma once

#include <flipper_format.h>

#include <one_wire/one_wire_host.h>
#include <one_wire/one_wire_slave.h>

#define DALLAS_COMMON_MANUFACTURER_NAME "Dallas"

#define DALLAS_COMMON_CMD_READ_ROM 0x33U
#define DALLAS_COMMON_CMD_MATCH_ROM 0x55U
#define DALLAS_COMMON_CMD_SKIP_ROM 0xCCU
#define DALLAS_COMMON_CMD_COND_SEARCH 0xECU
#define DALLAS_COMMON_CMD_SEARCH_ROM 0xF0U

#define DALLAS_COMMON_CMD_READ_SCRATCH 0xAAU
#define DALLAS_COMMON_CMD_WRITE_SCRATCH 0x0FU
#define DALLAS_COMMON_CMD_COPY_SCRATCH 0x55U

#define DALLAS_COMMON_CMD_READ_MEM 0xF0U

#define DALLAS_COMMON_CMD_OVERDRIVE_SKIP_ROM 0x3CU
#define DALLAS_COMMON_CMD_OVERDRIVE_MATCH_ROM 0x69U

typedef enum {
    DallasCommonCommandStateIdle,
    DallasCommonCommandStateRomCmd,
    DallasCommonCommandStateMemCmd,
} DallasCommonCommandState;

typedef union {
    struct {
        uint8_t family_code;
        uint8_t serial_number[6];
        uint8_t checksum;
    } fields;
    uint8_t bytes[8];
} DallasCommonRomData;

typedef union {
    struct {
        uint8_t address_lo;
        uint8_t address_hi;
        uint8_t status;
    } fields;
    uint8_t bytes[3];
} DallasCommonAddressRegs;

/* Standard(ish) iButton commands */
bool dallas_common_skip_rom(OneWireHost* host);

bool dallas_common_read_rom(OneWireHost* host, DallasCommonRomData* rom_data);

bool dallas_common_write_scratchpad(
    OneWireHost* host,
    uint16_t address,
    const uint8_t* data,
    size_t data_size);

bool dallas_common_read_scratchpad(
    OneWireHost* host,
    DallasCommonAddressRegs* regs,
    uint8_t* data,
    size_t data_size);

bool dallas_common_copy_scratchpad(
    OneWireHost* host,
    const DallasCommonAddressRegs* regs,
    uint32_t timeout_us);

bool dallas_common_read_mem(OneWireHost* host, uint16_t address, uint8_t* data, size_t data_size);

/* Combined operations */
bool dallas_common_write_mem(
    OneWireHost* host,
    uint32_t timeout_us,
    size_t page_size,
    const uint8_t* data,
    size_t data_size);

/* Emulation */
bool dallas_common_emulate_search_rom(OneWireSlave* bus, const DallasCommonRomData* rom_data);

bool dallas_common_emulate_read_rom(OneWireSlave* bus, const DallasCommonRomData* rom_data);

bool dallas_common_emulate_read_mem(OneWireSlave* bus, const uint8_t* data, size_t data_size);

/* Save & Load */
bool dallas_common_save_rom_data(FlipperFormat* ff, const DallasCommonRomData* rom_data);

bool dallas_common_load_rom_data(
    FlipperFormat* ff,
    uint32_t format_version,
    DallasCommonRomData* rom_data);

/* Miscellaneous */
bool dallas_common_is_valid_crc(const DallasCommonRomData* rom_data);

void dallas_common_render_brief_data(
    FuriString* result,
    const DallasCommonRomData* rom_data,
    const uint8_t* sram_data,
    size_t sram_data_size);

void dallas_common_render_crc_error(FuriString* result, const DallasCommonRomData* rom_data);

void dallas_common_apply_edits(DallasCommonRomData* rom_data, uint8_t family_code);
