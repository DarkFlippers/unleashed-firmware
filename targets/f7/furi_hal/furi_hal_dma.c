#include <furi_hal_dma.h>
#include <furi_hal_bus.h>

void furi_hal_dma_init_early(void) {
    furi_hal_bus_enable(FuriHalBusDMA1);
    furi_hal_bus_enable(FuriHalBusDMA2);
    furi_hal_bus_enable(FuriHalBusDMAMUX1);
}

void furi_hal_dma_deinit_early(void) {
    furi_hal_bus_disable(FuriHalBusDMA1);
    furi_hal_bus_disable(FuriHalBusDMA2);
    furi_hal_bus_disable(FuriHalBusDMAMUX1);
}
