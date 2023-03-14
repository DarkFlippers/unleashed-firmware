#pragma once

#include <furi.h>
#include "spi_mem_chip.h"

typedef enum {
    SPIMemChipVendorUnknown,
    SPIMemChipVendorADESTO,
    SPIMemChipVendorAMIC,
    SPIMemChipVendorBoya,
    SPIMemChipVendorEON,
    SPIMemChipVendorPFLASH,
    SPIMemChipVendorTERRA,
    SPIMemChipVendorGeneralplus,
    SPIMemChipVendorDEUTRON,
    SPIMemChipVendorEFST,
    SPIMemChipVendorEXCELSEMI,
    SPIMemChipVendorFIDELIX,
    SPIMemChipVendorGIGADEVICE,
    SPIMemChipVendorICE,
    SPIMemChipVendorINTEL,
    SPIMemChipVendorKHIC,
    SPIMemChipVendorMACRONIX,
    SPIMemChipVendorMICRON,
    SPIMemChipVendorMSHINE,
    SPIMemChipVendorNANTRONICS,
    SPIMemChipVendorNEXFLASH,
    SPIMemChipVendorNUMONYX,
    SPIMemChipVendorPCT,
    SPIMemChipVendorSPANSION,
    SPIMemChipVendorSST,
    SPIMemChipVendorST,
    SPIMemChipVendorWINBOND,
    SPIMemChipVendorZEMPRO,
    SPIMemChipVendorZbit,
    SPIMemChipVendorBerg_Micro,
    SPIMemChipVendorATMEL,
    SPIMemChipVendorACE,
    SPIMemChipVendorATO,
    SPIMemChipVendorDOUQI,
    SPIMemChipVendorFremont,
    SPIMemChipVendorFudan,
    SPIMemChipVendorGenitop,
    SPIMemChipVendorParagon
} SPIMemChipVendor;

typedef enum {
    SPIMemChipCMDReadJEDECChipID = 0x9F,
    SPIMemChipCMDReadData = 0x03,
    SPIMemChipCMDChipErase = 0xC7,
    SPIMemChipCMDWriteEnable = 0x06,
    SPIMemChipCMDWriteDisable = 0x04,
    SPIMemChipCMDReadStatus = 0x05,
    SPIMemChipCMDWriteData = 0x02,
    SPIMemChipCMDReleasePowerDown = 0xAB
} SPIMemChipCMD;

enum SPIMemChipStatusBit {
    SPIMemChipStatusBitBusy = (0x01 << 0),
    SPIMemChipStatusBitWriteEnabled = (0x01 << 1),
    SPIMemChipStatusBitBitProtection1 = (0x01 << 2),
    SPIMemChipStatusBitBitProtection2 = (0x01 << 3),
    SPIMemChipStatusBitBitProtection3 = (0x01 << 4),
    SPIMemChipStatusBitTopBottomProtection = (0x01 << 5),
    SPIMemChipStatusBitSectorProtect = (0x01 << 6),
    SPIMemChipStatusBitRegisterProtect = (0x01 << 7)
};

typedef struct {
    const char* vendor_name;
    SPIMemChipVendor vendor_enum;
} SPIMemChipVendorName;

struct SPIMemChip {
    uint8_t vendor_id;
    uint8_t type_id;
    uint8_t capacity_id;
    const char* model_name;
    size_t size;
    size_t page_size;
    SPIMemChipVendor vendor_enum;
    SPIMemChipWriteMode write_mode;
};

extern const SPIMemChip SPIMemChips[];
