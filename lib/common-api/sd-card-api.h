#pragma once
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SdApp SdApp;

typedef struct {
    SdApp* context;
    bool (*file_select)(
        SdApp* context,
        const char* path,
        const char* extension,
        char* result,
        uint8_t result_size,
        char* selected_filename);
    void (*check_error)(SdApp* context);
} SdCard_Api;

#ifdef __cplusplus
}
#endif