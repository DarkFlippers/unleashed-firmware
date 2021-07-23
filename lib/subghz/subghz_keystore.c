#include "subghz_keystore.h"

#include <furi.h>
#include <storage/storage.h>

#define FILE_BUFFER_SIZE 64

struct SubGhzKeystore {
    SubGhzKeyArray_t data;
};

SubGhzKeystore* subghz_keystore_alloc() {
    SubGhzKeystore* instance = furi_alloc(sizeof(SubGhzKeystore));

    SubGhzKeyArray_init(instance->data);

    return instance;
}

void subghz_keystore_free(SubGhzKeystore* instance) {
    furi_assert(instance);

    for
    M_EACH(manufacture_code, instance->data, SubGhzKeyArray_t) {
        string_clear(manufacture_code->name);
        manufacture_code->key = 0;
    }
    SubGhzKeyArray_clear(instance->data);

    free(instance);
}

static void subghz_keystore_add_key(SubGhzKeystore* instance, const char* name, uint64_t key, uint16_t type) {
    SubGhzKey* manufacture_code = SubGhzKeyArray_push_raw(instance->data);
    string_init_set_str(manufacture_code->name, name);
    manufacture_code->key = key;
    manufacture_code->type = type;
}

static void subghz_keystore_process_line(SubGhzKeystore* instance, string_t line) {
    uint64_t key = 0;
    uint16_t type = 0;
    char skey[17] = {0};
    char name[65] = {0};
    int ret = sscanf(string_get_cstr(line), "%16s:%hu:%64s", skey, &type, name);
    key = strtoull(skey, NULL, 16);
    if (ret == 3) {
        subghz_keystore_add_key(instance, name, key, type);
    } else {
        printf("Failed to load line: %s\r\n", string_get_cstr(line));
    }
}

void subghz_keystore_load(SubGhzKeystore* instance, const char* file_name) {
    File* manufacture_keys_file = storage_file_alloc(furi_record_open("storage"));
    string_t line;
    string_init(line);
    if(storage_file_open(manufacture_keys_file, file_name, FSAM_READ, FSOM_OPEN_EXISTING)) {
        printf("Loading manufacture keys file %s\r\n", file_name);
        char buffer[FILE_BUFFER_SIZE];
        uint16_t ret;
        do {
            ret = storage_file_read(manufacture_keys_file, buffer, FILE_BUFFER_SIZE);
            for (uint16_t i=0; i < ret; i++) {
                if (buffer[i] == '\n' && string_size(line) > 0) {
                    subghz_keystore_process_line(instance, line);
                    string_clean(line);
                } else {
                    string_push_back(line, buffer[i]);
                }
            }
        } while(ret > 0);
    } else {
        printf("Manufacture keys file is not found: %s\r\n", file_name);
    }
    string_clear(line);
    storage_file_close(manufacture_keys_file);
    storage_file_free(manufacture_keys_file);
    furi_record_close("storage");
}

SubGhzKeyArray_t* subghz_keystore_get_data(SubGhzKeystore* instance) {
    furi_assert(instance);
    return &instance->data;
}
