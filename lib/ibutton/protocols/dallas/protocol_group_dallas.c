#include "protocol_group_dallas.h"

#include <furi_hal_resources.h>

#include "protocol_group_dallas_defs.h"

#define IBUTTON_ONEWIRE_ROM_SIZE 8U

typedef struct {
    OneWireHost* host;
    OneWireSlave* bus;
} iButtonProtocolGroupDallas;

static iButtonProtocolGroupDallas* ibutton_protocol_group_dallas_alloc(void) {
    iButtonProtocolGroupDallas* group = malloc(sizeof(iButtonProtocolGroupDallas));

    group->host = onewire_host_alloc(&gpio_ibutton);
    group->bus = onewire_slave_alloc(&gpio_ibutton);

    return group;
}

static void ibutton_protocol_group_dallas_free(iButtonProtocolGroupDallas* group) {
    onewire_slave_free(group->bus);
    onewire_host_free(group->host);
    free(group);
}

static size_t ibutton_protocol_group_dallas_get_max_data_size(iButtonProtocolGroupDallas* group) {
    UNUSED(group);
    size_t max_data_size = 0;

    for(iButtonProtocolLocalId i = 0; i < iButtonProtocolDSMax; ++i) {
        const size_t current_rom_size = ibutton_protocols_dallas[i]->data_size;
        if(current_rom_size > max_data_size) {
            max_data_size = current_rom_size;
        }
    }

    return max_data_size;
}

static bool ibutton_protocol_group_dallas_get_id_by_name(
    iButtonProtocolGroupDallas* group,
    iButtonProtocolLocalId* id,
    const char* name) {
    UNUSED(group);
    // Handle older key files which refer to DS1990 as just "Dallas"
    if(strcmp(name, "Dallas") == 0) {
        *id = iButtonProtocolDS1990;
        return true;
    }

    // Handle files that refer to Dallas "Raw Data" as DSGeneric
    if(strcmp(name, "DSGeneric") == 0) {
        *id = iButtonProtocolDSGeneric;
        return true;
    }

    for(iButtonProtocolLocalId i = 0; i < iButtonProtocolDSMax; ++i) {
        if(strcmp(ibutton_protocols_dallas[i]->name, name) == 0) {
            *id = i;
            return true;
        }
    }
    return false;
}

static uint32_t ibutton_protocol_group_dallas_get_features(
    iButtonProtocolGroupDallas* group,
    iButtonProtocolLocalId id) {
    UNUSED(group);
    furi_assert(id < iButtonProtocolDSMax);
    return ibutton_protocols_dallas[id]->features;
}

static const char* ibutton_protocol_group_dallas_get_manufacturer(
    iButtonProtocolGroupDallas* group,
    iButtonProtocolLocalId id) {
    UNUSED(group);
    furi_assert(id < iButtonProtocolDSMax);
    return ibutton_protocols_dallas[id]->manufacturer;
}

static const char* ibutton_protocol_group_dallas_get_name(
    iButtonProtocolGroupDallas* group,
    iButtonProtocolLocalId id) {
    UNUSED(group);
    furi_assert(id < iButtonProtocolDSMax);
    return ibutton_protocols_dallas[id]->name;
}

static iButtonProtocolLocalId
    ibutton_protocol_group_dallas_get_id_by_family_code(uint8_t family_code) {
    iButtonProtocolLocalId id;

    for(id = 0; id < iButtonProtocolDSGeneric; ++id) {
        if(ibutton_protocols_dallas[id]->family_code == family_code) break;
    }

    return id;
}

