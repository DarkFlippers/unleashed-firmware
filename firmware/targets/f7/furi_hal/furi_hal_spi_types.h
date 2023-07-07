#pragma once

#include <stdint.h>
#include <stddef.h>

#include <furi_hal_gpio.h>

#include <stm32wbxx_ll_spi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FuriHalSpiBus FuriHalSpiBus;
typedef struct FuriHalSpiBusHandle FuriHalSpiBusHandle;

/** FuriHal spi bus states */
typedef enum {
    FuriHalSpiBusEventInit, /**< Bus initialization event, called on system start */
    FuriHalSpiBusEventDeinit, /**< Bus deinitialization event, called on system stop */
    FuriHalSpiBusEventLock, /**< Bus lock event, called before activation */
    FuriHalSpiBusEventUnlock, /**< Bus unlock event, called after deactivation */
    FuriHalSpiBusEventActivate, /**< Bus activation event, called before handle activation */
    FuriHalSpiBusEventDeactivate, /**< Bus deactivation event, called after handle deactivation  */
} FuriHalSpiBusEvent;

/** FuriHal spi bus event callback */
typedef void (*FuriHalSpiBusEventCallback)(FuriHalSpiBus* bus, FuriHalSpiBusEvent event);

/** FuriHal spi bus */
struct FuriHalSpiBus {
    SPI_TypeDef* spi;
    FuriHalSpiBusEventCallback callback;
    FuriHalSpiBusHandle* current_handle;
};

/** FuriHal spi handle states */
typedef enum {
    FuriHalSpiBusHandleEventInit, /**< Handle init, called on system start, initialize gpio for idle state */
    FuriHalSpiBusHandleEventDeinit, /**< Handle deinit, called on system stop, deinitialize gpio for default state */
    FuriHalSpiBusHandleEventActivate, /**< Handle activate: connect gpio and apply bus config */
    FuriHalSpiBusHandleEventDeactivate, /**< Handle deactivate: disconnect gpio and reset bus config */
} FuriHalSpiBusHandleEvent;

/** FuriHal spi handle event callback */
typedef void (*FuriHalSpiBusHandleEventCallback)(
    FuriHalSpiBusHandle* handle,
    FuriHalSpiBusHandleEvent event);

/** FuriHal spi handle */
struct FuriHalSpiBusHandle {
    FuriHalSpiBus* bus;
    FuriHalSpiBusHandleEventCallback callback;
    const GpioPin* miso;
    const GpioPin* mosi;
    const GpioPin* sck;
    const GpioPin* cs;
};

#ifdef __cplusplus
}
#endif
