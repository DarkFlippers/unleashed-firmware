
#include <furi.h>
#include <stdlib.h>

#include "adi.h"
#include "swd_probe_app.h"

/* https://github.com/openocd-org/openocd/blob/master/src/target/arm_adi_v5.c */

/*
static const char* class_description[16] = {
    [0x0] = "Generic verification component",
    [0x1] = "(ROM Table)",
    [0x2] = "Reserved",
    [0x3] = "Reserved",
    [0x4] = "Reserved",
    [0x5] = "Reserved",
    [0x6] = "Reserved",
    [0x7] = "Reserved",
    [0x8] = "Reserved",
    [0x9] = "CoreSight component",
    [0xA] = "Reserved",
    [0xB] = "Peripheral Test Block",
    [0xC] = "Reserved",
    [0xD] = "OptimoDE DESS",
    [0xE] = "Generic IP component",
    [0xF] = "CoreLink, PrimeCell or System component",
};
*/

static const struct {
    uint32_t arch_id;
    const char* description;
} class0x9_devarch[] = {
    /* keep same unsorted order as in ARM IHI0029E */
    {ARCH_ID(ARM_ID, 0x0A00), "RAS architecture"},
    {ARCH_ID(ARM_ID, 0x1A01), "Instrumentation Trace Macrocell (ITM) architecture"},
    {ARCH_ID(ARM_ID, 0x1A02), "DWT architecture"},
    {ARCH_ID(ARM_ID, 0x1A03), "Flash Patch and Breakpoint unit (FPB) architecture"},
    {ARCH_ID(ARM_ID, 0x2A04), "Processor debug architecture (ARMv8-M)"},
    {ARCH_ID(ARM_ID, 0x6A05), "Processor debug architecture (ARMv8-R)"},
    {ARCH_ID(ARM_ID, 0x0A10), "PC sample-based profiling"},
    {ARCH_ID(ARM_ID, 0x4A13), "Embedded Trace Macrocell (ETM) architecture"},
    {ARCH_ID(ARM_ID, 0x1A14), "Cross Trigger Interface (CTI) architecture"},
    {ARCH_ID(ARM_ID, 0x6A15), "Processor debug architecture (v8.0-A)"},
    {ARCH_ID(ARM_ID, 0x7A15), "Processor debug architecture (v8.1-A)"},
    {ARCH_ID(ARM_ID, 0x8A15), "Processor debug architecture (v8.2-A)"},
    {ARCH_ID(ARM_ID, 0x2A16), "Processor Performance Monitor (PMU) architecture"},
    {ARCH_ID(ARM_ID, 0x0A17), "Memory Access Port v2 architecture"},
    {ARCH_ID(ARM_ID, 0x0A27), "JTAG Access Port v2 architecture"},
    {ARCH_ID(ARM_ID, 0x0A31), "Basic trace router"},
    {ARCH_ID(ARM_ID, 0x0A37), "Power requestor"},
    {ARCH_ID(ARM_ID, 0x0A47), "Unknown Access Port v2 architecture"},
    {ARCH_ID(ARM_ID, 0x0A50), "HSSTP architecture"},
    {ARCH_ID(ARM_ID, 0x0A63), "System Trace Macrocell (STM) architecture"},
    {ARCH_ID(ARM_ID, 0x0A75), "CoreSight ELA architecture"},
    {ARCH_ID(ARM_ID, 0x0AF7), "CoreSight ROM architecture"},
};

/* Part number interpretations are from Cortex
 * core specs, the CoreSight components TRM
 * (ARM DDI 0314H), CoreSight System Design
 * Guide (ARM DGI 0012D) and ETM specs; also
 * from chip observation (e.g. TI SDTI).
 */