static bool ibutton_protocol_group_dallas_read(
    iButtonProtocolGroupDallas* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId* id) {
    bool success = false;
    uint8_t rom_data[IBUTTON_ONEWIRE_ROM_SIZE];
    OneWireHost* host = group->host;

    onewire_host_start(host);
    furi_delay_ms(100);

    FURI_CRITICAL_ENTER();

    if(onewire_host_search(host, rom_data, OneWireHostSearchModeNormal)) {
        /* Considering any found 1-Wire device a success.
         * It can be checked later with ibutton_key_is_valid(). */
        success = true;

        /* If a 1-Wire device was found, id is guaranteed to be
         * one of the known keys or DSGeneric. */
        *id = ibutton_protocol_group_dallas_get_id_by_family_code(rom_data[0]);
        ibutton_protocols_dallas[*id]->read(host, data);
    }

    onewire_host_reset_search(host);
    onewire_host_stop(host);

    FURI_CRITICAL_EXIT();

    return success;
}

static bool ibutton_protocol_group_dallas_write_id(
    iButtonProtocolGroupDallas* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId id) {
    furi_assert(id < iButtonProtocolDSMax);
    const iButtonProtocolDallasBase* protocol = ibutton_protocols_dallas[id];
    furi_assert(protocol->features & iButtonProtocolFeatureWriteId);

    OneWireHost* host = group->host;

    onewire_host_start(host);
    furi_delay_ms(100);

    FURI_CRITICAL_ENTER();

    const bool success = protocol->write_id(host, data);
    onewire_host_stop(host);

    FURI_CRITICAL_EXIT();
    return success;
}

static bool ibutton_protocol_group_dallas_write_copy(
    iButtonProtocolGroupDallas* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId id) {
    furi_assert(id < iButtonProtocolDSMax);

    const iButtonProtocolDallasBase* protocol = ibutton_protocols_dallas[id];
    furi_assert(protocol->features & iButtonProtocolFeatureWriteCopy);

    OneWireHost* host = group->host;

    onewire_host_start(host);
    furi_delay_ms(100);

    FURI_CRITICAL_ENTER();

    const bool success = protocol->write_copy(host, data);
    onewire_host_stop(host);

    FURI_CRITICAL_EXIT();
    return success;
}

static void ibutton_protocol_group_dallas_emulate_start(
    iButtonProtocolGroupDallas* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId id) {
    furi_assert(id < iButtonProtocolDSMax);
    OneWireSlave* bus = group->bus;
    ibutton_protocols_dallas[id]->emulate(bus, data);
    onewire_slave_start(bus);
}

static void ibutton_protocol_group_dallas_emulate_stop(
    iButtonProtocolGroupDallas* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId id) {
    furi_assert(id < iButtonProtocolDSMax);
    UNUSED(data);
    onewire_slave_stop(group->bus);
}

static bool ibutton_protocol_group_dallas_save(
    iButtonProtocolGroupDallas* group,
    const iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    FlipperFormat* ff) {
    UNUSED(group);
    furi_assert(id < iButtonProtocolDSMax);
    return ibutton_protocols_dallas[id]->save(ff, data);
}

static bool ibutton_protocol_group_dallas_load(
    iButtonProtocolGroupDallas* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    uint32_t version,
    FlipperFormat* ff) {
    UNUSED(group);
    furi_assert(id < iButtonProtocolDSMax);
    return ibutton_protocols_dallas[id]->load(ff, version, data);
}

static void ibutton_protocol_group_dallas_render_uid(
    iButtonProtocolGroupDallas* group,
    const iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    FuriString* result) {
    UNUSED(group);
    furi_assert(id < iButtonProtocolDSMax);
    const iButtonProtocolDallasBase* protocol = ibutton_protocols_dallas[id];
    furi_assert(protocol->render_uid);
    protocol->render_uid(result, data);
}

static void ibutton_protocol_group_dallas_render_data(
    iButtonProtocolGroupDallas* group,
    const iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    FuriString* result) {
    UNUSED(group);
    furi_assert(id < iButtonProtocolDSMax);
    const iButtonProtocolDallasBase* protocol = ibutton_protocols_dallas[id];
    furi_assert(protocol->render_data);
    protocol->render_data(result, data);
}

static void ibutton_protocol_group_dallas_render_brief_data(
    iButtonProtocolGroupDallas* group,
    const iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    FuriString* result) {
    UNUSED(group);
    furi_assert(id < iButtonProtocolDSMax);
    ibutton_protocols_dallas[id]->render_brief_data(result, data);
}

