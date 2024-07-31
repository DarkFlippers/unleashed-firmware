#include "protocol_ds1971.h"

#include <core/core_defines.h>
#include <toolbox/pretty_format.h>

#include "dallas_common.h"

#include "../blanks/tm2004.h"

#define DS1971_FAMILY_CODE 0x14U
#define DS1971_FAMILY_NAME "DS1971"

#define DS1971_EEPROM_DATA_SIZE      32U
#define DS1971_SRAM_PAGE_SIZE        32U
#define DS1971_COPY_SCRATCH_DELAY_US 250U

#define DS1971_DATA_BYTE_COUNT 4U

#define DS1971_EEPROM_DATA_KEY "Eeprom Data"
#define DS1971_MEMORY_TYPE     "EEPROM"

#define DS1971_CMD_FINALIZATION 0xA5

typedef struct {
    OneWireSlave* bus;
    DallasCommonCommandState command_state;
} DS1971ProtocolState;

typedef struct {
    DallasCommonRomData rom_data;
    uint8_t eeprom_data[DS1971_EEPROM_DATA_SIZE];
    DS1971ProtocolState state;
} DS1971ProtocolData;

static bool dallas_ds1971_read(OneWireHost*, void*);
static bool dallas_ds1971_write_id(OneWireHost*, iButtonProtocolData*);
static bool dallas_ds1971_write_copy(OneWireHost*, iButtonProtocolData*);
static void dallas_ds1971_emulate(OneWireSlave*, iButtonProtocolData*);
static bool dallas_ds1971_load(FlipperFormat*, uint32_t, iButtonProtocolData*);
static bool dallas_ds1971_save(FlipperFormat*, const iButtonProtocolData*);
static void dallas_ds1971_render_uid(FuriString*, const iButtonProtocolData*);
static void dallas_ds1971_render_data(FuriString*, const iButtonProtocolData*);
static void dallas_ds1971_render_brief_data(FuriString*, const iButtonProtocolData*);
static void dallas_ds1971_render_error(FuriString*, const iButtonProtocolData*);
static bool dallas_ds1971_is_data_valid(const iButtonProtocolData*);
static void dallas_ds1971_get_editable_data(iButtonEditableData*, iButtonProtocolData*);
static void dallas_ds1971_apply_edits(iButtonProtocolData*);
static bool
    dallas_ds1971_read_mem(OneWireHost* host, uint8_t address, uint8_t* data, size_t data_size);
static bool ds1971_emulate_read_mem(OneWireSlave* bus, const uint8_t* data, size_t data_size);

const iButtonProtocolDallasBase ibutton_protocol_ds1971 = {
    .family_code = DS1971_FAMILY_CODE,
    .features = iButtonProtocolFeatureExtData | iButtonProtocolFeatureWriteId |
                iButtonProtocolFeatureWriteCopy,
    .data_size = sizeof(DS1971ProtocolData),
    .manufacturer = DALLAS_COMMON_MANUFACTURER_NAME,
    .name = DS1971_FAMILY_NAME,

    .read = dallas_ds1971_read,
    .write_id = dallas_ds1971_write_id,
    .write_copy = dallas_ds1971_write_copy,
    .emulate = dallas_ds1971_emulate,
    .save = dallas_ds1971_save,
    .load = dallas_ds1971_load,
    .render_uid = dallas_ds1971_render_uid,
    .render_data = dallas_ds1971_render_data,
    .render_brief_data = dallas_ds1971_render_brief_data,
    .render_error = dallas_ds1971_render_error,
    .is_valid = dallas_ds1971_is_data_valid,
    .get_editable_data = dallas_ds1971_get_editable_data,
    .apply_edits = dallas_ds1971_apply_edits,
};

bool dallas_ds1971_read(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DS1971ProtocolData* data = protocol_data;
    return onewire_host_reset(host) && dallas_common_read_rom(host, &data->rom_data) &&
           dallas_ds1971_read_mem(host, 0, data->eeprom_data, DS1971_EEPROM_DATA_SIZE);
}