static const struct dap_part_nums {
    uint16_t designer_id;
    uint16_t part_num;
    const char* type;
    const char* full;
} dap_part_nums[] = {
    {
        ARM_ID,
        0x000,
        "Cortex-M3 SCS",
        "(System Control Space)",
    },
    {
        ARM_ID,
        0x001,
        "Cortex-M3 ITM",
        "(Instrumentation Trace Module)",
    },
    {
        ARM_ID,
        0x002,
        "Cortex-M3 DWT",
        "(Data Watchpoint and Trace)",
    },
    {
        ARM_ID,
        0x003,
        "Cortex-M3 FPB",
        "(Flash Patch and Breakpoint)",
    },
    {
        ARM_ID,
        0x008,
        "Cortex-M0 SCS",
        "(System Control Space)",
    },
    {
        ARM_ID,
        0x00a,
        "Cortex-M0 DWT",
        "(Data Watchpoint and Trace)",
    },
    {
        ARM_ID,
        0x00b,
        "Cortex-M0 BPU",
        "(Breakpoint Unit)",
    },
    {
        ARM_ID,
        0x00c,
        "Cortex-M4 SCS",
        "(System Control Space)",
    },
    {
        ARM_ID,
        0x00d,
        "CoreSight ETM11",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x00e,
        "Cortex-M7 FPB",
        "(Flash Patch and Breakpoint)",
    },
    {
        ARM_ID,
        0x193,
        "SoC-600 TSGEN",
        "(Timestamp Generator)",
    },
    {
        ARM_ID,
        0x470,
        "Cortex-M1 ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x471,
        "Cortex-M0 ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x490,
        "Cortex-A15 GIC",
        "(Generic Interrupt Controller)",
    },
    {
        ARM_ID,
        0x492,
        "Cortex-R52 GICD",
        "(Distributor)",
    },
    {
        ARM_ID,
        0x493,
        "Cortex-R52 GICR",
        "(Redistributor)",
    },
    {
        ARM_ID,
        0x4a1,
        "Cortex-A53 ROM",
        "(v8 Memory Map ROM Table)",
    },
    {
        ARM_ID,
        0x4a2,
        "Cortex-A57 ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x4a3,
        "Cortex-A53 ROM",
        "(v7 Memory Map ROM Table)",
    },
    {
        ARM_ID,
        0x4a4,
        "Cortex-A72 ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x4a9,
        "Cortex-A9 ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x4aa,
        "Cortex-A35 ROM",
        "(v8 Memory Map ROM Table)",
    },
    {
        ARM_ID,
        0x4af,
        "Cortex-A15 ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x4b5,
        "Cortex-R5 ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x4b8,
        "Cortex-R52 ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x4c0,
        "Cortex-M0+ ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x4c3,
        "Cortex-M3 ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x4c4,
        "Cortex-M4 ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x4c7,
        "Cortex-M7 PPB ROM",
        "(Private Peripheral Bus ROM Table)",
    },
    {
        ARM_ID,
        0x4c8,
        "Cortex-M7 ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x4e0,
        "Cortex-A35 ROM",
        "(v7 Memory Map ROM Table)",
    },
    {
        ARM_ID,
        0x4e4,
        "Cortex-A76 ROM",
        "(ROM Table)",
    },
    {
        ARM_ID,
        0x906,
        "CoreSight CTI",
        "(Cross Trigger)",
    },
    {
        ARM_ID,
        0x907,
        "CoreSight ETB",
        "(Trace Buffer)",
    },
    {
        ARM_ID,
        0x908,
        "CoreSight CSTF",
        "(Trace Funnel)",
    },
    {
        ARM_ID,
        0x909,
        "CoreSight ATBR",
        "(Advanced Trace Bus Replicator)",
    },
    {
        ARM_ID,
        0x910,
        "CoreSight ETM9",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x912,
        "CoreSight TPIU",
        "(Trace Port Interface Unit)",
    },
    {
        ARM_ID,
        0x913,
        "CoreSight ITM",
        "(Instrumentation Trace Macrocell)",
    },
    {
        ARM_ID,
        0x914,
        "CoreSight SWO",
        "(Single Wire Output)",
    },
    {
        ARM_ID,
        0x917,
        "CoreSight HTM",
        "(AHB Trace Macrocell)",
    },
    {
        ARM_ID,
        0x920,
        "CoreSight ETM11",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x921,
        "Cortex-A8 ETM",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x922,
        "Cortex-A8 CTI",
        "(Cross Trigger)",
    },
    {
        ARM_ID,
        0x923,
        "Cortex-M3 TPIU",
        "(Trace Port Interface Unit)",
    },
    {
        ARM_ID,
        0x924,
        "Cortex-M3 ETM",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x925,
        "Cortex-M4 ETM",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x930,
        "Cortex-R4 ETM",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x931,
        "Cortex-R5 ETM",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x932,
        "CoreSight MTB-M0+",
        "(Micro Trace Buffer)",
    },
    {
        ARM_ID,
        0x941,
        "CoreSight TPIU-Lite",
        "(Trace Port Interface Unit)",
    },
    {
        ARM_ID,
        0x950,
        "Cortex-A9 PTM",
        "(Program Trace Macrocell)",
    },
    {
        ARM_ID,
        0x955,
        "Cortex-A5 ETM",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x95a,
        "Cortex-A72 ETM",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x95b,
        "Cortex-A17 PTM",
        "(Program Trace Macrocell)",
    },
    {
        ARM_ID,
        0x95d,
        "Cortex-A53 ETM",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x95e,
        "Cortex-A57 ETM",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x95f,
        "Cortex-A15 PTM",
        "(Program Trace Macrocell)",
    },
    {
        ARM_ID,
        0x961,
        "CoreSight TMC",
        "(Trace Memory Controller)",
    },
    {
        ARM_ID,
        0x962,
        "CoreSight STM",
        "(System Trace Macrocell)",
    },
    {
        ARM_ID,
        0x975,
        "Cortex-M7 ETM",
        "(Embedded Trace)",
    },
    {
        ARM_ID,
        0x9a0,
        "CoreSight PMU",
        "(Performance Monitoring Unit)",
    },
    {
        ARM_ID,
        0x9a1,
        "Cortex-M4 TPIU",
        "(Trace Port Interface Unit)",
    },
    {
        ARM_ID,
        0x9a4,
        "CoreSight GPR",
        "(Granular Power Requester)",
    },
    {
        ARM_ID,
        0x9a5,
        "Cortex-A5 PMU",
        "(Performance Monitor Unit)",
    },
    {
        ARM_ID,
        0x9a7,
        "Cortex-A7 PMU",
        "(Performance Monitor Unit)",
    },
    {
        ARM_ID,
        0x9a8,
        "Cortex-A53 CTI",
        "(Cross Trigger)",
    },
    {
        ARM_ID,
        0x9a9,
        "Cortex-M7 TPIU",
        "(Trace Port Interface Unit)",
    },
    {
        ARM_ID,
        0x9ae,
        "Cortex-A17 PMU",
        "(Performance Monitor Unit)",
    },
    {
        ARM_ID,
        0x9af,
        "Cortex-A15 PMU",
        "(Performance Monitor Unit)",
    },
    {
        ARM_ID,
        0x9b6,
        "Cortex-R52 PMU/CTI/ETM",
        "(Performance Monitor Unit/Cross Trigger/ETM)",
    },
    {
        ARM_ID,
        0x9b7,
        "Cortex-R7 PMU",
        "(Performance Monitor Unit)",
    },
    {
        ARM_ID,
        0x9d3,
        "Cortex-A53 PMU",
        "(Performance Monitor Unit)",
    },
    {
        ARM_ID,
        0x9d7,
        "Cortex-A57 PMU",
        "(Performance Monitor Unit)",
    },
    {
        ARM_ID,
        0x9d8,
        "Cortex-A72 PMU",
        "(Performance Monitor Unit)",
    },
    {
        ARM_ID,
        0x9da,
        "Cortex-A35 PMU/CTI/ETM",
        "(Performance Monitor Unit/Cross Trigger/ETM)",
    },
    {
        ARM_ID,
        0x9e2,
        "SoC-600 APB-AP",
        "(APB4 Memory Access Port)",
    },
    {
        ARM_ID,
        0x9e3,
        "SoC-600 AHB-AP",
        "(AHB5 Memory Access Port)",
    },
    {
        ARM_ID,
        0x9e4,
        "SoC-600 AXI-AP",
        "(AXI Memory Access Port)",
    },
    {
        ARM_ID,
        0x9e5,
        "SoC-600 APv1 Adapter",
        "(Access Port v1 Adapter)",
    },
    {
        ARM_ID,
        0x9e6,
        "SoC-600 JTAG-AP",
        "(JTAG Access Port)",
    },
    {
        ARM_ID,
        0x9e7,
        "SoC-600 TPIU",
        "(Trace Port Interface Unit)",
    },
    {
        ARM_ID,
        0x9e8,
        "SoC-600 TMC ETR/ETS",
        "(Embedded Trace Router/Streamer)",
    },
    {
        ARM_ID,
        0x9e9,
        "SoC-600 TMC ETB",
        "(Embedded Trace Buffer)",
    },
    {
        ARM_ID,
        0x9ea,
        "SoC-600 TMC ETF",
        "(Embedded Trace FIFO)",
    },
    {
        ARM_ID,
        0x9eb,
        "SoC-600 ATB Funnel",
        "(Trace Funnel)",
    },
    {
        ARM_ID,
        0x9ec,
        "SoC-600 ATB Replicator",
        "(Trace Replicator)",
    },
    {
        ARM_ID,
        0x9ed,
        "SoC-600 CTI",
        "(Cross Trigger)",
    },
    {
        ARM_ID,
        0x9ee,
        "SoC-600 CATU",
        "(Address Translation Unit)",
    },
    {
        ARM_ID,
        0xc05,
        "Cortex-A5 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xc07,
        "Cortex-A7 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xc08,
        "Cortex-A8 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xc09,
        "Cortex-A9 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xc0e,
        "Cortex-A17 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xc0f,
        "Cortex-A15 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xc14,
        "Cortex-R4 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xc15,
        "Cortex-R5 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xc17,
        "Cortex-R7 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xd03,
        "Cortex-A53 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xd04,
        "Cortex-A35 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xd07,
        "Cortex-A57 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xd08,
        "Cortex-A72 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xd0b,
        "Cortex-A76 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xd0c,
        "Neoverse N1",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xd13,
        "Cortex-R52 Debug",
        "(Debug Unit)",
    },
    {
        ARM_ID,
        0xd49,
        "Neoverse N2",
        "(Debug Unit)",
    },
    {
        0x017,
        0x120,
        "TI SDTI",
        "(System Debug Trace Interface)",
    }, /* from OMAP3 memmap */
    {
        0x017,
        0x343,
        "TI DAPCTL",
        "",
    }, /* from OMAP3 memmap */
    {0x017, 0x9af, "MSP432 ROM", "(ROM Table)"},
    {0x01f, 0xcd0, "Atmel CPU with DSU", "(CPU)"},
    {0x041, 0x1db, "XMC4500 ROM", "(ROM Table)"},
    {0x041, 0x1df, "XMC4700/4800 ROM", "(ROM Table)"},
    {0x041, 0x1ed, "XMC1000 ROM", "(ROM Table)"},
    {
        0x065,
        0x000,
        "SHARC+/Blackfin+",
        "",
    },
    {
        0x070,
        0x440,
        "Qualcomm QDSS Component v1",
        "(Qualcomm Designed CoreSight Component v1)",
    },
    {
        0x0bf,
        0x100,
        "Brahma-B53 Debug",
        "(Debug Unit)",
    },
    {
        0x0bf,
        0x9d3,
        "Brahma-B53 PMU",
        "(Performance Monitor Unit)",
    },
    {
        0x0bf,
        0x4a1,
        "Brahma-B53 ROM",
        "(ROM Table)",
    },
    {
        0x0bf,
        0x721,
        "Brahma-B53 ROM",
        "(ROM Table)",
    },
    {
        0x1eb,
        0x181,
        "Tegra 186 ROM",
        "(ROM Table)",
    },
    {
        0x1eb,
        0x202,
        "Denver ETM",
        "(Denver Embedded Trace)",
    },
    {
        0x1eb,
        0x211,
        "Tegra 210 ROM",
        "(ROM Table)",
    },
    {
        0x1eb,
        0x302,
        "Denver Debug",
        "(Debug Unit)",
    },
    {
        0x1eb,
        0x402,
        "Denver PMU",
        "(Performance Monitor Unit)",
    },
    {0x20, 0x410, "STM32F10 (med)", "(ROM Table)"},
    {0x20, 0x411, "STM32F2", "(ROM Table)"},
    {0x20, 0x412, "STM32F10 (low)", "(ROM Table)"},
    {0x20, 0x413, "STM32F40/41", "(ROM Table)"},
    {0x20, 0x414, "STM32F10 (high)", "(ROM Table)"},
    {0x20, 0x415, "STM32L47/48", "(ROM Table)"},
    {0x20, 0x416, "STM32L1xxx6/8/B", "(ROM Table)"},
    {0x20, 0x417, "STM32L05/06", "(ROM Table)"},
    {0x20, 0x418, "STM32F105xx/107", "(ROM Table)"},
    {0x20, 0x419, "STM32F42/43", "(ROM Table)"},
    {0x20, 0x420, "STM32F10 (med)", "(ROM Table)"},
    {0x20, 0x421, "STM32F446xx", "(ROM Table)"},
    {0x20, 0x422, "STM32FF358/02/03", "(ROM Table)"},
    {0x20, 0x423, "STM32F401xB/C", "(ROM Table)"},
    {0x20, 0x425, "STM32L031/41", "(ROM Table)"},
    {0x20, 0x427, "STM32L1xxxC", "(ROM Table)"},
    {0x20, 0x428, "STM32F10 (high)", "(ROM Table)"},
    {0x20, 0x429, "STM32L1xxx6A/8A/BA", "(ROM Table)"},
    {0x20, 0x430, "STM32F10 (xl)", "(ROM Table)"},
    {0x20, 0x431, "STM32F411xx", "(ROM Table)"},
    {0x20, 0x432, "STM32F373/8", "(ROM Table)"},
    {0x20, 0x433, "STM32F401xD/E", "(ROM Table)"},
    {0x20, 0x434, "STM32F469/79", "(ROM Table)"},
    {0x20, 0x435, "STM32L43/44", "(ROM Table)"},
    {0x20, 0x436, "STM32L1xxxD", "(ROM Table)"},
    {0x20, 0x437, "STM32L1xxxE", "(ROM Table)"},
    {0x20, 0x438, "STM32F303/34/28", "(ROM Table)"},
    {0x20, 0x439, "STM32F301/02/18	", "(ROM Table)"},
    {0x20, 0x440, "STM32F03/5", "(ROM Table)"},
    {0x20, 0x441, "STM32F412xx", "(ROM Table)"},
    {0x20, 0x442, "STM32F03/9", "(ROM Table)"},
    {0x20, 0x444, "STM32F03xx4", "(ROM Table)"},
    {0x20, 0x445, "STM32F04/7", "(ROM Table)"},
    {0x20, 0x446, "STM32F302/03/98", "(ROM Table)"},
    {0x20, 0x447, "STM32L07/08", "(ROM Table)"},
    {0x20, 0x448, "STM32F070/1/2", "(ROM Table)"},
    {0x20, 0x449, "STM32F74/5", "(ROM Table)"},
    {0x20, 0x450, "STM32H74/5", "(ROM Table)"},
    {0x20, 0x451, "STM32F76/7", "(ROM Table)"},
    {0x20, 0x452, "STM32F72/3", "(ROM Table)"},
    {0x20, 0x457, "STM32L01/2", "(ROM Table)"},
    {0x20, 0x458, "STM32F410xx", "(ROM Table)"},
    {0x20, 0x460, "STM32G07/8", "(ROM Table)"},
    {0x20, 0x461, "STM32L496/A6", "(ROM Table)"},
    {0x20, 0x462, "STM32L45/46", "(ROM Table)"},
    {0x20, 0x463, "STM32F413/23", "(ROM Table)"},
    {0x20, 0x464, "STM32L412/22", "(ROM Table)"},
    {0x20, 0x466, "STM32G03/04", "(ROM Table)"},
    {0x20, 0x468, "STM32G431/41", "(ROM Table)"},
    {0x20, 0x469, "STM32G47/48", "(ROM Table)"},
    {0x20, 0x470, "STM32L4R/S", "(ROM Table)"},
    {0x20, 0x471, "STM32L4P5/Q5", "(ROM Table)"},
    {0x20, 0x479, "STM32G491xx", "(ROM Table)"},
    {0x20, 0x480, "STM32H7A/B", "(ROM Table)"},
    {0x20, 0x495, "STM32WB50/55", "(ROM Table)"},
    {0x20, 0x497, "STM32WLE5xx", "(ROM Table)"}};