static void ibutton_protocol_group_dallas_render_error(
    iButtonProtocolGroupDallas* group,
    const iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    FuriString* result) {
    UNUSED(group);
    furi_assert(id < iButtonProtocolDSMax);
    ibutton_protocols_dallas[id]->render_error(result, data);
}

static bool ibutton_protocol_group_dallas_is_valid(
    iButtonProtocolGroupDallas* group,
    const iButtonProtocolData* data,
    iButtonProtocolLocalId id) {
    UNUSED(group);
    furi_assert(id < iButtonProtocolDSMax);
    return ibutton_protocols_dallas[id]->is_valid(data);
}

static void ibutton_protocol_group_dallas_get_editable_data(
    iButtonProtocolGroupDallas* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    iButtonEditableData* editable) {
    UNUSED(group);
    furi_assert(id < iButtonProtocolDSMax);
    ibutton_protocols_dallas[id]->get_editable_data(editable, data);
}

static void ibutton_protocol_group_dallas_apply_edits(
    iButtonProtocolGroupDallas* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId id) {
    UNUSED(group);
    furi_assert(id < iButtonProtocolDSMax);
    ibutton_protocols_dallas[id]->apply_edits(data);
}

const iButtonProtocolGroupBase ibutton_protocol_group_dallas = {
    .protocol_count = iButtonProtocolDSMax,

    .alloc = (iButtonProtocolGroupAllocFunc)ibutton_protocol_group_dallas_alloc,
    .free = (iButtonProtocolGroupFreeFunc)ibutton_protocol_group_dallas_free,

    .get_max_data_size =
        (iButtonProtocolGropuGetSizeFunc)ibutton_protocol_group_dallas_get_max_data_size,
    .get_id_by_name = (iButtonProtocolGroupGetIdFunc)ibutton_protocol_group_dallas_get_id_by_name,
    .get_features =
        (iButtonProtocolGroupGetFeaturesFunc)ibutton_protocol_group_dallas_get_features,

    .get_manufacturer =
        (iButtonProtocolGroupGetStringFunc)ibutton_protocol_group_dallas_get_manufacturer,
    .get_name = (iButtonProtocolGroupGetStringFunc)ibutton_protocol_group_dallas_get_name,

    .read = (iButtonProtocolGroupReadFunc)ibutton_protocol_group_dallas_read,
    .write_id = (iButtonProtocolGroupWriteFunc)ibutton_protocol_group_dallas_write_id,
    .write_copy = (iButtonProtocolGroupWriteFunc)ibutton_protocol_group_dallas_write_copy,

    .emulate_start = (iButtonProtocolGroupApplyFunc)ibutton_protocol_group_dallas_emulate_start,
    .emulate_stop = (iButtonProtocolGroupApplyFunc)ibutton_protocol_group_dallas_emulate_stop,

    .save = (iButtonProtocolGroupSaveFunc)ibutton_protocol_group_dallas_save,
    .load = (iButtonProtocolGroupLoadFunc)ibutton_protocol_group_dallas_load,

    .render_uid = (iButtonProtocolGroupRenderFunc)ibutton_protocol_group_dallas_render_uid,
    .render_data = (iButtonProtocolGroupRenderFunc)ibutton_protocol_group_dallas_render_data,
    .render_brief_data =
        (iButtonProtocolGroupRenderFunc)ibutton_protocol_group_dallas_render_brief_data,
    .render_error = (iButtonProtocolGroupRenderFunc)ibutton_protocol_group_dallas_render_error,

    .is_valid = (iButtonProtocolGroupIsValidFunc)ibutton_protocol_group_dallas_is_valid,
    .get_editable_data =
        (iButtonProtocolGroupGetDataFunc)ibutton_protocol_group_dallas_get_editable_data,
    .apply_edits = (iButtonProtocolGroupApplyFunc)ibutton_protocol_group_dallas_apply_edits,
};
