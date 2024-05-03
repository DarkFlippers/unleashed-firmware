#pragma once

#include <lib/nfc/protocols/felica/felica.h>

#ifdef __cplusplus
extern "C" {
#endif

FelicaAuthenticationContext* felica_auth_alloc();

void felica_auth_free(FelicaAuthenticationContext* instance);

void felica_auth_reset(FelicaAuthenticationContext* instance);

#ifdef __cplusplus
}
#endif
