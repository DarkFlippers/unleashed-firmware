#pragma once

#include <nfc/protocols/mf_classic/mf_classic.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Mfkey32Logger Mfkey32Logger;

Mfkey32Logger* mfkey32_logger_alloc(uint32_t cuid);

void mfkey32_logger_free(Mfkey32Logger* instance);

void mfkey32_logger_add_nonce(Mfkey32Logger* instance, MfClassicAuthContext* auth_context);

size_t mfkey32_logger_get_params_num(Mfkey32Logger* instance);

bool mfkey32_logger_save_params(Mfkey32Logger* instance, const char* path);

void mfkey32_logger_get_params_data(Mfkey32Logger* instance, FuriString* str);

#ifdef __cplusplus
}
#endif
