#include "../test.h" // IWYU pragma: keep
#include <furi.h>
#include <m-dict.h>
#include <toolbox/dir_walk.h>

static const char* const storage_test_dirwalk_paths[] = {
    "1",
    "11",
    "111",
    "1/2",
    "1/22",
    "1/222",
    "11/2",
    "111/2",
    "111/22",
    "111/22/33",
};

static const char* const storage_test_dirwalk_files[] = {
    "file1.test",
    "file2.test",
    "file3.ext_test",
    "1/file1.test",
    "111/22/33/file1.test",
    "111/22/33/file2.test",
    "111/22/33/file3.ext_test",
    "111/22/33/file4.ext_test",
};

typedef struct {
    const char* const path;
    bool is_dir;
} StorageTestPathDesc;

const StorageTestPathDesc storage_test_dirwalk_full[] = {
    {.path = "1", .is_dir = true},
    {.path = "11", .is_dir = true},
    {.path = "111", .is_dir = true},
    {.path = "1/2", .is_dir = true},
    {.path = "1/22", .is_dir = true},
    {.path = "1/222", .is_dir = true},
    {.path = "11/2", .is_dir = true},
    {.path = "111/2", .is_dir = true},
    {.path = "111/22", .is_dir = true},
    {.path = "111/22/33", .is_dir = true},
    {.path = "file1.test", .is_dir = false},
    {.path = "file2.test", .is_dir = false},
    {.path = "file3.ext_test", .is_dir = false},
    {.path = "1/file1.test", .is_dir = false},
    {.path = "111/22/33/file1.test", .is_dir = false},
    {.path = "111/22/33/file2.test", .is_dir = false},
    {.path = "111/22/33/file3.ext_test", .is_dir = false},
    {.path = "111/22/33/file4.ext_test", .is_dir = false},
};

const StorageTestPathDesc storage_test_dirwalk_no_recursive[] = {
    {.path = "1", .is_dir = true},
    {.path = "11", .is_dir = true},
    {.path = "111", .is_dir = true},
    {.path = "file1.test", .is_dir = false},
    {.path = "file2.test", .is_dir = false},
    {.path = "file3.ext_test", .is_dir = false},
};

const StorageTestPathDesc storage_test_dirwalk_filter[] = {
    {.path = "file1.test", .is_dir = false},
    {.path = "file2.test", .is_dir = false},
    {.path = "1/file1.test", .is_dir = false},
    {.path = "111/22/33/file1.test", .is_dir = false},
    {.path = "111/22/33/file2.test", .is_dir = false},
};

typedef struct {
    bool is_dir;
    bool visited;
} StorageTestPath;

DICT_DEF2(StorageTestPathDict, FuriString*, FURI_STRING_OPLIST, StorageTestPath, M_POD_OPLIST)

static StorageTestPathDict_t*
    storage_test_paths_alloc(const StorageTestPathDesc paths[], size_t paths_count) {
    StorageTestPathDict_t* data = malloc(sizeof(StorageTestPathDict_t));
    StorageTestPathDict_init(*data);

    for(size_t i = 0; i < paths_count; i++) {
        FuriString* key;
        key = furi_string_alloc_set(paths[i].path);
        StorageTestPath value = {
            .is_dir = paths[i].is_dir,
            .visited = false,
        };

        StorageTestPathDict_set_at(*data, key, value);
        furi_string_free(key);
    }

    return data;
}

static void storage_test_paths_free(StorageTestPathDict_t* data) {
    StorageTestPathDict_clear(*data);
    free(data);
}

static bool storage_test_paths_mark(StorageTestPathDict_t* data, FuriString* path, bool is_dir) {
    bool found = false;

    StorageTestPath* record = StorageTestPathDict_get(*data, path);
    if(record) {
        if(is_dir == record->is_dir) {
            if(record->visited == false) {
                record->visited = true;
                found = true;
            }
        }
    }

    return found;
}

static bool storage_test_paths_check(StorageTestPathDict_t* data) {
    bool error = false;

    StorageTestPathDict_it_t it;
    for(StorageTestPathDict_it(it, *data); !StorageTestPathDict_end_p(it);
        StorageTestPathDict_next(it)) {
        const StorageTestPathDict_itref_t* itref = StorageTestPathDict_cref(it);

        if(itref->value.visited == false) {
            error = true;
            break;
        }
    }

    return error;
}

static bool write_file_13DA(Storage* storage, const char* path) {
    File* file = storage_file_alloc(storage);
    bool result = false;
    if(storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        result = (storage_file_write(file, "13DA", 4) == 4);
    }
    storage_file_close(file);
    storage_file_free(file);

    return result;
}

