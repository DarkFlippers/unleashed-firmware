#include "../js_modules.h" // IWYU pragma: keep
#include <path.h>

// ---=== file ops ===---

static void js_storage_file_close(struct mjs* mjs) {
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY); // 0 args
    File* file = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_file_close(file)));
}

static void js_storage_file_is_open(struct mjs* mjs) {
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY); // 0 args
    File* file = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_file_is_open(file)));
}

static void js_storage_file_read(struct mjs* mjs) {
    enum {
        ReadModeAscii,
        ReadModeBinary,
    } read_mode;
    JS_ENUM_MAP(read_mode, {"ascii", ReadModeAscii}, {"binary", ReadModeBinary});
    int32_t length;
    JS_FETCH_ARGS_OR_RETURN(
        mjs, JS_EXACTLY, JS_ARG_ENUM(read_mode, "ReadMode"), JS_ARG_INT32(&length));
    File* file = JS_GET_CONTEXT(mjs);
    char buffer[length];
    size_t actually_read = storage_file_read(file, buffer, length);
    if(read_mode == ReadModeAscii) {
        mjs_return(mjs, mjs_mk_string(mjs, buffer, actually_read, true));
    } else if(read_mode == ReadModeBinary) {
        mjs_return(mjs, mjs_mk_array_buf(mjs, buffer, actually_read));
    }
}

static void js_storage_file_write(struct mjs* mjs) {
    mjs_val_t data;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_ANY(&data));
    const void* buf;
    size_t len;
    if(mjs_is_string(data)) {
        buf = mjs_get_string(mjs, &data, &len);
    } else if(mjs_is_array_buf(data)) {
        buf = mjs_array_buf_get_ptr(mjs, data, &len);
    } else {
        JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "argument 0: expected string or ArrayBuffer");
    }
    File* file = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_number(mjs, storage_file_write(file, buf, len)));
}

static void js_storage_file_seek_relative(struct mjs* mjs) {
    int32_t offset;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_INT32(&offset));
    File* file = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_file_seek(file, offset, false)));
}

static void js_storage_file_seek_absolute(struct mjs* mjs) {
    int32_t offset;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_INT32(&offset));
    File* file = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_file_seek(file, offset, true)));
}

static void js_storage_file_tell(struct mjs* mjs) {
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY); // 0 args
    File* file = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_number(mjs, storage_file_tell(file)));
}

static void js_storage_file_truncate(struct mjs* mjs) {
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY); // 0 args
    File* file = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_file_truncate(file)));
}

static void js_storage_file_size(struct mjs* mjs) {
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY); // 0 args
    File* file = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_number(mjs, storage_file_size(file)));
}

static void js_storage_file_eof(struct mjs* mjs) {
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY); // 0 args
    File* file = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_file_eof(file)));
}

static void js_storage_file_copy_to(struct mjs* mjs) {
    File* source = JS_GET_CONTEXT(mjs);
    mjs_val_t dest_obj;
    int32_t bytes;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_OBJ(&dest_obj), JS_ARG_INT32(&bytes));
    File* destination = JS_GET_INST(mjs, dest_obj);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_file_copy_to_file(source, destination, bytes)));
}

// ---=== top-level file ops ===---

// common destructor for file and dir objects
static void js_storage_file_destructor(struct mjs* mjs, mjs_val_t obj) {
    File* file = JS_GET_INST(mjs, obj);
    storage_file_free(file);
}

static void js_storage_open_file(struct mjs* mjs) {
    const char* path;
    FS_AccessMode access_mode;
    FS_OpenMode open_mode;
    JS_ENUM_MAP(access_mode, {"r", FSAM_READ}, {"w", FSAM_WRITE}, {"rw", FSAM_READ_WRITE});
    JS_ENUM_MAP(
        open_mode,
        {"open_existing", FSOM_OPEN_EXISTING},
        {"open_always", FSOM_OPEN_ALWAYS},
        {"open_append", FSOM_OPEN_APPEND},
        {"create_new", FSOM_CREATE_NEW},
        {"create_always", FSOM_CREATE_ALWAYS});
    JS_FETCH_ARGS_OR_RETURN(
        mjs,
        JS_EXACTLY,
        JS_ARG_STR(&path),
        JS_ARG_ENUM(access_mode, "AccessMode"),
        JS_ARG_ENUM(open_mode, "OpenMode"));

    Storage* storage = JS_GET_CONTEXT(mjs);
    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, path, access_mode, open_mode)) {
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    mjs_val_t file_obj = mjs_mk_object(mjs);
    JS_ASSIGN_MULTI(mjs, file_obj) {
        JS_FIELD(INST_PROP_NAME, mjs_mk_foreign(mjs, file));
        JS_FIELD(MJS_DESTRUCTOR_PROP_NAME, MJS_MK_FN(js_storage_file_destructor));
        JS_FIELD("close", MJS_MK_FN(js_storage_file_close));
        JS_FIELD("isOpen", MJS_MK_FN(js_storage_file_is_open));
        JS_FIELD("read", MJS_MK_FN(js_storage_file_read));
        JS_FIELD("write", MJS_MK_FN(js_storage_file_write));
        JS_FIELD("seekRelative", MJS_MK_FN(js_storage_file_seek_relative));
        JS_FIELD("seekAbsolute", MJS_MK_FN(js_storage_file_seek_absolute));
        JS_FIELD("tell", MJS_MK_FN(js_storage_file_tell));
        JS_FIELD("truncate", MJS_MK_FN(js_storage_file_truncate));
        JS_FIELD("size", MJS_MK_FN(js_storage_file_size));
        JS_FIELD("eof", MJS_MK_FN(js_storage_file_eof));
        JS_FIELD("copyTo", MJS_MK_FN(js_storage_file_copy_to));
    }
    mjs_return(mjs, file_obj);
}

