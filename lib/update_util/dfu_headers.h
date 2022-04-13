#pragma once

#include <stdint.h>

#pragma pack(push, 1)

typedef struct {
    char szSignature[5];
    uint8_t bVersion;
    uint32_t DFUImageSize;
    uint8_t bTargets;
} DfuPrefix;

typedef struct {
    uint16_t bcdDevice;
    uint16_t idProduct;
    uint16_t idVendor;
    uint16_t bcdDFU;
    uint8_t ucDfuSignature_U;
    uint8_t ucDfuSignature_F;
    uint8_t ucDfuSignature_D;
    uint8_t bLength;
    uint32_t dwCRC;
} DfuSuffix;

typedef struct {
    char szSignature[6];
    uint8_t bAlternateSetting;
    uint8_t bTargetNamed;
    uint8_t _pad[3];
    char szTargetName[255];
    uint32_t dwTargetSize;
    uint32_t dwNbElements;
} TargetPrefix;

typedef struct {
    uint32_t dwElementAddress;
    uint32_t dwElementSize;
} ImageElementHeader;

#pragma pack(pop)
