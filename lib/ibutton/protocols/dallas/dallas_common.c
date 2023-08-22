#include "dallas_common.h"

#include <core/common_defines.h>
#include <one_wire/maxim_crc.h>

#define BITS_IN_BYTE 8U

#define DALLAS_COMMON_ROM_DATA_KEY_V1 "Data"
#define DALLAS_COMMON_ROM_DATA_KEY_V2 "Rom Data"

#define DALLAS_COMMON_COPY_SCRATCH_MIN_TIMEOUT_US 5U
#define DALLAS_COMMON_COPY_SCRATCH_POLL_COUNT 20U

#define DALLAS_COMMON_END_ADDRESS_MASK 0x01F
#define DALLAS_COMMON_STATUS_FLAG_PF (1U << 5)
#define DALLAS_COMMON_STATUS_FLAG_OF (1U << 6)
#define DALLAS_COMMON_STATUS_FLAG_AA (1U << 7)

#define DALLAS_COMMON_BRIEF_HEAD_COUNT 4U
#define DALLAS_COMMON_BRIEF_TAIL_COUNT 3U

#define BITS_IN_BYTE 8U
#define BITS_IN_KBIT 1024U
#define BITS_IN_MBIT (BITS_IN_KBIT * 1024U)

bool dallas_common_skip_rom(OneWireHost* host) {
    onewire_host_write(host, DALLAS_COMMON_CMD_SKIP_ROM);
    return true;
}

bool dallas_common_read_rom(OneWireHost* host, DallasCommonRomData* rom_data) {
    onewire_host_write(host, DALLAS_COMMON_CMD_READ_ROM);
    onewire_host_read_bytes(host, rom_data->bytes, sizeof(DallasCommonRomData));

    return dallas_common_is_valid_crc(rom_data);
}

bool dallas_common_write_scratchpad(
    OneWireHost* host,
    uint16_t address,
    const uint8_t* data,
    size_t data_size) {
    onewire_host_write(host, DALLAS_COMMON_CMD_WRITE_SCRATCH);
    onewire_host_write(host, (uint8_t)address);
    onewire_host_write(host, (uint8_t)(address >> BITS_IN_BYTE));

    onewire_host_write_bytes(host, data, data_size);

    return true;
}

bool dallas_common_read_scratchpad(
    OneWireHost* host,
    DallasCommonAddressRegs* regs,
    uint8_t* data,
    size_t data_size) {
    onewire_host_write(host, DALLAS_COMMON_CMD_READ_SCRATCH);
    onewire_host_read_bytes(host, regs->bytes, sizeof(DallasCommonAddressRegs));
    onewire_host_read_bytes(host, data, data_size);

    return true;
}

bool dallas_common_copy_scratchpad(
    OneWireHost* host,
    const DallasCommonAddressRegs* regs,
    uint32_t timeout_us) {
    onewire_host_write(host, DALLAS_COMMON_CMD_COPY_SCRATCH);
    onewire_host_write_bytes(host, regs->bytes, sizeof(DallasCommonAddressRegs));

    const uint32_t poll_delay =
        MAX(timeout_us / DALLAS_COMMON_COPY_SCRATCH_POLL_COUNT,
            DALLAS_COMMON_COPY_SCRATCH_MIN_TIMEOUT_US);

    uint32_t time_elapsed;
    for(time_elapsed = 0; time_elapsed < timeout_us; time_elapsed += poll_delay) {
        if(!onewire_host_read_bit(host)) break;
        furi_delay_us(poll_delay);
    }

    return time_elapsed < timeout_us;
}

bool dallas_common_read_mem(OneWireHost* host, uint16_t address, uint8_t* data, size_t data_size) {
    onewire_host_write(host, DALLAS_COMMON_CMD_READ_MEM);

    onewire_host_write(host, (uint8_t)address);
    onewire_host_write(host, (uint8_t)(address >> BITS_IN_BYTE));

    onewire_host_read_bytes(host, data, (uint16_t)data_size);

    return true;
}

bool dallas_common_write_mem(
    OneWireHost* host,
    uint32_t timeout_us,
    size_t page_size,
    const uint8_t* data,
    size_t data_size) {
    // Data size must be a multiple of page size
    furi_check(data_size % page_size == 0);

    DallasCommonAddressRegs regs;
    uint8_t* scratch = malloc(page_size);

    size_t i;
    for(i = 0; i < data_size; i += page_size) {
        const uint8_t* data_ptr = data + i;

        // Write scratchpad with the next page value
        if(!onewire_host_reset(host)) break;
        if(!dallas_common_skip_rom(host)) break;
        if(!dallas_common_write_scratchpad(host, i, data_ptr, page_size)) break;

        // Read back the scratchpad contents and address registers
        if(!onewire_host_reset(host)) break;
        if(!dallas_common_skip_rom(host)) break;
        if(!dallas_common_read_scratchpad(host, &regs, scratch, page_size)) break;

        // Verify scratchpad contents
        if(memcmp(data_ptr, scratch, page_size) != 0) break;

        // Write scratchpad to internal memory
        if(!onewire_host_reset(host)) break;
        if(!dallas_common_skip_rom(host)) break;
        if(!dallas_common_copy_scratchpad(host, &regs, timeout_us)) break;

        // Read back the address registers again
        if(!onewire_host_reset(host)) break;
        if(!dallas_common_skip_rom(host)) break;
        if(!dallas_common_read_scratchpad(host, &regs, scratch, 0)) break;

        // Check if AA flag is set
        if(!(regs.fields.status & DALLAS_COMMON_STATUS_FLAG_AA)) break;
    }

    free(scratch);

    return i == data_size;
}

