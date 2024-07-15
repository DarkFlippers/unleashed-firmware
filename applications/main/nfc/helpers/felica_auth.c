#include "felica_auth.h"

FelicaAuthenticationContext* felica_auth_alloc() {
    FelicaAuthenticationContext* instance = malloc(sizeof(FelicaAuthenticationContext));
    memset(instance->card_key.data, 0, FELICA_DATA_BLOCK_SIZE);
    instance->skip_auth = true;
    return instance;
}

void felica_auth_free(FelicaAuthenticationContext* instance) {
    furi_assert(instance);
    free(instance);
}

void felica_auth_reset(FelicaAuthenticationContext* instance) {
    furi_assert(instance);
    memset(instance->card_key.data, 0, FELICA_DATA_BLOCK_SIZE);
    instance->skip_auth = true;
    instance->auth_status.external = 0;
    instance->auth_status.internal = 0;
}
