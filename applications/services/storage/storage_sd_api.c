#include "storage_sd_api.h"

const char* sd_api_get_fs_type_text(SDFsType fs_type) {
    switch(fs_type) {
    case(FST_FAT12):
        return "FAT12";
        break;
    case(FST_FAT16):
        return "FAT16";
        break;
    case(FST_FAT32):
        return "FAT32";
        break;
    case(FST_EXFAT):
        return "EXFAT";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}
