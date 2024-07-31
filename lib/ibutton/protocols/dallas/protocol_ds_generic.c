#include "protocol_ds_generic.h"

#include <core/string.h>
#include <core/core_defines.h>

#include "dallas_common.h"

#include "../blanks/tm2004.h"

#define DALLAS_GENERIC_FAMILY_CODE 0x00U
#define DALLAS_GENERIC_FAMILY_NAME "(non-specific)"

typedef struct {
    OneWireSlave* bus;
} DallasGenericProtocolState;

typedef struct {
    DallasCommonRomData rom_data;
    DallasGenericProtocolState state;
} DallasGenericProtocolData;

static bool ds_generic_read(OneWireHost*, iButtonProtocolData*);
static bool ds_generic_write_id(OneWireHost*, iButtonProtocolData*);
static void ds_generic_emulate(OneWireSlave*, iButtonProtocolData*);
static bool ds_generic_load(FlipperFormat*, uint32_t, iButtonProtocolData*);
static bool ds_generic_save(FlipperFormat*, const iButtonProtocolData*);
static void ds_generic_render_uid(FuriString*, const iButtonProtocolData*);
static void ds_generic_render_brief_data(FuriString*, const iButtonProtocolData*);
static void ds_generic_render_error(FuriString*, const iButtonProtocolData*);
static bool ds_generic_is_data_valid(const iButtonProtocolData*);
static void ds_generic_get_editable_data(iButtonEditableData*, iButtonProtocolData*);
static void ds_generic_apply_edits(iButtonProtocolData*);

const iButtonProtocolDallasBase ibutton_protocol_ds_generic = {
    .family_code = DALLAS_GENERIC_FAMILY_CODE,
    .features = iButtonProtocolFeatureWriteId,
    .data_size = sizeof(DallasGenericProtocolData),
    .manufacturer = DALLAS_COMMON_MANUFACTURER_NAME,
    .name = DALLAS_GENERIC_FAMILY_NAME,

    .read = ds_generic_read,
    .write_id = ds_generic_write_id,
    .write_copy = NULL, /* No data to write a copy */
    .emulate = ds_generic_emulate,
    .save = ds_generic_save,
    .load = ds_generic_load,
    .render_data = NULL, /* No data to render */
    .render_uid = ds_generic_render_uid,
    .render_brief_data = ds_generic_render_brief_data,
    .render_error = ds_generic_render_error,
    .is_valid = ds_generic_is_data_valid,
    .get_editable_data = ds_generic_get_editable_data,
    .apply_edits = ds_generic_apply_edits,
};

bool ds_generic_read(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DallasGenericProtocolData* data = protocol_data;
    return onewire_host_reset(host) && dallas_common_read_rom(host, &data->rom_data);
}

bool ds_generic_write_id(OneWireHost* host, iButtonProtocolData* protocol_data) {
    DallasGenericProtocolData* data = protocol_data;
    return tm2004_write(host, data->rom_data.bytes, sizeof(DallasCommonRomData));
}

static bool ds_generic_reset_callback(bool is_short, void* context) {
    furi_assert(context);
    DallasGenericProtocolData* data = context;
    if(!is_short) {
        onewire_slave_set_overdrive(data->state.bus, is_short);
    }
    return !is_short;
}

static bool ds_generic_command_callback(uint8_t command, void* context) {
    furi_assert(context);
    DallasGenericProtocolData* data = context;
    OneWireSlave* bus = data->state.bus;

    switch(command) {
    case DALLAS_COMMON_CMD_SEARCH_ROM:
        dallas_common_emulate_search_rom(bus, &data->rom_data);
        break;
    case DALLAS_COMMON_CMD_READ_ROM:
        dallas_common_emulate_read_rom(bus, &data->rom_data);
        break;
    default:
        break;
    }

    // No support for multiple consecutive commands
    return false;
}

void ds_generic_emulate(OneWireSlave* bus, iButtonProtocolData* protocol_data) {
    DallasGenericProtocolData* data = protocol_data;
    data->state.bus = bus;

    onewire_slave_set_reset_callback(bus, ds_generic_reset_callback, protocol_data);
    onewire_slave_set_command_callback(bus, ds_generic_command_callback, protocol_data);
}

bool ds_generic_save(FlipperFormat* ff, const iButtonProtocolData* protocol_data) {
    const DallasGenericProtocolData* data = protocol_data;
    return dallas_common_save_rom_data(ff, &data->rom_data);
}

bool ds_generic_load(
    FlipperFormat* ff,
    uint32_t format_version,
    iButtonProtocolData* protocol_data) {
    DallasGenericProtocolData* data = protocol_data;
    return dallas_common_load_rom_data(ff, format_version, &data->rom_data);
}

void ds_generic_render_uid(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DallasGenericProtocolData* data = protocol_data;
    dallas_common_render_uid(result, &data->rom_data);
}

void ds_generic_render_brief_data(FuriString* result, const iButtonProtocolData* protocol_data) {
    const DallasGenericProtocolData* data = protocol_data;

    furi_string_cat_printf(result, "ID: ");
    for(size_t i = 0; i < sizeof(DallasCommonRomData); ++i) {
        furi_string_cat_printf(result, "%02X ", data->rom_data.bytes[i]);
    }
}

void ds_generic_render_error(FuriString* result, const iButtonProtocolData* protocol_data) {
    UNUSED(result);
    UNUSED(protocol_data);
}

bool ds_generic_is_data_valid(const iButtonProtocolData* protocol_data) {
    UNUSED(protocol_data);
    return true;
}

void ds_generic_get_editable_data(
    iButtonEditableData* editable_data,
    iButtonProtocolData* protocol_data) {
    DallasGenericProtocolData* data = protocol_data;
    editable_data->ptr = data->rom_data.bytes;
    editable_data->size = sizeof(DallasCommonRomData);
}

void ds_generic_apply_edits(iButtonProtocolData* protocol_data) {
    UNUSED(protocol_data);
}
