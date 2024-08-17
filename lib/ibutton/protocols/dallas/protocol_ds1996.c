#include "protocol_ds1996.h"

#include <core/core_defines.h>
#include <toolbox/pretty_format.h>

#include "dallas_common.h"

#include "../blanks/tm2004.h"

#define DS1996_FAMILY_CODE 0x0CU
#define DS1996_FAMILY_NAME "DS1996"

#define DS1996_SRAM_DATA_SIZE          8192U
#define DS1996_SRAM_PAGE_SIZE          32U
#define DS1996_COPY_SCRATCH_TIMEOUT_US 100U

#define DS1996_DATA_BYTE_COUNT 4U

#define DS1996_SRAM_DATA_KEY "Sram Data"
#define DS1996_MEMORY_TYPE   "SRAM"

typedef struct {
    OneWireSlave* bus;
    DallasCommonCommandState command_state;
} DS1996ProtocolState;

typedef struct {
    DallasCommonRomData rom_data;
    uint8_t sram_data[DS1996_SRAM_DATA_SIZE];
    DS1996ProtocolState state;
} DS1996ProtocolData;

static bool dallas_ds1996_read(OneWireHost*, void*);
static bool dallas_ds1996_write_id(OneWireHost*, iButtonProtocolData*);
static bool dallas_ds1996_write_copy(OneWireHost*, iButtonProtocolData*);
static void dallas_ds1996_emulate(OneWireSlave*, iButtonProtocolData*);
static bool dallas_ds1996_load(FlipperFormat*, uint32_t, iButtonProtocolData*);
static bool dallas_ds1996_save(FlipperFormat*, const iButtonProtocolData*);
static void dallas_ds1996_render_uid(FuriString*, const iButtonProtocolData*);
static void dallas_ds1996_render_data(FuriString*, const iButtonProtocolData*);
static void dallas_ds1996_render_brief_data(FuriString*, const iButtonProtocolData*);
static void dallas_ds1996_render_error(FuriString*, const iButtonProtocolData*);
static bool dallas_ds1996_is_data_valid(const iButtonProtocolData*);
static void dallas_ds1996_get_editable_data(iButtonEditableData*, iButtonProtocolData*);
static void dallas_ds1996_apply_edits(iButtonProtocolData*);

const iButtonProtocolDallasBase ibutton_protocol_ds1996 = {
    .family_code = DS1996_FAMILY_CODE,
    .features = iButtonProtocolFeatureExtData | iButtonProtocolFeatureWriteId |
                iButtonProtocolFeatureWriteCopy,
    .data_size = sizeof(DS1996ProtocolData),
    .manufacturer = DALLAS_COMMON_MANUFACTURER_NAME,
    .name = DS1996_FAMILY_NAME,

    .read = dallas_ds1996_read,
    .write_id = dallas_ds1996_write_id,
    .write_copy = dallas_ds1996_write_copy,
    .emulate = dallas_ds1996_emulate,
    .save = dallas_ds1996_save,
    .load = dallas_ds1996_load,
    .render_uid = dallas_ds1996_render_uid,
    .render_data = dallas_ds1996_render_data,
    .render_brief_data = dallas_ds1996_render_brief_data,
    .render_error = dallas_ds1996_render_error,
    .is_valid = dallas_ds1996_is_data_valid,
    .get_editable_data = dallas_ds1996_get_editable_data,
    .apply_edits = dallas_ds1996_apply_edits,
};

bool dallas_ds1996_read(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DS1996ProtocolData* data = protocol_data;
    bool success = false;

    do {
        if(!onewire_host_reset(host)) break;
        if(!dallas_common_read_rom(host, &data->rom_data)) break;
        if(!onewire_host_reset(host)) break;

        onewire_host_write(host, DALLAS_COMMON_CMD_OVERDRIVE_SKIP_ROM);
        onewire_host_set_overdrive(host, true);

        if(!dallas_common_read_mem(host, 0, data->sram_data, DS1996_SRAM_DATA_SIZE)) break;
        success = true;
    } while(false);

    onewire_host_set_overdrive(host, false);
    return success;
}

bool dallas_ds1996_write_id(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DS1996ProtocolData* data = protocol_data;
    return tm2004_write(host, data->rom_data.bytes, sizeof(DallasCommonRomData));
}

bool dallas_ds1996_write_copy(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DS1996ProtocolData* data = protocol_data;
    bool success = false;

    do {
        if(!onewire_host_reset(host)) break;

        onewire_host_write(host, DALLAS_COMMON_CMD_OVERDRIVE_SKIP_ROM);
        onewire_host_set_overdrive(host, true);

        if(!dallas_common_write_mem(
               host,
               DS1996_COPY_SCRATCH_TIMEOUT_US,
               DS1996_SRAM_PAGE_SIZE,
               data->sram_data,
               DS1996_SRAM_DATA_SIZE))
            break;
        success = true;
    } while(false);

    onewire_host_set_overdrive(host, false);
    return success;
}