bool dallas_common_emulate_search_rom(OneWireSlave* bus, const DallasCommonRomData* rom_data) {
    for(size_t i = 0; i < sizeof(DallasCommonRomData); i++) {
        for(size_t j = 0; j < BITS_IN_BYTE; j++) {
            bool bit = (rom_data->bytes[i] >> j) & 0x01;

            if(!onewire_slave_send_bit(bus, bit)) return false;
            if(!onewire_slave_send_bit(bus, !bit)) return false;

            onewire_slave_receive_bit(bus);
            // TODO FL-3530: check for errors and return if any
        }
    }

    return true;
}

bool dallas_common_emulate_read_rom(OneWireSlave* bus, const DallasCommonRomData* rom_data) {
    return onewire_slave_send(bus, rom_data->bytes, sizeof(DallasCommonRomData));
}

bool dallas_common_emulate_read_mem(OneWireSlave* bus, const uint8_t* data, size_t data_size) {
    bool success = false;

    union {
        uint8_t bytes[sizeof(uint16_t)];
        uint16_t word;
    } address;

    do {
        if(!onewire_slave_receive(bus, address.bytes, sizeof(address))) break;
        if(address.word >= data_size) break;
        if(!onewire_slave_send(bus, data + address.word, data_size - address.word)) break;

        success = true;
    } while(false);

    return success;
}

bool dallas_common_save_rom_data(FlipperFormat* ff, const DallasCommonRomData* rom_data) {
    return flipper_format_write_hex(
        ff, DALLAS_COMMON_ROM_DATA_KEY_V2, rom_data->bytes, sizeof(DallasCommonRomData));
}

bool dallas_common_load_rom_data(
    FlipperFormat* ff,
    uint32_t format_version,
    DallasCommonRomData* rom_data) {
    switch(format_version) {
    case 1:
        return flipper_format_read_hex(
            ff, DALLAS_COMMON_ROM_DATA_KEY_V1, rom_data->bytes, sizeof(DallasCommonRomData));
    case 2:
        return flipper_format_read_hex(
            ff, DALLAS_COMMON_ROM_DATA_KEY_V2, rom_data->bytes, sizeof(DallasCommonRomData));
    default:
        return false;
    }
}

bool dallas_common_is_valid_crc(const DallasCommonRomData* rom_data) {
    const uint8_t crc_calculated =
        maxim_crc8(rom_data->bytes, sizeof(DallasCommonRomData) - 1, MAXIM_CRC8_INIT);
    const uint8_t crc_received = rom_data->fields.checksum;

    return crc_calculated == crc_received;
}

void dallas_common_render_brief_data(
    FuriString* result,
    const DallasCommonRomData* rom_data,
    const uint8_t* mem_data,
    size_t mem_size,
    const char* mem_name) {
    for(size_t i = 0; i < sizeof(rom_data->bytes); ++i) {
        furi_string_cat_printf(result, "%02X ", rom_data->bytes[i]);
    }

    const char* size_prefix = "";
    size_t mem_size_bits = mem_size * BITS_IN_BYTE;

    if(mem_size_bits >= BITS_IN_MBIT) {
        size_prefix = "M";
        mem_size_bits /= BITS_IN_MBIT;
    } else if(mem_size_bits >= BITS_IN_KBIT) {
        size_prefix = "K";
        mem_size_bits /= BITS_IN_KBIT;
    }

    furi_string_cat_printf(
        result, "\nInternal %s: %zu %sbit\n", mem_name, mem_size_bits, size_prefix);

    for(size_t i = 0; i < DALLAS_COMMON_BRIEF_HEAD_COUNT; ++i) {
        furi_string_cat_printf(result, "%02X ", mem_data[i]);
    }

    furi_string_cat_printf(result, "[  . . .  ]");

    for(size_t i = mem_size - DALLAS_COMMON_BRIEF_TAIL_COUNT; i < mem_size; ++i) {
        furi_string_cat_printf(result, " %02X", mem_data[i]);
    }
}

void dallas_common_render_crc_error(FuriString* result, const DallasCommonRomData* rom_data) {
    furi_string_set(result, "CRC Error\n");

    const size_t data_size = sizeof(DallasCommonRomData);

    for(size_t i = 0; i < data_size; ++i) {
        furi_string_cat_printf(result, (i < data_size - 1) ? "%02X " : "%02X", rom_data->bytes[i]);
    }
}

void dallas_common_apply_edits(DallasCommonRomData* rom_data, uint8_t family_code) {
    rom_data->fields.family_code = family_code;
    const uint8_t crc =
        maxim_crc8(rom_data->bytes, sizeof(DallasCommonRomData) - 1, MAXIM_CRC8_INIT);
    rom_data->fields.checksum = crc;
}