static void storage_dirs_create(Storage* storage, const char* base) {
    FuriString* path;
    path = furi_string_alloc();

    storage_common_mkdir(storage, base);

    for(size_t i = 0; i < COUNT_OF(storage_test_dirwalk_paths); i++) {
        furi_string_printf(path, "%s/%s", base, storage_test_dirwalk_paths[i]);
        storage_common_mkdir(storage, furi_string_get_cstr(path));
    }

    for(size_t i = 0; i < COUNT_OF(storage_test_dirwalk_files); i++) {
        furi_string_printf(path, "%s/%s", base, storage_test_dirwalk_files[i]);
        write_file_13DA(storage, furi_string_get_cstr(path));
    }

    furi_string_free(path);
}

MU_TEST_1(test_dirwalk_full, Storage* storage) {
    FuriString* path;
    path = furi_string_alloc();
    FileInfo fileinfo;

    StorageTestPathDict_t* paths =
        storage_test_paths_alloc(storage_test_dirwalk_full, COUNT_OF(storage_test_dirwalk_full));

    DirWalk* dir_walk = dir_walk_alloc(storage);
    mu_check(dir_walk_open(dir_walk, EXT_PATH("dirwalk")));

    while(dir_walk_read(dir_walk, path, &fileinfo) == DirWalkOK) {
        furi_string_right(path, strlen(EXT_PATH("dirwalk/")));
        mu_check(storage_test_paths_mark(paths, path, file_info_is_dir(&fileinfo)));
    }

    dir_walk_free(dir_walk);
    furi_string_free(path);

    mu_check(storage_test_paths_check(paths) == false);

    storage_test_paths_free(paths);
}

MU_TEST_1(test_dirwalk_no_recursive, Storage* storage) {
    FuriString* path;
    path = furi_string_alloc();
    FileInfo fileinfo;

    StorageTestPathDict_t* paths = storage_test_paths_alloc(
        storage_test_dirwalk_no_recursive, COUNT_OF(storage_test_dirwalk_no_recursive));

    DirWalk* dir_walk = dir_walk_alloc(storage);
    dir_walk_set_recursive(dir_walk, false);
    mu_check(dir_walk_open(dir_walk, EXT_PATH("dirwalk")));

    while(dir_walk_read(dir_walk, path, &fileinfo) == DirWalkOK) {
        furi_string_right(path, strlen(EXT_PATH("dirwalk/")));
        mu_check(storage_test_paths_mark(paths, path, file_info_is_dir(&fileinfo)));
    }

    dir_walk_free(dir_walk);
    furi_string_free(path);

    mu_check(storage_test_paths_check(paths) == false);

    storage_test_paths_free(paths);
}

static bool test_dirwalk_filter_no_folder_ext(const char* name, FileInfo* fileinfo, void* ctx) {
    UNUSED(ctx);

    // only files
    if(!file_info_is_dir(fileinfo)) {
        // with ".test" in name
        if(strstr(name, ".test") != NULL) {
            return true;
        }
    }

    return false;
}

MU_TEST_1(test_dirwalk_filter, Storage* storage) {
    FuriString* path;
    path = furi_string_alloc();
    FileInfo fileinfo;

    StorageTestPathDict_t* paths = storage_test_paths_alloc(
        storage_test_dirwalk_filter, COUNT_OF(storage_test_dirwalk_filter));

    DirWalk* dir_walk = dir_walk_alloc(storage);
    dir_walk_set_filter_cb(dir_walk, test_dirwalk_filter_no_folder_ext, NULL);
    mu_check(dir_walk_open(dir_walk, EXT_PATH("dirwalk")));

    while(dir_walk_read(dir_walk, path, &fileinfo) == DirWalkOK) {
        furi_string_right(path, strlen(EXT_PATH("dirwalk/")));
        mu_check(storage_test_paths_mark(paths, path, file_info_is_dir(&fileinfo)));
    }

    dir_walk_free(dir_walk);
    furi_string_free(path);

    mu_check(storage_test_paths_check(paths) == false);

    storage_test_paths_free(paths);
}

MU_TEST_SUITE(test_dirwalk_suite) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_dirs_create(storage, EXT_PATH("dirwalk"));

    MU_RUN_TEST_1(test_dirwalk_full, storage);
    MU_RUN_TEST_1(test_dirwalk_no_recursive, storage);
    MU_RUN_TEST_1(test_dirwalk_filter, storage);

    storage_simply_remove_recursive(storage, EXT_PATH("dirwalk"));
    furi_record_close(RECORD_STORAGE);
}

int run_minunit_test_dirwalk(void) {
    MU_RUN_SUITE(test_dirwalk_suite);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_dirwalk)
