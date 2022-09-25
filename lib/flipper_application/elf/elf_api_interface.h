#pragma once

#include <elf.h>
#include <stdbool.h>

#define ELF_INVALID_ADDRESS 0xFFFFFFFF

typedef struct {
    uint16_t api_version_major;
    uint16_t api_version_minor;
    bool (*resolver_callback)(const char* name, Elf32_Addr* address);
} ElfApiInterface;
