#include "slix_unlock.h"

#include <furi/furi.h>

#define SLIX_UNLOCK_PASSWORD_NUM_MAX (2)

struct SlixUnlock {
    SlixUnlockMethod method;
    SlixPassword password_arr[SLIX_UNLOCK_PASSWORD_NUM_MAX];
    size_t password_arr_len;
    size_t password_idx;
};

static const SlixPassword tonie_box_pass_arr[] = {0x5B6EFD7F, 0x0F0F0F0F};

SlixUnlock* slix_unlock_alloc(void) {
    SlixUnlock* instance = malloc(sizeof(SlixUnlock));

    return instance;
}

void slix_unlock_free(SlixUnlock* instance) {
    furi_assert(instance);

    free(instance);
}

void slix_unlock_reset(SlixUnlock* instance) {
    furi_assert(instance);

    memset(instance, 0, sizeof(SlixUnlock));
}

void slix_unlock_set_method(SlixUnlock* instance, SlixUnlockMethod method) {
    furi_assert(instance);

    instance->method = method;
    if(method == SlixUnlockMethodTonieBox) {
        instance->password_arr_len = COUNT_OF(tonie_box_pass_arr);
        memcpy(instance->password_arr, tonie_box_pass_arr, sizeof(tonie_box_pass_arr));
    }
}

void slix_unlock_set_password(SlixUnlock* instance, SlixPassword password) {
    furi_assert(instance);
    furi_assert(instance->method == SlixUnlockMethodManual);

    instance->password_arr[0] = password;
    instance->password_arr_len = 1;
}

bool slix_unlock_get_next_password(SlixUnlock* instance, SlixPassword* password) {
    furi_assert(instance);
    furi_assert(password);

    bool password_set = false;
    if(instance->password_arr_len) {
        *password = instance->password_arr[instance->password_idx++];
        instance->password_idx %= instance->password_arr_len;
        password_set = true;
    }

    return password_set;
}
