#include "../js_modules.h"
#include <storage/storage.h>

typedef struct {
    Storage* api;
} JsStorageInst;

static JsStorageInst* get_this_ctx(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsStorageInst* storage = mjs_get_ptr(mjs, obj_inst);
    furi_assert(storage);
    return storage;
}

static void ret_bad_args(struct mjs* mjs, const char* error) {
    mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "%s", error);
    mjs_return(mjs, MJS_UNDEFINED);
}

static void ret_int_err(struct mjs* mjs, const char* error) {
    mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "%s", error);
    mjs_return(mjs, MJS_UNDEFINED);
}

static bool check_arg_count(struct mjs* mjs, size_t count) {
    size_t num_args = mjs_nargs(mjs);
    if(num_args != count) {
        ret_bad_args(mjs, "Wrong argument count");
        return false;
    }
    return true;
}

static bool get_path_arg(struct mjs* mjs, const char** path, size_t index) {
    mjs_val_t path_obj = mjs_arg(mjs, index);
    if(!mjs_is_string(path_obj)) {
        ret_bad_args(mjs, "Path must be a string");
        return false;
    }
    size_t path_len = 0;
    *path = mjs_get_string(mjs, &path_obj, &path_len);
    if((path_len == 0) || (*path == NULL)) {
        ret_bad_args(mjs, "Bad path argument");
        return false;
    }
    return true;
}

static void js_storage_read(struct mjs* mjs) {
    JsStorageInst* storage = get_this_ctx(mjs);

    const char* path;
    if(!get_path_arg(mjs, &path, 0)) return;

    File* file = storage_file_alloc(storage->api);
    do {
        if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            ret_int_err(mjs, storage_file_get_error_desc(file));
            break;
        }

        uint64_t size = storage_file_size(file);
        mjs_val_t size_arg = mjs_arg(mjs, 1);
        if(mjs_is_number(size_arg)) {
            size = mjs_get_int32(mjs, size_arg);
        }

        mjs_val_t seek_arg = mjs_arg(mjs, 2);
        if(mjs_is_number(seek_arg)) {
            storage_file_seek(file, mjs_get_int32(mjs, seek_arg), true);
            size = MIN(size, storage_file_size(file) - storage_file_tell(file));
        }

        if(size > memmgr_heap_get_max_free_block()) {
            ret_int_err(mjs, "Read size too large");
            break;
        }

        uint8_t* data = malloc(size);
        size_t read = storage_file_read(file, data, size);
        if(read == size) {
            mjs_return(mjs, mjs_mk_array_buf(mjs, (char*)data, size));
        } else {
            ret_int_err(mjs, "File read failed");
        }
        free(data);
    } while(0);
    storage_file_free(file);
}

static void js_storage_write(struct mjs* mjs) {
    JsStorageInst* storage = get_this_ctx(mjs);

    const char* path;
    if(!get_path_arg(mjs, &path, 0)) return;

    mjs_val_t data_arg = mjs_arg(mjs, 1);
    if(!mjs_is_typed_array(data_arg) && !mjs_is_string(data_arg)) {
        ret_bad_args(mjs, "Data must be string, arraybuf or dataview");
        return;
    }
    if(mjs_is_data_view(data_arg)) {
        data_arg = mjs_dataview_get_buf(mjs, data_arg);
    }
    size_t data_len = 0;
    const char* data = NULL;
    if(mjs_is_string(data_arg)) {
        data = mjs_get_string(mjs, &data_arg, &data_len);
    } else if(mjs_is_typed_array(data_arg)) {
        data = mjs_array_buf_get_ptr(mjs, data_arg, &data_len);
    }

    mjs_val_t seek_arg = mjs_arg(mjs, 2);

    File* file = storage_file_alloc(storage->api);
    if(!storage_file_open(
           file,
           path,
           FSAM_WRITE,
           mjs_is_number(seek_arg) ? FSOM_OPEN_ALWAYS : FSOM_CREATE_ALWAYS)) {
        ret_int_err(mjs, storage_file_get_error_desc(file));

    } else {
        if(mjs_is_number(seek_arg)) {
            storage_file_seek(file, mjs_get_int32(mjs, seek_arg), true);
        }

        size_t write = storage_file_write(file, data, data_len);
        mjs_return(mjs, mjs_mk_boolean(mjs, write == data_len));
    }
    storage_file_free(file);
}

