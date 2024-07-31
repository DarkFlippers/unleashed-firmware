#include "protocol_group_misc.h"

#include <furi_hal_rfid.h>
#include <furi_hal_ibutton.h>

#include <toolbox/protocols/protocol_dict.h>

#include "protocol_group_misc_defs.h"

#define IBUTTON_MISC_READ_TIMEOUT 100

#define IBUTTON_MISC_DATA_KEY_KEY_COMMON "Data"

typedef struct {
    ProtocolDict* dict;
    ProtocolId emulate_id;
} iButtonProtocolGroupMisc;

static iButtonProtocolGroupMisc* ibutton_protocol_group_misc_alloc(void) {
    iButtonProtocolGroupMisc* group = malloc(sizeof(iButtonProtocolGroupMisc));

    group->dict = protocol_dict_alloc(ibutton_protocols_misc, iButtonProtocolMiscMax);
    group->emulate_id = PROTOCOL_NO;

    return group;
}

static void ibutton_protocol_group_misc_free(iButtonProtocolGroupMisc* group) {
    protocol_dict_free(group->dict);
    free(group);
}

static size_t ibutton_protocol_group_misc_get_max_data_size(iButtonProtocolGroupMisc* group) {
    return protocol_dict_get_max_data_size(group->dict);
}

static bool ibutton_protocol_group_misc_get_id_by_name(
    iButtonProtocolGroupMisc* group,
    iButtonProtocolLocalId* id,
    const char* name) {
    const ProtocolId found_id = protocol_dict_get_protocol_by_name(group->dict, name);

    if(found_id != PROTOCOL_NO) {
        *id = found_id;
        return true;
    }
    return false;
}

static uint32_t ibutton_protocol_group_misc_get_features(
    iButtonProtocolGroupMisc* group,
    iButtonProtocolLocalId id) {
    UNUSED(group);
    UNUSED(id);
    return 0;
}

static const char* ibutton_protocol_group_misc_get_manufacturer(
    iButtonProtocolGroupMisc* group,
    iButtonProtocolLocalId id) {
    return protocol_dict_get_manufacturer(group->dict, id);
}

static const char* ibutton_protocol_group_misc_get_name(
    iButtonProtocolGroupMisc* group,
    iButtonProtocolLocalId id) {
    return protocol_dict_get_name(group->dict, id);
}

typedef struct {
    uint32_t last_dwt_value;
    FuriStreamBuffer* stream;
} iButtonReadContext;

static void ibutton_protocols_comparator_callback(bool level, void* context) {
    iButtonReadContext* read_context = context;

    uint32_t current_dwt_value = DWT->CYCCNT;

    LevelDuration data =
        level_duration_make(level, current_dwt_value - read_context->last_dwt_value);
    furi_stream_buffer_send(read_context->stream, &data, sizeof(LevelDuration), 0);

    read_context->last_dwt_value = current_dwt_value;
}

static bool ibutton_protocol_group_misc_read(
    iButtonProtocolGroupMisc* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId* id) {
    bool result = false;

    protocol_dict_decoders_start(group->dict);

    furi_hal_rfid_pins_reset();
    // pulldown pull pin, we sense the signal through the analog part of the RFID schematic
    furi_hal_rfid_pin_pull_pulldown();

    iButtonReadContext read_context = {
        .last_dwt_value = DWT->CYCCNT,
        .stream = furi_stream_buffer_alloc(sizeof(LevelDuration) * 512, 1),
    };

    furi_hal_rfid_comp_set_callback(ibutton_protocols_comparator_callback, &read_context);
    furi_hal_rfid_comp_start();

    const uint32_t tick_start = furi_get_tick();

    for(;;) {
        LevelDuration level;
        size_t ret = furi_stream_buffer_receive(
            read_context.stream, &level, sizeof(LevelDuration), IBUTTON_MISC_READ_TIMEOUT);

        if((furi_get_tick() - tick_start) > IBUTTON_MISC_READ_TIMEOUT) {
            break;
        }

        if(ret > 0) {
            ProtocolId decoded_index = protocol_dict_decoders_feed(
                group->dict, level_duration_get_level(level), level_duration_get_duration(level));

            if(decoded_index == PROTOCOL_NO) continue;

            *id = decoded_index;

            protocol_dict_get_data(
                group->dict,
                decoded_index,
                data,
                protocol_dict_get_data_size(group->dict, decoded_index));

            result = true;
        }
    }

    furi_hal_rfid_comp_stop();
    furi_hal_rfid_comp_set_callback(NULL, NULL);
    furi_hal_rfid_pins_reset();

    furi_stream_buffer_free(read_context.stream);

    return result;
}

static void ibutton_protocol_group_misc_emulate_callback(void* context) {
    iButtonProtocolGroupMisc* group = context;

    const LevelDuration level_duration =
        protocol_dict_encoder_yield(group->dict, group->emulate_id);

    furi_hal_ibutton_emulate_set_next(level_duration_get_duration(level_duration));
    furi_hal_ibutton_pin_write(level_duration_get_level(level_duration));
}

static void ibutton_protocol_group_misc_emulate_start(
    iButtonProtocolGroupMisc* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId id) {
    group->emulate_id = id;
    protocol_dict_set_data(group->dict, id, data, protocol_dict_get_data_size(group->dict, id));
    protocol_dict_encoder_start(group->dict, group->emulate_id);

    furi_hal_ibutton_pin_configure();
    furi_hal_ibutton_emulate_start(0, ibutton_protocol_group_misc_emulate_callback, group);
}

