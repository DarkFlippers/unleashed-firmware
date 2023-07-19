# Using FuriHalBus API

## Basic info

On system startup, most of the peripheral devices are under reset and not clocked by default. This is done to reduce power consumption and to guarantee that the device will always be in the same state before use.
Some crucial peripherals are enabled right away by the system, others must be explicitly enabled by the user code.

**NOTE:** Here and afterwards the word *"system"* refers to any code belonging to the operating system, hardware drivers or built-in applications.

To **ENABLE** a peripheral, call `furi_hal_bus_enable()`. At the time of the call, the peripheral in question MUST be disabled, otherwise a crash will occur to indicate improper use. This means that any given peripheral cannot be enabled twice or more without disabling it first.

To **DISABLE** a peripheral, call `furi_hal_bus_disable()`. Likewise, the peripheral in question MUST be enabled, otherwise a crash will occur.

To **RESET** a peripheral, call `furi_hal_bus_reset()`. The peripheral in question MUST be enabled, otherwise a crash will occur. This method is used whenever it is necessary to reset all the peripheral's registers to their initial states without disabling it.

## Peripherals

Built-in peripherals are divided into three categories: 
- Enabled by the system on startup, never disabled;
- Enabled and disabled by the system on demand;
- Enabled and disabled by the user code.

### Always-on peripherals

Below is the list of peripherals that are enabled by the system. The user code must NEVER attempt to disable them. If a corresponding API is provided, the user code must employ it in order to access the peripheral.

*Table 1* - Peripherals enabled by the system

| Peripheral    | Enabled at                |
| :-----------: | :-----------------------: |
| DMA1          | `furi_hal_dma.c`          |
| DMA2          | --                        |
| DMAMUX        | --                        |
| GPIOA         | `furi_hal_resources.c`    |
| GPIOB         | --                        |
| GPIOC         | --                        |
| GPIOD         | --                        |
| GPIOE         | --                        |
| GPIOH         | --                        |
| PKA           | `furi_hal_bt.c`           |
| AES2          | --                        |
| HSEM          | --                        |
| IPCC          | --                        |
| FLASH         | enabled by hardware       |

### On-demand system peripherals

Below is the list of peripherals that are enabled and disabled by the system. The user code must avoid using them directly, preferring the respective APIs instead.

When not using the API, these peripherals MUST be enabled by the user code and then disabled when not needed anymore.

*Table 2* - Peripherals enabled and disabled by the system

| Peripheral    | API header file       |
| :-----------: | :-------------------: |
| RNG           | `furi_hal_random.h`   |
| SPI1          | `furi_hal_spi.h`      |
| SPI2          | --                    |
| I2C1          | `furi_hal_i2c.h`      |
| I2C3          | --                    |
| USART1        | `furi_hal_uart.h`     |
| LPUART1       | --                    |
| USB           | `furi_hal_usb.h`      |

### On-demand shared peripherals

Below is the list of peripherals that are not enabled by default and MUST be enabled by the user code each time it accesses them. 

Note that some of these peripherals may also be used by the system to implement its certain features.
The system will take over any given peripheral only when the respective feature is in use.

*Table 3* - Peripherals enabled and disabled by user

| Peripheral    | System    | Purpose                               |
| :-----------: | :-------: | ------------------------------------- |
| CRC           |           |                                       |
| TSC           |           |                                       |
| ADC           |           |                                       |
| QUADSPI       |           |                                       |
| TIM1          | yes       | subghz, lfrfid, nfc, infrared, etc... |
| TIM2          | yes       | subghz, infrared, etc...              |
| TIM16         | yes       | speaker                               |
| TIM17         | yes       | cc1101_ext                            |
| LPTIM1        | yes       | tickless idle timer                   |
| LPTIM2        | yes       | pwm                                   |
| SAI1          |           |                                       |
| LCD           |           |                                       |


## DMA

The DMA1,2 peripherals are a special case in that they have multiple independent channels. Some of the channels may be in use by the system.

Below is the list of DMA channels and their usage by the system.

*Table 4* - DMA channels

| DMA   | Channel   | System    | Purpose                   |
| :---: | :-------: | :-------: | ------------------------- |
| DMA1  | 1         | yes       | digital signal            |
|  --   | 2         | yes       | --                        |
|  --   | 3         |           |                           |
|  --   | 4         | yes       | pulse reader              |
|  --   | 5         |           |                           |
|  --   | 6         |           |                           |
|  --   | 7         |           |                           |
| DMA2  | 1         | yes       | infrared, lfrfid, subghz, |
|  --   | 2         | yes       | --                        |
|  --   | 3         | yes       | cc1101_ext                |
|  --   | 4         | yes       | cc1101_ext                |
|  --   | 5         | yes       | cc1101_ext                |
|  --   | 6         | yes       | SPI                       |
|  --   | 7         | yes       | SPI                       |
