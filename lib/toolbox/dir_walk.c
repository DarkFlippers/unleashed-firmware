#include "dir_walk.h"
#include <m-list.h>

LIST_DEF(DirIndexList, uint32_t);

struct DirWalk {
    File* file;
    FuriString* path;
    DirIndexList_t index_list;
    uint32_t current_index;
    bool recursive;
    DirWalkFilterCb filter_cb;
    void* filter_context;
};

DirWalk* dir_walk_alloc(Storage* storage) {
    furi_check(storage);

    DirWalk* dir_walk = malloc(sizeof(DirWalk));
    dir_walk->path = furi_string_alloc();
    dir_walk->file = storage_file_alloc(storage);
    DirIndexList_init(dir_walk->index_list);
    dir_walk->recursive = true;
    dir_walk->filter_cb = NULL;
    return dir_walk;
}

void dir_walk_free(DirWalk* dir_walk) {
    furi_check(dir_walk);

    storage_file_free(dir_walk->file);
    furi_string_free(dir_walk->path);
    DirIndexList_clear(dir_walk->index_list);
    free(dir_walk);
}

void dir_walk_set_recursive(DirWalk* dir_walk, bool recursive) {
    furi_check(dir_walk);
    dir_walk->recursive = recursive;
}

void dir_walk_set_filter_cb(DirWalk* dir_walk, DirWalkFilterCb cb, void* context) {
    furi_check(dir_walk);
    dir_walk->filter_cb = cb;
    dir_walk->filter_context = context;
}

bool dir_walk_open(DirWalk* dir_walk, const char* path) {
    furi_check(dir_walk);
    furi_string_set(dir_walk->path, path);
    dir_walk->current_index = 0;
    return storage_dir_open(dir_walk->file, path);
}

static bool dir_walk_filter(DirWalk* dir_walk, const char* name, FileInfo* fileinfo) {
    if(dir_walk->filter_cb) {
        return dir_walk->filter_cb(name, fileinfo, dir_walk->filter_context);
    } else {
        return true;
    }
}

static DirWalkResult
    dir_walk_iter(DirWalk* dir_walk, FuriString* return_path, FileInfo* fileinfo) {
    DirWalkResult result = DirWalkError;
    char* name = malloc(256); // FIXME: remove magic number
    FileInfo info;
    bool end = false;

    while(!end) {
        storage_dir_read(dir_walk->file, &info, name, 255);

        if(storage_file_get_error(dir_walk->file) == FSE_OK) {
            result = DirWalkOK;
            dir_walk->current_index++;

            if(dir_walk_filter(dir_walk, name, &info)) {
                if(return_path != NULL) {
                    furi_string_printf( //-V576
                        return_path,
                        "%s/%s",
                        furi_string_get_cstr(dir_walk->path),
                        name);
                }

                if(fileinfo != NULL) {
                    memcpy(fileinfo, &info, sizeof(FileInfo));
                }

                end = true;
            }

            if(file_info_is_dir(&info) && dir_walk->recursive) {
                // step into
                DirIndexList_push_back(dir_walk->index_list, dir_walk->current_index);
                dir_walk->current_index = 0;
                storage_dir_close(dir_walk->file);

                furi_string_cat_printf(dir_walk->path, "/%s", name);
                storage_dir_open(dir_walk->file, furi_string_get_cstr(dir_walk->path));
            }
        } else if(storage_file_get_error(dir_walk->file) == FSE_NOT_EXIST) {
            if(DirIndexList_size(dir_walk->index_list) == 0) {
                // last
                result = DirWalkLast;
                end = true;
            } else {
                // step out
                uint32_t index;
                DirIndexList_pop_back(&index, dir_walk->index_list);
                dir_walk->current_index = 0;

                storage_dir_close(dir_walk->file);

                size_t last_char = furi_string_search_rchar(dir_walk->path, '/');
                if(last_char != FURI_STRING_FAILURE) {
                    furi_string_left(dir_walk->path, last_char);
                }

                storage_dir_open(dir_walk->file, furi_string_get_cstr(dir_walk->path));

                // rewind
                while(true) {
                    if(index == dir_walk->current_index) {
                        result = DirWalkOK;
                        break;
                    }

                    if(!storage_dir_read(dir_walk->file, &info, name, 255)) {
                        result = DirWalkError;
                        end = true;
                        break;
                    }

                    dir_walk->current_index++;
                }
            }
        } else {
            result = DirWalkError;
            end = true;
        }
    }

    free(name);
    return result;
}

FS_Error dir_walk_get_error(DirWalk* dir_walk) {
    furi_check(dir_walk);
    return storage_file_get_error(dir_walk->file);
}

DirWalkResult dir_walk_read(DirWalk* dir_walk, FuriString* return_path, FileInfo* fileinfo) {
    furi_check(dir_walk);
    return dir_walk_iter(dir_walk, return_path, fileinfo);
}

void dir_walk_close(DirWalk* dir_walk) {
    furi_check(dir_walk);
    if(storage_file_is_open(dir_walk->file)) {
        storage_dir_close(dir_walk->file);
    }

    DirIndexList_reset(dir_walk->index_list);
    furi_string_reset(dir_walk->path);
    dir_walk->current_index = 0;
}
