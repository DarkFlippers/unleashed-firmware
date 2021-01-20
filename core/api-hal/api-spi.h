#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
struct used for handling SPI info.
*/
typedef struct {
    SPI_HandleTypeDef* spi;
    PubSubCallback cb;
    void* ctx;
} SpiHandle;

/*
For transmit/receive data use `spi_xfer` function.

* `tx_data` and `rx_data` size must be equal (and equal `len`)
* `cb` called after spi operation is completed, `(NULL, ctx)` passed to callback.
*/
bool spi_xfer(
    SPI_HandleTypeDef* spi,
    uint8_t* tx_data,
    uint8_t* rx_data,
    size_t len,
    PubSubCallback cb,
    void* ctx);

/*
Blocking verison:
*/
static inline bool
spi_xfer_block(SPI_HandleTypeDef* spi, uint8_t* tx_data, uint8_t* rx_data, size_t len) {
    semaphoreInfo s;
    osSemaphore block = createSemaphoreStatic(s);
    if(!spi_xfer(spi, tx_data, rx_data, len, RELEASE_SEMAPHORE, (void*)block)) {
        osReleaseSemaphore(block);
        return false;
    }
    osWaitSemaphore(block);
    return false;
}

/*
Common implementation of SPI bus: serial interface + CS pin
*/
typedef struct {
    GpioPin* cs; ///< CS pin
    ValueMutex* spi; ///< <SpiHandle*>
} SpiBus;

/*
For dedicated work with one device there is `SpiDevice` entity.
It contains ValueMutex around SpiBus: after you acquire device
you can acquire spi to work with it (don't forget SPI bus is shared
around many device, release it after every transaction as quick as possible).
*/
typedef struct {
    ValueMutex* bus; ///< <SpiBus*>
} SpiDevice;

##SPI IRQ device

    /*
Many devices (like CC1101 and NFC) present as SPI bus and IRQ line.
For work with it there is special entity `SpiIrqDevice`.
Use `subscribe_pubsub` for subscribinq to irq events.
*/

    typedef struct {
    ValueMutex* bus; ///< <SpiBus*>
    PubSub* irq;
} SpiIrqDevice;

/*
Special implementation of SPI bus: serial interface + CS, Res, D/I lines.
*/
typedef struct {
    GpioPin* cs; ///< CS pin
    GpioPin* res; ///< reset pin
    GpioPin* di; ///< D/I pin
    ValueMutex* spi; ///< <SPI_HandleTypeDef*>
} DisplayBus;

typedef struct {
    ValueMutex* bus; ///< <DisplayBus*>
} DisplayDevice;

/*
# SPI devices (F2)

* `/dev/sdcard` - SD card SPI, `SpiDevice`
* `/dev/cc1101_bus` - Sub-GHz radio (CC1101), `SpiIrqDevice`
* `/dev/nfc` - NFC (ST25R3916), `SpiIrqDevice`
* `/dev/display` - `DisplayDevice`
* `/dev/spiext` - External SPI (warning! Lock PA4, PA5, PA6, PA7)

### Application example

```C
// Be careful, this function called from IRQ context
void handle_irq(void* _arg, void* _ctx) {
}

void cc1101_example() {
    SpiIrqDevice* cc1101_device = open_input("/dev/cc1101_bus");
    if(cc1101_device == NULL) return; // bus not available, critical error

    subscribe_pubsub(cc1101_device->irq, handle_irq, NULL);

    {
        // acquire device as device bus
        SpiBus* spi_bus = acquire_mutex(cc1101_device->bus, 0);
        if(spi_bus == NULL) {
            printf("Device busy\n");
            // wait for device
            spi_bus = acquire_mutex_block(cc1101_device->bus);
        }
        
        // make transaction
        uint8_t request[4] = {0xDE, 0xAD, 0xBE, 0xEF};
        uint8_t response[4];

        {
            SPI_HandleTypeDef* spi = acquire_mutex_block(spi_bus->spi);

            gpio_write(spi_bus->cs, false);
            spi_xfer_block(spi, request, response, 4);
            gpio_write(spi_bus->cs, true);

            release_mutex(cc1101_device->spi, spi);
        }

        // release device (device bus)
        release_mutex(cc1101_device->bus, spi_bus);
    }
}
```
*/

#ifdef __cplusplus
}
#endif
