#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MagicTypeClassicGen1,
    MagicTypeClassicDirectWrite,
    MagicTypeClassicAPDU,
    MagicTypeUltralightGen1,
    MagicTypeUltralightDirectWrite,
    MagicTypeUltralightC_Gen1,
    MagicTypeUltralightC_DirectWrite,
    MagicTypeGen4,
} MagicType;

bool magic_activate();

void magic_deactivate();