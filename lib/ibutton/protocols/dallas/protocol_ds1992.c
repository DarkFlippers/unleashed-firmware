#include "protocol_ds1992.h"

#include <core/core_defines.h>
#include <toolbox/pretty_format.h>

#include "dallas_common.h"

#include "../blanks/tm2004.h"

#define DS1992_FAMILY_CODE 0x08U
#define DS1992_FAMILY_NAME "DS1992"

#define DS1992_SRAM_DATA_SIZE 128U
#define DS1992_SRAM_PAGE_SIZE 4U
#define DS1992_COPY_SCRATCH_TIMEOUT_US 100U

#define DS1992_DATA_BYTE_COUNT 4U

#define DS1992_SRAM_DATA_KEY "Sram Data"
#define DS1992_MEMORY_TYPE "SRAM"

typedef struct {
    OneWireSlave* bus;
    DallasCommonCommandState command_state;
} DS1992ProtocolState;

typedef struct {
    DallasCommonRomData rom_data;
    uint8_t sram_data[DS1992_SRAM_DATA_SIZE];
    DS1992ProtocolState state;
} DS1992ProtocolData;

static bool dallas_ds1992_read(OneWireHost*, void*);
static bool dallas_ds1992_write_blank(OneWireHost*, iButtonProtocolData*);
static bool dallas_ds1992_write_copy(OneWireHost*, iButtonProtocolData*);
static void dallas_ds1992_emulate(OneWireSlave*, iButtonProtocolData*);
static bool dallas_ds1992_load(FlipperFormat*, uint32_t, iButtonProtocolData*);
static bool dallas_ds1992_save(FlipperFormat*, const iButtonProtocolData*);
static void dallas_ds1992_render_data(FuriString*, const iButtonProtocolData*);
static void dallas_ds1992_render_brief_data(FuriString*, const iButtonProtocolData*);
static void dallas_ds1992_render_error(FuriString*, const iButtonProtocolData*);
static bool dallas_ds1992_is_data_valid(const iButtonProtocolData*);
static void dallas_ds1992_get_editable_data(iButtonEditableData*, iButtonProtocolData*);
static void dallas_ds1992_apply_edits(iButtonProtocolData*);

const iButtonProtocolDallasBase ibutton_protocol_ds1992 = {
    .family_code = DS1992_FAMILY_CODE,
    .features = iButtonProtocolFeatureExtData | iButtonProtocolFeatureWriteBlank |
                iButtonProtocolFeatureWriteCopy,
    .data_size = sizeof(DS1992ProtocolData),
    .manufacturer = DALLAS_COMMON_MANUFACTURER_NAME,
    .name = DS1992_FAMILY_NAME,

    .read = dallas_ds1992_read,
    .write_blank = dallas_ds1992_write_blank,
    .write_copy = dallas_ds1992_write_copy,
    .emulate = dallas_ds1992_emulate,
    .save = dallas_ds1992_save,
    .load = dallas_ds1992_load,
    .render_data = dallas_ds1992_render_data,
    .render_brief_data = dallas_ds1992_render_brief_data,
    .render_error = dallas_ds1992_render_error,
    .is_valid = dallas_ds1992_is_data_valid,
    .get_editable_data = dallas_ds1992_get_editable_data,
    .apply_edits = dallas_ds1992_apply_edits,
};

bool dallas_ds1992_read(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DS1992ProtocolData* data = protocol_data;
    return onewire_host_reset(host) && dallas_common_read_rom(host, &data->rom_data) &&
           dallas_common_read_mem(host, 0, data->sram_data, DS1992_SRAM_DATA_SIZE);
}

bool dallas_ds1992_write_blank(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DS1992ProtocolData* data = protocol_data;
    // TODO FL-3532: Make this work, currently broken
    return tm2004_write(host, (uint8_t*)data, sizeof(DallasCommonRomData) + DS1992_SRAM_DATA_SIZE);
}

bool dallas_ds1992_write_copy(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DS1992ProtocolData* data = protocol_data;
    return dallas_common_write_mem(
        host,
        DS1992_COPY_SCRATCH_TIMEOUT_US,
        DS1992_SRAM_PAGE_SIZE,
        data->sram_data,
        DS1992_SRAM_DATA_SIZE);
}

