#pragma once

#include <nfc/protocols/slix/slix.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SlixUnlockMethodManual,
    SlixUnlockMethodTonieBox,
} SlixUnlockMethod;

typedef struct SlixUnlock SlixUnlock;

SlixUnlock* slix_unlock_alloc(void);

void slix_unlock_free(SlixUnlock* instance);

void slix_unlock_reset(SlixUnlock* instance);

void slix_unlock_set_method(SlixUnlock* instance, SlixUnlockMethod method);

void slix_unlock_set_password(SlixUnlock* instance, SlixPassword password);

bool slix_unlock_get_next_password(SlixUnlock* instance, SlixPassword* password);

#ifdef __cplusplus
}
#endif