const char* adi_devarch_desc(uint32_t devarch) {
    if(!(devarch & ARM_CS_C9_DEVARCH_PRESENT)) {
        return "not present";
    }

    for(unsigned int i = 0; i < ARRAY_SIZE(class0x9_devarch); i++) {
        if((devarch & DEVARCH_ID_MASK) == class0x9_devarch[i].arch_id) {
            return class0x9_devarch[i].description;
        }
    }

    return "unknown";
}

const struct dap_part_nums* adi_part_num(unsigned int des, unsigned int part) {
    static char buf[32];
    static struct dap_part_nums unknown = {
        .type = "Unrecognized",
        .full = "",
    };

    for(unsigned int i = 0; i < ARRAY_SIZE(dap_part_nums); i++) {
        if(dap_part_nums[i].designer_id == des && dap_part_nums[i].part_num == part) {
            return &dap_part_nums[i];
        }
    }

    snprintf(buf, sizeof(buf), "D:%x P:%x", des, part);
    unknown.full = buf;

    return &unknown;
}

bool adi_get_pidr(AppFSM* const ctx, uint32_t base, pidr_data_t* data) {
    uint32_t pidrs[7];
    uint32_t offsets[] = {0xFE0, 0xFE4, 0xFE8, 0xFEC, 0xFD0, 0xFD4, 0xFD8, 0xFDC};

    furi_mutex_acquire(ctx->swd_mutex, FuriWaitForever);
    for(size_t pos = 0; pos < COUNT(pidrs); pos++) {
        uint8_t ret = swd_read_memory(ctx, ctx->ap_pos, base + offsets[pos], &pidrs[pos]);
        if(ret != 1) {
            DBGS("Read failed");
            furi_mutex_release(ctx->swd_mutex);
            return false;
        }
    }
    furi_mutex_release(ctx->swd_mutex);

    data->designer = ((pidrs[4] & 0x0F) << 7) | ((pidrs[2] & 0x07) << 4) |
                     ((pidrs[1] >> 4) & 0x0F);
    data->part = (pidrs[0] & 0xFF) | ((pidrs[1] & 0x0F) << 8);
    data->revand = ((pidrs[3] >> 4) & 0x0F);
    data->cmod = (pidrs[3] & 0x0F);
    data->revision = ((pidrs[2] >> 4) & 0x0F);
    data->size = ((pidrs[2] >> 4) & 0x0F);

    return true;
}

