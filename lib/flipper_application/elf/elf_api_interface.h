#pragma once

#include <elf.h>
#include <stdbool.h>

/**
 * @brief Interface for ELF loader to resolve symbols
 */
typedef struct ElfApiInterface {
    uint16_t api_version_major;
    uint16_t api_version_minor;
    bool (*resolver_callback)(
        const struct ElfApiInterface* interface,
        uint32_t hash,
        Elf32_Addr* address);
} ElfApiInterface;
