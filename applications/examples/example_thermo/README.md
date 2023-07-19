# 1-Wire Thermometer
This example application demonstrates the use of the 1-Wire library with a DS18B20 thermometer. 
It also covers basic GUI, input handling, threads and localisation.

## Electrical connections
Before launching the application, connect the sensor to Flipper's external GPIO according to the table below:
| DS18B20 | Flipper |
| :-----: | :-----: |
| VDD | 9 |
| GND | 18 |
| DQ  | 17 |

*NOTE 1*: GND is also available on pins 8 and 11.

*NOTE 2*: For any other pin than 17, connect an external 4.7k pull-up resistor to pin 9.

## Launching the application
In order to launch this demo, follow the steps below:
1. Make sure your Flipper has an SD card installed.
2. Connect your Flipper to the computer via a USB cable.
3. Run `./fbt launch APPSRC=example_thermo` in your terminal emulator of choice.

## Changing the data pin
It is possible to use other GPIO pin as a 1-Wire data pin. In order to change it, set the `THERMO_GPIO_PIN` macro to any of the options listed below:

```c
/* Possible GPIO pin choices:
 - gpio_ext_pc0
 - gpio_ext_pc1
 - gpio_ext_pc3
 - gpio_ext_pb2
 - gpio_ext_pb3
 - gpio_ext_pa4
 - gpio_ext_pa6
 - gpio_ext_pa7
 - gpio_ibutton
*/

#define THERMO_GPIO_PIN (gpio_ibutton)
```
Do not forget about the external pull-up resistor as these pins do not have one built-in.

With the changes been made, recompile and launch the application again. 
The on-screen text should reflect it by asking to connect the thermometer to another pin.