static void ibutton_protocol_group_misc_emulate_stop(
    iButtonProtocolGroupMisc* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId id) {
    UNUSED(group);
    UNUSED(data);
    UNUSED(id);
    furi_hal_ibutton_emulate_stop();
    furi_hal_ibutton_pin_reset();
}

static bool ibutton_protocol_group_misc_save(
    iButtonProtocolGroupMisc* group,
    const iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    FlipperFormat* ff) {
    const size_t data_size = protocol_dict_get_data_size(group->dict, id);
    return flipper_format_write_hex(ff, IBUTTON_MISC_DATA_KEY_KEY_COMMON, data, data_size);
}

static bool ibutton_protocol_group_misc_load(
    iButtonProtocolGroupMisc* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    uint32_t version,
    FlipperFormat* ff) {
    const size_t data_size = protocol_dict_get_data_size(group->dict, id);
    switch(version) {
    case 1:
    case 2:
        return flipper_format_read_hex(ff, IBUTTON_MISC_DATA_KEY_KEY_COMMON, data, data_size);
    default:
        return false;
    }
}

static void ibutton_protocol_group_misc_render_uid(
    iButtonProtocolGroupMisc* group,
    const iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    FuriString* result) {
    const size_t data_size = protocol_dict_get_data_size(group->dict, id);
    protocol_dict_set_data(group->dict, id, data, data_size);
    protocol_dict_render_uid(group->dict, result, id);
}

static void ibutton_protocol_group_misc_render_data(
    iButtonProtocolGroupMisc* group,
    const iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    FuriString* result) {
    const size_t data_size = protocol_dict_get_data_size(group->dict, id);
    protocol_dict_set_data(group->dict, id, data, data_size);
    protocol_dict_render_data(group->dict, result, id);
}

static void ibutton_protocol_group_misc_render_brief_data(
    iButtonProtocolGroupMisc* group,
    const iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    FuriString* result) {
    const size_t data_size = protocol_dict_get_data_size(group->dict, id);
    protocol_dict_set_data(group->dict, id, data, data_size);
    protocol_dict_render_brief_data(group->dict, result, id);
}

static void ibutton_protocol_group_misc_render_error(
    iButtonProtocolGroupMisc* group,
    const iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    FuriString* result) {
    UNUSED(group);
    UNUSED(data);
    UNUSED(id);
    UNUSED(result);
}

static bool ibutton_protocol_group_misc_is_valid(
    iButtonProtocolGroupMisc* group,
    const iButtonProtocolData* data,
    iButtonProtocolLocalId id) {
    UNUSED(group);
    UNUSED(data);
    UNUSED(id);
    return true;
}

static void ibutton_protocol_group_misc_get_editable_data(
    iButtonProtocolGroupMisc* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId id,
    iButtonEditableData* editable) {
    editable->ptr = data;
    editable->size = protocol_dict_get_data_size(group->dict, id);
}

static void ibutton_protocol_group_misc_apply_edits(
    iButtonProtocolGroupMisc* group,
    iButtonProtocolData* data,
    iButtonProtocolLocalId id) {
    const size_t data_size = protocol_dict_get_data_size(group->dict, id);
    protocol_dict_set_data(group->dict, id, data, data_size);
}

const iButtonProtocolGroupBase ibutton_protocol_group_misc = {
    .protocol_count = iButtonProtocolMiscMax,

    .alloc = (iButtonProtocolGroupAllocFunc)ibutton_protocol_group_misc_alloc,
    .free = (iButtonProtocolGroupFreeFunc)ibutton_protocol_group_misc_free,

    .get_max_data_size =
        (iButtonProtocolGropuGetSizeFunc)ibutton_protocol_group_misc_get_max_data_size,
    .get_id_by_name = (iButtonProtocolGroupGetIdFunc)ibutton_protocol_group_misc_get_id_by_name,
    .get_features = (iButtonProtocolGroupGetFeaturesFunc)ibutton_protocol_group_misc_get_features,

    .get_manufacturer =
        (iButtonProtocolGroupGetStringFunc)ibutton_protocol_group_misc_get_manufacturer,
    .get_name = (iButtonProtocolGroupGetStringFunc)ibutton_protocol_group_misc_get_name,

    .read = (iButtonProtocolGroupReadFunc)ibutton_protocol_group_misc_read,
    .write_id = NULL,
    .write_copy = NULL,

    .emulate_start = (iButtonProtocolGroupApplyFunc)ibutton_protocol_group_misc_emulate_start,
    .emulate_stop = (iButtonProtocolGroupApplyFunc)ibutton_protocol_group_misc_emulate_stop,

    .save = (iButtonProtocolGroupSaveFunc)ibutton_protocol_group_misc_save,
    .load = (iButtonProtocolGroupLoadFunc)ibutton_protocol_group_misc_load,

    .render_uid = (iButtonProtocolGroupRenderFunc)ibutton_protocol_group_misc_render_uid,
    .render_data = (iButtonProtocolGroupRenderFunc)ibutton_protocol_group_misc_render_data,
    .render_brief_data =
        (iButtonProtocolGroupRenderFunc)ibutton_protocol_group_misc_render_brief_data,
    .render_error = (iButtonProtocolGroupRenderFunc)ibutton_protocol_group_misc_render_error,

    .is_valid = (iButtonProtocolGroupIsValidFunc)ibutton_protocol_group_misc_is_valid,
    .get_editable_data =
        (iButtonProtocolGroupGetDataFunc)ibutton_protocol_group_misc_get_editable_data,
    .apply_edits = (iButtonProtocolGroupApplyFunc)ibutton_protocol_group_misc_apply_edits,
};