static void js_storage_append(struct mjs* mjs) {
    JsStorageInst* storage = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 2)) return;

    const char* path;
    if(!get_path_arg(mjs, &path, 0)) return;

    mjs_val_t data_arg = mjs_arg(mjs, 1);
    if(!mjs_is_typed_array(data_arg) && !mjs_is_string(data_arg)) {
        ret_bad_args(mjs, "Data must be string, arraybuf or dataview");
        return;
    }
    if(mjs_is_data_view(data_arg)) {
        data_arg = mjs_dataview_get_buf(mjs, data_arg);
    }
    size_t data_len = 0;
    const char* data = NULL;
    if(mjs_is_string(data_arg)) {
        data = mjs_get_string(mjs, &data_arg, &data_len);
    } else if(mjs_is_typed_array(data_arg)) {
        data = mjs_array_buf_get_ptr(mjs, data_arg, &data_len);
    }

    File* file = storage_file_alloc(storage->api);
    if(!storage_file_open(file, path, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        ret_int_err(mjs, storage_file_get_error_desc(file));
    } else {
        size_t write = storage_file_write(file, data, data_len);
        mjs_return(mjs, mjs_mk_boolean(mjs, write == data_len));
    }
    storage_file_free(file);
}

static void js_storage_exists(struct mjs* mjs) {
    JsStorageInst* storage = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 1)) return;

    const char* path;
    if(!get_path_arg(mjs, &path, 0)) return;

    mjs_return(mjs, mjs_mk_boolean(mjs, storage_common_exists(storage->api, path)));
}

static void js_storage_remove(struct mjs* mjs) {
    JsStorageInst* storage = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 1)) return;

    const char* path;
    if(!get_path_arg(mjs, &path, 0)) return;

    mjs_return(mjs, mjs_mk_boolean(mjs, storage_simply_remove(storage->api, path)));
}

static void js_storage_copy(struct mjs* mjs) {
    JsStorageInst* storage = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 2)) return;

    const char* old_path;
    if(!get_path_arg(mjs, &old_path, 0)) return;

    const char* new_path;
    if(!get_path_arg(mjs, &new_path, 1)) return;

    FS_Error error = storage_common_copy(storage->api, old_path, new_path);
    if(error == FSE_OK) {
        mjs_return(mjs, MJS_UNDEFINED);
    } else {
        ret_int_err(mjs, storage_error_get_desc(error));
    }
}

static void js_storage_move(struct mjs* mjs) {
    JsStorageInst* storage = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 2)) return;

    const char* old_path;
    if(!get_path_arg(mjs, &old_path, 0)) return;

    const char* new_path;
    if(!get_path_arg(mjs, &new_path, 1)) return;

    FS_Error error = storage_common_rename(storage->api, old_path, new_path);
    if(error == FSE_OK) {
        mjs_return(mjs, MJS_UNDEFINED);
    } else {
        ret_int_err(mjs, storage_error_get_desc(error));
    }
}

static void js_storage_mkdir(struct mjs* mjs) {
    JsStorageInst* storage = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 1)) return;

    const char* path;
    if(!get_path_arg(mjs, &path, 0)) return;

    mjs_return(mjs, mjs_mk_boolean(mjs, storage_simply_mkdir(storage->api, path)));
}

static void* js_storage_create(struct mjs* mjs, mjs_val_t* object) {
    JsStorageInst* storage = malloc(sizeof(JsStorageInst));
    mjs_val_t storage_obj = mjs_mk_object(mjs);
    mjs_set(mjs, storage_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, storage));
    mjs_set(mjs, storage_obj, "read", ~0, MJS_MK_FN(js_storage_read));
    mjs_set(mjs, storage_obj, "write", ~0, MJS_MK_FN(js_storage_write));
    mjs_set(mjs, storage_obj, "append", ~0, MJS_MK_FN(js_storage_append));
    mjs_set(mjs, storage_obj, "exists", ~0, MJS_MK_FN(js_storage_exists));
    mjs_set(mjs, storage_obj, "remove", ~0, MJS_MK_FN(js_storage_remove));
    mjs_set(mjs, storage_obj, "copy", ~0, MJS_MK_FN(js_storage_copy));
    mjs_set(mjs, storage_obj, "move", ~0, MJS_MK_FN(js_storage_move));
    mjs_set(mjs, storage_obj, "mkdir", ~0, MJS_MK_FN(js_storage_mkdir));
    storage->api = furi_record_open(RECORD_STORAGE);
    *object = storage_obj;
    return storage;
}

static void js_storage_destroy(void* inst) {
    JsStorageInst* storage = inst;
    furi_record_close(RECORD_STORAGE);
    free(storage);
}

static const JsModuleDescriptor js_storage_desc = {
    "storage",
    js_storage_create,
    js_storage_destroy,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_storage_desc,
};

const FlipperAppPluginDescriptor* js_storage_ep(void) {
    return &plugin_descriptor;
}