static bool dallas_ds1992_reset_callback(bool is_short, void* context) {
    furi_assert(context);
    DS1992ProtocolData* data = context;

    if(!is_short) {
        data->state.command_state = DallasCommonCommandStateIdle;
        onewire_slave_set_overdrive(data->state.bus, is_short);
    }

    return !is_short;
}

static bool dallas_ds1992_command_callback(uint8_t command, void* context) {
    furi_assert(context);
    DS1992ProtocolData* data = context;
    OneWireSlave* bus = data->state.bus;

    switch(command) {
    case DALLAS_COMMON_CMD_SEARCH_ROM:
        if(data->state.command_state == DallasCommonCommandStateIdle) {
            data->state.command_state = DallasCommonCommandStateRomCmd;
            return dallas_common_emulate_search_rom(bus, &data->rom_data);

        } else if(data->state.command_state == DallasCommonCommandStateRomCmd) {
            data->state.command_state = DallasCommonCommandStateMemCmd;
            dallas_common_emulate_read_mem(bus, data->sram_data, DS1992_SRAM_DATA_SIZE);
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

void dallas_ds1992_emulate(OneWireSlave* bus, iButtonProtocolData* protocol_data) {
    DS1992ProtocolData* data = protocol_data;
    data->state.bus = bus;

    onewire_slave_set_reset_callback(bus, dallas_ds1992_reset_callback, protocol_data);
    onewire_slave_set_command_callback(bus, dallas_ds1992_command_callback, protocol_data);
}

bool dallas_ds1992_load(
    FlipperFormat* ff,
    uint32_t format_version,
    iButtonProtocolData* protocol_data) {
    DS1992ProtocolData* data = protocol_data;
    bool success = false;

    do {
        if(format_version < 2) break;
        if(!dallas_common_load_rom_data(ff, format_version, &data->rom_data)) break;
        if(!flipper_format_read_hex(
               ff, DS1992_SRAM_DATA_KEY, data->sram_data, DS1992_SRAM_DATA_SIZE))
            break;
        success = true;
    } while(false);

    return success;
}

bool dallas_ds1992_save(FlipperFormat* ff, const iButtonProtocolData* protocol_data) {
    const DS1992ProtocolData* data = protocol_data;
    bool success = false;

    do {
        if(!dallas_common_save_rom_data(ff, &data->rom_data)) break;
        if(!flipper_format_write_hex(
               ff, DS1992_SRAM_DATA_KEY, data->sram_data, DS1992_SRAM_DATA_SIZE))
            break;
        success = true;
    } while(false);

    return success;
}

void dallas_ds1992_render_data(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1992ProtocolData* data = protocol_data;
    pretty_format_bytes_hex_canonical(
        result,
        DS1992_DATA_BYTE_COUNT,
        PRETTY_FORMAT_FONT_MONOSPACE,
        data->sram_data,
        DS1992_SRAM_DATA_SIZE);
}

void dallas_ds1992_render_brief_data(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1992ProtocolData* data = protocol_data;
    dallas_common_render_brief_data(
        result, &data->rom_data, data->sram_data, DS1992_SRAM_DATA_SIZE, DS1992_MEMORY_TYPE);
}

void dallas_ds1992_render_error(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1992ProtocolData* data = protocol_data;

    if(!dallas_common_is_valid_crc(&data->rom_data)) {
        dallas_common_render_crc_error(result, &data->rom_data);
    }
}

bool dallas_ds1992_is_data_valid(const iButtonProtocolData* protocol_data) {
    const DS1992ProtocolData* data = protocol_data;
    return dallas_common_is_valid_crc(&data->rom_data);
}

void dallas_ds1992_get_editable_data(
    iButtonEditableData* editable_data,
    iButtonProtocolData* protocol_data) {
    DS1992ProtocolData* data = protocol_data;
    editable_data->ptr = data->rom_data.bytes;
    editable_data->size = sizeof(DallasCommonRomData);
}

void dallas_ds1992_apply_edits(iButtonProtocolData* protocol_data) {
    DS1992ProtocolData* data = protocol_data;
    dallas_common_apply_edits(&data->rom_data, DS1992_FAMILY_CODE);
}