bool adi_get_class(AppFSM* const ctx, uint32_t base, uint8_t* class) {
    uint32_t cidrs[4];
    uint32_t offsets[] = {0xFF0, 0xFF4, 0xFF8, 0xFFC};

    furi_mutex_acquire(ctx->swd_mutex, FuriWaitForever);
    for(size_t pos = 0; pos < COUNT(cidrs); pos++) {
        uint8_t ret = swd_read_memory(ctx, ctx->ap_pos, base + offsets[pos], &cidrs[pos]);
        if(ret != 1) {
            DBGS("Read failed");
            furi_mutex_release(ctx->swd_mutex);
            return false;
        }
    }
    furi_mutex_release(ctx->swd_mutex);

    if((cidrs[0] & 0xFF) != 0x0D) {
        return false;
    }
    if((cidrs[1] & 0x0F) != 0x00) {
        return false;
    }
    if((cidrs[2] & 0xFF) != 0x05) {
        return false;
    }
    if((cidrs[3] & 0xFF) != 0xB1) {
        return false;
    }

    *class = ((cidrs[1] >> 4) & 0x0F);

    return true;
}

const char* adi_romtable_type(AppFSM* const ctx, uint32_t base) {
    pidr_data_t data;

    if(!adi_get_pidr(ctx, base, &data)) {
        return "fail";
    }
    const struct dap_part_nums* info = adi_part_num(data.designer, data.part);

    return info->type;
}

