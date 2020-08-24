* Firmware target: F2
* Body version: no body
* Interconnection version: 1

**[Device schematic (pdf)](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/F2B0C1.1.sch.pdf)**

# Device layout

* [main PCB](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/F2B0C1.1.main.pdf)
* [NFC PCB](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/F2B0C1.1.NFC.pdf)
* [iButton PCB](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/F2B0C1.1.ibtn.pdf)

# Pinout:
| Pin | Mode | Define | Description |
|-|-|-|-|
| PC13 | EXTI13 | BUTTON_BACK | Button |
| PC14 | RCC_OSC32_IN | | 32768 kHz XTAL |
| PC15 | RCC_OSC32_OUT | | 32768 kHz XTAL |
| PH0 | RCC_OSC_IN | | 16 MHz XTAL |
| PH1 | RCC_OSC_OUT	| | 16 MHz XTAL |
| PC0 | GPIO_Analog | | External GPIO |
| PC1 | GPIO_Analog	| | External GPIO |
| PC2 | EXTI2 | CHRG | Charge state |
| PC3 | ADC1_IN4 | BATT_V | Battery voltage |
| PA0 | ADC1_IN5 | IR_RX | Infrared receiver |
| PA1 | EXTI1 | BUTTON_DOWN | Button |
| PA2 | GPIO_Output | DISPLAY_DI | Display |
| PA3 | TIM5_CH4 | SPEAKER | |
| PA4 | GPIO_Analog | | External GPIO |
| PA5 | GPIO_Analog	| | External GPIO |
| PA6 | GPIO_Analog	| | External GPIO |
| PA7 | GPIO_Analog	| | External GPIO |
| PC4 | GPIO_Output	| NFC_CS | NFC SPI CS |
| PC5 | COMP1_INP | RFID_RF_IN | 125 kHz RFID RX, contact keys analog input (for Cyfral/metakom) |
| PB0 | EXTI0 | BUTTON_UP | Button |
| PB1 | GPIO_Output | LED_BLUE | Led |
| PB2 | GPIO_Analog	| | External GPIO |
| PB10 | GPIO_Output | DISPLAY_RST | Display |
| PB11 | GPIO_Output | IR_TX | Infrared transmit |
| PB12 | GPIO_Analog | | External GPIO |
| PB13 | TIM15_CH1N	| RFID_OUT | RFID 125 kHz read master tone/write TX |
| PB14 | GPIO_Output | LED_GREEN | Led |
| PB15 | TIM15_CH2 | RFID_PULL | RFID 125 kHz write/emulate pulling | NFC IRQ |
| PC6 | GPIO_Output | VIBRO | |
| PC7 | TIM8_CH2 | iButton | contact key |
| PC8 | GPIO_Output	| DISPLAY_CS | Dispay |
| PC9 | GPIO_Output	| SD_CS | microSD card SD |
| PA8 | GPIO_Output	| LED_RED | Led |
| PA9 | USART1_TX | | External GPIO/USART TX |
| PA10 | USART1_RX | | External GPIO/USART RX |
| PA11 | USB_OTG_FS_DM | | USB |
| PA12 | USB_OTG_FS_DP | | USB |
| PA13 | SWDIO | | External GPIO/SWDIO |
| PA14 | SWCLK | | External GPIO/SWDCLK |
| PA15 | GPIO_Output | CC1101_CS | CC1101 CS |
| PC10 | SPI3_SCK | | SD/CC1101/NFC SPI |
| PC11 | SPI3_MISO | | SD/CC1101/NFC SPI |
| PC12 | SPI3_MOSI | | SD/CC1101/NFC SPI |
| PD2 | GPIO_Analog	| | External GPIO |
| PB3 | SPI1_SCK | | Display |
| PB4 | EXTI4 | BUTTON_LEFT | Button |
| PB5 | SPI1_MOSI | | Display |
| PB6 | GPIO_Output	| DISPLAY_BACKLIGHT | |
| PB7 | GPIO_Input | CC1101_G0 | |
| PB8 | EXTI8 | BUTTON_RIGHT | Button |
| PB9 | EXTI9 | BUTTON_OK | Button |

# External GPIO

| Pin | | | | | |
|-|-|-|-|-|-|
| PC0 | GPIO | LPUART RX | ADC1/2/3-1 | I²C SCL | |
| PC1 | GPIO | LPUART TX | ADC1/2/3-2 | I²C SDA | |
| PA4 | GPIO | DAC1-1 | ADC1/2-9 | SPI1 NSS | |
| PA5 | GPIO | DAC1-2 | ADC1/2-10 | SPI1 SCK | TIM2/8 OUT |
| PA6 | GPIO | | ADC1/2-11 | SPI1 MISO | TIM3/16 OUT |
| PA7 | GPIO | | ADC1/2-12 | SPI1 MOSI | TIM1/2/8/17 OUT |
| PB2 | GPIO | | | | RTC Alarm |
| PB12 | GPIO | | | | |
| PA9 | GPIO | USART1 TX | | | |
| PA10 | GPIO | USART1 RX | | | |
| PA13 | GPIO | SWDIO | | | |
| PA14 | GPIO | SWCLK | | | |
| PD2 | GPIO | UART5 RX | | | |