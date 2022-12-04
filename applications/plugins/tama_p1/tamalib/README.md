# TamaLIB - A hardware agnostic Tamagotchi P1 emulation library


## Synopsis

TamaLib is a hardware agnostic Tamagotchi P1 emulation library built from scratch. It is self-contained and aims at running on any platform powerful enough, from microcontrollers (MCUs) to desktop computers, thus spreading virtual life across the digital world.

So far, it has been successfully implemented on different platforms:
- Desktop computers (check out [TamaTool](https://github.com/jcrona/tamatool/) for more information) 
- STM32F072 MCU based board (check out [MCUGotchi](https://github.com/jcrona/mcugotchi/) for more information).
- OpenTama which is an STM32L072 MCU based board (check out [OpenTama](https://github.com/Sparkr-tech/opentama) and [MCUGotchi](https://github.com/jcrona/mcugotchi/) for more information).
- Arduino UNO (check out [ArduinoGotchi](https://github.com/GaryZ88/ArduinoGotchi/) for more information).

## Importing TamaLIB

TamaLIB cannot be used as is. In order to create life on a specific target, you need to import all TamaLIB related __.c__ and __.h__ files in your project (for instance in a __lib__ subfolder), to create a __hal_types.h__ file using the template provided and to implement the __hal_t__ structure, that will act as an abstraction layer between TamaLIB and your OS or SDK (detailed information can be found in __hal.h__). This abstraction layer basically connects TamaLIB to your target's buttons, clock, audio and screen, while also defining the C types that TamaLIB should use to represent 4-bit, 5-bit, 8-bit, 12-bit, 13-bit and 32-bit variables. Once done, you will be able to call the TamaLIB API from your project.


## Using the TamaLIB API

Basically:
```
/* ... */

/* Register the HAL */
tamalib_register_hal(&my_hal);

/* ... */

/* Initialize TamaLIB */
tamalib_init(my_program, my_breakpoints, 1000000); // my_breakpoints can be NULL, 1000000 means that timestamps will be expressed in us

/* ... */

/* Enter TamaLIB's loop */
tamalib_mainloop();

/* ... */

/* Release TamaLIB */
tamalib_release();

/* ... */
```
Your main project should then forward any button input to TamaLIB using the `tamalib_set_button()` function.

As an alternative to `tamalib_mainloop()`, you can call `tamalib_step()` directly if your execution flow requires something more complex than a simple mainloop. In that case, TamaLIB will neither call the HAL `handler()` function, nor the HAL `update_screen()` function by itslef.


## License

TamaLIB is distributed under the GPLv2 license. See the LICENSE file for more information.


## Hardware information

The Tamagotchi P1 is based on an
[E0C6S46 Epson MCU](https://download.epson-europe.com/pub/electronics-de/asmic/4bit/62family/technicalmanual/tm_6s46.pdf),
and runs at 32,768 kHz. Its LCD is 32x16 B/W pixels, with 8 icons.
To my knowledge, the ROM available online has been extracted from a high-res picture of a die. The ROM mask was clear enough to be optically read. The pictures can be seen [there](https://siliconpr0n.org/map/bandai/tamagotchi-v1/) (thx asterick for the link !).  
I would love to see the same work done on a P2 and add support for it in TamaLIB/TamaTool !

__  
Copyright (C) 2021 Jean-Christophe Rona