const char* adi_romtable_full(AppFSM* const ctx, uint32_t base) {
    pidr_data_t data;

    if(!adi_get_pidr(ctx, base, &data)) {
        return "fail";
    }
    const struct dap_part_nums* info = adi_part_num(data.designer, data.part);

    return info->full;
}

uint32_t adi_romtable_entry_count(AppFSM* const ctx, uint32_t base) {
    uint32_t count = 0;
    uint32_t entry = 0;

    furi_mutex_acquire(ctx->swd_mutex, FuriWaitForever);
    for(size_t pos = 0; pos < 960; pos++) {
        uint8_t ret = 0;
        for(int tries = 0; tries < 10 && ret != 1; tries++) {
            ret = swd_read_memory(ctx, ctx->ap_pos, base + pos * 4, &entry);
        }
        if(ret != 1) {
            DBGS("Read failed");
            break;
        }
        if(!(entry & 1)) {
            break;
        }
        if(entry & 0x00000FFC) {
            break;
        }
        count++;
    }
    furi_mutex_release(ctx->swd_mutex);
    return count;
}

uint32_t adi_romtable_get(AppFSM* const ctx, uint32_t base, uint32_t pos) {
    uint32_t entry = 0;

    furi_mutex_acquire(ctx->swd_mutex, FuriWaitForever);
    uint8_t ret = swd_read_memory(ctx, ctx->ap_pos, base + pos * 4, &entry);
    if(ret != 1) {
        DBGS("Read failed");
        furi_mutex_release(ctx->swd_mutex);
        return 0;
    }
    furi_mutex_release(ctx->swd_mutex);

    return base + (entry & 0xFFFFF000);
}

bool adi_is_romtable(AppFSM* const ctx, uint32_t base) {
    uint8_t class = 0;

    if(!adi_get_class(ctx, base, &class)) {
        return false;
    }

    if(class != CIDR_CLASS_ROMTABLE) {
        return false;
    }

    return true;
}