bool dallas_ds1971_write_id(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DS1971ProtocolData* data = protocol_data;
    return tm2004_write(host, data->rom_data.bytes, sizeof(DallasCommonRomData));
}

bool dallas_ds1971_write_copy(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DS1971ProtocolData* data = protocol_data;

    onewire_host_reset(host);
    onewire_host_write(host, DALLAS_COMMON_CMD_SKIP_ROM);
    // Starting writing from address 0x0000
    onewire_host_write(host, DALLAS_COMMON_CMD_WRITE_SCRATCH);
    onewire_host_write(host, 0x00);
    // Write data to scratchpad
    onewire_host_write_bytes(host, data->eeprom_data, DS1971_EEPROM_DATA_SIZE);

    // Read data from scratchpad and verify
    bool pad_valid = false;
    if(onewire_host_reset(host)) {
        pad_valid = true;
        onewire_host_write(host, DALLAS_COMMON_CMD_SKIP_ROM);
        onewire_host_write(host, DALLAS_COMMON_CMD_READ_SCRATCH);
        onewire_host_write(host, 0x00);

        for(size_t i = 0; i < DS1971_EEPROM_DATA_SIZE; ++i) {
            uint8_t scratch = onewire_host_read(host);
            if(data->eeprom_data[i] != scratch) {
                pad_valid = false;
                break;
            }
        }
    }

    // Copy scratchpad to memory and confirm
    if(pad_valid) {
        if(onewire_host_reset(host)) {
            onewire_host_write(host, DALLAS_COMMON_CMD_SKIP_ROM);
            onewire_host_write(host, DALLAS_COMMON_CMD_COPY_SCRATCH);
            onewire_host_write(host, DS1971_CMD_FINALIZATION);

            furi_delay_us(DS1971_COPY_SCRATCH_DELAY_US);
        }
    }

    return pad_valid;
}

static bool dallas_ds1971_reset_callback(bool is_short, void* context) {
    furi_assert(context);
    DS1971ProtocolData* data = context;

    if(!is_short) {
        data->state.command_state = DallasCommonCommandStateIdle;
        onewire_slave_set_overdrive(data->state.bus, is_short);
    }

    return !is_short;
}

static bool dallas_ds1971_command_callback(uint8_t command, void* context) {
    furi_assert(context);
    DS1971ProtocolData* data = context;
    OneWireSlave* bus = data->state.bus;

    switch(command) {
    case DALLAS_COMMON_CMD_SEARCH_ROM:
        if(data->state.command_state == DallasCommonCommandStateIdle) {
            data->state.command_state = DallasCommonCommandStateRomCmd;
            return dallas_common_emulate_search_rom(bus, &data->rom_data);

        } else if(data->state.command_state == DallasCommonCommandStateRomCmd) {
            data->state.command_state = DallasCommonCommandStateMemCmd;
            ds1971_emulate_read_mem(bus, data->eeprom_data, DS1971_EEPROM_DATA_SIZE);
            return false;

        } else {
            return false;
        }

    case DALLAS_COMMON_CMD_READ_ROM:
        if(data->state.command_state == DallasCommonCommandStateIdle) {
            data->state.command_state = DallasCommonCommandStateRomCmd;
            return dallas_common_emulate_read_rom(bus, &data->rom_data);
        } else {
            return false;
        }

    case DALLAS_COMMON_CMD_SKIP_ROM:
        if(data->state.command_state == DallasCommonCommandStateIdle) {
            data->state.command_state = DallasCommonCommandStateRomCmd;
            return true;
        } else {
            return false;
        }

    default:
        return false;
    }
}

