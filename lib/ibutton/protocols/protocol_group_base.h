#pragma once

#include <stdbool.h>
#include <flipper_format.h>

#include "protocol_common_i.h"

typedef void iButtonProtocolGroupData;
typedef int32_t iButtonProtocolGroupId;

typedef iButtonProtocolGroupData* (*iButtonProtocolGroupAllocFunc)(void);

typedef void (*iButtonProtocolGroupFreeFunc)(iButtonProtocolGroupData*);

typedef void (*iButtonProtocolGroupRenderFunc)(
    iButtonProtocolGroupData*,
    const iButtonProtocolData*,
    iButtonProtocolLocalId,
    FuriString*);

typedef bool (*iButtonProtocolGroupIsValidFunc)(
    iButtonProtocolGroupData*,
    const iButtonProtocolData*,
    iButtonProtocolLocalId);

typedef void (*iButtonProtocolGroupGetDataFunc)(
    iButtonProtocolGroupData*,
    iButtonProtocolData*,
    iButtonProtocolLocalId,
    iButtonEditableData*);

typedef void (*iButtonProtocolGroupApplyFunc)(
    iButtonProtocolGroupData*,
    iButtonProtocolData*,
    iButtonProtocolLocalId);

typedef size_t (*iButtonProtocolGropuGetSizeFunc)(iButtonProtocolGroupData*);

typedef uint32_t (
    *iButtonProtocolGroupGetFeaturesFunc)(iButtonProtocolGroupData*, iButtonProtocolLocalId);

typedef const char* (
    *iButtonProtocolGroupGetStringFunc)(iButtonProtocolGroupData*, iButtonProtocolLocalId);

typedef bool (*iButtonProtocolGroupGetIdFunc)(
    iButtonProtocolGroupData*,
    iButtonProtocolLocalId*,
    const char*);

typedef bool (*iButtonProtocolGroupReadFunc)(
    iButtonProtocolGroupData*,
    iButtonProtocolData*,
    iButtonProtocolLocalId*);

typedef bool (*iButtonProtocolGroupWriteFunc)(
    iButtonProtocolGroupData*,
    iButtonProtocolData*,
    iButtonProtocolLocalId);

typedef bool (*iButtonProtocolGroupSaveFunc)(
    iButtonProtocolGroupData*,
    const iButtonProtocolData*,
    iButtonProtocolLocalId,
    FlipperFormat*);

typedef bool (*iButtonProtocolGroupLoadFunc)(
    iButtonProtocolGroupData*,
    iButtonProtocolData*,
    iButtonProtocolLocalId,
    uint32_t,
    FlipperFormat*);

typedef struct {
    const uint32_t protocol_count;

    iButtonProtocolGroupAllocFunc alloc;
    iButtonProtocolGroupFreeFunc free;

    iButtonProtocolGropuGetSizeFunc get_max_data_size;
    iButtonProtocolGroupGetIdFunc get_id_by_name;
    iButtonProtocolGroupGetFeaturesFunc get_features;

    iButtonProtocolGroupGetStringFunc get_manufacturer;
    iButtonProtocolGroupGetStringFunc get_name;

    iButtonProtocolGroupReadFunc read;
    iButtonProtocolGroupWriteFunc write_id;
    iButtonProtocolGroupWriteFunc write_copy;

    iButtonProtocolGroupApplyFunc emulate_start;
    iButtonProtocolGroupApplyFunc emulate_stop;

    iButtonProtocolGroupSaveFunc save;
    iButtonProtocolGroupLoadFunc load;

    iButtonProtocolGroupRenderFunc render_uid;
    iButtonProtocolGroupRenderFunc render_data;
    iButtonProtocolGroupRenderFunc render_brief_data;
    iButtonProtocolGroupRenderFunc render_error;

    iButtonProtocolGroupIsValidFunc is_valid;
    iButtonProtocolGroupGetDataFunc get_editable_data;

    iButtonProtocolGroupApplyFunc apply_edits;
} iButtonProtocolGroupBase;