static void js_storage_file_exists(struct mjs* mjs) {
    const char* path;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&path));
    Storage* storage = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_file_exists(storage, path)));
}

// ---=== dir ops ===---

static void js_storage_read_directory(struct mjs* mjs) {
    const char* path;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&path));

    Storage* storage = JS_GET_CONTEXT(mjs);
    File* dir = storage_file_alloc(storage);
    if(!storage_dir_open(dir, path)) {
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    FileInfo file_info;
    char name[128];
    FuriString* file_path = furi_string_alloc_set_str(path);
    size_t path_size = furi_string_size(file_path);
    uint32_t timestamp;

    mjs_val_t ret = mjs_mk_array(mjs);
    while(storage_dir_read(dir, &file_info, name, sizeof(name))) {
        furi_string_left(file_path, path_size);
        path_append(file_path, name);
        furi_check(
            storage_common_timestamp(storage, furi_string_get_cstr(file_path), &timestamp) ==
            FSE_OK);
        mjs_val_t obj = mjs_mk_object(mjs);
        JS_ASSIGN_MULTI(mjs, obj) {
            JS_FIELD("path", mjs_mk_string(mjs, name, ~0, true));
            JS_FIELD("isDirectory", mjs_mk_boolean(mjs, file_info_is_dir(&file_info)));
            JS_FIELD("size", mjs_mk_number(mjs, file_info.size));
            JS_FIELD("timestamp", mjs_mk_number(mjs, timestamp));
        }
        mjs_array_push(mjs, ret, obj);
    }

    storage_file_free(dir);
    furi_string_free(file_path);
    mjs_return(mjs, ret);
}

static void js_storage_directory_exists(struct mjs* mjs) {
    const char* path;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&path));
    Storage* storage = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_dir_exists(storage, path)));
}

static void js_storage_make_directory(struct mjs* mjs) {
    const char* path;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&path));
    Storage* storage = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_simply_mkdir(storage, path)));
}

// ---=== common ops ===---

static void js_storage_file_or_dir_exists(struct mjs* mjs) {
    const char* path;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&path));
    Storage* storage = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_common_exists(storage, path)));
}

static void js_storage_stat(struct mjs* mjs) {
    const char* path;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&path));
    Storage* storage = JS_GET_CONTEXT(mjs);
    FileInfo file_info;
    uint32_t timestamp;
    if((storage_common_stat(storage, path, &file_info) |
        storage_common_timestamp(storage, path, &timestamp)) != FSE_OK) {
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }
    mjs_val_t ret = mjs_mk_object(mjs);
    JS_ASSIGN_MULTI(mjs, ret) {
        JS_FIELD("path", mjs_mk_string(mjs, path, ~0, 1));
        JS_FIELD("isDirectory", mjs_mk_boolean(mjs, file_info_is_dir(&file_info)));
        JS_FIELD("size", mjs_mk_number(mjs, file_info.size));
        JS_FIELD("accessTime", mjs_mk_number(mjs, timestamp));
    }
    mjs_return(mjs, ret);
}

static void js_storage_remove(struct mjs* mjs) {
    const char* path;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&path));
    Storage* storage = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_simply_remove(storage, path)));
}

static void js_storage_rmrf(struct mjs* mjs) {
    const char* path;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&path));
    Storage* storage = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_simply_remove_recursive(storage, path)));
}

static void js_storage_rename(struct mjs* mjs) {
    const char *old, *new;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&old), JS_ARG_STR(&new));
    Storage* storage = JS_GET_CONTEXT(mjs);
    FS_Error status = storage_common_rename(storage, old, new);
    mjs_return(mjs, mjs_mk_boolean(mjs, status == FSE_OK));
}

