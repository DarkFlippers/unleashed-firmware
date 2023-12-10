#include <bq27220_data_memory.h>

const BQ27220DMGaugingConfig furi_hal_power_gauge_data_memory_gauging_config = {
    .CCT = 1,
    .CSYNC = 0,
    .EDV_CMP = 0,
    .SC = 1,
    .FIXED_EDV0 = 1,
    .FCC_LIM = 1,
    .FC_FOR_VDQ = 1,
    .IGNORE_SD = 1,
    .SME0 = 0,
};

const BQ27220DMData furi_hal_power_gauge_data_memory[] = {
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1GaugingConfig,
        .type = BQ27220DMTypePtr16,
        .value.u32 = (uint32_t)&furi_hal_power_gauge_data_memory_gauging_config,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1FullChargeCapacity,
        .type = BQ27220DMTypeU16,
        .value.u16 = 1300,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1DesignCapacity,
        .type = BQ27220DMTypeU16,
        .value.u16 = 1300,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1EMF,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3679,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1C0,
        .type = BQ27220DMTypeU16,
        .value.u16 = 430,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1R0,
        .type = BQ27220DMTypeU16,
        .value.u16 = 334,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1T0,
        .type = BQ27220DMTypeU16,
        .value.u16 = 4626,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1R1,
        .type = BQ27220DMTypeU16,
        .value.u16 = 408,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1TC,
        .type = BQ27220DMTypeU8,
        .value.u8 = 11,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1C1,
        .type = BQ27220DMTypeU8,
        .value.u8 = 0,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1StartDOD0,
        .type = BQ27220DMTypeU16,
        .value.u16 = 4044,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1StartDOD10,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3905,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1StartDOD20,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3807,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1StartDOD30,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3718,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1StartDOD40,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3642,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1StartDOD50,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3585,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1StartDOD60,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3546,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1StartDOD70,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3514,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1StartDOD80,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3477,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1StartDOD90,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3411,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1StartDOD100,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3299,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1EDV0,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3300,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1EDV1,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3321,
    },
    {
        .address = BQ27220DMAddressGasGaugingCEDVProfile1EDV2,
        .type = BQ27220DMTypeU16,
        .value.u16 = 3355,
    },
    {
        .address = BQ27220DMAddressCalibrationCurrentDeadband,
        .type = BQ27220DMTypeU8,
        .value.u8 = 1,
    },
    {
        .address = BQ27220DMAddressConfigurationPowerSleepCurrent,
        .type = BQ27220DMTypeI16,
        .value.i16 = 1,
    },
    {
        .type = BQ27220DMTypeEnd,
    },
};
