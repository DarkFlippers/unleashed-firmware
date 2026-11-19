#include "ducky_layout.h"
#include <furi_hal.h>
#include <storage/storage.h>
#include <string.h>
#include <stdio.h>

#define TAG "DuckyLayout"

static bool ducky_layout_token_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
           c == '-' || c == '_';
}

static bool ducky_layout_path_valid(const char* path) {
    const size_t prefix = sizeof(DUCKY_LAYOUT_ROOT "/") - 1;
    if(!path || strncmp(path, DUCKY_LAYOUT_ROOT "/", prefix) != 0) return false;
    const char* name = path + prefix;
    const size_t len = strlen(name);
    if(len < 4 || len > 35 || strcmp(name + len - 3, ".kl") != 0) return false;
    for(size_t i = 0; i < len - 3; ++i) {
        if(!ducky_layout_token_char(name[i])) return false;
    }
    return true;
}

bool ducky_layout_load(uint16_t layout[DUCKY_LAYOUT_KEYS], const char* path) {
    furi_check(layout);
    // Reset first, never leave a previous language active on failure.
    memset(layout, 0, DUCKY_LAYOUT_KEYS * sizeof(uint16_t));
    memcpy(layout, hid_asciimap, MIN(sizeof(hid_asciimap), DUCKY_LAYOUT_KEYS * sizeof(uint16_t)));
    if(!path || !path[0]) return true; // explicit built-in en-US
    bool loaded = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    uint8_t bytes[DUCKY_LAYOUT_KEYS * sizeof(uint16_t)];
    if(ducky_layout_path_valid(path) &&
       storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING) &&
       storage_file_size(file) == sizeof(bytes) &&
       storage_file_read(file, bytes, sizeof(bytes)) == sizeof(bytes)) {
        for(size_t i = 0; i < DUCKY_LAYOUT_KEYS; ++i) {
            layout[i] = bytes[2 * i] | ((uint16_t)bytes[2 * i + 1] << 8);
        }
        loaded = true;
    }
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    if(!loaded) FURI_LOG_W(TAG, "Invalid/missing .kl, using built-in en-US");
    return loaded;
}

bool ducky_layout_language(uint16_t layout[DUCKY_LAYOUT_KEYS], const char* argument) {
    furi_check(layout && argument);
    char code[33];
    size_t len = 0;
    while(*argument == ' ' || *argument == '\t')
        ++argument;
    while(ducky_layout_token_char(*argument) && len < sizeof(code) - 1) {
        code[len++] = *argument++;
    }
    code[len] = '\0';
    while(*argument == ' ' || *argument == '\t' || *argument == '\r' || *argument == '\n') {
        ++argument;
    }
    if(!len || *argument) {
        ducky_layout_load(layout, NULL);
        FURI_LOG_W(TAG, "Invalid DUCKY_LANG argument, using built-in en-US");
        return false;
    }
    // Only aliases confirmed by files in this exact repository.
    const char* stem = code;
    if(strcmp(code, "en") == 0)
        stem = "en-US";
    else if(strcmp(code, "de") == 0)
        stem = "de-DE";
    else if(strcmp(code, "fr") == 0)
        stem = "fr-FR";
    char path[sizeof(DUCKY_LAYOUT_ROOT) + sizeof(code) + 4];
    snprintf(path, sizeof(path), DUCKY_LAYOUT_ROOT "/%s.kl", stem);
    return ducky_layout_load(layout, path);
}

bool ducky_layout_ascii_string(const char* text) {
    furi_check(text);
    for(; *text; ++text) {
        if((uint8_t)*text >= DUCKY_LAYOUT_KEYS) return false;
    }
    return true;
}
