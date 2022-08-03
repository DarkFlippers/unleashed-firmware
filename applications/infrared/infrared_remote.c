#include "infrared_remote.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <m-string.h>
#include <m-array.h>
#include <toolbox/path.h>
#include <storage/storage.h>
#include <core/common_defines.h>

#define TAG "InfraredRemote"

ARRAY_DEF(InfraredButtonArray, InfraredRemoteButton*, M_PTR_OPLIST);

struct InfraredRemote {
    InfraredButtonArray_t buttons;
    string_t name;
    string_t path;
};

static void infrared_remote_clear_buttons(InfraredRemote* remote) {
    InfraredButtonArray_it_t it;
    for(InfraredButtonArray_it(it, remote->buttons); !InfraredButtonArray_end_p(it);
        InfraredButtonArray_next(it)) {
        infrared_remote_button_free(*InfraredButtonArray_cref(it));
    }
    InfraredButtonArray_reset(remote->buttons);
}

InfraredRemote* infrared_remote_alloc() {
    InfraredRemote* remote = malloc(sizeof(InfraredRemote));
    InfraredButtonArray_init(remote->buttons);
    string_init(remote->name);
    string_init(remote->path);
    return remote;
}

void infrared_remote_free(InfraredRemote* remote) {
    infrared_remote_clear_buttons(remote);
    InfraredButtonArray_clear(remote->buttons);
    string_clear(remote->path);
    string_clear(remote->name);
    free(remote);
}

void infrared_remote_reset(InfraredRemote* remote) {
    infrared_remote_clear_buttons(remote);
    string_reset(remote->name);
    string_reset(remote->path);
}

void infrared_remote_set_name(InfraredRemote* remote, const char* name) {
    string_set_str(remote->name, name);
}

const char* infrared_remote_get_name(InfraredRemote* remote) {
    return string_get_cstr(remote->name);
}

void infrared_remote_set_path(InfraredRemote* remote, const char* path) {
    string_set_str(remote->path, path);
}

const char* infrared_remote_get_path(InfraredRemote* remote) {
    return string_get_cstr(remote->path);
}

size_t infrared_remote_get_button_count(InfraredRemote* remote) {
    return InfraredButtonArray_size(remote->buttons);
}

InfraredRemoteButton* infrared_remote_get_button(InfraredRemote* remote, size_t index) {
    furi_assert(index < InfraredButtonArray_size(remote->buttons));
    return *InfraredButtonArray_get(remote->buttons, index);
}

bool infrared_remote_find_button_by_name(InfraredRemote* remote, const char* name, size_t* index) {
    for(size_t i = 0; i < InfraredButtonArray_size(remote->buttons); i++) {
        InfraredRemoteButton* button = *InfraredButtonArray_get(remote->buttons, i);
        if(!strcmp(infrared_remote_button_get_name(button), name)) {
            *index = i;
            return true;
        }
    }
    return false;
}

bool infrared_remote_add_button(InfraredRemote* remote, const char* name, InfraredSignal* signal) {
    InfraredRemoteButton* button = infrared_remote_button_alloc();
    infrared_remote_button_set_name(button, name);
    infrared_remote_button_set_signal(button, signal);
    InfraredButtonArray_push_back(remote->buttons, button);
    return infrared_remote_store(remote);
}

bool infrared_remote_rename_button(InfraredRemote* remote, const char* new_name, size_t index) {
    furi_assert(index < InfraredButtonArray_size(remote->buttons));
    InfraredRemoteButton* button = *InfraredButtonArray_get(remote->buttons, index);
    infrared_remote_button_set_name(button, new_name);
    return infrared_remote_store(remote);
}

bool infrared_remote_delete_button(InfraredRemote* remote, size_t index) {
    furi_assert(index < InfraredButtonArray_size(remote->buttons));
    InfraredRemoteButton* button;
    InfraredButtonArray_pop_at(&button, remote->buttons, index);
    infrared_remote_button_free(button);
    return infrared_remote_store(remote);
}

bool infrared_remote_store(InfraredRemote* remote) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);
    const char* path = string_get_cstr(remote->path);

    FURI_LOG_I(TAG, "store file: \'%s\'", path);

    bool success = flipper_format_file_open_always(ff, path) &&
                   flipper_format_write_header_cstr(ff, "IR signals file", 1);
    if(success) {
        InfraredButtonArray_it_t it;
        for(InfraredButtonArray_it(it, remote->buttons); !InfraredButtonArray_end_p(it);
            InfraredButtonArray_next(it)) {
            InfraredRemoteButton* button = *InfraredButtonArray_cref(it);
            success = infrared_signal_save(
                infrared_remote_button_get_signal(button),
                ff,
                infrared_remote_button_get_name(button));
            if(!success) {
                break;
            }
        }
    }

    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
    return success;
}

bool infrared_remote_load(InfraredRemote* remote, string_t path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_buffered_file_alloc(storage);

    string_t buf;
    string_init(buf);

    FURI_LOG_I(TAG, "load file: \'%s\'", string_get_cstr(path));
    bool success = flipper_format_buffered_file_open_existing(ff, string_get_cstr(path));

    if(success) {
        uint32_t version;
        success = flipper_format_read_header(ff, buf, &version) &&
                  !string_cmp_str(buf, "IR signals file") && (version == 1);
    }

    if(success) {
        path_extract_filename(path, buf, true);
        infrared_remote_clear_buttons(remote);
        infrared_remote_set_name(remote, string_get_cstr(buf));
        infrared_remote_set_path(remote, string_get_cstr(path));

        for(bool can_read = true; can_read;) {
            InfraredRemoteButton* button = infrared_remote_button_alloc();
            can_read = infrared_signal_read(infrared_remote_button_get_signal(button), ff, buf);
            if(can_read) {
                infrared_remote_button_set_name(button, string_get_cstr(buf));
                InfraredButtonArray_push_back(remote->buttons, button);
            } else {
                infrared_remote_button_free(button);
            }
        }
    }

    string_clear(buf);
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
    return success;
}

bool infrared_remote_remove(InfraredRemote* remote) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    FS_Error status = storage_common_remove(storage, string_get_cstr(remote->path));
    infrared_remote_reset(remote);

    furi_record_close(RECORD_STORAGE);
    return (status == FSE_OK || status == FSE_NOT_EXIST);
}