void dallas_ds1971_emulate(OneWireSlave* bus, iButtonProtocolData* protocol_data) {
    DS1971ProtocolData* data = protocol_data;
    data->state.bus = bus;

    onewire_slave_set_reset_callback(bus, dallas_ds1971_reset_callback, protocol_data);
    onewire_slave_set_command_callback(bus, dallas_ds1971_command_callback, protocol_data);
}

bool dallas_ds1971_load(
    FlipperFormat* ff,
    uint32_t format_version,
    iButtonProtocolData* protocol_data) {
    DS1971ProtocolData* data = protocol_data;
    bool success = false;

    do {
        if(format_version < 2) break;
        if(!dallas_common_load_rom_data(ff, format_version, &data->rom_data)) break;
        if(!flipper_format_read_hex(
               ff, DS1971_EEPROM_DATA_KEY, data->eeprom_data, DS1971_EEPROM_DATA_SIZE))
            break;
        success = true;
    } while(false);

    return success;
}

bool dallas_ds1971_save(FlipperFormat* ff, const iButtonProtocolData* protocol_data) {
    const DS1971ProtocolData* data = protocol_data;
    bool success = false;

    do {
        if(!dallas_common_save_rom_data(ff, &data->rom_data)) break;
        if(!flipper_format_write_hex(
               ff, DS1971_EEPROM_DATA_KEY, data->eeprom_data, DS1971_EEPROM_DATA_SIZE))
            break;
        success = true;
    } while(false);

    return success;
}

void dallas_ds1971_render_uid(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1971ProtocolData* data = protocol_data;
    dallas_common_render_uid(result, &data->rom_data);
}

void dallas_ds1971_render_data(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1971ProtocolData* data = protocol_data;

    furi_string_cat_printf(result, "\e#Memory Data\n--------------------\n");

    pretty_format_bytes_hex_canonical(
        result,
        DS1971_DATA_BYTE_COUNT,
        PRETTY_FORMAT_FONT_MONOSPACE,
        data->eeprom_data,
        DS1971_EEPROM_DATA_SIZE);
}

void dallas_ds1971_render_brief_data(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1971ProtocolData* data = protocol_data;
    dallas_common_render_brief_data(
        result, &data->rom_data, data->eeprom_data, DS1971_EEPROM_DATA_SIZE, DS1971_MEMORY_TYPE);
}

void dallas_ds1971_render_error(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1971ProtocolData* data = protocol_data;

    if(!dallas_common_is_valid_crc(&data->rom_data)) {
        dallas_common_render_crc_error(result, &data->rom_data);
    }
}

bool dallas_ds1971_is_data_valid(const iButtonProtocolData* protocol_data) {
    const DS1971ProtocolData* data = protocol_data;
    return dallas_common_is_valid_crc(&data->rom_data);
}

void dallas_ds1971_get_editable_data(
    iButtonEditableData* editable_data,
    iButtonProtocolData* protocol_data) {
    DS1971ProtocolData* data = protocol_data;
    editable_data->ptr = data->rom_data.bytes;
    editable_data->size = sizeof(DallasCommonRomData);
}

void dallas_ds1971_apply_edits(iButtonProtocolData* protocol_data) {
    DS1971ProtocolData* data = protocol_data;
    dallas_common_apply_edits(&data->rom_data, DS1971_FAMILY_CODE);
}

bool dallas_ds1971_read_mem(OneWireHost* host, uint8_t address, uint8_t* data, size_t data_size) {
    onewire_host_write(host, DALLAS_COMMON_CMD_READ_MEM);

    onewire_host_write(host, address);
    onewire_host_read_bytes(host, data, (uint8_t)data_size);

    return true;
}

bool ds1971_emulate_read_mem(OneWireSlave* bus, const uint8_t* data, size_t data_size) {
    bool success = false;

    do {
        uint8_t address;
        if(!onewire_slave_receive(bus, &address, sizeof(address))) break;
        if(address >= data_size) break;
        if(!onewire_slave_send(bus, data + address, data_size - address)) break;

        success = true;
    } while(false);

    return success;
}
