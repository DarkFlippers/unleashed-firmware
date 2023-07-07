#include "types.h"

const char* nfc_magic_type(MagicType type) {
    if(type == MagicTypeClassicGen1) {
        return "Classic Gen 1A/B";
    } else if(type == MagicTypeClassicDirectWrite) {
        return "Classic DirectWrite";
    } else if(type == MagicTypeClassicAPDU) {
        return "Classic APDU";
    } else if(type == MagicTypeUltralightGen1) {
        return "Ultralight Gen 1";
    } else if(type == MagicTypeUltralightDirectWrite) {
        return "Ultralight DirectWrite";
    } else if(type == MagicTypeUltralightC_Gen1) {
        return "Ultralight-C Gen 1";
    } else if(type == MagicTypeUltralightC_DirectWrite) {
        return "Ultralight-C DirectWrite";
    } else if(type == MagicTypeGen4) {
        return "Gen 4 GTU";
    } else {
        return "Unknown";
    }
}
