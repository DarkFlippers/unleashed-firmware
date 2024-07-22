#pragma once
#include <m-core.h>

#define M_INIT_DUP(a)        ((a) = strdup(""))
#define M_INIT_SET_DUP(a, b) ((a) = strdup(b))
#define M_SET_DUP(a, b)      (free((void*)a), (a) = strdup(b))
#define M_CLEAR_DUP(a)       (free((void*)a))

#define M_CSTR_DUP_OPLIST      \
    (INIT(M_INIT_DUP),         \
     INIT_SET(M_INIT_SET_DUP), \
     SET(M_SET_DUP),           \
     CLEAR(M_CLEAR_DUP),       \
     HASH(m_core_cstr_hash),   \
     EQUAL(M_CSTR_EQUAL),      \
     CMP(strcmp),              \
     TYPE(const char*))