static void js_storage_copy(struct mjs* mjs) {
    const char *source, *dest;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&source), JS_ARG_STR(&dest));
    Storage* storage = JS_GET_CONTEXT(mjs);
    FS_Error status = storage_common_copy(storage, source, dest);
    mjs_return(mjs, mjs_mk_boolean(mjs, status == FSE_OK || status == FSE_EXIST));
}

static void js_storage_fs_info(struct mjs* mjs) {
    const char* fs;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&fs));
    Storage* storage = JS_GET_CONTEXT(mjs);
    uint64_t total_space, free_space;
    if(storage_common_fs_info(storage, fs, &total_space, &free_space) != FSE_OK) {
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }
    mjs_val_t ret = mjs_mk_object(mjs);
    JS_ASSIGN_MULTI(mjs, ret) {
        JS_FIELD("totalSpace", mjs_mk_number(mjs, total_space));
        JS_FIELD("freeSpace", mjs_mk_number(mjs, free_space));
    }
    mjs_return(mjs, ret);
}

static void js_storage_next_available_filename(struct mjs* mjs) {
    const char *dir_path, *file_name, *file_ext;
    int32_t max_len;
    JS_FETCH_ARGS_OR_RETURN(
        mjs,
        JS_EXACTLY,
        JS_ARG_STR(&dir_path),
        JS_ARG_STR(&file_name),
        JS_ARG_STR(&file_ext),
        JS_ARG_INT32(&max_len));
    Storage* storage = JS_GET_CONTEXT(mjs);
    FuriString* next_name = furi_string_alloc();
    storage_get_next_filename(storage, dir_path, file_name, file_ext, next_name, max_len);
    mjs_return(mjs, mjs_mk_string(mjs, furi_string_get_cstr(next_name), ~0, true));
    furi_string_free(next_name);
}

// ---=== path ops ===---

static void js_storage_are_paths_equal(struct mjs* mjs) {
    const char *path1, *path2;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&path1), JS_ARG_STR(&path2));
    Storage* storage = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_common_equivalent_path(storage, path1, path2)));
}

static void js_storage_is_subpath_of(struct mjs* mjs) {
    const char *parent, *child;
    JS_FETCH_ARGS_OR_RETURN(mjs, JS_EXACTLY, JS_ARG_STR(&parent), JS_ARG_STR(&child));
    Storage* storage = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, mjs_mk_boolean(mjs, storage_common_is_subdir(storage, parent, child)));
}

// ---=== module ctor & dtor ===---

static void* js_storage_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules) {
    UNUSED(modules);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    UNUSED(storage);
    *object = mjs_mk_object(mjs);
    JS_ASSIGN_MULTI(mjs, *object) {
        JS_FIELD(INST_PROP_NAME, mjs_mk_foreign(mjs, storage));

        // top-level file ops
        JS_FIELD("openFile", MJS_MK_FN(js_storage_open_file));
        JS_FIELD("fileExists", MJS_MK_FN(js_storage_file_exists));

        // dir ops
        JS_FIELD("readDirectory", MJS_MK_FN(js_storage_read_directory));
        JS_FIELD("directoryExists", MJS_MK_FN(js_storage_directory_exists));
        JS_FIELD("makeDirectory", MJS_MK_FN(js_storage_make_directory));

        // common ops
        JS_FIELD("fileOrDirExists", MJS_MK_FN(js_storage_file_or_dir_exists));
        JS_FIELD("stat", MJS_MK_FN(js_storage_stat));
        JS_FIELD("remove", MJS_MK_FN(js_storage_remove));
        JS_FIELD("rmrf", MJS_MK_FN(js_storage_rmrf));
        JS_FIELD("rename", MJS_MK_FN(js_storage_rename));
        JS_FIELD("copy", MJS_MK_FN(js_storage_copy));
        JS_FIELD("fsInfo", MJS_MK_FN(js_storage_fs_info));
        JS_FIELD("nextAvailableFilename", MJS_MK_FN(js_storage_next_available_filename));

        // path ops
        JS_FIELD("arePathsEqual", MJS_MK_FN(js_storage_are_paths_equal));
        JS_FIELD("isSubpathOf", MJS_MK_FN(js_storage_is_subpath_of));
    }
    return NULL;
}

static void js_storage_destroy(void* data) {
    UNUSED(data);
    furi_record_close(RECORD_STORAGE);
}

// ---=== boilerplate ===---

static const JsModuleDescriptor js_storage_desc = {
    "storage",
    js_storage_create,
    js_storage_destroy,
    NULL,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_storage_desc,
};

const FlipperAppPluginDescriptor* js_storage_ep(void) {
    return &plugin_descriptor;
}
