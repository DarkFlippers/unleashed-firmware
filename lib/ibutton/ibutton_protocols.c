#include "ibutton_protocols.h"

#include <storage/storage.h>

#include "ibutton_key_i.h"

#include "protocols/protocol_group_defs.h"

#define IBUTTON_FILE_TYPE "Flipper iButton key"

#define IBUTTON_PROTOCOL_KEY_V1 "Key type"
#define IBUTTON_PROTOCOL_KEY_V2 "Protocol"

#define IBUTTON_CURRENT_FORMAT_VERSION 2U

#define GET_PROTOCOL_GROUP(id)     \
    iButtonProtocolGroupInfo info; \
    ibutton_protocols_get_group_by_id(protocols, (id), &info);

#define GROUP_BASE (info.base)
#define GROUP_DATA (info.group)
#define PROTOCOL_ID (info.id)

struct iButtonProtocols {
    iButtonProtocolGroupData** group_datas;
};

typedef struct {
    const iButtonProtocolGroupBase* base;
    iButtonProtocolGroupData* group;
    iButtonProtocolLocalId id;
} iButtonProtocolGroupInfo;

static void ibutton_protocols_get_group_by_id(
    iButtonProtocols* protocols,
    iButtonProtocolId id,
    iButtonProtocolGroupInfo* info) {
    iButtonProtocolLocalId local_id = id;

    for(iButtonProtocolGroupId i = 0; i < iButtonProtocolGroupMax; ++i) {
        if(local_id < (signed)ibutton_protocol_groups[i]->protocol_count) {
            info->base = ibutton_protocol_groups[i];
            info->group = protocols->group_datas[i];
            info->id = local_id;
            return;

        } else {
            local_id -= ibutton_protocol_groups[i]->protocol_count;
        }
    }
    furi_crash(NULL);
}

iButtonProtocols* ibutton_protocols_alloc() {
    iButtonProtocols* protocols = malloc(sizeof(iButtonProtocols*));

    protocols->group_datas = malloc(sizeof(iButtonProtocolGroupData*) * iButtonProtocolGroupMax);

    for(iButtonProtocolGroupId i = 0; i < iButtonProtocolGroupMax; ++i) {
        protocols->group_datas[i] = ibutton_protocol_groups[i]->alloc();
    }

    return protocols;
}

void ibutton_protocols_free(iButtonProtocols* protocols) {
    for(iButtonProtocolGroupId i = 0; i < iButtonProtocolGroupMax; ++i) {
        ibutton_protocol_groups[i]->free(protocols->group_datas[i]);
    }

    free(protocols->group_datas);
    free(protocols);
}

uint32_t ibutton_protocols_get_protocol_count() {
    uint32_t count = 0;

    for(iButtonProtocolGroupId i = 0; i < iButtonProtocolGroupMax; ++i) {
        count += ibutton_protocol_groups[i]->protocol_count;
    }

    return count;
}

iButtonProtocolId ibutton_protocols_get_id_by_name(iButtonProtocols* protocols, const char* name) {
    iButtonProtocolLocalId offset = 0;

    for(iButtonProtocolGroupId i = 0; i < iButtonProtocolGroupMax; ++i) {
        iButtonProtocolLocalId local_id;
        if(ibutton_protocol_groups[i]->get_id_by_name(protocols->group_datas[i], &local_id, name)) {
            return local_id + offset;
        }
        offset += ibutton_protocol_groups[i]->protocol_count;
    }
    return iButtonProtocolIdInvalid;
}

uint32_t ibutton_protocols_get_features(iButtonProtocols* protocols, iButtonProtocolId id) {
    GET_PROTOCOL_GROUP(id);
    return GROUP_BASE->get_features(GROUP_DATA, PROTOCOL_ID);
}

size_t ibutton_protocols_get_max_data_size(iButtonProtocols* protocols) {
    size_t max_size = 0;

    for(iButtonProtocolGroupId i = 0; i < iButtonProtocolGroupMax; ++i) {
        const size_t current_max_size =
            ibutton_protocol_groups[i]->get_max_data_size(protocols->group_datas[i]);
        if(current_max_size > max_size) {
            max_size = current_max_size;
        }
    }

    return max_size;
}

const char* ibutton_protocols_get_manufacturer(iButtonProtocols* protocols, iButtonProtocolId id) {
    GET_PROTOCOL_GROUP(id);
    return GROUP_BASE->get_manufacturer(GROUP_DATA, PROTOCOL_ID);
}

const char* ibutton_protocols_get_name(iButtonProtocols* protocols, iButtonProtocolId id) {
    GET_PROTOCOL_GROUP(id);
    return GROUP_BASE->get_name(GROUP_DATA, PROTOCOL_ID);
}

bool ibutton_protocols_read(iButtonProtocols* protocols, iButtonKey* key) {
    iButtonProtocolLocalId id = iButtonProtocolIdInvalid;
    iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    iButtonProtocolLocalId offset = 0;
    for(iButtonProtocolGroupId i = 0; i < iButtonProtocolGroupMax; ++i) {
        if(ibutton_protocol_groups[i]->read(protocols->group_datas[i], data, &id)) {
            id += offset;
            break;
        }
        offset += ibutton_protocol_groups[i]->protocol_count;
    }

    ibutton_key_set_protocol_id(key, id);
    return id != iButtonProtocolIdInvalid;
}

bool ibutton_protocols_write_blank(iButtonProtocols* protocols, iButtonKey* key) {
    const iButtonProtocolId id = ibutton_key_get_protocol_id(key);
    iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    GET_PROTOCOL_GROUP(id);
    return GROUP_BASE->write_blank(GROUP_DATA, data, PROTOCOL_ID);
}

