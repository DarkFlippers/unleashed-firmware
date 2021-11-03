#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

bool saved_struct_load(const char* path,
                       void* data,
                       size_t size,
                       uint8_t magic,
                       uint8_t version);

bool saved_struct_save(const char* path,
                       void* data,
                       size_t size,
                       uint8_t magic,
                       uint8_t version);

