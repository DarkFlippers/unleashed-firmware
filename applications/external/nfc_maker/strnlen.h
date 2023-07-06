#pragma once
#pragma weak strnlen

#include <stddef.h>

size_t strnlen(const char* s, size_t maxlen);