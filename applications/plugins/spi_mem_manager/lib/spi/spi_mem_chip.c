#include "spi_mem_chip_i.h"

const SPIMemChipVendorName spi_mem_chip_vendor_names[] = {
    {"Adesto", SPIMemChipVendorADESTO},
    {"AMIC", SPIMemChipVendorAMIC},
    {"Boya", SPIMemChipVendorBoya},
    {"EON", SPIMemChipVendorEON},
    {"PFlash", SPIMemChipVendorPFLASH},
    {"Terra", SPIMemChipVendorTERRA},
    {"Generalplus", SPIMemChipVendorGeneralplus},
    {"Deutron", SPIMemChipVendorDEUTRON},
    {"EFST", SPIMemChipVendorEFST},
    {"Excel Semi.", SPIMemChipVendorEXCELSEMI},
    {"Fidelix", SPIMemChipVendorFIDELIX},
    {"GigaDevice", SPIMemChipVendorGIGADEVICE},
    {"ICE", SPIMemChipVendorICE},
    {"Intel", SPIMemChipVendorINTEL},
    {"KHIC", SPIMemChipVendorKHIC},
    {"Macronix", SPIMemChipVendorMACRONIX},
    {"Micron", SPIMemChipVendorMICRON},
    {"Mshine", SPIMemChipVendorMSHINE},
    {"Nantronics", SPIMemChipVendorNANTRONICS},
    {"Nexflash", SPIMemChipVendorNEXFLASH},
    {"Numonyx", SPIMemChipVendorNUMONYX},
    {"PCT", SPIMemChipVendorPCT},
    {"Spansion", SPIMemChipVendorSPANSION},
    {"SST", SPIMemChipVendorSST},
    {"ST", SPIMemChipVendorST},
    {"Winbond", SPIMemChipVendorWINBOND},
    {"Zempro", SPIMemChipVendorZEMPRO},
    {"Zbit", SPIMemChipVendorZbit},
    {"Berg Micro.", SPIMemChipVendorBerg_Micro},
    {"Atmel", SPIMemChipVendorATMEL},
    {"ACE", SPIMemChipVendorACE},
    {"ATO", SPIMemChipVendorATO},
    {"Douqi", SPIMemChipVendorDOUQI},
    {"Fremont", SPIMemChipVendorFremont},
    {"Fudan", SPIMemChipVendorFudan},
    {"Genitop", SPIMemChipVendorGenitop},
    {"Paragon", SPIMemChipVendorParagon},
    {"Unknown", SPIMemChipVendorUnknown}};

static const char* spi_mem_chip_search_vendor_name(SPIMemChipVendor vendor_enum) {
    const SPIMemChipVendorName* vendor = spi_mem_chip_vendor_names;
    while(vendor->vendor_enum != SPIMemChipVendorUnknown && vendor->vendor_enum != vendor_enum)
        vendor++;
    return vendor->vendor_name;
}

bool spi_mem_chip_find_all(SPIMemChip* chip_info, found_chips_t found_chips) {
    const SPIMemChip* chip_info_arr;
    found_chips_reset(found_chips);
    for(chip_info_arr = SPIMemChips; chip_info_arr->model_name != NULL; chip_info_arr++) {
        if(chip_info->vendor_id != chip_info_arr->vendor_id) continue;
        if(chip_info->type_id != chip_info_arr->type_id) continue;
        if(chip_info->capacity_id != chip_info_arr->capacity_id) continue;
        found_chips_push_back(found_chips, chip_info_arr);
    }
    if(found_chips_size(found_chips)) return true;
    return false;
}

void spi_mem_chip_copy_chip_info(SPIMemChip* dest, const SPIMemChip* src) {
    memcpy(dest, src, sizeof(SPIMemChip));
}

size_t spi_mem_chip_get_size(SPIMemChip* chip) {
    return (chip->size);
}

const char* spi_mem_chip_get_vendor_name(const SPIMemChip* chip) {
    return (spi_mem_chip_search_vendor_name(chip->vendor_enum));
}

const char* spi_mem_chip_get_vendor_name_by_enum(uint32_t vendor_enum) {
    return (spi_mem_chip_search_vendor_name(vendor_enum));
}

const char* spi_mem_chip_get_model_name(const SPIMemChip* chip) {
    return (chip->model_name);
}

uint8_t spi_mem_chip_get_vendor_id(SPIMemChip* chip) {
    return (chip->vendor_id);
}

uint8_t spi_mem_chip_get_type_id(SPIMemChip* chip) {
    return (chip->type_id);
}

uint8_t spi_mem_chip_get_capacity_id(SPIMemChip* chip) {
    return (chip->capacity_id);
}

SPIMemChipWriteMode spi_mem_chip_get_write_mode(SPIMemChip* chip) {
    return (chip->write_mode);
}

size_t spi_mem_chip_get_page_size(SPIMemChip* chip) {
    return (chip->page_size);
}

uint32_t spi_mem_chip_get_vendor_enum(const SPIMemChip* chip) {
    return ((uint32_t)chip->vendor_enum);
}
