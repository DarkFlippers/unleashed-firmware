#include "protocol_ds1990.h"

#include <core/string.h>
#include <core/core_defines.h>

#include "dallas_common.h"

#include "../blanks/rw1990.h"
#include "../blanks/tm2004.h"

#define DS1990_FAMILY_CODE 0x01U
#define DS1990_FAMILY_NAME "DS1990"

#define DS1990_CMD_READ_ROM 0x0FU

typedef struct {
    OneWireSlave* bus;
} DS1990ProtocolState;

typedef struct {
    DallasCommonRomData rom_data;
    DS1990ProtocolState state;
} DS1990ProtocolData;

static bool dallas_ds1990_read(OneWireHost*, iButtonProtocolData*);
static bool dallas_ds1990_write_blank(OneWireHost*, iButtonProtocolData*);
static void dallas_ds1990_emulate(OneWireSlave*, iButtonProtocolData*);
static bool dallas_ds1990_load(FlipperFormat*, uint32_t, iButtonProtocolData*);
static bool dallas_ds1990_save(FlipperFormat*, const iButtonProtocolData*);
static void dallas_ds1990_render_brief_data(FuriString*, const iButtonProtocolData*);
static void dallas_ds1990_render_error(FuriString*, const iButtonProtocolData*);
static bool dallas_ds1990_is_data_valid(const iButtonProtocolData*);
static void dallas_ds1990_get_editable_data(iButtonEditableData*, iButtonProtocolData*);
static void dallas_ds1990_apply_edits(iButtonProtocolData*);

const iButtonProtocolDallasBase ibutton_protocol_ds1990 = {
    .family_code = DS1990_FAMILY_CODE,
    .features = iButtonProtocolFeatureWriteBlank,
    .data_size = sizeof(DS1990ProtocolData),
    .manufacturer = DALLAS_COMMON_MANUFACTURER_NAME,
    .name = DS1990_FAMILY_NAME,

    .read = dallas_ds1990_read,
    .write_blank = dallas_ds1990_write_blank,
    .write_copy = NULL, /* No data to write a copy */
    .emulate = dallas_ds1990_emulate,
    .save = dallas_ds1990_save,
    .load = dallas_ds1990_load,
    .render_data = NULL, /* No data to render */
    .render_brief_data = dallas_ds1990_render_brief_data,
    .render_error = dallas_ds1990_render_error,
    .is_valid = dallas_ds1990_is_data_valid,
    .get_editable_data = dallas_ds1990_get_editable_data,
    .apply_edits = dallas_ds1990_apply_edits,
};

bool dallas_ds1990_read(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DS1990ProtocolData* data = protocol_data;
    return onewire_host_reset(host) && dallas_common_read_rom(host, &data->rom_data);
}

bool dallas_ds1990_write_blank(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DS1990ProtocolData* data = protocol_data;

    return rw1990_write_v1(host, data->rom_data.bytes, sizeof(DallasCommonRomData)) ||
           rw1990_write_v2(host, data->rom_data.bytes, sizeof(DallasCommonRomData)) ||
           tm2004_write(host, data->rom_data.bytes, sizeof(DallasCommonRomData));
}

static bool dallas_ds1990_command_callback(uint8_t command, void* context) {
    furi_assert(context);
    DS1990ProtocolData* data = context;
    OneWireSlave* bus = data->state.bus;

    switch(command) {
    case DALLAS_COMMON_CMD_SEARCH_ROM:
        dallas_common_emulate_search_rom(bus, &data->rom_data);
        break;
    case DALLAS_COMMON_CMD_READ_ROM:
    case DS1990_CMD_READ_ROM:
        dallas_common_emulate_read_rom(bus, &data->rom_data);
        break;
    default:
        break;
    }

    // No support for multiple consecutive commands
    return false;
}

void dallas_ds1990_emulate(OneWireSlave* bus, iButtonProtocolData* protocol_data) {
    DS1990ProtocolData* data = protocol_data;
    data->state.bus = bus;

    onewire_slave_set_reset_callback(bus, NULL, NULL);
    onewire_slave_set_command_callback(bus, dallas_ds1990_command_callback, protocol_data);
}

bool dallas_ds1990_save(FlipperFormat* ff, const iButtonProtocolData* protocol_data) {
    const DS1990ProtocolData* data = protocol_data;
    return dallas_common_save_rom_data(ff, &data->rom_data);
}

bool dallas_ds1990_load(
    FlipperFormat* ff,
    uint32_t format_version,
    iButtonProtocolData* protocol_data) {
    DS1990ProtocolData* data = protocol_data;
    return dallas_common_load_rom_data(ff, format_version, &data->rom_data);
}

void dallas_ds1990_render_brief_data(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1990ProtocolData* data = protocol_data;

    for(size_t i = 0; i < sizeof(DallasCommonRomData); ++i) {
        furi_string_cat_printf(result, "%02X ", data->rom_data.bytes[i]);
    }
}

void dallas_ds1990_render_error(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DS1990ProtocolData* data = protocol_data;

    if(!dallas_common_is_valid_crc(&data->rom_data)) {
        dallas_common_render_crc_error(result, &data->rom_data);
    }
}

bool dallas_ds1990_is_data_valid(const iButtonProtocolData* protocol_data) {
    const DS1990ProtocolData* data = protocol_data;
    return dallas_common_is_valid_crc(&data->rom_data);
}

void dallas_ds1990_get_editable_data(
    iButtonEditableData* editable_data,
    iButtonProtocolData* protocol_data) {
    DS1990ProtocolData* data = protocol_data;
    editable_data->ptr = data->rom_data.bytes;
    editable_data->size = sizeof(DallasCommonRomData);
}

void dallas_ds1990_apply_edits(iButtonProtocolData* protocol_data) {
    DS1990ProtocolData* data = protocol_data;
    dallas_common_apply_edits(&data->rom_data, DS1990_FAMILY_CODE);
}