static bool dallas_ds1996_reset_callback(bool is_short, void* context) {
    furi_assert(context);
    DS1996ProtocolData* data = context;
    data->state.command_state = DallasCommonCommandStateIdle;
    onewire_slave_set_overdrive(data->state.bus, is_short);
    return true;
}

static bool dallas_ds1996_command_callback(uint8_t command, void* context) {
    furi_assert(context);
    DS1996ProtocolData* data = context;
    OneWireSlave* bus = data->state.bus;

    switch(command) {
    case DALLAS_COMMON_CMD_SEARCH_ROM:
        if(data->state.command_state == DallasCommonCommandStateIdle) {
            data->state.command_state = DallasCommonCommandStateRomCmd;
            return dallas_common_emulate_search_rom(bus, &data->rom_data);

        } else if(data->state.command_state == DallasCommonCommandStateRomCmd) {
            data->state.command_state = DallasCommonCommandStateMemCmd;
            return dallas_common_emulate_read_mem(bus, data->sram_data, DS1996_SRAM_DATA_SIZE);

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

    case DALLAS_COMMON_CMD_OVERDRIVE_SKIP_ROM:
        if(data->state.command_state == DallasCommonCommandStateIdle) {
            data->state.command_state = DallasCommonCommandStateRomCmd;
            onewire_slave_set_overdrive(bus, true);
            return true;
        } else {
            return false;
        }

    case DALLAS_COMMON_CMD_MATCH_ROM:
    case DALLAS_COMMON_CMD_OVERDRIVE_MATCH_ROM:
        /* TODO FL-3533: Match ROM command support */
    default:
        return false;
    }
}

void dallas_ds1996_emulate(OneWireSlave* bus, iButtonProtocolData* protocol_data) {
    DS1996ProtocolData* data = protocol_data;
    data->state.bus = bus;

    onewire_slave_set_reset_callback(bus, dallas_ds1996_reset_callback, protocol_data);
    onewire_slave_set_command_callback(bus, dallas_ds1996_command_callback, protocol_data);
}

bool dallas_ds1996_load(
    FlipperFormat* ff,
    uint32_t format_version,
    iButtonProtocolData* protocol_data) {
    DS1996ProtocolData* data = protocol_data;
    bool success = false;

    do {
        if(format_version < 2) break;
        if(!dallas_common_load_rom_data(ff, format_version, &data->rom_data)) break;
        if(!flipper_format_read_hex(
               ff, DS1996_SRAM_DATA_KEY, data->sram_data, DS1996_SRAM_DATA_SIZE))
            break;
        success = true;
    } while(false);

    return success;
}

bool dallas_ds1996_save(FlipperFormat* ff, const iButtonProtocolData* protocol_data) {
    const DS1996ProtocolData* data = protocol_data;
    bool success = false;

    do {
        if(!dallas_common_save_rom_data(ff, &data->rom_data)) break;
        if(!flipper_format_write_hex(
               ff, DS1996_SRAM_DATA_KEY, data->sram_data, DS1996_SRAM_DATA_SIZE))
            break;
        success = true;
    } while(false);

    return success;
}

void dallas_ds1996_render_uid(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1996ProtocolData* data = protocol_data;
    dallas_common_render_uid(result, &data->rom_data);
}

void dallas_ds1996_render_data(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1996ProtocolData* data = protocol_data;

    furi_string_cat_printf(result, "\e#Memory Data\n--------------------\n");

    pretty_format_bytes_hex_canonical(
        result,
        DS1996_DATA_BYTE_COUNT,
        PRETTY_FORMAT_FONT_MONOSPACE,
        data->sram_data,
        DS1996_SRAM_DATA_SIZE);
}

void dallas_ds1996_render_brief_data(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1996ProtocolData* data = protocol_data;
    dallas_common_render_brief_data(
        result, &data->rom_data, data->sram_data, DS1996_SRAM_DATA_SIZE, DS1996_MEMORY_TYPE);
}

void dallas_ds1996_render_error(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1996ProtocolData* data = protocol_data;

    if(!dallas_common_is_valid_crc(&data->rom_data)) {
        dallas_common_render_crc_error(result, &data->rom_data);
    }
}

bool dallas_ds1996_is_data_valid(const iButtonProtocolData* protocol_data) {
    const DS1996ProtocolData* data = protocol_data;
    return dallas_common_is_valid_crc(&data->rom_data);
}

void dallas_ds1996_get_editable_data(
    iButtonEditableData* editable_data,
    iButtonProtocolData* protocol_data) {
    DS1996ProtocolData* data = protocol_data;
    editable_data->ptr = data->rom_data.bytes;
    editable_data->size = sizeof(DallasCommonRomData);
}

void dallas_ds1996_apply_edits(iButtonProtocolData* protocol_data) {
    DS1996ProtocolData* data = protocol_data;
    dallas_common_apply_edits(&data->rom_data, DS1996_FAMILY_CODE);
}