bool ibutton_protocols_write_copy(iButtonProtocols* protocols, iButtonKey* key) {
    const iButtonProtocolId id = ibutton_key_get_protocol_id(key);
    iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    GET_PROTOCOL_GROUP(id);
    return GROUP_BASE->write_copy(GROUP_DATA, data, PROTOCOL_ID);
}

void ibutton_protocols_emulate_start(iButtonProtocols* protocols, iButtonKey* key) {
    const iButtonProtocolId id = ibutton_key_get_protocol_id(key);
    iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    GET_PROTOCOL_GROUP(id);
    GROUP_BASE->emulate_start(GROUP_DATA, data, PROTOCOL_ID);
}

void ibutton_protocols_emulate_stop(iButtonProtocols* protocols, iButtonKey* key) {
    const iButtonProtocolId id = ibutton_key_get_protocol_id(key);
    iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    GET_PROTOCOL_GROUP(id);
    GROUP_BASE->emulate_stop(GROUP_DATA, data, PROTOCOL_ID);
}

bool ibutton_protocols_save(
    iButtonProtocols* protocols,
    const iButtonKey* key,
    const char* file_name) {
    const iButtonProtocolId id = ibutton_key_get_protocol_id(key);
    const iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    bool success = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);

    FlipperFormat* ff = flipper_format_buffered_file_alloc(storage);

    do {
        const char* protocol_name = ibutton_protocols_get_name(protocols, id);

        if(!flipper_format_buffered_file_open_always(ff, file_name)) break;

        if(!flipper_format_write_header_cstr(ff, IBUTTON_FILE_TYPE, IBUTTON_CURRENT_FORMAT_VERSION))
            break;
        if(!flipper_format_write_string_cstr(ff, IBUTTON_PROTOCOL_KEY_V2, protocol_name)) break;

        GET_PROTOCOL_GROUP(id);
        if(!GROUP_BASE->save(GROUP_DATA, data, PROTOCOL_ID, ff)) break;

        success = true;
    } while(false);

    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);

    return success;
}

bool ibutton_protocols_load(iButtonProtocols* protocols, iButtonKey* key, const char* file_name) {
    iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    bool success = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);

    FlipperFormat* ff = flipper_format_buffered_file_alloc(storage);
    FuriString* tmp = furi_string_alloc();

    do {
        if(!flipper_format_buffered_file_open_existing(ff, file_name)) break;

        uint32_t version;

        if(!flipper_format_read_header(ff, tmp, &version)) break;
        if(!furi_string_equal(tmp, IBUTTON_FILE_TYPE)) break;

        if(version == 1) {
            if(!flipper_format_read_string(ff, IBUTTON_PROTOCOL_KEY_V1, tmp)) break;
        } else if(version == 2) {
            if(!flipper_format_read_string(ff, IBUTTON_PROTOCOL_KEY_V2, tmp)) break;
        } else {
            break;
        }

        const iButtonProtocolId id =
            ibutton_protocols_get_id_by_name(protocols, furi_string_get_cstr(tmp));
        ibutton_key_set_protocol_id(key, id);

        GET_PROTOCOL_GROUP(id);
        if(!GROUP_BASE->load(GROUP_DATA, data, PROTOCOL_ID, version, ff)) break;

        success = true;
    } while(false);

    flipper_format_free(ff);
    furi_string_free(tmp);
    furi_record_close(RECORD_STORAGE);

    return success;
}

void ibutton_protocols_render_data(
    iButtonProtocols* protocols,
    const iButtonKey* key,
    FuriString* result) {
    const iButtonProtocolId id = ibutton_key_get_protocol_id(key);
    const iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    GET_PROTOCOL_GROUP(id);
    GROUP_BASE->render_data(GROUP_DATA, data, PROTOCOL_ID, result);
}

void ibutton_protocols_render_brief_data(
    iButtonProtocols* protocols,
    const iButtonKey* key,
    FuriString* result) {
    const iButtonProtocolId id = ibutton_key_get_protocol_id(key);
    const iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    GET_PROTOCOL_GROUP(id);
    GROUP_BASE->render_brief_data(GROUP_DATA, data, PROTOCOL_ID, result);
}

void ibutton_protocols_render_error(
    iButtonProtocols* protocols,
    const iButtonKey* key,
    FuriString* result) {
    const iButtonProtocolId id = ibutton_key_get_protocol_id(key);
    const iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    GET_PROTOCOL_GROUP(id);
    GROUP_BASE->render_error(GROUP_DATA, data, PROTOCOL_ID, result);
}

bool ibutton_protocols_is_valid(iButtonProtocols* protocols, const iButtonKey* key) {
    const iButtonProtocolId id = ibutton_key_get_protocol_id(key);
    const iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    GET_PROTOCOL_GROUP(id);
    return GROUP_BASE->is_valid(GROUP_DATA, data, PROTOCOL_ID);
}

void ibutton_protocols_get_editable_data(
    iButtonProtocols* protocols,
    const iButtonKey* key,
    iButtonEditableData* editable) {
    const iButtonProtocolId id = ibutton_key_get_protocol_id(key);
    iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    GET_PROTOCOL_GROUP(id);
    GROUP_BASE->get_editable_data(GROUP_DATA, data, PROTOCOL_ID, editable);
}

void ibutton_protocols_apply_edits(iButtonProtocols* protocols, const iButtonKey* key) {
    const iButtonProtocolId id = ibutton_key_get_protocol_id(key);
    iButtonProtocolData* data = ibutton_key_get_protocol_data(key);

    GET_PROTOCOL_GROUP(id);
    GROUP_BASE->apply_edits(GROUP_DATA, data, PROTOCOL_ID);
}
