#pragma once

#include <stm32wbxx_ll_i2c.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FuriHalI2cBus FuriHalI2cBus;
typedef struct FuriHalI2cBusHandle FuriHalI2cBusHandle;

/** FuriHal i2c bus states */
typedef enum {
    FuriHalI2cBusEventInit, /**< Bus initialization event, called on system start */
    FuriHalI2cBusEventDeinit, /**< Bus deinitialization event, called on system stop */
    FuriHalI2cBusEventLock, /**< Bus lock event, called before activation */
    FuriHalI2cBusEventUnlock, /**< Bus unlock event, called after deactivation */
    FuriHalI2cBusEventActivate, /**< Bus activation event, called before handle activation */
    FuriHalI2cBusEventDeactivate, /**< Bus deactivation event, called after handle deactivation  */
} FuriHalI2cBusEvent;

/** FuriHal i2c bus event callback */
typedef void (*FuriHalI2cBusEventCallback)(FuriHalI2cBus* bus, FuriHalI2cBusEvent event);

/** FuriHal i2c bus */
struct FuriHalI2cBus {
    I2C_TypeDef* i2c;
    FuriHalI2cBusHandle* current_handle;
    FuriHalI2cBusEventCallback callback;
};

/** FuriHal i2c handle states */
typedef enum {
    FuriHalI2cBusHandleEventActivate, /**< Handle activate: connect gpio and apply bus config */
    FuriHalI2cBusHandleEventDeactivate, /**< Handle deactivate: disconnect gpio and reset bus config */
} FuriHalI2cBusHandleEvent;

/** FuriHal i2c handle event callback */
typedef void (*FuriHalI2cBusHandleEventCallback)(
    FuriHalI2cBusHandle* handle,
    FuriHalI2cBusHandleEvent event);

/** FuriHal i2c handle */
struct FuriHalI2cBusHandle {
    FuriHalI2cBus* bus;
    FuriHalI2cBusHandleEventCallback callback;
};

#ifdef __cplusplus
}
#endif
