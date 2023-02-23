
#ifndef __ADI_H__
#define __ADI_H__

#include "swd_probe_app.h"

#define ARM_ID 0x23B

#define ARCH_ID(architect, archid) ((architect) << 21) | (archid)

#define BIT(nr) (1UL << (nr))
#define ARM_CS_C9_DEVARCH_PRESENT BIT(20)
#define ARM_CS_C9_DEVARCH_ARCHITECT_MASK (0xFFE00000)
#define ARM_CS_C9_DEVARCH_ARCHID_MASK (0x0000FFFF)
#define DEVARCH_ID_MASK (ARM_CS_C9_DEVARCH_ARCHITECT_MASK | ARM_CS_C9_DEVARCH_ARCHID_MASK)

typedef struct {
    uint16_t designer;
    uint16_t part;
    uint8_t revision;
    uint8_t cmod;
    uint8_t revand;
    uint8_t size;
} pidr_data_t;

typedef enum { CIDR_CLASS_ROMTABLE = 0x01, CIDR_CLASS_CORESIGHT = 0x09 } cidr_classes_t;

uint32_t adi_romtable_entry_count(AppFSM* const ctx, uint32_t base);
uint32_t adi_romtable_get(AppFSM* const ctx, uint32_t base, uint32_t pos);
bool adi_is_romtable(AppFSM* const ctx, uint32_t base);
const char* adi_romtable_type(AppFSM* const ctx, uint32_t base);
const char* adi_romtable_full(AppFSM* const ctx, uint32_t base);

#endif
