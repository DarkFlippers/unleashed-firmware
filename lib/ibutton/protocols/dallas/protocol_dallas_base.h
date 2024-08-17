#pragma once

#include "../protocol_common_i.h"

#include <one_wire/one_wire_host.h>
#include <one_wire/one_wire_slave.h>

#include <flipper_format/flipper_format.h>

typedef bool (*iButtonProtocolDallasReadWriteFunc)(OneWireHost*, iButtonProtocolData*);
typedef void (*iButtonProtocolDallasEmulateFunc)(OneWireSlave*, iButtonProtocolData*);
typedef bool (*iButtonProtocolDallasSaveFunc)(FlipperFormat*, const iButtonProtocolData*);
typedef bool (*iButtonProtocolDallasLoadFunc)(FlipperFormat*, uint32_t, iButtonProtocolData*);
typedef void (*iButtonProtocolDallasRenderDataFunc)(FuriString*, const iButtonProtocolData*);
typedef bool (*iButtonProtocolDallasIsValidFunc)(const iButtonProtocolData*);
typedef void (
    *iButtonProtocolDallasGetEditableDataFunc)(iButtonEditableData*, iButtonProtocolData*);
typedef void (*iButtonProtocolDallasApplyEditsFunc)(iButtonProtocolData*);

typedef struct {
    const uint8_t family_code;
    const uint32_t features;
    const size_t data_size;
    const char* manufacturer;
    const char* name;

    iButtonProtocolDallasReadWriteFunc read;
    iButtonProtocolDallasReadWriteFunc write_id;
    iButtonProtocolDallasReadWriteFunc write_copy;
    iButtonProtocolDallasEmulateFunc emulate;
    iButtonProtocolDallasSaveFunc save;
    iButtonProtocolDallasLoadFunc load;
    iButtonProtocolDallasRenderDataFunc render_uid;
    iButtonProtocolDallasRenderDataFunc render_data;
    iButtonProtocolDallasRenderDataFunc render_brief_data;
    iButtonProtocolDallasRenderDataFunc render_error;
    iButtonProtocolDallasIsValidFunc is_valid;
    iButtonProtocolDallasGetEditableDataFunc get_editable_data;
    iButtonProtocolDallasApplyEditsFunc apply_edits;
} iButtonProtocolDallasBase;
