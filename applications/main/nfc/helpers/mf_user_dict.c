#include "mf_user_dict.h"

#include <toolbox/keys_dict.h>
#include <nfc/protocols/mf_classic/mf_classic.h>
#include <furi/furi.h>

#define NFC_APP_FOLDER                    EXT_PATH("nfc")
#define NFC_APP_MF_CLASSIC_DICT_USER_PATH (NFC_APP_FOLDER "/assets/mf_classic_dict_user.nfc")

struct MfUserDict {
    size_t keys_num;
    MfClassicKey* keys_arr;
};

MfUserDict* mf_user_dict_alloc(size_t max_keys_to_load) {
    MfUserDict* instance = malloc(sizeof(MfUserDict));

    KeysDict* dict = keys_dict_alloc(
        NFC_APP_MF_CLASSIC_DICT_USER_PATH, KeysDictModeOpenAlways, sizeof(MfClassicKey));

    size_t dict_keys_num = keys_dict_get_total_keys(dict);
    instance->keys_num = MIN(max_keys_to_load, dict_keys_num);

    if(instance->keys_num > 0) {
        instance->keys_arr = malloc(instance->keys_num * sizeof(MfClassicKey));
        for(size_t i = 0; i < instance->keys_num; i++) {
            bool key_loaded =
                keys_dict_get_next_key(dict, instance->keys_arr[i].data, sizeof(MfClassicKey));
            furi_assert(key_loaded);
        }
    }
    keys_dict_free(dict);

    return instance;
}

void mf_user_dict_free(MfUserDict* instance) {
    furi_assert(instance);

    if(instance->keys_num > 0) {
        free(instance->keys_arr);
    }
    free(instance);
}

size_t mf_user_dict_get_keys_cnt(MfUserDict* instance) {
    furi_assert(instance);

    return instance->keys_num;
}

void mf_user_dict_get_key_str(MfUserDict* instance, uint32_t index, FuriString* str) {
    furi_assert(instance);
    furi_assert(str);
    furi_assert(index < instance->keys_num);
    furi_assert(instance->keys_arr);

    furi_string_reset(str);
    for(size_t i = 0; i < sizeof(MfClassicKey); i++) {
        furi_string_cat_printf(str, "%02X", instance->keys_arr[index].data[i]);
    }
}

bool mf_user_dict_delete_key(MfUserDict* instance, uint32_t index) {
    furi_assert(instance);
    furi_assert(index < instance->keys_num);
    furi_assert(instance->keys_arr);

    KeysDict* dict = keys_dict_alloc(
        NFC_APP_MF_CLASSIC_DICT_USER_PATH, KeysDictModeOpenAlways, sizeof(MfClassicKey));

    bool key_delete_success =
        keys_dict_delete_key(dict, instance->keys_arr[index].data, sizeof(MfClassicKey));
    keys_dict_free(dict);

    if(key_delete_success) {
        instance->keys_num--;
    }

    return key_delete_success;
}
