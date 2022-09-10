#include <furi_hal_mpu.h>
#include <stm32wbxx_ll_cortex.h>

#define FURI_HAL_MPU_ATTRIBUTES                                                     \
    (LL_MPU_ACCESS_BUFFERABLE | LL_MPU_ACCESS_CACHEABLE | LL_MPU_ACCESS_SHAREABLE | \
     LL_MPU_TEX_LEVEL1 | LL_MPU_INSTRUCTION_ACCESS_ENABLE)

#define FURI_HAL_MPU_STACK_PROTECT_REGION FuriHalMPURegionSize32B

void furi_hal_mpu_init() {
    furi_hal_mpu_enable();

    // NULL pointer dereference protection
    furi_hal_mpu_protect_no_access(FuriHalMpuRegionNULL, 0x00, FuriHalMPURegionSize1MB);
}

void furi_hal_mpu_enable() {
    LL_MPU_Enable(LL_MPU_CTRL_PRIVILEGED_DEFAULT);
}

void furi_hal_mpu_disable() {
    LL_MPU_Disable();
}

void furi_hal_mpu_protect_no_access(
    FuriHalMpuRegion region,
    uint32_t address,
    FuriHalMPURegionSize size) {
    uint32_t size_ll = size;
    size_ll = size_ll << MPU_RASR_SIZE_Pos;

    furi_hal_mpu_disable();
    LL_MPU_ConfigRegion(
        region, 0x00, address, FURI_HAL_MPU_ATTRIBUTES | LL_MPU_REGION_NO_ACCESS | size_ll);
    furi_hal_mpu_enable();
}

void furi_hal_mpu_protect_read_only(
    FuriHalMpuRegion region,
    uint32_t address,
    FuriHalMPURegionSize size) {
    uint32_t size_ll = size;
    size_ll = size_ll << MPU_RASR_SIZE_Pos;

    furi_hal_mpu_disable();
    LL_MPU_ConfigRegion(
        region, 0x00, address, FURI_HAL_MPU_ATTRIBUTES | LL_MPU_REGION_PRIV_RO_URO | size_ll);
    furi_hal_mpu_enable();
}

void furi_hal_mpu_protect_disable(FuriHalMpuRegion region) {
    furi_hal_mpu_disable();
    LL_MPU_DisableRegion(region);
    furi_hal_mpu_enable();
}

void furi_hal_mpu_set_stack_protection(uint32_t* stack) {
    // Protection area address must be aligned to region size
    uint32_t stack_ptr = (uint32_t)stack;
    uint32_t mask = ((1 << (FURI_HAL_MPU_STACK_PROTECT_REGION + 2)) - 1);
    stack_ptr &= ~mask;
    if(stack_ptr < (uint32_t)stack) stack_ptr += (mask + 1);

    furi_hal_mpu_protect_read_only(
        FuriHalMpuRegionStack, stack_ptr, FURI_HAL_MPU_STACK_PROTECT_REGION);
}